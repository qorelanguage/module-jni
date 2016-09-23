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
#include "Dispatcher.h"

namespace jni {

GlobalReference<jclass> Globals::classVoid;
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

GlobalReference<jclass> Globals::classMethod;
jmethodID Globals::methodMethodGetReturnType;
jmethodID Globals::methodMethodGetParameterTypes;
jmethodID Globals::methodMethodGetDeclaringClass;
jmethodID Globals::methodMethodGetModifiers;

GlobalReference<jclass> Globals::classInvocationHandlerImpl;
jmethodID Globals::ctorInvocationHandlerImpl;
jmethodID Globals::methodInvocationHandlerImplDeref;

static void JNICALL invocation_handler_finalize(JNIEnv *, jclass, jlong ptr) {
   delete reinterpret_cast<Dispatcher *>(ptr);
}

static jobject JNICALL invocation_handler_invoke(JNIEnv *jenv, jobject, jlong ptr, jobject proxy, jobject method, jobjectArray args) {
   Env env(jenv);
   Dispatcher *dispatcher = reinterpret_cast<Dispatcher *>(ptr);
   return dispatcher->dispatch(env, proxy, method, args);
}

static JNINativeMethod invocationHandlerNativeMethods[2] = {
      {
            const_cast<char *>("finalize0"),
            const_cast<char *>("(J)V"),
            reinterpret_cast<void*>(invocation_handler_finalize)
      },
      {
            const_cast<char *>("invoke0"),
            const_cast<char *>("(JLjava/lang/Object;Ljava/lang/reflect/Method;[Ljava/lang/Object;)Ljava/lang/Object;"),
            reinterpret_cast<void*>(invocation_handler_invoke)
      }
};

static GlobalReference<jclass> getPrimitiveClass(Env &env, const char *wrapperName) {
   LocalReference<jclass> wrapperClass = env.findClass(wrapperName);
   jfieldID typeFieldId = env.getStaticField(wrapperClass, "TYPE", "Ljava/lang/Class;");
   return std::move(env.getStaticObjectField(wrapperClass, typeFieldId).as<jclass>().makeGlobal());
}

void Globals::init() {
   Env env;
   classVoid = getPrimitiveClass(env, "java/lang/Void");
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

   classMethod = env.findClass("java/lang/reflect/Method").makeGlobal();
   methodMethodGetReturnType = env.getMethod(classMethod, "getReturnType", "()Ljava/lang/Class;");
   methodMethodGetParameterTypes = env.getMethod(classMethod, "getParameterTypes", "()[Ljava/lang/Class;");
   methodMethodGetDeclaringClass = env.getMethod(classMethod, "getDeclaringClass", "()Ljava/lang/Class;");
   methodMethodGetModifiers = env.getMethod(classMethod, "getModifiers", "()I");

   classInvocationHandlerImpl = env.findClass("org/qore/jni/InvocationHandlerImpl").makeGlobal();
   env.registerNatives(classInvocationHandlerImpl, invocationHandlerNativeMethods, 2);
   ctorInvocationHandlerImpl = env.getMethod(classInvocationHandlerImpl, "<init>", "(J)V");
   methodInvocationHandlerImplDeref = env.getMethod(classInvocationHandlerImpl, "deref", "()V");
}

void Globals::cleanup() {
   classVoid = nullptr;
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
   classMethod = nullptr;
   classInvocationHandlerImpl = nullptr;
}

Type Globals::getType(jclass clazz) {
   Env env;
   if (env.isSameObject(clazz, classInt)) {
      return Type::Int;
   }
   if (env.isSameObject(clazz, classVoid)) {
      return Type::Void;
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
