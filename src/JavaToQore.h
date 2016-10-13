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
/// \brief Defines the JavaToQore class
//------------------------------------------------------------------------------
#ifndef QORE_JNI_JAVATOQORE_H_
#define QORE_JNI_JAVATOQORE_H_

#include <qore/Qore.h>
#include <jni.h>

#include "LocalReference.h"
#include "Object.h"
#include "Array.h"
#include "Class.h"
#include "Globals.h"
#include "Throwable.h"
#include "Env.h"
#include "Method.h"
#include "Field.h"

namespace jni {

/**
 * \brief Provides functions for converting Java values to Qore.
 *
 * Although the majority of the methods is trivial, their purpose is to have a single point where a given conversion
 * is performed. This will allow to change the conversion (e.g. add range checking) in the future consistently.
 */
class JavaToQore {

public:
   static QoreValue convert(jboolean v) {
      return QoreValue(v == JNI_TRUE);
   }

   static QoreValue convert(jbyte v) {
      return QoreValue(v);
   }

   static QoreValue convert(jchar v) {
      return QoreValue(v);
   }

   static QoreValue convert(jdouble v) {
      return QoreValue(v);
   }

   static QoreValue convert(jfloat v) {
      return QoreValue(v);
   }

   static QoreValue convert(jint v) {
      return QoreValue(v);
   }

   static QoreValue convert(jlong v) {
      return QoreValue(v);
   }

   static QoreValue convert(jshort v) {
      return QoreValue(v);
   }

   static QoreValue convertToQore(LocalReference<jobject> v);

   static QoreValue convert(LocalReference<jobject> v) {
      if (v == nullptr) {
         return QoreValue();
      }
      Env env;
      if (env.isInstanceOf(v, Globals::classClass)) {
         return QoreValue(new QoreObject(QC_JAVACLASS, getProgram(), new Class(v.as<jclass>())));
      }
      if (env.isInstanceOf(v, Globals::classThrowable)) {
         return QoreValue(new QoreObject(QC_JAVATHROWABLE, getProgram(), new Throwable(v.as<jthrowable>())));
      }
      if (env.isInstanceOf(v, Globals::classString)) {
         Env::GetStringUtfChars chars(env, v.as<jstring>());
         return QoreValue(new QoreStringNode(chars.c_str(), QCS_UTF8));
      }
      if (env.isInstanceOf(v, Globals::classMethod)) {
         int mods = env.callIntMethod(v, Globals::methodMethodGetModifiers, nullptr);
         return QoreValue(new QoreObject(mods & 8 ? QC_JAVASTATICMETHOD : QC_JAVAMETHOD, getProgram(), new Method(v)));
      }
      if (env.isInstanceOf(v, Globals::classConstructor)) {
         return QoreValue(new QoreObject(QC_JAVACONSTRUCTOR, getProgram(), new Method(v)));
      }
      if (env.isInstanceOf(v, Globals::classField)) {
         int mods = env.callIntMethod(v, Globals::methodFieldGetModifiers, nullptr);
         return QoreValue(new QoreObject(mods & 8 ? QC_JAVASTATICFIELD : QC_JAVAFIELD, getProgram(), new Field(v)));
      }
      if (env.callBooleanMethod(env.getObjectClass(v), Globals::methodClassIsArray, nullptr)) {
         return QoreValue(new QoreObject(QC_JAVAARRAY, getProgram(), new Array(v.as<jarray>())));
      }
      return QoreValue(new QoreObject(QC_JAVAOBJECT, getProgram(), new Object(v)));
   }

private:
   JavaToQore() = delete;
};

} // namespace jni

#endif // QORE_JNI_JAVATOQORE_H_
