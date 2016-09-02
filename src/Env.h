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

namespace jni {

class Class;

/**
 * \brief Provides access to JNI functions.
 */
class Env {

public:
   /**
    * \brief Returns the JNIEnv* associated with this thread.
    *
    * Assumes that the thread has been attached to the JVM.
    * \return the JNIEnv* associated with this thread
    */
   static JNIEnv *getEnv() {
      assert(env != nullptr);
      return env;
   }

   /**
    * \brief Returns the JNIEnv* associated with this thread. Attaches the thread to the JVM if needed.
    * \return the JNIEnv* associated with this thread
    * \throws UnableToAttachException if the thread cannot be attached to the JVM
    */
   static JNIEnv *attachAndGetEnv();

   /**
    * \brief Creates the JVM.
    * \return true if successful
    */
   static bool createVM();

   /**
    * \brief Destroys the JVM.
    */
   static void destroyVM();

   /**
    * \brief Detaches the current thread from the JVM.
    */
   static void threadCleanup();

   /**
    * \brief Determines the version of the JVM.
    * \return the version of the JVM
    * \throws UnableToAttachException if the thread cannot be attached to the JVM
    */
   static QoreStringNode *getVersion();

   /**
    * \brief Loads a Java class.
    * \param name a fully-qualified class name or an array type signature
    * \return loaded class
    * \throws Exception if an error occurs
    */
   static Class *loadClass(const QoreStringNode *name);

private:
   /**
    * \brief This is a static class - no instances are allowed.
    */
   Env() = delete;

   /**
    * \brief Makes sure that this thread is attached to the JVM and thus the env member is a valid JNIEnv pointer.
    * \throws UnableToAttachException if the thread cannot be attached to the JVM
    */
   static void ensureAttached() {
      attachAndGetEnv();
   }

private:
   static JavaVM *vm;
   static thread_local JNIEnv *env;
};

} // namespace jni

#endif // QORE_JNI_JNIENV_H_
