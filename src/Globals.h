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
#ifndef QORE_JNI_GLOBALS_H_
#define QORE_JNI_GLOBALS_H_

#include "GlobalReference.h"

namespace jni {

enum class Type {
   Void, Boolean, Byte, Char, Short, Int, Long, Float, Double, Reference
};

class Globals {

public:
   static GlobalReference<jclass> classPrimitiveVoid;                   // class for the void return type
   static GlobalReference<jclass> classPrimitiveBoolean;                // class for the primitive type boolean
   static GlobalReference<jclass> classPrimitiveByte;                   // class for the primitive type byte
   static GlobalReference<jclass> classPrimitiveChar;                   // class for the primitive type char
   static GlobalReference<jclass> classPrimitiveShort;                  // class for the primitive type short
   static GlobalReference<jclass> classPrimitiveInt;                    // class for the primitive type int
   static GlobalReference<jclass> classPrimitiveLong;                   // class for the primitive type long
   static GlobalReference<jclass> classPrimitiveFloat;                  // class for the primitive type float
   static GlobalReference<jclass> classPrimitiveDouble;                 // class for the primitive type double

   static GlobalReference<jclass> classObject;                          // java.lang.Object
   static jmethodID methodObjectGetClass;                               // Class<?> Object.getClass()

   static GlobalReference<jclass> classClass;                           // java.lang.Class
   static jmethodID methodClassIsArray;                                 // boolean Class.isArray()
   static jmethodID methodClassGetComponentType;                        // Class<?> Class.getComponentType()
   static jmethodID methodClassGetClassLoader;                          // ClassLoader Class.getClassLoader()
   static jmethodID methodClassGetName;                                 // String Class.getName()
   static jmethodID methodClassGetDeclaredFields;                       // Field[] Class.getDeclaredFields()
   static jmethodID methodClassGetSuperClass;                           // Class<? super T> Class.getSuperClass()
   static jmethodID methodClassGetInterfaces;                           // Class<?>[] Class.getInterfacess()
   static jmethodID methodClassGetDeclaredConstructors;                 // Constructor<?>[] Class.getDeclaredConstructors()
   static jmethodID methodClassGetModifiers;                            // int Class.getModifiers()
   static jmethodID methodClassIsPrimitive;                             // boolean Class.isPrimitive()
   static jmethodID methodClassGetDeclaredMethods;                      // Method[] Class.getDeclaredMethods()
   static jmethodID methodClassGetCanonicalName;                        // String Class.getCanonicalName()
   static jmethodID methodClassGetDeclaredField;                        // Field Class.getField()

   static GlobalReference<jclass> classThrowable;                       // java.lang.Throwable
   static jmethodID methodThrowableGetMessage;                          // String Throwable.getMessage()
   static jmethodID methodThrowableGetStackTrace;                       // String Throwable.getStackTrace()

   static GlobalReference<jclass> classStackTraceElement;               // java.lang.StackTraceElement
   static jmethodID methodStackTraceElementGetClassName;                // String StackTraceElement.getClassName()
   static jmethodID methodStackTraceElementGetFileName;                 // String StackTraceElement.getFileName()
   static jmethodID methodStackTraceElementGetLineNumber;               // int StackTraceElement.getLineNumber()
   static jmethodID methodStackTraceElementGetMethodName;               // String StackTraceElement.getMethodName()
   static jmethodID methodStackTraceElementIsNativeMethod;              // boolean StackTraceElement.isNativeMethod()

   static GlobalReference<jclass> classString;                          // java.lang.String

   static GlobalReference<jclass> classField;                           // java.lang.reflect.Field
   static jmethodID methodFieldGetType;                                 // Class<?> Field.getType()
   static jmethodID methodFieldGetDeclaringClass;                       // Class<?> Field.getDeclaringClass()
   static jmethodID methodFieldGetModifiers;                            // int Field.getModifiers()
   static jmethodID methodFieldGetName;                                 // String Field.getName()
   static jmethodID methodFieldGet;                                     // Object Field.get(Object)

   static GlobalReference<jclass> classMethod;                          // java.lang.reflect.Method
   static jmethodID methodMethodGetReturnType;                          // Class<?> Method.getReturnType()
   static jmethodID methodMethodGetParameterTypes;                      // Class<?>[] Method.getParameterTypes()
   static jmethodID methodMethodGetDeclaringClass;                      // Class<?> Method.getDeclaringClass()
   static jmethodID methodMethodGetModifiers;                           // int Method.getModifiers()
   static jmethodID methodMethodIsVarArgs;                              // bool Method.isVarArgs()
   static jmethodID methodMethodGetName;                                // String Method.getName()

