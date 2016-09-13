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
#include "DescriptorParser.h"
#include "Env.h"
#include "JavaToQore.h"
#include "QoreToJava.h"

namespace jni {

QoreValue Field::get(jobject object) {
   Env env;
   DescriptorParser parser(descriptor);
   switch (parser.getType()) {
      case 'Z':
         return JavaToQore::convert(env.getBooleanField(object, id));
      case 'B':
         return JavaToQore::convert(env.getByteField(object, id));
      case 'C':
         return JavaToQore::convert(env.getCharField(object, id));
      case 'S':
         return JavaToQore::convert(env.getShortField(object, id));
      case 'I':
         return JavaToQore::convert(env.getIntField(object, id));
      case 'J':
         return JavaToQore::convert(env.getLongField(object, id));
      case 'F':
         return JavaToQore::convert(env.getFloatField(object, id));
      case 'D':
         return JavaToQore::convert(env.getDoubleField(object, id));
      case 'L':
         return JavaToQore::convertObject(env.getObjectField(object, id), parser.getClassName());
      case '[':
         return JavaToQore::convertArray(env.getObjectField(object, id), parser.getArrayDescriptor());
      default:
         assert(false);         //invalid descriptor - should not happen
         return QoreValue();
   }
}

void Field::set(jobject object, const QoreValue &value) {
   Env env;
   DescriptorParser parser(descriptor);
   switch (parser.getType()) {
      case 'Z':
         env.setBooleanField(object, id, QoreToJava::toBoolean(value));
         break;
      case 'B':
         env.setByteField(object, id, QoreToJava::toByte(value));
         break;
      case 'C':
         env.setCharField(object, id, QoreToJava::toChar(value));
         break;
      case 'S':
         env.setShortField(object, id, QoreToJava::toShort(value));
         break;
      case 'I':
         env.setIntField(object, id, QoreToJava::toInt(value));
         break;
      case 'J':
         env.setLongField(object, id, QoreToJava::toLong(value));
         break;
      case 'F':
         env.setFloatField(object, id, QoreToJava::toFloat(value));
         break;
      case 'D':
         env.setDoubleField(object, id, QoreToJava::toDouble(value));
         break;
      case 'L':
         env.setObjectField(object, id, QoreToJava::toObject(value, parser.getClassName()));
         break;
      case '[':
         env.setObjectField(object, id, QoreToJava::toArray(value, parser.getArrayDescriptor()));
         break;
      default:
         assert(false);         //invalid descriptor - should not happen
   }
}

QoreValue Field::getStatic() {
   Env env;
   DescriptorParser parser(descriptor);
   switch (parser.getType()) {
      case 'Z':
         return JavaToQore::convert(env.getStaticBooleanField(clazz->getJavaObject(), id));
      case 'B':
         return JavaToQore::convert(env.getStaticByteField(clazz->getJavaObject(), id));
      case 'C':
         return JavaToQore::convert(env.getStaticCharField(clazz->getJavaObject(), id));
      case 'S':
         return JavaToQore::convert(env.getStaticShortField(clazz->getJavaObject(), id));
      case 'I':
         return JavaToQore::convert(env.getStaticIntField(clazz->getJavaObject(), id));
      case 'J':
         return JavaToQore::convert(env.getStaticLongField(clazz->getJavaObject(), id));
      case 'F':
         return JavaToQore::convert(env.getStaticFloatField(clazz->getJavaObject(), id));
      case 'D':
         return JavaToQore::convert(env.getStaticDoubleField(clazz->getJavaObject(), id));
      case 'L':
         return JavaToQore::convertObject(env.getStaticObjectField(clazz->getJavaObject(), id), parser.getClassName());
      case '[':
         return JavaToQore::convertArray(env.getStaticObjectField(clazz->getJavaObject(), id), parser.getArrayDescriptor());
      default:
         assert(false);         //invalid descriptor - should not happen
         return QoreValue();
   }
}

void Field::setStatic(const QoreValue &value) {
   Env env;
   DescriptorParser parser(descriptor);
   switch (parser.getType()) {
      case 'Z':
         env.setStaticBooleanField(clazz->getJavaObject(), id, QoreToJava::toBoolean(value));
         break;
      case 'B':
         env.setStaticByteField(clazz->getJavaObject(), id, QoreToJava::toByte(value));
         break;
      case 'C':
         env.setStaticCharField(clazz->getJavaObject(), id, QoreToJava::toChar(value));
         break;
      case 'S':
         env.setStaticShortField(clazz->getJavaObject(), id, QoreToJava::toShort(value));
         break;
      case 'I':
         env.setStaticIntField(clazz->getJavaObject(), id, QoreToJava::toInt(value));
         break;
      case 'J':
         env.setStaticLongField(clazz->getJavaObject(), id, QoreToJava::toLong(value));
         break;
      case 'F':
         env.setStaticFloatField(clazz->getJavaObject(), id, QoreToJava::toFloat(value));
         break;
      case 'D':
         env.setStaticDoubleField(clazz->getJavaObject(), id, QoreToJava::toDouble(value));
         break;
      case 'L':
         env.setStaticObjectField(clazz->getJavaObject(), id, QoreToJava::toObject(value, parser.getClassName()));
         break;
      case '[':
         env.setStaticObjectField(clazz->getJavaObject(), id, QoreToJava::toArray(value, parser.getArrayDescriptor()));
         break;
      default:
         assert(false);         //invalid descriptor - should not happen
   }
}

} // namespace jni
