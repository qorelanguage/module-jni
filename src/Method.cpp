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
#include "Method.h"
#include "DescriptorParser.h"
#include "Env.h"
#include "JavaToQore.h"
#include "QoreToJava.h"

namespace jni {

//FIXME WIP

class MethodDescriptorParser : public DescriptorParser {

public:
   MethodDescriptorParser(const std::string &descriptor) : DescriptorParser(descriptor) {
      //we assume that the descriptor is well-formed since it was successfully used to get the jmethodID
      assert(descriptor[pos++] == '(');
   }

   bool hasNextArg() {
      if (descriptor[pos] == ')') {
         ++pos;
         return false;
      }
      return true;
   }
};

std::vector<jvalue> convertArgs(MethodDescriptorParser &parser, const QoreValueList* args) {
   std::vector<jvalue> jargs(args == nullptr ? 0 : args->size());

   size_t index = 0;
   while (parser.hasNextArg()) {
      if (index >= jargs.size()) {
         throw BasicException("Too few arguments in a Java method invocation");
      }
      QoreValue qv = args->retrieveEntry(index);

      switch (parser.getType()) {
         case 'Z':
            jargs[index].z = QoreToJava::toBoolean(qv);
            break;
         case 'B':
            jargs[index].b = QoreToJava::toByte(qv);
            break;
         case 'C':
            jargs[index].c = QoreToJava::toChar(qv);
            break;
         case 'S':
            jargs[index].s = QoreToJava::toShort(qv);
            break;
         case 'I':
            jargs[index].i = QoreToJava::toInt(qv);
            break;
         case 'J':
            jargs[index].j = QoreToJava::toLong(qv);
            break;
         case 'F':
            jargs[index].f = QoreToJava::toFloat(qv);
            break;
         case 'D':
            jargs[index].d = QoreToJava::toDouble(qv);
            break;
         case 'L':
            jargs[index].l = QoreToJava::toObject(qv, parser.getClassName());
            break;
         case '[':
            jargs[index].l = QoreToJava::toArray(qv, parser.getArrayDescriptor());
            break;
         default:
            assert(false);      //invalid descriptor - should not happen
      }
      ++index;
   }

   if (index != jargs.size()) {
      throw BasicException("Too many arguments in a Java method invocation");
   }
   return std::move(jargs);
}

QoreValue Method::invokeStatic(const QoreValueList* args) {
   MethodDescriptorParser parser(descriptor);
   std::vector<jvalue> jargs = convertArgs(parser, args);

   Env env;
   switch (parser.getType()) {
      case 'V':
         env.callStaticVoidMethod(clazz->getJavaObject(), id, &jargs[0]);
         return QoreValue();
      case 'Z':
         return JavaToQore::convert(env.callStaticBooleanMethod(clazz->getJavaObject(), id, &jargs[0]));
      case 'B':
         return JavaToQore::convert(env.callStaticByteMethod(clazz->getJavaObject(), id, &jargs[0]));
      case 'C':
         return JavaToQore::convert(env.callStaticCharMethod(clazz->getJavaObject(), id, &jargs[0]));
      case 'S':
         return JavaToQore::convert(env.callStaticShortMethod(clazz->getJavaObject(), id, &jargs[0]));
      case 'I':
         return JavaToQore::convert(env.callStaticIntMethod(clazz->getJavaObject(), id, &jargs[0]));
      case 'J':
         return JavaToQore::convert(env.callStaticLongMethod(clazz->getJavaObject(), id, &jargs[0]));
      case 'F':
         return JavaToQore::convert(env.callStaticFloatMethod(clazz->getJavaObject(), id, &jargs[0]));
      case 'D':
         return JavaToQore::convert(env.callStaticDoubleMethod(clazz->getJavaObject(), id, &jargs[0]));
      case 'L':
         return JavaToQore::convertObject(env.callStaticObjectMethod(clazz->getJavaObject(), id, &jargs[0]), parser.getClassName());
      case '[':
         return JavaToQore::convertArray(env.callStaticObjectMethod(clazz->getJavaObject(), id, &jargs[0]), parser.getArrayDescriptor());
      default:
         assert(false);         //invalid descriptor - should not happen
         return QoreValue();
   }
}

} // namespace jni
