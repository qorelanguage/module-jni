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
#include "JniEnv.h"
#include <jni.h>
#include "LocalReference.h"
#include "ModifiedUtf8String.h"
#include "defs.h"

JavaVM *JniEnv::vm = nullptr;
thread_local JNIEnv *JniEnv::env = nullptr;

QoreStringNode *JniEnv::createVM() {
   assert(vm == nullptr);

   JavaVMInitArgs vm_args;
   vm_args.version = JNI_VERSION_1_6;
   vm_args.nOptions = 0;
   vm_args.options = nullptr;
   vm_args.ignoreUnrecognized = false;

   if (JNI_CreateJavaVM(&vm, reinterpret_cast<void **>(&env), &vm_args) != JNI_OK) {
      return new QoreStringNode("unable to create Java VM");
   }
   return nullptr;
}

void JniEnv::destroyVM() {
   assert(vm != nullptr);

   vm->DestroyJavaVM();
   vm = nullptr;
}

bool JniEnv::attach(ExceptionSink *xsink) {
   assert(vm != nullptr);

   if (env == nullptr) {
      jint err = vm->AttachCurrentThread(reinterpret_cast<void **>(&env), nullptr);
      if (err != JNI_OK) {
         printd(LogLevel, "JNI - error attaching thread %d, err: %d\n", gettid(), err);
         xsink->raiseException("JNI-ERROR", "Unable to attach thread to the JVM, error code %d", err);
         return false;
      }
      printd(LogLevel, "JNI - thread %d attached, env: %p\n", gettid(), env);
   }
   return true;
}

void JniEnv::threadCleanup() {
   assert(vm != nullptr);

   if (env != nullptr) {
      printd(LogLevel, "JNI - detaching thread, env: %p\n", env);
      vm->DetachCurrentThread();
      env = nullptr;
   }
}

bool JniEnv::checkJavaException(ExceptionSink *xsink) {
   assert(env != nullptr);

   LocalReference<jthrowable> throwable = env->ExceptionOccurred();
   if (throwable == nullptr) {
      return false;
   }
   env->ExceptionDescribe();
   env->ExceptionClear();

   //TODO include exception class name
   //TODO include causes
   //TODO stacktrace?

   jmethodID m = env->GetMethodID(env->GetObjectClass(throwable), "getMessage", "()Ljava/lang/String;");
   if (m == nullptr) {
      xsink->raiseException("JNI-ERROR", "Unable to get exception message - getMessage() not found");
      return true;
   }

   LocalReference<jstring> msg = static_cast<jstring>(env->CallObjectMethod(throwable, m));
   if (env->ExceptionCheck()) {
      xsink->raiseException("JNI-ERROR", "Unable to get exception message - another exception thrown");
      return true;
   }

   //TODO helper class for getting strings
   const char *chars = env->GetStringUTFChars(msg, nullptr);
   if (!chars) {
      xsink->raiseException("JNI-ERROR", "Unable to get exception message - GetStringUTFChars() failed");
      return true;
   }

   //TODO encoding conversion
   xsink->raiseException("JNI-ERROR", chars);

   env->ReleaseStringUTFChars(msg, chars);      //TODO RAII
   return true;
}

QoreStringNode *JniEnv::getVersion(ExceptionSink *xsink) {
   if (!attach(xsink)) {
      return nullptr;
   }
   jint v = env->GetVersion();
   QoreStringNode *str = new QoreStringNode();
   str->sprintf("%d.%d", v >> 16, v & 0xFFFF);
   return str;
}

void JniEnv::loadClass(ExceptionSink *xsink, const QoreStringNode *name) {
   if (!attach(xsink)) {
      return;
   }

   ModifiedUtf8String nameUtf8(name, xsink);
   printd(LogLevel, "loadClass %s\n", nameUtf8.c_str());
   LocalReference<jclass> clazz = env->FindClass(nameUtf8.c_str());
   if (checkJavaException(xsink)) {
      return;
   }
   //TODO
}
