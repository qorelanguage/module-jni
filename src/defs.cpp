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
#include "defs.h"
#include <qore/Qore.h>
#include "Jvm.h"
#include "LocalReference.h"
#include "Globals.h"
#include "Throwable.h"

namespace jni {

thread_local QoreThreadAttacher qoreThreadAttacher;

void JavaException::ignore() {
   JNIEnv* env = Jvm::getEnv();         //not using the Env wrapper because we don't want any C++ exceptions here
   LocalReference<jthrowable> throwable = env->ExceptionOccurred();
   assert(throwable != nullptr);
   env->ExceptionClear();
}

QoreStringNode* JavaException::toString() const {
   JNIEnv* env = Jvm::getEnv();         //not using the Env wrapper because we don't want any C++ exceptions here
   LocalReference<jthrowable> throwable = env->ExceptionOccurred();
   assert(throwable != nullptr);
   env->ExceptionClear();

   if (env->IsInstanceOf(throwable, Globals::classQoreExceptionWrapper)) {
      jlong l = env->CallLongMethod(throwable, Globals::methodQoreExceptionWrapperGet);
      if (l != 0) {
         ExceptionSink *src = reinterpret_cast<ExceptionSink *>(l);
         const AbstractQoreNode* err = src->getExceptionErr();
         const AbstractQoreNode* desc = src->getExceptionDesc();
         const AbstractQoreNode* arg = src->getExceptionArg();

         QoreStringNodeValueHelper terr(err);
         QoreStringNodeValueHelper tdesc(desc);

         SimpleRefHolder<QoreStringNode> rv(new QoreStringNodeMaker("%s: %s", terr->c_str(), tdesc->c_str()));
         if (arg) {
            QoreStringNodeValueHelper targ(arg);
            rv->sprintf(" arg: %s", targ->c_str());
         }

         src->clear();
         return rv.release();
      }
      //if l is zero, it means that the xsink wrapped in QoreExceptionWrapper has already been consumed. This should
      //not happen, but if it does, we simply report the QoreExceptionWrapper as if it were a normal Java exception
   }

   LocalReference<jstring> excName = static_cast<jstring>(env->CallObjectMethod(env->GetObjectClass(throwable), Globals::methodClassGetName));
   if (env->ExceptionCheck()) {
      env->ExceptionClear();
      return new QoreStringNode("Unable to get exception class name: another exception thrown");
   }

   const char* chars = env->GetStringUTFChars(excName, nullptr);
   if (!chars) {
      env->ExceptionClear();
      return new QoreStringNode("Unable to get exception class name: GetStringUTFChars() failed");
   }
   SimpleRefHolder<QoreStringNode> desc(new QoreStringNode(chars, QCS_UTF8));
   env->ReleaseStringUTFChars(excName, chars);

   LocalReference<jstring> msg = static_cast<jstring>(env->CallObjectMethod(throwable, Globals::methodThrowableGetMessage));
   if (env->ExceptionCheck()) {
      env->ExceptionClear();
   } else if (msg != nullptr) {
      desc->concat(": ");
      chars = env->GetStringUTFChars(msg, nullptr);
      if (!chars) {
         env->ExceptionClear();
      } else {
         desc->concat(chars);
         env->ReleaseStringUTFChars(msg, chars);
      }
   }
   return desc.release();
}

void JavaException::convert(ExceptionSink *xsink) {
   JNIEnv* env = Jvm::getEnv();         //not using the Env wrapper because we don't want any C++ exceptions here
   LocalReference<jthrowable> throwable = env->ExceptionOccurred();
   assert(throwable != nullptr);
   env->ExceptionClear();

   if (env->IsInstanceOf(throwable, Globals::classQoreExceptionWrapper)) {
      jlong l = env->CallLongMethod(throwable, Globals::methodQoreExceptionWrapperGet);
      if (l != 0) {
         ExceptionSink *src = reinterpret_cast<ExceptionSink *>(l);
         xsink->assimilate(src);
         return;
      }
      //if l is zero, it means that the xsink wrapped in QoreExceptionWrapper has already been consumed. This should
      //not happen, but if it does, we simply report the QoreExceptionWrapper as if it was normal Java exception
   }

   LocalReference<jstring> excName = static_cast<jstring>(env->CallObjectMethod(env->GetObjectClass(throwable), Globals::methodClassGetName));
   if (env->ExceptionCheck()) {
      env->ExceptionClear();
      xsink->raiseException("JNI-ERROR", "Unable to get exception class name - another exception thrown");
      return;
   }

   const char* chars = env->GetStringUTFChars(excName, nullptr);
   if (!chars) {
      env->ExceptionClear();
      xsink->raiseException("JNI-ERROR", "Unable to get exception class name - GetStringUTFChars() failed");
      return;
   }
   SimpleRefHolder<QoreStringNode> desc(new QoreStringNode(chars, QCS_UTF8));
   env->ReleaseStringUTFChars(excName, chars);

   LocalReference<jstring> msg = static_cast<jstring>(env->CallObjectMethod(throwable, Globals::methodThrowableGetMessage));
   if (env->ExceptionCheck()) {
      env->ExceptionClear();
   } else if (msg != nullptr) {
      desc->concat(": ");
      chars = env->GetStringUTFChars(msg, nullptr);
      if (!chars) {
         env->ExceptionClear();
      } else {
         desc->concat(chars);
         env->ReleaseStringUTFChars(msg, chars);
      }
   }
   xsink->raiseExceptionArg("JNI-ERROR", new QoreObject(QC_THROWABLE, getProgram(), new Throwable(throwable)), desc.release());
}

void JavaException::ignoreOrRethrow(const char* cls) {
   JNIEnv* env = Jvm::getEnv();         //not using the Env wrapper because we don't want any C++ exceptions here
   LocalReference<jthrowable> throwable = env->ExceptionOccurred();
   assert(throwable != nullptr);
   env->ExceptionClear();

   // to show an unhandled exception on the console
   ExceptionSink xsink;

   if (env->IsInstanceOf(throwable, Globals::classQoreExceptionWrapper)) {
      jlong l = env->CallLongMethod(throwable, Globals::methodQoreExceptionWrapperGet);
      if (l != 0) {
         ExceptionSink *src = reinterpret_cast<ExceptionSink *>(l);
         xsink.assimilate(src);
         return;
      }
      //if l is zero, it means that the xsink wrapped in QoreExceptionWrapper has already been consumed. This should
      //not happen, but if it does, we simply report the QoreExceptionWrapper as if it was normal Java exception
   }

   LocalReference<jstring> excName = static_cast<jstring>(env->CallObjectMethod(env->GetObjectClass(throwable), Globals::methodClassGetName));
   if (env->ExceptionCheck()) {
      env->ExceptionClear();
      xsink.raiseException("JNI-ERROR", "Unable to get exception class name: another exception thrown");
      return;
   }

   const char* chars = env->GetStringUTFChars(excName, nullptr);
   if (!chars) {
      env->ExceptionClear();
      xsink.raiseException("JNI-ERROR", "Unable to get exception class name: GetStringUTFChars() failed");
      return;
   }
   // return if this is the exception we should ignore
   if (!strcmp(chars, cls))
      return;

   SimpleRefHolder<QoreStringNode> desc(new QoreStringNode(chars, QCS_UTF8));
   env->ReleaseStringUTFChars(excName, chars);

   LocalReference<jstring> msg = static_cast<jstring>(env->CallObjectMethod(throwable, Globals::methodThrowableGetMessage));
   if (env->ExceptionCheck()) {
      env->ExceptionClear();
   } else if (msg != nullptr) {
      desc->concat(": ");
      chars = env->GetStringUTFChars(msg, nullptr);
      if (!chars) {
         env->ExceptionClear();
      } else {
         desc->concat(chars);
         env->ReleaseStringUTFChars(msg, chars);
      }
   }
   xsink.raiseExceptionArg("JNI-ERROR", new QoreObject(QC_THROWABLE, getProgram(), new Throwable(throwable)), desc.release());
}

} // namespace jni
