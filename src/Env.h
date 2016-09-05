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
/// \brief Defines the Env class.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_ENV_H_
#define QORE_JNI_ENV_H_

#include "Jvm.h"
#include "LocalReference.h"

namespace jni {

/**
 * \brief Provides access to JNI functions.
 *
 * Wraps JNI references to RAII objects and uses C++ exceptions for error reporting.
 */
class Env {

public:
   /**
    * \brief Default constructor. Attaches current thread to the JVM.
    * \throws UnableToAttachException if the thread cannot be attached to the JVM
    */
   Env() : env(Jvm::attachAndGetEnv()) {
   }

   /**
    * \brief Returns the major version number in the higher 16 bits and the minor version number in the lower 16 bits.
    * \return the major version number in the higher 16 bits and the minor version number in the lower 16 bits
    */
   jint getVersion() {
      return env->GetVersion();
   }

   /**
    * \brief Finds a class with given name.
    * \param name fully-qualified class name or an array type signature
    * \return a local reference to the class object
    * \throws JavaException if the class cannot be found
    */
   LocalReference<jclass> findClass(const char *name) {
      jclass c = env->FindClass(name);
      if (c == nullptr) {
         throw JavaException();
      }
      return c;
   }

   /**
    * \brief Finds a method in a class.
    * \param clazz the class
    * \param name the name of the method
    * \param descriptor the method signature
    * \return method id
    * \throws JavaException if the class cannot be found
    */
   jmethodID getMethod(jclass clazz, const char *name, const char *descriptor) {
      jmethodID id = env->GetMethodID(clazz, name, descriptor);
      if (id == nullptr) {
         throw JavaException();
      }
      return id;
   }

   /**
    * \brief Finds a static method in a class.
    * \param clazz the class
    * \param name the name of the method
    * \param descriptor the method signature
    * \return method id
    * \throws JavaException if the class cannot be found
    */
   jmethodID getStaticMethod(jclass clazz, const char *name, const char *descriptor) {
      jmethodID id = env->GetStaticMethodID(clazz, name, descriptor);
      if (id == nullptr) {
         throw JavaException();
      }
      return id;
   }

private:
   JNIEnv *env;
};

} // namespace jni

#endif // QORE_JNI_ENV_H_
