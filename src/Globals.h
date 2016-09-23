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
#ifndef QORE_JNI_GLOBALS_H_
#define QORE_JNI_GLOBALS_H_

#include "GlobalReference.h"

namespace jni {

enum class Type {
   Void, Boolean, Byte, Char, Short, Int, Long, Float, Double, Reference
};

class Globals {

public:
   static GlobalReference<jclass> classVoid;                            // class for the void return type
   static GlobalReference<jclass> classBoolean;                         // class for the primitive type boolean
   static GlobalReference<jclass> classByte;                            // class for the primitive type byte
   static GlobalReference<jclass> classChar;                            // class for the primitive type char
   static GlobalReference<jclass> classShort;                           // class for the primitive type short
   static GlobalReference<jclass> classInt;                             // class for the primitive type int
   static GlobalReference<jclass> classLong;                            // class for the primitive type long
   static GlobalReference<jclass> classFloat;                           // class for the primitive type float
   static GlobalReference<jclass> classDouble;                          // class for the primitive type double

   static GlobalReference<jclass> classClass;                           // java.lang.Class
   static jmethodID methodClassIsArray;                                 // boolean Class.isArray();

   static GlobalReference<jclass> classField;                           // java.lang.reflect.Field
   static jmethodID methodFieldGetType;                                 // Class<?> Field.getType();

   static GlobalReference<jclass> classMethod;                          // java.lang.reflect.Method
   static jmethodID methodMethodGetReturnType;                          // Class<?> Method.getReturnType();
   static jmethodID methodMethodGetParameterTypes;                      // Class<?>[] Method.getParameterTypes();
   static jmethodID methodMethodGetDeclaringClass;                      // Class<?> Method.getDeclaringClass();
   static jmethodID methodMethodGetModifiers;                           // int Method.getModifiers();

   static GlobalReference<jclass> classInvocationHandlerImpl;           // qore.org.jni.InvocationHandlerImpl
   static jmethodID ctorInvocationHandlerImpl;                          // InvocationHandlerImpl(long)
   static jmethodID methodInvocationHandlerImplDeref;                   // void InvocationHandlerImpl.deref()

   static GlobalReference<jclass> classQoreExceptionWrapper;            // qore.org.jni.QoreExceptionWrapper
   static jmethodID ctorQoreExceptionWrapper;                           // QoreExceptionWrapper(long)
   static jmethodID methodQoreExceptionWrapperGet;                      // long QoreExceptionWrapper.get()

public:
   static void init();
   static void cleanup();
   static Type getType(jclass clazz);
};

} // namespace jni

#endif // QORE_JNI_GLOBALS_H_
