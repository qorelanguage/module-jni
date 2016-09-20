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
/// \brief TODO file description
///
//------------------------------------------------------------------------------
#include "Globals.h"
#include "Env.h"

namespace jni {

GlobalReference<jclass> Globals::classBoolean;
GlobalReference<jclass> Globals::classByte;
GlobalReference<jclass> Globals::classChar;
GlobalReference<jclass> Globals::classShort;
GlobalReference<jclass> Globals::classInt;
GlobalReference<jclass> Globals::classLong;
GlobalReference<jclass> Globals::classFloat;
GlobalReference<jclass> Globals::classDouble;

GlobalReference<jclass> Globals::classClass;
jmethodID Globals::methodClassIsArray;

GlobalReference<jclass> Globals::classField;
jmethodID Globals::methodFieldGetType;

static GlobalReference<jclass> getPrimitiveClass(Env &env, const char *wrapperName) {
   LocalReference<jclass> wrapperClass = env.findClass(wrapperName);
   jfieldID typeFieldId = env.getStaticField(wrapperClass, "TYPE", "Ljava/lang/Class;");
   return std::move(env.getStaticObjectField(wrapperClass, typeFieldId).as<jclass>().makeGlobal());
}

void Globals::init() {
   Env env;
   classBoolean = getPrimitiveClass(env, "java/lang/Boolean");
   classByte = getPrimitiveClass(env, "java/lang/Byte");
   classChar = getPrimitiveClass(env, "java/lang/Character");
   classShort = getPrimitiveClass(env, "java/lang/Short");
   classInt = getPrimitiveClass(env, "java/lang/Integer");
   classLong = getPrimitiveClass(env, "java/lang/Long");
   classFloat = getPrimitiveClass(env, "java/lang/Float");
   classDouble = getPrimitiveClass(env, "java/lang/Double");

   classClass = env.findClass("java/lang/Class").makeGlobal();
   methodClassIsArray = env.getMethod(classClass, "isArray", "()Z");

   classField = env.findClass("java/lang/reflect/Field").makeGlobal();
   methodFieldGetType = env.getMethod(classField, "getType", "()Ljava/lang/Class;");
}

void Globals::cleanup() {
   classBoolean = nullptr;
   classByte = nullptr;
   classChar = nullptr;
   classShort = nullptr;
   classInt = nullptr;
   classLong = nullptr;
   classFloat = nullptr;
   classDouble = nullptr;
   classClass = nullptr;
   classField = nullptr;
}

Type Globals::getType(jclass clazz) {
   Env env;
   if (env.isSameObject(clazz, classInt)) {
      return Type::Int;
   }
   if (env.isSameObject(clazz, classBoolean)) {
      return Type::Boolean;
   }
   if (env.isSameObject(clazz, classByte)) {
      return Type::Byte;
   }
   if (env.isSameObject(clazz, classChar)) {
      return Type::Char;
   }
   if (env.isSameObject(clazz, classShort)) {
      return Type::Short;
   }
   if (env.isSameObject(clazz, classLong)) {
      return Type::Long;
   }
   if (env.isSameObject(clazz, classFloat)) {
      return Type::Float;
   }
   if (env.isSameObject(clazz, classDouble)) {
      return Type::Double;
   }
   return Type::Reference;
}

} // namespace jni
