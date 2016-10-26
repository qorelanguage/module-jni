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
/// \brief TODO file description
///
//------------------------------------------------------------------------------
#include "Globals.h"
#include "Env.h"
#include "Dispatcher.h"
#include "ModifiedUtf8String.h"

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

GlobalReference<jclass> Globals::classObject;
jmethodID Globals::methodObjectGetClass;

GlobalReference<jclass> Globals::classClass;
jmethodID Globals::methodClassIsArray;
jmethodID Globals::methodClassGetComponentType;
jmethodID Globals::methodClassGetClassLoader;
jmethodID Globals::methodClassGetName;
jmethodID Globals::methodClassGetDeclaredFields;
jmethodID Globals::methodClassGetSuperClass;
jmethodID Globals::methodClassGetInterfaces;
jmethodID Globals::methodClassGetDeclaredConstructors;
jmethodID Globals::methodClassGetModifiers;
jmethodID Globals::methodClassIsPrimitive;
jmethodID Globals::methodClassGetDeclaredMethods;
jmethodID Globals::methodClassGetCanonicalName;

GlobalReference<jclass> Globals::classThrowable;
jmethodID Globals::methodThrowableGetMessage;

GlobalReference<jclass> Globals::classString;

GlobalReference<jclass> Globals::classField;
jmethodID Globals::methodFieldGetDeclaringClass;
jmethodID Globals::methodFieldGetType;
jmethodID Globals::methodFieldGetModifiers;
jmethodID Globals::methodFieldGetName;
jmethodID Globals::methodFieldGet;

GlobalReference<jclass> Globals::classMethod;
jmethodID Globals::methodMethodGetReturnType;
jmethodID Globals::methodMethodGetParameterTypes;
jmethodID Globals::methodMethodGetDeclaringClass;
jmethodID Globals::methodMethodGetModifiers;
jmethodID Globals::methodMethodIsVarArgs;
jmethodID Globals::methodMethodGetName;

GlobalReference<jclass> Globals::classConstructor;
jmethodID Globals::methodConstructorGetParameterTypes;
jmethodID Globals::methodConstructorToString;
jmethodID Globals::methodConstructorGetModifiers;
jmethodID Globals::methodConstructorIsVarArgs;

GlobalReference<jclass> Globals::classInvocationHandlerImpl;
jmethodID Globals::ctorInvocationHandlerImpl;
jmethodID Globals::methodInvocationHandlerImplDestroy;

GlobalReference<jclass> Globals::classQoreExceptionWrapper;
jmethodID Globals::ctorQoreExceptionWrapper;
jmethodID Globals::methodQoreExceptionWrapperGet;

GlobalReference<jclass> Globals::classProxy;
jmethodID Globals::methodProxyNewProxyInstance;

GlobalReference<jclass> Globals::classQoreURLClassLoader;
jmethodID Globals::ctorQoreURLClassLoader;
jmethodID Globals::methodQoreURLClassLoaderAddPath;
jmethodID Globals::methodQoreURLClassLoaderLoadClass;

static void JNICALL invocation_handler_finalize(JNIEnv *, jclass, jlong ptr) {
   delete reinterpret_cast<Dispatcher *>(ptr);
}

static jobject JNICALL invocation_handler_invoke(JNIEnv *jenv, jobject, jlong ptr, jobject proxy, jobject method, jobjectArray args) {
   Env env(jenv);
   Dispatcher *dispatcher = reinterpret_cast<Dispatcher *>(ptr);
   return dispatcher->dispatch(env, proxy, method, args);
}

static void JNICALL qore_exception_wrapper_finalize(JNIEnv *, jclass, jlong ptr) {
   ExceptionSink *xsink = reinterpret_cast<ExceptionSink *>(ptr);
   if (xsink != nullptr) {
      xsink->clear();
      delete xsink;
   }
}