   static GlobalReference<jclass> classConstructor;                     // java.lang.reflect.Constructor
   static jmethodID methodConstructorGetParameterTypes;                 // Class<?>[] Constructor.getParameterTypes()
   static jmethodID methodConstructorToString;                          // String Constructor.toString()
   static jmethodID methodConstructorGetModifiers;                      // int Constructor.getModifiers()
   static jmethodID methodConstructorIsVarArgs;                         // boolean Constructor.isVarArgs()

   static GlobalReference<jclass> classQoreInvocationHandler;           // org.qore.jni.QoreInvocationHandler
   static jmethodID ctorQoreInvocationHandler;                          // QoreInvocationHandler(long)
   static jmethodID methodQoreInvocationHandlerDestroy;                 // void QoreInvocationHandler.destroy()

   static GlobalReference<jclass> classQoreExceptionWrapper;            // org.qore.jni.QoreExceptionWrapper
   static jmethodID ctorQoreExceptionWrapper;                           // QoreExceptionWrapper(long)
   static jmethodID methodQoreExceptionWrapperGet;                      // long QoreExceptionWrapper.get()

   static GlobalReference<jclass> classProxy;                           // java.lang.reflect.Proxy
   static jmethodID methodProxyNewProxyInstance;                        // Object Proxy.newProxyInstance(ClassLoader, Class[], InvocationHandler)

   static GlobalReference<jclass> classClassLoader;                     // java.lang.ClassLoader
   static jmethodID methodClassLoaderLoadClass;                         // Class ClassLoader.loadClass(String)

   static GlobalReference<jclass> classQoreURLClassLoader;              // org.qore.jni.QoreURLClassLoader
   static jmethodID ctorQoreURLClassLoader;                             // QoreURLClassLoader()
   static jmethodID methodQoreURLClassLoaderAddPath;                    // void QoreURLClassLoader.addPath(String)
   static jmethodID methodQoreURLClassLoaderLoadClass;                  // Class QoreURLClassLoader.loadClass(String)

   static GlobalReference<jclass> classThread;                          // java.lang.Thread
   static jmethodID methodThreadCurrentThread;                          // Thread Thread.currentThread()
   static jmethodID methodThreadGetContextClassLoader;                  // ClassLoader Thread.getContextClassLoader()

   static GlobalReference<jclass> classBoolean;                         // java.lang.Boolean
   static jmethodID ctorBoolean;                                        // Boolean(boolean)
   static jmethodID methodBooleanBooleanValue;                          // boolean Boolean.booleanValue()

   static GlobalReference<jclass> classInteger;                         // java.lang.Integer
   static jmethodID ctorInteger;                                        // Integer(int)
   static jmethodID methodIntegerIntValue;                              // int Integer.intValue()

   static GlobalReference<jclass> classLong;                            // java.lang.Long
   static jmethodID ctorLong;                                           // Long(long)
   static jmethodID methodLongLongValue;                                // long Long.longValue()

   static GlobalReference<jclass> classShort;                           // java.lang.Short
   static jmethodID ctorShort;                                          // Short(short)
   static jmethodID methodShortShortValue;                              // short Short.shortValue()

   static GlobalReference<jclass> classByte;                            // java.lang.Byte
   static jmethodID ctorByte;                                           // Byte(byte)
   static jmethodID methodByteByteValue;                                // byte Byte.byteValue()

   static GlobalReference<jclass> classDouble;                          // java.lang.Double
   static jmethodID ctorDouble;                                         // Double(double)
   static jmethodID methodDoubleDoubleValue;                            // double Double.doubleValue()

   static GlobalReference<jclass> classFloat;                           // java.lang.Float
   static jmethodID ctorFloat;                                          // Float(float)
   static jmethodID methodFloatFloatValue;                              // float Float.floatValue()

   static GlobalReference<jclass> classCharacter;                       // java.lang.Character
   static jmethodID ctorCharacter;                                      // Character(char)
   static jmethodID methodCharacterCharValue;                           // char Character.charValue()

public:
   static void init();
   static void cleanup();
   static Type getType(jclass cls);
};

} // namespace jni

#endif // QORE_JNI_GLOBALS_H_
