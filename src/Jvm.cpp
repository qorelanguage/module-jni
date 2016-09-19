//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2015 Qore Technologies
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//------------------------------------------------------------------------------
#include "Jvm.h"
#include <qore/Qore.h>
#include "defs.h"
#include "InvocationHandler.h"

namespace jni {

JavaVM *Jvm::vm = nullptr;
thread_local JNIEnv *Jvm::env;

bool Jvm::createVM() {
   assert(vm == nullptr);

   JavaVMInitArgs vm_args;
   vm_args.version = JNI_VERSION_1_6;
   vm_args.ignoreUnrecognized = false;

   JavaVMOption option;
   std::string classpath;
   const char *classpathEnv = getenv("QORE_JNI_CLASSPATH");
   if (classpathEnv == nullptr) {
      vm_args.nOptions = 0;
      vm_args.options = nullptr;
   } else {
      classpath = std::string("-Djava.class.path=") + classpathEnv;
      option.optionString = &classpath[0];
      vm_args.nOptions = 1;
      vm_args.options = &option;
   }

   if (JNI_CreateJavaVM(&vm, reinterpret_cast<void **>(&env), &vm_args) != JNI_OK) {
      return false;
   }
   try {
      InvocationHandler::init();
   } catch (Exception &e) {
      return false;
   }
   return true;
}

void Jvm::destroyVM() {
   assert(vm != nullptr);

   InvocationHandler::cleanup();
   vm->DestroyJavaVM();
   vm = nullptr;
   env = nullptr;
}

JNIEnv *Jvm::attachAndGetEnv() {
   assert(vm != nullptr);

   if (env == nullptr) {
      jint err = vm->AttachCurrentThread(reinterpret_cast<void **>(&env), nullptr);
      if (err != JNI_OK) {
         throw UnableToAttachException(err);
      }
      printd(LogLevel, "JNI - thread %d attached, env: %p\n", gettid(), env);
   }
   return env;
}

void Jvm::threadCleanup() {
   assert(vm != nullptr);

   if (env != nullptr) {
      printd(LogLevel, "JNI - detaching thread, env: %p\n", env);
      vm->DetachCurrentThread();
      env = nullptr;
   }
}

} // namespace jni
