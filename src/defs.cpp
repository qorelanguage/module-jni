//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2016 - 2018 Qore Technologies, s.r.o.
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
#include "QoreJniClassMap.h"

namespace jni {

thread_local QoreThreadAttacher qoreThreadAttacher;

class JniCallStack : public QoreCallStack {
public:
   DLLLOCAL JniCallStack(jobject throwable) {
      Env env;

      try {
         LocalReference<jobjectArray> jstack = env.callObjectMethod(throwable, Globals::methodThrowableGetStackTrace, nullptr).as<jobjectArray>();
         if (jstack) {
            Type elementType = Globals::getType(Globals::classStackTraceElement);
            for (jsize i = 0, e = env.getArrayLength(jstack); i < e; ++i) {
               LocalReference<jobject> jste = env.getObjectArrayElement(jstack, i);

               LocalReference<jstring> jcls = env.callObjectMethod(jste, Globals::methodStackTraceElementGetClassName, nullptr).as<jstring>();
               jni::Env::GetStringUtfChars cname(env, jcls);
               LocalReference<jstring> jmethod = env.callObjectMethod(jste, Globals::methodStackTraceElementGetMethodName, nullptr).as<jstring>();
               jni::Env::GetStringUtfChars mname(env, jmethod);
               LocalReference<jstring> jfilename = env.callObjectMethod(jste, Globals::methodStackTraceElementGetFileName, nullptr).as<jstring>();
               jni::Env::GetStringUtfChars file(env, jfilename);
               jint line = env.callIntMethod(jste, Globals::methodStackTraceElementGetLineNumber, nullptr);
               jboolean native = env.callBooleanMethod(jste, Globals::methodStackTraceElementIsNativeMethod, nullptr);

               QoreStringMaker code("%s.%s", cname.c_str(), mname.c_str());

               //printd(LogLevel, "JniCallStack::JniCallStack() adding %s\n", code.c_str());
               add(native ? CT_BUILTIN : CT_USER, file.c_str(), line, line, code.c_str());
            }
         }
      }
      catch (jni::Exception& e) {
         e.ignore();
      }
   }
};

void JavaException::ignore() {
   // not using the Env wrapper because we don't want any C++ exceptions here
   JNIEnv* env = Jvm::getEnv();
   LocalReference<jthrowable> throwable = env->ExceptionOccurred();
   assert(throwable != nullptr);
   env->ExceptionClear();
}

jthrowable JavaException::save() {
   // not using the Env wrapper because we don't want any C++ exceptions here
   JNIEnv* env = Jvm::getEnv();
   LocalReference<jthrowable> throwable = env->ExceptionOccurred();
   assert(throwable != nullptr);
   env->ExceptionClear();
   return throwable.release();
}

void JavaException::restore(jthrowable je) {
   // not using the Env wrapper because we don't want any C++ exceptions here
   JNIEnv* env = Jvm::getEnv();
   env->Throw(je);
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
            QoreValue err = src->getExceptionErr();
            QoreValue desc = src->getExceptionDesc();
            QoreValue arg = src->getExceptionArg();

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
      //not happen, but if it does, we simply report the QoreExceptionWrapper as if it was a normal Java exception
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

   // add Java call stack to Qore call stack
   JniCallStack stack(throwable);

   LocalReference<jclass> tcls(env->GetObjectClass(throwable));
   xsink->raiseExceptionArg("JNI-ERROR", new QoreObject(qjcm.findCreateQoreClass(tcls), getProgram(), new QoreJniPrivateData(throwable.as<jobject>())), desc.release(), stack);
}

void JavaException::ignoreOrRethrowNoClass() {
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
   if (!strcmp(chars, "java.lang.ClassNotFoundException")
       || !strcmp(chars, "java.lang.NoClassDefFoundError"))
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

   // add Java call stack to Qore call stack
   JniCallStack stack(throwable);

   xsink.raiseExceptionArg("JNI-ERROR", new QoreObject(QC_THROWABLE, getProgram(), new QoreJniPrivateData(throwable.as<jobject>())), desc.release(), stack);
}
} // namespace jni
