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
#include "Field.h"
#include <memory>
#include "Env.h"
#include "JavaToQore.h"
#include "QoreToJava.h"

namespace jni {

QoreValue Field::get(jobject object) {
   Env env;
   if (!env.isInstanceOf(object, clazz->getJavaObject())) {
      throw BasicException("Passed instance does not match the field's class");
   }
   switch (type) {
      case Type::Boolean:
         return JavaToQore::convert(env.getBooleanField(object, id));
      case Type::Byte:
         return JavaToQore::convert(env.getByteField(object, id));
      case Type::Char:
         return JavaToQore::convert(env.getCharField(object, id));
      case Type::Short:
         return JavaToQore::convert(env.getShortField(object, id));
      case Type::Int:
         return JavaToQore::convert(env.getIntField(object, id));
      case Type::Long:
         return JavaToQore::convert(env.getLongField(object, id));
      case Type::Float:
         return JavaToQore::convert(env.getFloatField(object, id));
      case Type::Double:
         return JavaToQore::convert(env.getDoubleField(object, id));
      case Type::Reference:
      default:
         assert(type == Type::Reference);
         return JavaToQore::convert(env.getObjectField(object, id));
   }
}

void Field::set(jobject object, const QoreValue &value) {
   Env env;
   if (!env.isInstanceOf(object, clazz->getJavaObject())) {
      throw BasicException("Passed instance does not match the field's class");
   }
   switch (type) {
      case Type::Boolean:
         env.setBooleanField(object, id, QoreToJava::toBoolean(value));
         break;
      case Type::Byte:
         env.setByteField(object, id, QoreToJava::toByte(value));
         break;
      case Type::Char:
         env.setCharField(object, id, QoreToJava::toChar(value));
         break;
      case Type::Short:
         env.setShortField(object, id, QoreToJava::toShort(value));
         break;
      case Type::Int:
         env.setIntField(object, id, QoreToJava::toInt(value));
         break;
      case Type::Long:
         env.setLongField(object, id, QoreToJava::toLong(value));
         break;
      case Type::Float:
         env.setFloatField(object, id, QoreToJava::toFloat(value));
         break;
      case Type::Double:
         env.setDoubleField(object, id, QoreToJava::toDouble(value));
         break;
      case Type::Reference:
      default:
         assert(type == Type::Reference);
         env.setObjectField(object, id, QoreToJava::toObject(value, typeClass));
         break;
   }
}

QoreValue Field::getStatic() {
   Env env;
   switch (type) {
      case Type::Boolean:
         return JavaToQore::convert(env.getStaticBooleanField(clazz->getJavaObject(), id));
      case Type::Byte:
         return JavaToQore::convert(env.getStaticByteField(clazz->getJavaObject(), id));
      case Type::Char:
         return JavaToQore::convert(env.getStaticCharField(clazz->getJavaObject(), id));
      case Type::Short:
         return JavaToQore::convert(env.getStaticShortField(clazz->getJavaObject(), id));
      case Type::Int:
         return JavaToQore::convert(env.getStaticIntField(clazz->getJavaObject(), id));
      case Type::Long:
         return JavaToQore::convert(env.getStaticLongField(clazz->getJavaObject(), id));
      case Type::Float:
         return JavaToQore::convert(env.getStaticFloatField(clazz->getJavaObject(), id));
      case Type::Double:
         return JavaToQore::convert(env.getStaticDoubleField(clazz->getJavaObject(), id));
      case Type::Reference:
      default:
         assert(type == Type::Reference);
         return JavaToQore::convert(env.getStaticObjectField(clazz->getJavaObject(), id));
   }
}

void Field::setStatic(const QoreValue &value) {
   Env env;
   switch (type) {
      case Type::Boolean:
         env.setStaticBooleanField(clazz->getJavaObject(), id, QoreToJava::toBoolean(value));
         break;
      case Type::Byte:
         env.setStaticByteField(clazz->getJavaObject(), id, QoreToJava::toByte(value));
         break;
      case Type::Char:
         env.setStaticCharField(clazz->getJavaObject(), id, QoreToJava::toChar(value));
         break;
      case Type::Short:
         env.setStaticShortField(clazz->getJavaObject(), id, QoreToJava::toShort(value));
         break;
      case Type::Int:
         env.setStaticIntField(clazz->getJavaObject(), id, QoreToJava::toInt(value));
         break;
      case Type::Long:
         env.setStaticLongField(clazz->getJavaObject(), id, QoreToJava::toLong(value));
         break;
      case Type::Float:
         env.setStaticFloatField(clazz->getJavaObject(), id, QoreToJava::toFloat(value));
         break;
      case Type::Double:
         env.setStaticDoubleField(clazz->getJavaObject(), id, QoreToJava::toDouble(value));
         break;
      case Type::Reference:
      default:
         assert(type == Type::Reference);
         env.setStaticObjectField(clazz->getJavaObject(), id, QoreToJava::toObject(value, typeClass));
         break;
   }
}

} // namespace jni
