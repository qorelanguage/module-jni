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
///
/// \file
/// \brief Defines the Object class.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_OBJECT_H_
#define QORE_JNI_OBJECT_H_

#include <qore/Qore.h>
#include "LocalReference.h"

namespace jni {

/**
 * \brief Base class for private data of Qore objects that represent Java object.
 */
class ObjectBase : public AbstractPrivateData {

public:
   /**
    * \brief Constructor.
    */
   ObjectBase() = default;

   virtual ~ObjectBase() = default;

   /**
    * \brief Returns the reference to the JNI object.
    * \return the reference to the JNI object
    */
   virtual jobject getJavaObject() const = 0;

   /**
    * \brief Returns a local reference to the JNI object.
    * \return a local reference to the JNI object
    */
   DLLLOCAL LocalReference<jobject> makeLocal() const {
      jobject ref = getJavaObject();
      if (ref == nullptr)
	 return nullptr;
      jobject local = static_cast<jobject>(Jvm::getEnv()->NewLocalRef(ref));
      if (local == nullptr)
         throw JavaException();
      return local;
   }
};

/**
 * \brief Represents a Java object instance.
 */
class Object : public ObjectBase {

public:
   /**
    * \brief Constructor.
    * \param object a local reference to a Java object instance
    * \throws JavaException if a global reference cannot be created
    */
   Object(const LocalReference<jobject> &object) : object(object.makeGlobal()) {
      printd(LogLevel, "Object::Object(), this: %p, object: %p\n", this, static_cast<jobject>(this->object));
   }

   ~Object() {
      printd(LogLevel, "Object::~Object(), this: %p, object: %p\n", this, static_cast<jobject>(this->object));
   }

   /**
    * \brief Returns the reference to the JNI jobject object.
    * \return the reference to the JNI jobject object
    */
   jobject getJavaObject() const override {
      return object;
   }

private:
   GlobalReference<jobject> object;
};

} // namespace jni

#endif // QORE_JNI_OBJECT_H_
