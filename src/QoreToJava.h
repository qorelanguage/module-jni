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
/// \brief Defines the QoreToJava class
//------------------------------------------------------------------------------
#ifndef QORE_JNI_QORETOJAVA_H_
#define QORE_JNI_QORETOJAVA_H_

#include <qore/Qore.h>
#include <jni.h>
#include "Array.h"
#include "LocalReference.h"
#include "Object.h"
#include "defs.h"

namespace jni {

/**
 * \brief Provides functions for converting Qore values to Java.
 *
 * Although the majority of the methods is trivial, their purpose is to have a single point where a given conversion
 * is performed. This will allow to change the conversion (e.g. add range checking) in the future consistently.
 */
class QoreToJava {

public:
   static jboolean toBoolean(const QoreValue &value) {
      return value.getAsBool() ? JNI_TRUE : JNI_FALSE;
   }

   static jbyte toByte(const QoreValue &value) {
      return static_cast<jbyte>(value.getAsBigInt());
   }

   static jchar toChar(const QoreValue &value) {
      return static_cast<jchar>(value.getAsBigInt());
   }

   static jdouble toDouble(const QoreValue &value) {
      return static_cast<jdouble>(value.getAsFloat());
   }

   static jfloat toFloat(const QoreValue &value) {
      return static_cast<jfloat>(value.getAsFloat());
   }

   static jint toInt(const QoreValue &value) {
      return static_cast<jint>(value.getAsBigInt());
   }

   static jlong toLong(const QoreValue &value) {
      return static_cast<jlong>(value.getAsBigInt());
   }

   static jshort toShort(const QoreValue &value) {
      return static_cast<jshort>(value.getAsBigInt());
   }

   static jobject toObject(const QoreValue &value, jclass clazz) {
      if (value.getType() == NT_NOTHING) {
         return nullptr;
      }
      if (value.getType() != NT_OBJECT) {
         //TODO string, autobox primitives?
         throw BasicException("A Java object argument expected");
      }
      const QoreObject *o = value.get<QoreObject>();
      if (o->getClass(CID_OBJECT) == nullptr) {
         throw BasicException("A Java object argument expected");
      }

      ExceptionSink xsink;
      SimpleRefHolder<ObjectBase> obj(static_cast<ObjectBase *>(o->getReferencedPrivateData(CID_OBJECT, &xsink)));
      if (xsink) {
         throw XsinkException(xsink);
      }
      jobject javaObjectRef = obj->getJavaObject();

      Env env;
      if (!env.isInstanceOf(javaObjectRef, clazz)) {
         throw BasicException("Passed object is not an instance of expected class");
      }
      return javaObjectRef;
   }

private:
   QoreToJava() = delete;
};

} // namespace jni

#endif // QORE_JNI_QORETOJAVA_H_
