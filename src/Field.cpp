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

QoreValue Field::getStatic() {
   Env env;
   DescriptorParser parser(descriptor);
   switch (parser.getType()) {
      case 'Z':
         return JavaToQore::convert(env.getStaticBooleanField(clazz->getRef(), id));
      case 'B':
         return JavaToQore::convert(env.getStaticByteField(clazz->getRef(), id));
      case 'C':
         return JavaToQore::convert(env.getStaticCharField(clazz->getRef(), id));
      case 'S':
         return JavaToQore::convert(env.getStaticShortField(clazz->getRef(), id));
      case 'I':
         return JavaToQore::convert(env.getStaticIntField(clazz->getRef(), id));
      case 'J':
         return JavaToQore::convert(env.getStaticLongField(clazz->getRef(), id));
      case 'F':
         return JavaToQore::convert(env.getStaticFloatField(clazz->getRef(), id));
      case 'D':
         return JavaToQore::convert(env.getStaticDoubleField(clazz->getRef(), id));
      case 'L':
         return JavaToQore::convertObject(env.getStaticObjectField(clazz->getRef(), id), parser.getClassName());
      case '[':
         return JavaToQore::convertArray(env.getStaticObjectField(clazz->getRef(), id), parser.getArrayDescriptor());
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
         env.setStaticBooleanField(clazz->getRef(), id, QoreToJava::toBoolean(value));
         break;
      case 'B':
         env.setStaticByteField(clazz->getRef(), id, QoreToJava::toByte(value));
         break;
      case 'C':
         env.setStaticCharField(clazz->getRef(), id, QoreToJava::toChar(value));
         break;
      case 'S':
         env.setStaticShortField(clazz->getRef(), id, QoreToJava::toShort(value));
         break;
      case 'I':
         env.setStaticIntField(clazz->getRef(), id, QoreToJava::toInt(value));
         break;
      case 'J':
         env.setStaticLongField(clazz->getRef(), id, QoreToJava::toLong(value));
         break;
      case 'F':
         env.setStaticFloatField(clazz->getRef(), id, QoreToJava::toFloat(value));
         break;
      case 'D':
         env.setStaticDoubleField(clazz->getRef(), id, QoreToJava::toDouble(value));
         break;
      case 'L':
         env.setStaticObjectField(clazz->getRef(), id, QoreToJava::toObject(value, parser.getClassName()));
         break;
      case '[':
         env.setStaticObjectField(clazz->getRef(), id, QoreToJava::toArray(value, parser.getArrayDescriptor()));
         break;
      default:
         assert(false);         //invalid descriptor - should not happen
   }
}

} // namespace jni
