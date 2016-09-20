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

std::vector<jvalue> convertArgs(MethodDescriptorParser &parser, const QoreValueList* args, size_t base = 0) {
   assert(base == 0 || (args != nullptr && args->size() >= base));
   std::vector<jvalue> jargs(args == nullptr ? 0 : (args->size() - base));

   size_t index = 0;
   while (parser.hasNextArg()) {
      if (index >= jargs.size()) {
         throw BasicException("Too few arguments in a Java method invocation");
      }
      QoreValue qv = args->retrieveEntry(index + base);

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

QoreValue Method::invoke(jobject object, const QoreValueList* args) {
   MethodDescriptorParser parser(descriptor);
   std::vector<jvalue> jargs = convertArgs(parser, args, 1);

   Env env;
   if (!env.isInstanceOf(object, clazz->getJavaObject())) {
      throw BasicException("Passed instance does not match the method's class");
   }
   switch (retValType) {
      case Type::Boolean:
         return JavaToQore::convert(env.callBooleanMethod(object, id, &jargs[0]));
      case Type::Byte:
         return JavaToQore::convert(env.callByteMethod(object, id, &jargs[0]));
      case Type::Char:
         return JavaToQore::convert(env.callCharMethod(object, id, &jargs[0]));
      case Type::Short:
         return JavaToQore::convert(env.callShortMethod(object, id, &jargs[0]));
      case Type::Int:
         return JavaToQore::convert(env.callIntMethod(object, id, &jargs[0]));
      case Type::Long:
         return JavaToQore::convert(env.callLongMethod(object, id, &jargs[0]));
      case Type::Float:
         return JavaToQore::convert(env.callFloatMethod(object, id, &jargs[0]));
      case Type::Double:
         return JavaToQore::convert(env.callDoubleMethod(object, id, &jargs[0]));
      case Type::Reference:
         return JavaToQore::convert(env.callObjectMethod(object, id, &jargs[0]));
      case Type::Void:
      default:
         assert(retValType == Type::Void);
         env.callVoidMethod(object, id, &jargs[0]);
         return QoreValue();
   }
}

QoreValue Method::invokeNonvirtual(jobject object, const QoreValueList* args) {
   MethodDescriptorParser parser(descriptor);
   std::vector<jvalue> jargs = convertArgs(parser, args, 1);

   Env env;
   if (!env.isInstanceOf(object, clazz->getJavaObject())) {
      throw BasicException("Passed instance does not match the method's class");
   }
   switch (retValType) {
      case Type::Boolean:
         return JavaToQore::convert(env.callNonvirtualBooleanMethod(object, clazz->getJavaObject(), id, &jargs[0]));
      case Type::Byte:
         return JavaToQore::convert(env.callNonvirtualByteMethod(object, clazz->getJavaObject(), id, &jargs[0]));
      case Type::Char:
         return JavaToQore::convert(env.callNonvirtualCharMethod(object, clazz->getJavaObject(), id, &jargs[0]));
      case Type::Short:
         return JavaToQore::convert(env.callNonvirtualShortMethod(object, clazz->getJavaObject(), id, &jargs[0]));
      case Type::Int:
         return JavaToQore::convert(env.callNonvirtualIntMethod(object, clazz->getJavaObject(), id, &jargs[0]));
      case Type::Long:
         return JavaToQore::convert(env.callNonvirtualLongMethod(object, clazz->getJavaObject(), id, &jargs[0]));
      case Type::Float:
         return JavaToQore::convert(env.callNonvirtualFloatMethod(object, clazz->getJavaObject(), id, &jargs[0]));
      case Type::Double:
         return JavaToQore::convert(env.callNonvirtualDoubleMethod(object, clazz->getJavaObject(), id, &jargs[0]));
      case Type::Reference:
         return JavaToQore::convert(env.callNonvirtualObjectMethod(object, clazz->getJavaObject(), id, &jargs[0]));
      case Type::Void:
      default:
         assert(retValType == Type::Void);
         env.callNonvirtualVoidMethod(object, clazz->getJavaObject(), id, &jargs[0]);
         return QoreValue();
   }
}

QoreValue Method::invokeStatic(const QoreValueList* args) {
   MethodDescriptorParser parser(descriptor);
   std::vector<jvalue> jargs = convertArgs(parser, args);

   Env env;
   switch (retValType) {
      case Type::Boolean:
         return JavaToQore::convert(env.callStaticBooleanMethod(clazz->getJavaObject(), id, &jargs[0]));
      case Type::Byte:
         return JavaToQore::convert(env.callStaticByteMethod(clazz->getJavaObject(), id, &jargs[0]));
      case Type::Char:
         return JavaToQore::convert(env.callStaticCharMethod(clazz->getJavaObject(), id, &jargs[0]));
      case Type::Short:
         return JavaToQore::convert(env.callStaticShortMethod(clazz->getJavaObject(), id, &jargs[0]));
      case Type::Int:
         return JavaToQore::convert(env.callStaticIntMethod(clazz->getJavaObject(), id, &jargs[0]));
      case Type::Long:
         return JavaToQore::convert(env.callStaticLongMethod(clazz->getJavaObject(), id, &jargs[0]));
      case Type::Float:
         return JavaToQore::convert(env.callStaticFloatMethod(clazz->getJavaObject(), id, &jargs[0]));
      case Type::Double:
         return JavaToQore::convert(env.callStaticDoubleMethod(clazz->getJavaObject(), id, &jargs[0]));
      case Type::Reference:
         return JavaToQore::convert(env.callStaticObjectMethod(clazz->getJavaObject(), id, &jargs[0]));
      case Type::Void:
      default:
         assert(retValType == Type::Void);
         env.callStaticVoidMethod(clazz->getJavaObject(), id, &jargs[0]);
         return QoreValue();
   }
}

} // namespace jni
