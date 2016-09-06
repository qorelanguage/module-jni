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
#include "defs.h"
#include "Jvm.h"
#include "LocalReference.h"

namespace jni {

void JavaException::convert(ExceptionSink *xsink) {
   JNIEnv *env = Jvm::getEnv();
   LocalReference<jthrowable> throwable = env->ExceptionOccurred();
   assert(throwable != nullptr);
   env->ExceptionDescribe();
   env->ExceptionClear();

   //TODO include exception class name
   //TODO include causes
   //TODO stacktrace?

//we should make throwable a global reference and attach it to Qore exception
// - user code might want to use the Throwable object by passing it to a Java method

   jmethodID m = env->GetMethodID(env->GetObjectClass(throwable), "getMessage", "()Ljava/lang/String;");
   if (m == nullptr) {
      xsink->raiseException("JNI-ERROR", "Unable to get exception message - getMessage() not found");
      return;
   }

   LocalReference<jstring> msg = static_cast<jstring>(env->CallObjectMethod(throwable, m));
   if (env->ExceptionCheck()) {
      xsink->raiseException("JNI-ERROR", "Unable to get exception message - another exception thrown");
      return;
   }

   //TODO helper class for getting strings
   if (msg == nullptr) {
      xsink->raiseException("JNI-ERROR", "no messsage");
   } else {
      const char *chars = env->GetStringUTFChars(msg, nullptr);
      if (!chars) {
         xsink->raiseException("JNI-ERROR", "Unable to get exception message - GetStringUTFChars() failed");
         return;
      }

      //TODO encoding conversion
      xsink->raiseException("JNI-ERROR", chars);

      env->ReleaseStringUTFChars(msg, chars);      //TODO RAII
   }
}

} // namespace jni
