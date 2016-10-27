//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2016 Qore Technologies, s.r.o.
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
#include "Globals.h"

namespace jni {

JavaVM *Jvm::vm = nullptr;
thread_local JNIEnv *Jvm::env;

QoreStringNode* Jvm::createVM() {
   assert(vm == nullptr);

   JavaVMInitArgs vm_args;
   vm_args.version = JNI_VERSION_1_6;
   vm_args.ignoreUnrecognized = false;

   JavaVMOption options[2];
   // "reduced signals"
   options[0].optionString = (char*)"-Xrs";
   vm_args.nOptions = 1;

   ///*
   std::string classpath;
   const char* classpathEnv = getenv("QORE_JNI_CLASSPATH");
   if (classpathEnv) {
      classpath = std::string("-Djava.class.path=") + classpathEnv;
      options[1].optionString = &classpath[0];
      ++vm_args.nOptions;
   }
   //*/
   vm_args.options = options;

   if (JNI_CreateJavaVM(&vm, reinterpret_cast<void **>(&env), &vm_args) != JNI_OK) {
      return new QoreStringNode("JNI_CreateJavaVM() failed");
   }
   try {
      Globals::init();
   } catch (JavaException& e) {
      return e.toString();
   }
   catch (Exception &e) {
      return new QoreStringNode("JVM initialization failed due to an unknown error");
   }
   return 0;
}

void Jvm::destroyVM() {
   assert(vm != nullptr);

   Globals::cleanup();
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
