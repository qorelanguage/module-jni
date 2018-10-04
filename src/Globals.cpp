//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2016 - 2018 Qore Technologies, s.r.o.
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
#include "Array.h"
#include "QoreToJava.h"

namespace jni {

GlobalReference<jclass> Globals::classPrimitiveVoid;
GlobalReference<jclass> Globals::classPrimitiveBoolean;
GlobalReference<jclass> Globals::classPrimitiveByte;
GlobalReference<jclass> Globals::classPrimitiveChar;
GlobalReference<jclass> Globals::classPrimitiveShort;
GlobalReference<jclass> Globals::classPrimitiveInt;
GlobalReference<jclass> Globals::classPrimitiveLong;
GlobalReference<jclass> Globals::classPrimitiveFloat;
GlobalReference<jclass> Globals::classPrimitiveDouble;

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
jmethodID Globals::methodClassGetDeclaredField;

GlobalReference<jclass> Globals::classThrowable;
jmethodID Globals::methodThrowableGetMessage;
jmethodID Globals::methodThrowableGetStackTrace;

GlobalReference<jclass> Globals::classStackTraceElement;
jmethodID Globals::methodStackTraceElementGetClassName;
jmethodID Globals::methodStackTraceElementGetFileName;
jmethodID Globals::methodStackTraceElementGetLineNumber;
jmethodID Globals::methodStackTraceElementGetMethodName;
jmethodID Globals::methodStackTraceElementIsNativeMethod;

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
jmethodID Globals::methodMethodToGenericString;

GlobalReference<jclass> Globals::classConstructor;
jmethodID Globals::methodConstructorGetParameterTypes;
jmethodID Globals::methodConstructorToString;
jmethodID Globals::methodConstructorGetModifiers;
jmethodID Globals::methodConstructorIsVarArgs;

GlobalReference<jclass> Globals::classQoreInvocationHandler;
jmethodID Globals::ctorQoreInvocationHandler;
jmethodID Globals::methodQoreInvocationHandlerDestroy;

GlobalReference<jclass> Globals::classQoreJavaApi;
jmethodID Globals::methodQoreJavaApiCallFunction;
jmethodID Globals::methodQoreJavaApiCallFunctionArgs;

GlobalReference<jclass> Globals::classQoreExceptionWrapper;
jmethodID Globals::ctorQoreExceptionWrapper;
jmethodID Globals::methodQoreExceptionWrapperGet;

GlobalReference<jclass> Globals::classProxy;
jmethodID Globals::methodProxyNewProxyInstance;

GlobalReference<jclass> Globals::classClassLoader;
jmethodID Globals::methodClassLoaderLoadClass;

GlobalReference<jclass> Globals::classQoreURLClassLoader;
jmethodID Globals::ctorQoreURLClassLoader;
jmethodID Globals::methodQoreURLClassLoaderAddPath;
jmethodID Globals::methodQoreURLClassLoaderLoadClass;
jmethodID Globals::methodQoreURLClassLoaderSetContext;
jmethodID Globals::methodQoreURLClassLoaderGetProgramPtr;

GlobalReference<jclass> Globals::classThread;
jmethodID Globals::methodThreadCurrentThread;
jmethodID Globals::methodThreadGetContextClassLoader;

GlobalReference<jclass> Globals::classHashMap;
jmethodID Globals::ctorHashMap;
jmethodID Globals::methodHashMapPut;

// java.time.ZonedDateTime
GlobalReference<jclass> Globals::classZonedDateTime;
jmethodID Globals::methodZonedDateTimeParse;
jmethodID Globals::methodZonedDateTimeToString;

// java.time.Period
GlobalReference<jclass> Globals::classPeriod;

GlobalReference<jclass> Globals::classBoolean;
jmethodID Globals::ctorBoolean;
jmethodID Globals::methodBooleanBooleanValue;

GlobalReference<jclass> Globals::classInteger;
jmethodID Globals::ctorInteger;
jmethodID Globals::methodIntegerIntValue;

GlobalReference<jclass> Globals::classLong;
jmethodID Globals::ctorLong;
jmethodID Globals::methodLongLongValue;

GlobalReference<jclass> Globals::classShort;
jmethodID Globals::ctorShort;
jmethodID Globals::methodShortShortValue;

GlobalReference<jclass> Globals::classByte;
jmethodID Globals::ctorByte;
jmethodID Globals::methodByteByteValue;

GlobalReference<jclass> Globals::classDouble;
jmethodID Globals::ctorDouble;
jmethodID Globals::methodDoubleDoubleValue;

GlobalReference<jclass> Globals::classFloat;
jmethodID Globals::ctorFloat;
jmethodID Globals::methodFloatFloatValue;

GlobalReference<jclass> Globals::classCharacter;
jmethodID Globals::ctorCharacter;
jmethodID Globals::methodCharacterCharValue;

static void JNICALL invocation_handler_finalize(JNIEnv *, jclass, jlong ptr) {
   delete reinterpret_cast<Dispatcher *>(ptr);
}

static jobject JNICALL invocation_handler_invoke(JNIEnv* jenv, jobject, jlong ptr, jobject proxy, jobject method, jobjectArray args) {
   Env env(jenv);
   Dispatcher* dispatcher = reinterpret_cast<Dispatcher*>(ptr);
   return dispatcher->dispatch(env, proxy, method, args);
}

static jobject JNICALL java_api_call_function(JNIEnv* jenv, jobject obj, jlong ptr, jstring name, jobjectArray args) {
   QoreProgram* pgm = reinterpret_cast<QoreProgram*>(ptr);
   qoreThreadAttacher.attach();

   Env env(jenv);

   QoreProgramContextHelper pch(pgm);

   ExceptionSink xsink;

   jsize len = args ? env.getArrayLength(args) : 0;
   ReferenceHolder<QoreListNode> qore_args(&xsink);

   if (len)
      qore_args = Array::getArgList(env, args);

   Env::GetStringUtfChars fname(env, name);
   //printd(LogLevel, "java_api_call_function() '%s()' args: %p %d\n", fname.c_str(), *qore_args, len);

   ValueHolder rv(pgm->callFunction(fname.c_str(), *qore_args, &xsink), &xsink);

   if (xsink) {
      QoreToJava::wrapException(xsink);
      return nullptr;
   }

   try {
      return QoreToJava::toAnyObject(*rv);
   }
   catch (jni::Exception& e) {
      e.convert(&xsink);
      QoreToJava::wrapException(xsink);
      return nullptr;
   }
}

static void JNICALL qore_exception_wrapper_finalize(JNIEnv*, jclass, jlong ptr) {
   ExceptionSink* xsink = reinterpret_cast<ExceptionSink*>(ptr);
   //printd(LogLevel, "qore_exception_wrapper_finalize() xsink: %p\n", xsink);
   if (xsink != nullptr) {
      xsink->clear();
      delete xsink;
   }
}

static jstring JNICALL qore_exception_wrapper_get_message(JNIEnv*, jclass, jlong ptr) {
    ExceptionSink* xsink = reinterpret_cast<ExceptionSink*>(ptr);

    QoreString jstr;
    QoreValue err = xsink->getExceptionErr();
    QoreStringValueHelper err_str(err);
    QoreValue desc = xsink->getExceptionDesc();
    QoreStringValueHelper desc_str(desc);

    if (!err_str->empty()) {
        if (!desc_str->empty()) {
            jstr.concat(err_str->c_str());
            jstr.concat(": ");
            jstr.concat(desc_str->c_str());
        }
        else {
            jstr.concat(err_str->c_str());
        }
    }
    else {
        if (!desc_str->empty())
            jstr.concat(desc_str->c_str());
        else
            jstr.concat("No message");
    }

    //printd(LogLevel, "qore_exception_wrapper_get_message() xsink: %p %s\n", xsink, jstr.c_str());

    Env env;
    ModifiedUtf8String str(jstr);
    return env.newString(str.c_str()).release();
}

static JNINativeMethod invocationHandlerNativeMethods[2] = {
      {
            const_cast<char*>("finalize0"),
            const_cast<char*>("(J)V"),
            reinterpret_cast<void*>(invocation_handler_finalize)
      },
      {
            const_cast<char*>("invoke0"),
            const_cast<char*>("(JLjava/lang/Object;Ljava/lang/reflect/Method;[Ljava/lang/Object;)Ljava/lang/Object;"),
            reinterpret_cast<void*>(invocation_handler_invoke)
      }
};

static JNINativeMethod qoreJavaApiNativeMethods[1] = {
      {
            const_cast<char*>("callFunction0"),
            const_cast<char*>("(JLjava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;"),
            reinterpret_cast<void*>(java_api_call_function)
      },
};

static JNINativeMethod qoreExceptionWrapperNativeMethods[2] = {
      {
            const_cast<char*>("finalize0"),
            const_cast<char*>("(J)V"),
            reinterpret_cast<void*>(qore_exception_wrapper_finalize)
      },
      {
            const_cast<char*>("getMessage0"),
            const_cast<char*>("(J)Ljava/lang/String;"),
            reinterpret_cast<void*>(qore_exception_wrapper_get_message)
      }
};

static GlobalReference<jclass> getPrimitiveClass(Env &env, const char *wrapperName) {
   LocalReference<jclass> wrapperClass = env.findClass(wrapperName);
   jfieldID typeFieldId = env.getStaticField(wrapperClass, "TYPE", "Ljava/lang/Class;");
   return std::move(env.getStaticObjectField(wrapperClass, typeFieldId).as<jclass>().makeGlobal());
}

#include "JavaClassQoreInvocationHandler.inc"
#include "JavaClassQoreExceptionWrapper.inc"
#include "JavaClassQoreURLClassLoader.inc"
#include "JavaClassQoreURLClassLoader_1.inc"
#include "JavaClassQoreJavaApi.inc"

void Globals::init() {
    Env env;

    // get exception info first
    classThrowable = env.findClass("java/lang/Throwable").makeGlobal();
    methodThrowableGetMessage = env.getMethod(classThrowable, "getMessage", "()Ljava/lang/String;");
    methodThrowableGetStackTrace = env.getMethod(classThrowable, "getStackTrace", "()[Ljava/lang/StackTraceElement;");

    classStackTraceElement = env.findClass("java/lang/StackTraceElement").makeGlobal();
    methodStackTraceElementGetClassName = env.getMethod(classStackTraceElement, "getClassName", "()Ljava/lang/String;");
    methodStackTraceElementGetFileName = env.getMethod(classStackTraceElement, "getFileName", "()Ljava/lang/String;");
    methodStackTraceElementGetLineNumber = env.getMethod(classStackTraceElement, "getLineNumber", "()I");
    methodStackTraceElementGetMethodName = env.getMethod(classStackTraceElement, "getMethodName", "()Ljava/lang/String;");
    methodStackTraceElementIsNativeMethod = env.getMethod(classStackTraceElement, "isNativeMethod", "()Z");

    classQoreExceptionWrapper = env.defineClass("org/qore/jni/QoreExceptionWrapper", nullptr, java_org_qore_jni_QoreExceptionWrapper_class, java_org_qore_jni_QoreExceptionWrapper_class_len).makeGlobal();
    env.registerNatives(classQoreExceptionWrapper, qoreExceptionWrapperNativeMethods, 2);
    ctorQoreExceptionWrapper = env.getMethod(classQoreExceptionWrapper, "<init>", "(J)V");
    methodQoreExceptionWrapperGet = env.getMethod(classQoreExceptionWrapper, "get", "()J");

    classPrimitiveVoid = getPrimitiveClass(env, "java/lang/Void");
    classPrimitiveBoolean = getPrimitiveClass(env, "java/lang/Boolean");
    classPrimitiveByte = getPrimitiveClass(env, "java/lang/Byte");
    classPrimitiveChar = getPrimitiveClass(env, "java/lang/Character");
    classPrimitiveShort = getPrimitiveClass(env, "java/lang/Short");
    classPrimitiveInt = getPrimitiveClass(env, "java/lang/Integer");
    classPrimitiveLong = getPrimitiveClass(env, "java/lang/Long");
    classPrimitiveFloat = getPrimitiveClass(env, "java/lang/Float");
    classPrimitiveDouble = getPrimitiveClass(env, "java/lang/Double");

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
    methodClassGetDeclaredField = env.getMethod(classClass, "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;");

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
    methodMethodToGenericString = env.getMethod(classMethod, "toGenericString", "()Ljava/lang/String;");

    classConstructor = env.findClass("java/lang/reflect/Constructor").makeGlobal();
    methodConstructorGetParameterTypes = env.getMethod(classConstructor, "getParameterTypes", "()[Ljava/lang/Class;");
    methodConstructorToString = env.getMethod(classConstructor, "toString", "()Ljava/lang/String;");
    methodConstructorGetModifiers = env.getMethod(classConstructor, "getModifiers", "()I");
    methodConstructorIsVarArgs = env.getMethod(classConstructor, "isVarArgs", "()Z");

    classQoreInvocationHandler = env.defineClass("org/qore/jni/QoreInvocationHandler", nullptr, java_org_qore_jni_QoreInvocationHandler_class, java_org_qore_jni_QoreInvocationHandler_class_len).makeGlobal();
    env.registerNatives(classQoreInvocationHandler, invocationHandlerNativeMethods, 2);
    ctorQoreInvocationHandler = env.getMethod(classQoreInvocationHandler, "<init>", "(J)V");
    methodQoreInvocationHandlerDestroy = env.getMethod(classQoreInvocationHandler, "destroy", "()V");

    classQoreJavaApi = env.defineClass("org/qore/jni/QoreJavaApi", nullptr, java_org_qore_jni_QoreJavaApi_class, java_org_qore_jni_QoreJavaApi_class_len).makeGlobal();
    env.registerNatives(classQoreJavaApi, qoreJavaApiNativeMethods, 1);

    classProxy = env.findClass("java/lang/reflect/Proxy").makeGlobal();
    methodProxyNewProxyInstance = env.getStaticMethod(classProxy, "newProxyInstance", "(Ljava/lang/ClassLoader;[Ljava/lang/Class;Ljava/lang/reflect/InvocationHandler;)Ljava/lang/Object;");

    classClassLoader = env.findClass("java/lang/ClassLoader").makeGlobal();
    methodClassLoaderLoadClass = env.getMethod(classClassLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    classQoreURLClassLoader = env.defineClass("org/qore/jni/QoreURLClassLoader", nullptr, java_org_qore_jni_QoreURLClassLoader_class, java_org_qore_jni_QoreURLClassLoader_class_len).makeGlobal();
    ctorQoreURLClassLoader = env.getMethod(classQoreURLClassLoader, "<init>", "(J)V");
    methodQoreURLClassLoaderAddPath = env.getMethod(classQoreURLClassLoader, "addPath", "(Ljava/lang/String;)V");
    methodQoreURLClassLoaderLoadClass = env.getMethod(classQoreURLClassLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    methodQoreURLClassLoaderSetContext = env.getMethod(classQoreURLClassLoader, "setContext", "()V");
    methodQoreURLClassLoaderGetProgramPtr = env.getStaticMethod(classQoreURLClassLoader, "getProgramPtr", "()J");

    env.defineClass("org/qore/jni/QoreURLClassLoader$1", nullptr, java_org_qore_jni_QoreURLClassLoader_1_class, java_org_qore_jni_QoreURLClassLoader_1_class_len);

    classThread = env.findClass("java/lang/Thread").makeGlobal();
    methodThreadCurrentThread = env.getStaticMethod(classThread, "currentThread", "()Ljava/lang/Thread;");
    methodThreadGetContextClassLoader = env.getMethod(classThread, "getContextClassLoader", "()Ljava/lang/ClassLoader;");

    classHashMap = env.findClass("java/util/HashMap").makeGlobal();
    ctorHashMap = env.getMethod(classHashMap, "<init>", "()V");
    methodHashMapPut = env.getMethod(classHashMap, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    classZonedDateTime = env.findClass("java/time/ZonedDateTime").makeGlobal();
    methodZonedDateTimeParse = env.getStaticMethod(classZonedDateTime, "parse", "(Ljava/lang/CharSequence;)Ljava/time/ZonedDateTime;");
    methodZonedDateTimeToString = env.getMethod(classZonedDateTime, "toString", "()Ljava/lang/String;");

    classPeriod = env.findClass("java/time/Period").makeGlobal();

    classBoolean = env.findClass("java/lang/Boolean").makeGlobal();
    ctorBoolean = env.getMethod(classBoolean, "<init>", "(Z)V");
    methodBooleanBooleanValue = env.getMethod(classBoolean, "booleanValue", "()Z");

    classInteger = env.findClass("java/lang/Integer").makeGlobal();
    ctorInteger = env.getMethod(classInteger, "<init>", "(I)V");
    methodIntegerIntValue = env.getMethod(classInteger, "intValue", "()I");

    classDouble = env.findClass("java/lang/Double").makeGlobal();
    ctorDouble = env.getMethod(classDouble, "<init>", "(D)V");
    methodDoubleDoubleValue = env.getMethod(classDouble, "doubleValue", "()D");

    classLong = env.findClass("java/lang/Long").makeGlobal();
    ctorLong = env.getMethod(classLong, "<init>", "(J)V");
    methodLongLongValue = env.getMethod(classLong, "longValue", "()J");

    classShort = env.findClass("java/lang/Short").makeGlobal();
    ctorShort = env.getMethod(classShort, "<init>", "(S)V");
    methodShortShortValue = env.getMethod(classShort, "shortValue", "()S");

    classByte = env.findClass("java/lang/Byte").makeGlobal();
    ctorByte = env.getMethod(classByte, "<init>", "(B)V");
    methodByteByteValue = env.getMethod(classByte, "byteValue", "()B");

    classFloat = env.findClass("java/lang/Float").makeGlobal();
    ctorFloat = env.getMethod(classFloat, "<init>", "(F)V");
    methodFloatFloatValue = env.getMethod(classFloat, "floatValue", "()F");

    classCharacter = env.findClass("java/lang/Character").makeGlobal();
    ctorCharacter = env.getMethod(classCharacter, "<init>", "(C)V");
    methodCharacterCharValue = env.getMethod(classCharacter, "charValue", "()C");
}

void Globals::cleanup() {
    classThrowable = nullptr;
    classStackTraceElement = nullptr;
    classPrimitiveVoid = nullptr;
    classPrimitiveBoolean = nullptr;
    classPrimitiveByte = nullptr;
    classPrimitiveChar = nullptr;
    classPrimitiveShort = nullptr;
    classPrimitiveInt = nullptr;
    classPrimitiveLong = nullptr;
    classPrimitiveFloat = nullptr;
    classPrimitiveDouble = nullptr;
    classObject = nullptr;
    classClass = nullptr;
    classString = nullptr;
    classField = nullptr;
    classMethod = nullptr;
    classConstructor = nullptr;
    classQoreInvocationHandler = nullptr;
    classQoreExceptionWrapper = nullptr;
    classQoreJavaApi = nullptr;
    classProxy = nullptr;
    classClassLoader = nullptr;
    classQoreURLClassLoader = nullptr;
    classThread = nullptr;
    classHashMap = nullptr;
    classZonedDateTime = nullptr;
    classPeriod = nullptr;
    classBoolean = nullptr;
    classInteger = nullptr;
    classLong = nullptr;
    classShort = nullptr;
    classByte = nullptr;
    classDouble = nullptr;
    classFloat = nullptr;
    classCharacter = nullptr;
}

Type Globals::getType(jclass cls) {
   Env env;
   if (env.isSameObject(cls, classPrimitiveInt)) {
      return Type::Int;
   }
   if (env.isSameObject(cls, classPrimitiveVoid)) {
      return Type::Void;
   }
   if (env.isSameObject(cls, classPrimitiveBoolean)) {
      return Type::Boolean;
   }
   if (env.isSameObject(cls, classPrimitiveByte)) {
      return Type::Byte;
   }
   if (env.isSameObject(cls, classPrimitiveChar)) {
      return Type::Char;
   }
   if (env.isSameObject(cls, classPrimitiveShort)) {
      return Type::Short;
   }
   if (env.isSameObject(cls, classPrimitiveLong)) {
      return Type::Long;
   }
   if (env.isSameObject(cls, classPrimitiveFloat)) {
      return Type::Float;
   }
   if (env.isSameObject(cls, classPrimitiveDouble)) {
      return Type::Double;
   }
   return Type::Reference;
}

} // namespace jni