static jstring JNICALL qore_exception_wrapper_get_message(JNIEnv *, jclass, jlong ptr) {
   Env env;
   ExceptionSink *xsink = reinterpret_cast<ExceptionSink *>(ptr);

   const AbstractQoreNode *desc = xsink->getExceptionDesc();
   if (desc != nullptr && desc->getType() == NT_STRING) {
      ModifiedUtf8String str(*static_cast<const QoreStringNode*>(desc));
      return env.newString(str.c_str()).release();
   }
   return env.newString("No message").release();
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

static JNINativeMethod qoreExceptionWrapperNativeMethods[2] = {
      {
            const_cast<char *>("finalize0"),
            const_cast<char *>("(J)V"),
            reinterpret_cast<void*>(qore_exception_wrapper_finalize)
      },
      {
            const_cast<char *>("getMessage0"),
            const_cast<char *>("(J)Ljava/lang/String;"),
            reinterpret_cast<void*>(qore_exception_wrapper_get_message)
      }
};

static GlobalReference<jclass> getPrimitiveClass(Env &env, const char *wrapperName) {
   LocalReference<jclass> wrapperClass = env.findClass(wrapperName);
   jfieldID typeFieldId = env.getStaticField(wrapperClass, "TYPE", "Ljava/lang/Class;");
   return std::move(env.getStaticObjectField(wrapperClass, typeFieldId).as<jclass>().makeGlobal());
}

#include "JavaClassInvocationHandlerImpl.inc"
#include "JavaClassQoreExceptionWrapper.inc"
#include "JavaClassQoreURLClassLoader.inc"

void Globals::init() {
   Env env;

   // get exception info first
   classThrowable = env.findClass("java/lang/Throwable").makeGlobal();
   methodThrowableGetMessage = env.getMethod(classThrowable, "getMessage", "()Ljava/lang/String;");

   classQoreExceptionWrapper = env.defineClass("org/qore/jni/QoreExceptionWrapper", nullptr, java_org_qore_jni_QoreExceptionWrapper_class, java_org_qore_jni_QoreExceptionWrapper_class_len).makeGlobal();
   env.registerNatives(classQoreExceptionWrapper, qoreExceptionWrapperNativeMethods, 2);
   ctorQoreExceptionWrapper = env.getMethod(classQoreExceptionWrapper, "<init>", "(J)V");
   methodQoreExceptionWrapperGet = env.getMethod(classQoreExceptionWrapper, "get", "()J");

   classVoid = getPrimitiveClass(env, "java/lang/Void");
   classBoolean = getPrimitiveClass(env, "java/lang/Boolean");
   classByte = getPrimitiveClass(env, "java/lang/Byte");
   classChar = getPrimitiveClass(env, "java/lang/Character");
   classShort = getPrimitiveClass(env, "java/lang/Short");
   classInt = getPrimitiveClass(env, "java/lang/Integer");
   classLong = getPrimitiveClass(env, "java/lang/Long");
   classFloat = getPrimitiveClass(env, "java/lang/Float");
   classDouble = getPrimitiveClass(env, "java/lang/Double");

   classObject = env.findClass("java/lang/Object").makeGlobal();
   methodObjectGetClass = env.getMethod(classObject, "getClass", "()Ljava/lang/Class;");

   classClass = env.findClass("java/lang/Class").makeGlobal();
   methodClassIsArray = env.getMethod(classClass, "isArray", "()Z");
   methodClassGetComponentType = env.getMethod(classClass, "getComponentType", "()Ljava/lang/Class;");
   methodClassGetClassLoader = env.getMethod(classClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
   methodClassGetName = env.getMethod(classClass, "getName", "()Ljava/lang/String;");
   methodClassGetDeclaredFields = env.getMethod(classClass, "getDeclaredFields", "()[Ljava/lang/reflect/Field;");
   methodClassGetSuperClass = env.getMethod(classClass, "getSuperclass", "()Ljava/lang/Class;");
   methodClassGetInterfaces = env.getMethod(classClass, "getInterfaces", "()[Ljava/lang/Class;");
   methodClassGetDeclaredConstructors = env.getMethod(classClass, "getDeclaredConstructors", "()[Ljava/lang/reflect/Constructor;");
   methodClassGetModifiers = env.getMethod(classClass, "getModifiers", "()I");
   methodClassIsPrimitive = env.getMethod(classClass, "isPrimitive", "()Z");
   methodClassGetDeclaredMethods = env.getMethod(classClass, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");
   methodClassGetCanonicalName = env.getMethod(classClass, "getCanonicalName", "()Ljava/lang/String;");

   classString = env.findClass("java/lang/String").makeGlobal();

   classField = env.findClass("java/lang/reflect/Field").makeGlobal();
   methodFieldGetType = env.getMethod(classField, "getType", "()Ljava/lang/Class;");
   methodFieldGetDeclaringClass = env.getMethod(classField, "getDeclaringClass", "()Ljava/lang/Class;");
   methodFieldGetModifiers = env.getMethod(classField, "getModifiers", "()I");
   methodFieldGetName = env.getMethod(classField, "getName", "()Ljava/lang/String;");
   methodFieldGet = env.getMethod(classField, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");

   classMethod = env.findClass("java/lang/reflect/Method").makeGlobal();
   methodMethodGetReturnType = env.getMethod(classMethod, "getReturnType", "()Ljava/lang/Class;");
   methodMethodGetParameterTypes = env.getMethod(classMethod, "getParameterTypes", "()[Ljava/lang/Class;");
   methodMethodGetDeclaringClass = env.getMethod(classMethod, "getDeclaringClass", "()Ljava/lang/Class;");
   methodMethodGetModifiers = env.getMethod(classMethod, "getModifiers", "()I");
   methodMethodIsVarArgs = env.getMethod(classMethod, "isVarArgs", "()Z");
   methodMethodGetName = env.getMethod(classMethod, "getName", "()Ljava/lang/String;");

   classConstructor = env.findClass("java/lang/reflect/Constructor").makeGlobal();
   methodConstructorGetParameterTypes = env.getMethod(classConstructor, "getParameterTypes", "()[Ljava/lang/Class;");
   methodConstructorToString = env.getMethod(classConstructor, "toString", "()Ljava/lang/String;");
   methodConstructorGetModifiers = env.getMethod(classConstructor, "getModifiers", "()I");
   methodConstructorIsVarArgs = env.getMethod(classConstructor, "isVarArgs", "()Z");

   classInvocationHandlerImpl = env.defineClass("org/qore/jni/InvocationHandlerImpl", nullptr, java_org_qore_jni_InvocationHandlerImpl_class, java_org_qore_jni_InvocationHandlerImpl_class_len).makeGlobal();
   env.registerNatives(classInvocationHandlerImpl, invocationHandlerNativeMethods, 2);
   ctorInvocationHandlerImpl = env.getMethod(classInvocationHandlerImpl, "<init>", "(J)V");
   methodInvocationHandlerImplDestroy = env.getMethod(classInvocationHandlerImpl, "destroy", "()V");

   classProxy = env.findClass("java/lang/reflect/Proxy").makeGlobal();
   methodProxyNewProxyInstance = env.getStaticMethod(classProxy, "newProxyInstance", "(Ljava/lang/ClassLoader;[Ljava/lang/Class;Ljava/lang/reflect/InvocationHandler;)Ljava/lang/Object;");

   classQoreURLClassLoader = env.defineClass("org/qore/jni/QoreURLClassLoader", nullptr, java_org_qore_jni_QoreURLClassLoader_class, java_org_qore_jni_QoreURLClassLoader_class_len).makeGlobal();
   ctorQoreURLClassLoader = env.getMethod(classQoreURLClassLoader, "<init>", "()V");
   methodQoreURLClassLoaderAddPath = env.getMethod(classQoreURLClassLoader, "addPath", "(Ljava/lang/String;)V");
   methodQoreURLClassLoaderLoadClass = env.getMethod(classQoreURLClassLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
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
   classObject = nullptr;
   classClass = nullptr;
   classThrowable = nullptr;
   classString = nullptr;
   classField = nullptr;
   classMethod = nullptr;
   classConstructor = nullptr;
   classInvocationHandlerImpl = nullptr;
   classQoreExceptionWrapper = nullptr;
   classProxy = nullptr;
   classQoreURLClassLoader = nullptr;
}

Type Globals::getType(jclass cls) {
   Env env;
   if (env.isSameObject(cls, classInt)) {
      return Type::Int;
   }
   if (env.isSameObject(cls, classVoid)) {
      return Type::Void;
   }
   if (env.isSameObject(cls, classBoolean)) {
      return Type::Boolean;
   }
   if (env.isSameObject(cls, classByte)) {
      return Type::Byte;
   }
   if (env.isSameObject(cls, classChar)) {
      return Type::Char;
   }
   if (env.isSameObject(cls, classShort)) {
      return Type::Short;
   }
   if (env.isSameObject(cls, classLong)) {
      return Type::Long;
   }
   if (env.isSameObject(cls, classFloat)) {
      return Type::Float;
   }
   if (env.isSameObject(cls, classDouble)) {
      return Type::Double;
   }
   return Type::Reference;
}

} // namespace jni
