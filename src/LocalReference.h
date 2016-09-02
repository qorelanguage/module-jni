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
/// \brief Defines the LocalReference helper class template
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_LOCALREFERENCE_H_
#define QORE_JNI_LOCALREFERENCE_H_

#include <qore/Qore.h>
#include "Env.h"
#include "GlobalReference.h"
#include "defs.h"

namespace jni {

/**
 * \brief A RAII wrapper for JNI's local references.
 *
 * Assumes that the current thread has been attached to the JVM.
 * \tparam T the type of the reference (jobject, jclass etc.)
 */
template<typename T>
class LocalReference {

public:
   /**
    * \brief Creates an instance.
    * \param ref the local reference
    */
   LocalReference(T ref = nullptr) : ref(ref) {
      assert(ref == nullptr || Env::getEnv()->GetObjectRefType(ref) == JNILocalRefType);
      if (ref != nullptr) {
         printd(LogLevel, "LocalReference created: %p\n", ref);
      }
   }

   /**
    * \brief Destroys the local reference represented by this instance.
    */
   ~LocalReference() {
      if (ref != nullptr) {
         printd(LogLevel, "LocalReference deleted: %p\n", ref);
         Env::getEnv()->DeleteLocalRef(ref);
      }
   }

   /**
    * \brief Move constructor.
    * \param src the source local reference wrapper
    */
   LocalReference(LocalReference &&src) : ref(src.ref) {
      src.ref = nullptr;
   }

   /**
    * \brief Move assignment.
    * \param src the source local reference wrapper
    * \return *this
    */
   LocalReference &operator=(LocalReference &&src) {
      ref = src.ref;
      src.ref = nullptr;
      return *this;
   }

   /**
    * \brief Implicit conversion to the reference type.
    */
   operator T() const {
      return ref;
   }

   /**
    * \brief Creates a corresponding GlobalReference.
    * \return a global reference representing the same object
    */
   GlobalReference<T> makeGlobal() const {
      return ref == nullptr ? nullptr : static_cast<T>(Env::getEnv()->NewGlobalRef(ref));
   }

private:
   LocalReference(const LocalReference &) = delete;
   LocalReference &operator=(const LocalReference &) = delete;

private:
   T ref;
};

} // namespace jni

#endif // QORE_JNI_LOCALREFERENCE_H_
