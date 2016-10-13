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
#include "Env.h"
#include "JavaToQore.h"
#include "QoreToJava.h"
#include "QoreJniClassMap.h"

namespace jni {

std::vector<jvalue> BaseMethod::convertArgs(const QoreValueList* args, size_t base) {
   assert(base == 0 || (args != nullptr && args->size() >= base));

   size_t paramCount = paramTypes.size();
   size_t argCount = args == nullptr ? 0 : args->size() - base;

   if (paramCount > argCount) {
      throw BasicException("Too few arguments in a Java method invocation");
   }
   if (paramCount < argCount) {
      throw BasicException("Too many arguments in a Java method invocation");
   }

   std::vector<jvalue> jargs(paramCount);
   for (size_t index = 0; index < paramCount; ++index) {
      QoreValue qv = args->retrieveEntry(index + base);

      switch (paramTypes[index].first) {
         case Type::Boolean:
            jargs[index].z = QoreToJava::toBoolean(qv);
            break;
         case Type::Byte:
            jargs[index].b = QoreToJava::toByte(qv);
            break;
         case Type::Char:
            jargs[index].c = QoreToJava::toChar(qv);
            break;
         case Type::Short:
            jargs[index].s = QoreToJava::toShort(qv);
            break;
         case Type::Int:
            jargs[index].i = QoreToJava::toInt(qv);
            break;
         case Type::Long:
            jargs[index].j = QoreToJava::toLong(qv);
            break;
         case Type::Float:
            jargs[index].f = QoreToJava::toFloat(qv);
            break;
         case Type::Double:
            jargs[index].d = QoreToJava::toDouble(qv);
            break;
         case Type::Reference:
         default:
            assert(paramTypes[index].first == Type::Reference);
            jargs[index].l = QoreToJava::toObject(qv, paramTypes[index].second);
            break;
      }
   }

   return std::move(jargs);
}

QoreValue BaseMethod::invoke(jobject object, const QoreValueList* args, int base) {
   std::vector<jvalue> jargs = convertArgs(args, base);

   Env env;
   if (!env.isInstanceOf(object, cls->getJavaObject())) {
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

QoreValue BaseMethod::invokeNonvirtual(jobject object, const QoreValueList* args, int base) {
   std::vector<jvalue> jargs = convertArgs(args, base);

   Env env;
   if (!env.isInstanceOf(object, cls->getJavaObject())) {
      throw BasicException("Passed instance does not match the method's class");
   }
   switch (retValType) {
      case Type::Boolean:
         return JavaToQore::convert(env.callNonvirtualBooleanMethod(object, cls->getJavaObject(), id, &jargs[0]));
      case Type::Byte:
         return JavaToQore::convert(env.callNonvirtualByteMethod(object, cls->getJavaObject(), id, &jargs[0]));
      case Type::Char:
         return JavaToQore::convert(env.callNonvirtualCharMethod(object, cls->getJavaObject(), id, &jargs[0]));
      case Type::Short:
         return JavaToQore::convert(env.callNonvirtualShortMethod(object, cls->getJavaObject(), id, &jargs[0]));
      case Type::Int:
         return JavaToQore::convert(env.callNonvirtualIntMethod(object, cls->getJavaObject(), id, &jargs[0]));
      case Type::Long:
         return JavaToQore::convert(env.callNonvirtualLongMethod(object, cls->getJavaObject(), id, &jargs[0]));
      case Type::Float:
         return JavaToQore::convert(env.callNonvirtualFloatMethod(object, cls->getJavaObject(), id, &jargs[0]));
      case Type::Double:
         return JavaToQore::convert(env.callNonvirtualDoubleMethod(object, cls->getJavaObject(), id, &jargs[0]));
      case Type::Reference:
         return JavaToQore::convert(env.callNonvirtualObjectMethod(object, cls->getJavaObject(), id, &jargs[0]));
      case Type::Void:
      default:
         assert(retValType == Type::Void);
         env.callNonvirtualVoidMethod(object, cls->getJavaObject(), id, &jargs[0]);
         return QoreValue();
   }
}

QoreValue BaseMethod::invokeStatic(const QoreValueList* args) {
   std::vector<jvalue> jargs = convertArgs(args);

   Env env;
   switch (retValType) {
      case Type::Boolean:
         return JavaToQore::convert(env.callStaticBooleanMethod(cls->getJavaObject(), id, &jargs[0]));
      case Type::Byte:
         return JavaToQore::convert(env.callStaticByteMethod(cls->getJavaObject(), id, &jargs[0]));
      case Type::Char:
         return JavaToQore::convert(env.callStaticCharMethod(cls->getJavaObject(), id, &jargs[0]));
      case Type::Short:
         return JavaToQore::convert(env.callStaticShortMethod(cls->getJavaObject(), id, &jargs[0]));
      case Type::Int:
         return JavaToQore::convert(env.callStaticIntMethod(cls->getJavaObject(), id, &jargs[0]));
      case Type::Long:
         return JavaToQore::convert(env.callStaticLongMethod(cls->getJavaObject(), id, &jargs[0]));
      case Type::Float:
         return JavaToQore::convert(env.callStaticFloatMethod(cls->getJavaObject(), id, &jargs[0]));
      case Type::Double:
         return JavaToQore::convert(env.callStaticDoubleMethod(cls->getJavaObject(), id, &jargs[0]));
      case Type::Reference:
         return JavaToQore::convert(env.callStaticObjectMethod(cls->getJavaObject(), id, &jargs[0]));
      case Type::Void:
      default:
         assert(retValType == Type::Void);
         env.callStaticVoidMethod(cls->getJavaObject(), id, &jargs[0]);
         return QoreValue();
   }
}

QoreValue BaseMethod::newInstance(const QoreValueList* args) {
   std::vector<jvalue> jargs = convertArgs(args);
   Env env;
   return JavaToQore::convert(env.newObject(cls->getJavaObject(), id, &jargs[0]));
}

LocalReference<jobject> BaseMethod::newQoreInstance(const QoreValueList* args) {
   std::vector<jvalue> jargs = convertArgs(args);
   Env env;
   return env.newObject(cls->getJavaObject(), id, &jargs[0]);
}

void BaseMethod::getName(QoreString& str) {
   Env env;
   // get Method name
   LocalReference<jstring> jmName = env.callObjectMethod(method, Globals::methodMethodGetName, nullptr).as<jstring>();
   Env::GetStringUtfChars mName(env, jmName);
   str.concat(mName.c_str());
}

int BaseMethod::getParamTypes(type_vec_t& paramTypeInfo, QoreJniClassMap& clsmap) {
   unsigned len = paramTypes.size();
   if (len)
      paramTypeInfo.reserve(len);

   for (auto& i : paramTypes)
      paramTypeInfo.push_back(clsmap.getQoreType(i.second));

   return 0;
}

const QoreTypeInfo* BaseMethod::getReturnTypeInfo(QoreJniClassMap& clsmap) {
   return clsmap.getQoreType(retValClass);
}

} // namespace jni
