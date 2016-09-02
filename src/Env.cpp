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
#include "Env.h"
#include "LocalReference.h"
#include "ModifiedUtf8String.h"
#include "Class.h"
#include "defs.h"

namespace jni {

JavaVM *Env::vm = nullptr;
thread_local JNIEnv *Env::env = nullptr;

bool Env::createVM() {
   assert(vm == nullptr);

   JavaVMInitArgs vm_args;
   vm_args.version = JNI_VERSION_1_6;
   vm_args.nOptions = 0;
   vm_args.options = nullptr;
   vm_args.ignoreUnrecognized = false;

   return JNI_CreateJavaVM(&vm, reinterpret_cast<void **>(&env), &vm_args) == JNI_OK;
}

void Env::destroyVM() {
   assert(vm != nullptr);

   vm->DestroyJavaVM();
   vm = nullptr;
}

JNIEnv *Env::attachAndGetEnv() {
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

void Env::threadCleanup() {
   assert(vm != nullptr);

   if (env != nullptr) {
      printd(LogLevel, "JNI - detaching thread, env: %p\n", env);
      vm->DetachCurrentThread();
      env = nullptr;
   }
}

QoreStringNode *Env::getVersion() {
   ensureAttached();
   jint v = env->GetVersion();
   QoreStringNode *str = new QoreStringNode();
   str->sprintf("%d.%d", v >> 16, v & 0xFFFF);
   return str;
}

static LocalReference<jclass> findClass(JNIEnv *env, const char *name) {
   jclass c = env->FindClass(name);
   if (c == nullptr) {
      throw JavaException();
   }
   return c;
}

Class *Env::loadClass(const QoreStringNode *name) {
   ensureAttached();

   ModifiedUtf8String nameUtf8(name);
   printd(LogLevel, "loadClass %s\n", nameUtf8.c_str());
   return new Class(findClass(env, nameUtf8.c_str()).makeGlobal());
}

} // namespace jni

