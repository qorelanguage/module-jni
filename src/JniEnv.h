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
///
/// \file
/// \brief Defines the JniEnv class.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_JNIENV_H_
#define QORE_JNI_JNIENV_H_

#include <cassert>
#include <qore/Qore.h>
#include <jni.h>

class Class;

/**
 * \brief Provides access to JNI functions.
 */
class JniEnv {

public:
   /**
    * \brief Returns the JNIEnv* associated with this thread.
    *
    * Assumes that the thread has been attached to the JVM.
    * \return the JNIEnv* associated with this thread.
    */
   static JNIEnv *getEnv() {
      assert(env != nullptr);
      return env;
   }

   /**
    * \brief Returns the JNIEnv* associated with this thread.
    *
    * Attaches the thread to the JVM if needed.
    * \return the JNIEnv* associated with this thread or nullptr if the thread cannot be attached to the JVM.
    */
   static JNIEnv *attachAndGetEnv();


   static QoreStringNode *createVM();
   static void destroyVM();
   static void threadCleanup();

   static QoreStringNode *getVersion(ExceptionSink *xsink);
   static Class *loadClass(ExceptionSink *xsink, const QoreStringNode *name);


private:
   /**
    * \brief This is a static class - no instances are allowed.
    */
   JniEnv() = delete;

   /**
    * \brief Makes sure that this thread is attached to the JVM and thus the env member is a valid JNIEnv pointer.
    * \param xsink the exception sink
    * \return true if this thread is attached (and env is a valid pointer) or false when an error occurs in which case
    *         it raises an exception
    */
   static bool ensureAttached(ExceptionSink *xsink);

   /**
    * \brief Raises a Qore exception if there is a pending Java exception.
    *
    * Assumes that the thread has been attached to the JVM. Marks the JAva exception as handled.
    * \param xsink the exception sink
    * \return true if an exception has been raised
    */
   static bool checkJavaException(ExceptionSink *xsink);

private:
   static JavaVM *vm;
   static thread_local JNIEnv *env;
};

#endif // QORE_JNI_JNIENV_H_
