//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2016 - 2020 Qore Technologies, s.r.o.
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
#include "Env.h"

DLLLOCAL QoreStringNode* jni_module_init_intern();

namespace jni {

enum class Type {
    Void, Boolean, Byte, Char, Short, Int, Long, Float, Double, Reference
};

DLLLOCAL extern bool jni_qore_init;
DLLLOCAL void jni_delete_pgm(ExceptionSink& xsink);
DLLLOCAL QoreProgram* jni_get_create_program(Env& env);

class Env;

class Globals {
public:
    DLLLOCAL static GlobalReference<jobject> syscl;                               // base/system classloader

    DLLLOCAL static GlobalReference<jclass> classPrimitiveVoid;                   // class for the void return type
    DLLLOCAL static GlobalReference<jclass> classPrimitiveBoolean;                // class for the primitive type boolean
    DLLLOCAL static GlobalReference<jclass> classPrimitiveByte;                   // class for the primitive type byte
    DLLLOCAL static GlobalReference<jclass> classPrimitiveChar;                   // class for the primitive type char
    DLLLOCAL static GlobalReference<jclass> classPrimitiveShort;                  // class for the primitive type short
    DLLLOCAL static GlobalReference<jclass> classPrimitiveInt;                    // class for the primitive type int
    DLLLOCAL static GlobalReference<jclass> classPrimitiveLong;                   // class for the primitive type long
    DLLLOCAL static GlobalReference<jclass> classPrimitiveFloat;                  // class for the primitive type float
    DLLLOCAL static GlobalReference<jclass> classPrimitiveDouble;                 // class for the primitive type double

    DLLLOCAL static GlobalReference<jclass> classSystem;                          // java.lang.System
    DLLLOCAL static jmethodID methodSystemSetProperty;                            // String System.setProperty()
    DLLLOCAL static jmethodID methodSystemGetProperty;                            // String System.getProperty()

    DLLLOCAL static GlobalReference<jclass> classObject;                          // java.lang.Object
    DLLLOCAL static jmethodID methodObjectGetClass;                               // Class<?> Object.getClass()

    DLLLOCAL static GlobalReference<jclass> classClass;                           // java.lang.Class
    DLLLOCAL static jmethodID methodClassIsArray;                                 // boolean Class.isArray()
    DLLLOCAL static jmethodID methodClassGetComponentType;                        // Class<?> Class.getComponentType()
    DLLLOCAL static jmethodID methodClassGetClassLoader;                          // ClassLoader Class.getClassLoader()
    DLLLOCAL static jmethodID methodClassGetName;                                 // String Class.getName()
    DLLLOCAL static jmethodID methodClassGetDeclaredFields;                       // Field[] Class.getDeclaredFields()
    DLLLOCAL static jmethodID methodClassGetSuperClass;                           // Class<? super T> Class.getSuperClass()
    DLLLOCAL static jmethodID methodClassGetInterfaces;                           // Class<?>[] Class.getInterfacess()
    DLLLOCAL static jmethodID methodClassGetDeclaredConstructors;                 // Constructor<?>[] Class.getDeclaredConstructors()
    DLLLOCAL static jmethodID methodClassGetModifiers;                            // int Class.getModifiers()
    DLLLOCAL static jmethodID methodClassIsPrimitive;                             // boolean Class.isPrimitive()
    DLLLOCAL static jmethodID methodClassGetDeclaredMethods;                      // Method[] Class.getDeclaredMethods()
    DLLLOCAL static jmethodID methodClassGetCanonicalName;                        // String Class.getCanonicalName()
    DLLLOCAL static jmethodID methodClassGetDeclaredField;                        // Field Class.getField()

    DLLLOCAL static GlobalReference<jclass> classThrowable;                       // java.lang.Throwable
    DLLLOCAL static jmethodID methodThrowableGetMessage;                          // String Throwable.getMessage()
    DLLLOCAL static jmethodID methodThrowableGetStackTrace;                       // String Throwable.getStackTrace()

    DLLLOCAL static GlobalReference<jclass> classStackTraceElement;               // java.lang.StackTraceElement
    DLLLOCAL static jmethodID methodStackTraceElementGetClassName;                // String StackTraceElement.getClassName()
    DLLLOCAL static jmethodID methodStackTraceElementGetFileName;                 // String StackTraceElement.getFileName()
    DLLLOCAL static jmethodID methodStackTraceElementGetLineNumber;               // int StackTraceElement.getLineNumber()
    DLLLOCAL static jmethodID methodStackTraceElementGetMethodName;               // String StackTraceElement.getMethodName()
    DLLLOCAL static jmethodID methodStackTraceElementIsNativeMethod;              // boolean StackTraceElement.isNativeMethod()

    DLLLOCAL static GlobalReference<jclass> classString;                          // java.lang.String

    DLLLOCAL static GlobalReference<jclass> classField;                           // java.lang.reflect.Field
    DLLLOCAL static jmethodID methodFieldGetType;                                 // Class<?> Field.getType()
    DLLLOCAL static jmethodID methodFieldGetDeclaringClass;                       // Class<?> Field.getDeclaringClass()
    DLLLOCAL static jmethodID methodFieldGetModifiers;                            // int Field.getModifiers()
    DLLLOCAL static jmethodID methodFieldGetName;                                 // String Field.getName()
    DLLLOCAL static jmethodID methodFieldGet;                                     // Object Field.get(Object)
    DLLLOCAL static jmethodID methodFieldSetAccessible;                           // void Field.setAccessible(boolean)

    DLLLOCAL static GlobalReference<jclass> classMethod;                          // java.lang.reflect.Method
    DLLLOCAL static jmethodID methodMethodGetReturnType;                          // Class<?> Method.getReturnType()
    DLLLOCAL static jmethodID methodMethodGetParameterTypes;                      // Class<?>[] Method.getParameterTypes()
    DLLLOCAL static jmethodID methodMethodGetDeclaringClass;                      // Class<?> Method.getDeclaringClass()
    DLLLOCAL static jmethodID methodMethodGetModifiers;                           // int Method.getModifiers()
    DLLLOCAL static jmethodID methodMethodIsVarArgs;                              // bool Method.isVarArgs()
    DLLLOCAL static jmethodID methodMethodGetName;                                // String Method.getName()
    DLLLOCAL static jmethodID methodMethodToGenericString;                        // String Method.getName()

    DLLLOCAL static GlobalReference<jclass> classConstructor;                     // java.lang.reflect.Constructor
    DLLLOCAL static jmethodID methodConstructorGetParameterTypes;                 // Class<?>[] Constructor.getParameterTypes()
    DLLLOCAL static jmethodID methodConstructorToString;                          // String Constructor.toString()
    DLLLOCAL static jmethodID methodConstructorGetModifiers;                      // int Constructor.getModifiers()
    DLLLOCAL static jmethodID methodConstructorIsVarArgs;                         // boolean Constructor.isVarArgs()

    DLLLOCAL static GlobalReference<jclass> classQoreInvocationHandler;           // org.qore.jni.QoreInvocationHandler
    DLLLOCAL static jmethodID ctorQoreInvocationHandler;                          // QoreInvocationHandler(long)
    DLLLOCAL static jmethodID methodQoreInvocationHandlerDestroy;                 // void QoreInvocationHandler.destroy()

    DLLLOCAL static GlobalReference<jclass> classQoreJavaApi;                     // org.qore.jni.QoreJavaApi
    DLLLOCAL static jmethodID methodQoreJavaApiGetStackTrace;                     // StackTraceElement[] getStackTrace()

    DLLLOCAL static GlobalReference<jclass> classQoreExceptionWrapper;            // org.qore.jni.QoreExceptionWrapper
    DLLLOCAL static jmethodID ctorQoreExceptionWrapper;                           // QoreExceptionWrapper(long)
    DLLLOCAL static jmethodID methodQoreExceptionWrapperGet;                      // long QoreExceptionWrapper.get()

    DLLLOCAL static GlobalReference<jclass> classQoreException;                   // org.qore.jni.QoreException
    DLLLOCAL static jmethodID methodQoreExceptionGetErr;                          // String QoreException.getErr()
    DLLLOCAL static jmethodID methodQoreExceptionGetDesc;                         // String QoreException.getDesc()
    DLLLOCAL static jmethodID methodQoreExceptionGetArg;                          // String QoreException.getArg()

    DLLLOCAL static GlobalReference<jclass> classQoreObjectBase;                  // org.qore.jni.QoreObjectBase

    DLLLOCAL static GlobalReference<jclass> classQoreObject;                      // org.qore.jni.QoreObject
    DLLLOCAL static jmethodID ctorQoreObject;                                     // QoreObject(long)
    DLLLOCAL static jmethodID methodQoreObjectGet;                                // long QoreObject.get()

    DLLLOCAL static GlobalReference<jclass> classQoreClosure;                     // org.qore.jni.QoreClosure
    DLLLOCAL static jmethodID ctorQoreClosure;                                    // QoreClosure(long)
    DLLLOCAL static jmethodID methodQoreClosureGet;                               // long QoreClosure.get()

    DLLLOCAL static GlobalReference<jclass> classQoreObjectWrapper;               // org.qore.jni.QoreObjectWrapper

    DLLLOCAL static GlobalReference<jclass> classQoreClosureMarker;               // org.qore.jni.QoreClosureMarker

    DLLLOCAL static GlobalReference<jclass> classProxy;                           // java.lang.reflect.Proxy
    DLLLOCAL static jmethodID methodProxyNewProxyInstance;                        // Object Proxy.newProxyInstance(ClassLoader, Class[], InvocationHandler)

    DLLLOCAL static GlobalReference<jclass> classClassLoader;                     // java.lang.ClassLoader
    DLLLOCAL static jmethodID methodClassLoaderLoadClass;                         // Class ClassLoader.loadClass(String)

    DLLLOCAL static GlobalReference<jclass> classQoreURLClassLoader;              // org.qore.jni.QoreURLClassLoader
    DLLLOCAL static jmethodID ctorQoreURLClassLoader;                             // QoreURLClassLoader(long)
    DLLLOCAL static jmethodID methodQoreURLClassLoaderAddPath;                    // void QoreURLClassLoader.addPath(String)
    DLLLOCAL static jmethodID methodQoreURLClassLoaderLoadClass;                  // Class QoreURLClassLoader.loadClass(String)
    DLLLOCAL static jmethodID methodQoreURLClassLoaderSetContext;                 // void QoreURLClassLoader.setContext()
    DLLLOCAL static jmethodID methodQoreURLClassLoaderGetProgramPtr;              // long QoreURLClassLoader.getProgramPtr()
    DLLLOCAL static jmethodID methodQoreURLClassLoaderAddPendingClass;            // void QoreURLClassLoader.addPendingClass(String, byte[])
    DLLLOCAL static jmethodID methodQoreURLClassLoaderDefineResolveClass;         // Class<?> QoreURLClassLoader.defineResolveClass​(String, byte[], int, int)
    DLLLOCAL static jmethodID methodQoreURLClassLoaderGetResolveClass;            // Class<?> QoreURLClassLoader.getResolveClass(String)
    DLLLOCAL static jmethodID methodQoreURLClassLoaderClearCache;                 // void QoreURLClassLoader.clearCache()
    DLLLOCAL static jmethodID methodQoreURLClassLoaderDefineClassUnconditional;   // Class<?> QoreURLClassLoader.defineClassUnconditional(String, byte[])

    DLLLOCAL static GlobalReference<jclass> classJavaClassBuilder;                // org.qore.jni.JavaClassBuilder
    DLLLOCAL static jmethodID methodJavaClassBuilderGetClassBuilder;              // static DynamicType.Builder<?> getClassBuilder(String, Class<?>, boolean, long)
    DLLLOCAL static jmethodID methodJavaClassBuilderAddConstructor;               // static DynamicType.Builder<?> addConstructor(DynamicType.Builder<?>, Class<?>, int, List<Type>)
    DLLLOCAL static jmethodID methodJavaClassBuilderAddNormalMethod;              // static DynamicType.Builder<?> methodJavaClassBuilderAddNormalMethod(DynamicType.Builder<?>, String, int, Class<?>, List<Type>)
    DLLLOCAL static jmethodID methodJavaClassBuilderAddStaticMethod;              // static DynamicType.Builder<?> methodJavaClassBuilderAddStaticMethod(DynamicType.Builder<?>, String, int, Class<?>, List<Type>)
    DLLLOCAL static jmethodID methodJavaClassBuilderGetClassFromBuilder;          // static Class<?> getClassFromBuilder(DynamicType.Builder<?>, ClassLoader)

    DLLLOCAL static GlobalReference<jclass> classThread;                          // java.lang.Thread
    DLLLOCAL static jmethodID methodThreadCurrentThread;                          // Thread Thread.currentThread()
    DLLLOCAL static jmethodID methodThreadGetContextClassLoader;                  // ClassLoader Thread.getContextClassLoader()

    DLLLOCAL static GlobalReference<jclass> classHashMap;                         // java.util.HashMap
    DLLLOCAL static GlobalReference<jclass> classHash;                            // java.util.LinkedHashMap
    DLLLOCAL static jmethodID ctorHash;                                           // Hash()
    DLLLOCAL static jmethodID methodHashPut;                                      // Object Hash.put(Object K, Object V)

    //DLLLOCAL static GlobalReference<jclass> classLinkedHashMap;                   // java.util.LinkedHashMap
    //DLLLOCAL static jmethodID ctorLinkedHashMap;                                  // LinkedHashMap()
    //DLLLOCAL static jmethodID methodLinkedHashMapPut;                             // Object LinkedHashMap.put(Object K, Object V)

    DLLLOCAL static GlobalReference<jclass> classMap;                             // java.util.Map
    DLLLOCAL static jmethodID methodMapEntrySet;                                  // Set<Map.Entry<K,V>> Map.entrySet()

    DLLLOCAL static GlobalReference<jclass> classList;                            // java.util.List
    DLLLOCAL static jmethodID methodListSize;                                     // int List.size()
    DLLLOCAL static jmethodID methodListGet;                                      // Object List.get(int index)

    DLLLOCAL static GlobalReference<jclass> classSet;                             // java.util.Set
    DLLLOCAL static jmethodID methodSetIterator;                                  // Set.iterator()

    DLLLOCAL static GlobalReference<jclass> classEntry;                           // java.util.Map.Entry
    DLLLOCAL static jmethodID methodEntryGetKey;                                  // Map.Entry.getKey()
    DLLLOCAL static jmethodID methodEntryGetValue;                                // Map.Entry.getValue()

    DLLLOCAL static GlobalReference<jclass> classIterator;                        // java.util.Iterator
    DLLLOCAL static jmethodID methodIteratorHasNext;                              // Iterator.hasNext()
    DLLLOCAL static jmethodID methodIteratorNext;                                 // Iterator.next()

    DLLLOCAL static GlobalReference<jclass> classZonedDateTime;                   // java.time.ZonedDateTime
    DLLLOCAL static jmethodID methodZonedDateTimeParse;                           // ZonedDateTime.parse()
    DLLLOCAL static jmethodID methodZonedDateTimeToString;                        // ZonedDateTime.toString()

    DLLLOCAL static GlobalReference<jclass> classQoreRelativeTime;                // org.qore.jni.QoreRelativeTime
    DLLLOCAL static jmethodID ctorQoreRelativeTime;                               // QoreRelativeTime(int, int, int, int, int, int, int)
    DLLLOCAL static jfieldID fieldQoreRelativeTimeYear;                           // QoreRelativeTime.year
    DLLLOCAL static jfieldID fieldQoreRelativeTimeMonth;                          // QoreRelativeTime.month
    DLLLOCAL static jfieldID fieldQoreRelativeTimeDay;                            // QoreRelativeTime.day
    DLLLOCAL static jfieldID fieldQoreRelativeTimeHour;                           // QoreRelativeTime.hour
    DLLLOCAL static jfieldID fieldQoreRelativeTimeMinute;                         // QoreRelativeTime.minute
    DLLLOCAL static jfieldID fieldQoreRelativeTimeSecond;                         // QoreRelativeTime.second
    DLLLOCAL static jfieldID fieldQoreRelativeTimeUs;                             // QoreRelativeTime.us

    DLLLOCAL static GlobalReference<jclass> classBigDecimal;                      // java.math.BigDecimal
    DLLLOCAL static jmethodID ctorBigDecimal;                                     // BigDecimal(String)
    DLLLOCAL static jmethodID methodBigDecimalToString;                           // BigDecimal.toString()

    DLLLOCAL static GlobalReference<jclass> classArrays;                          // java.util.Arrays
    DLLLOCAL static jmethodID methodArraysToString;                               // Arrays.toString()
    DLLLOCAL static jmethodID methodArraysDeepToString;                           // Arrays.deepToString()

    DLLLOCAL static GlobalReference<jclass> classBoolean;                         // java.lang.Boolean
    DLLLOCAL static jmethodID ctorBoolean;                                        // Boolean(boolean)
    DLLLOCAL static jmethodID methodBooleanBooleanValue;                          // boolean Boolean.booleanValue()

    DLLLOCAL static GlobalReference<jclass> classInteger;                         // java.lang.Integer
    DLLLOCAL static jmethodID ctorInteger;                                        // Integer(int)
    DLLLOCAL static jmethodID methodIntegerIntValue;                              // int Integer.intValue()

    DLLLOCAL static GlobalReference<jclass> classLong;                            // java.lang.Long
    DLLLOCAL static jmethodID ctorLong;                                           // Long(long)
    DLLLOCAL static jmethodID methodLongLongValue;                                // long Long.longValue()

    DLLLOCAL static GlobalReference<jclass> classShort;                           // java.lang.Short
    DLLLOCAL static jmethodID ctorShort;                                          // Short(short)
    DLLLOCAL static jmethodID methodShortShortValue;                              // short Short.shortValue()

    DLLLOCAL static GlobalReference<jclass> classByte;                            // java.lang.Byte
    DLLLOCAL static jmethodID ctorByte;                                           // Byte(byte)
    DLLLOCAL static jmethodID methodByteByteValue;                                // byte Byte.byteValue()

    DLLLOCAL static GlobalReference<jclass> classDouble;                          // java.lang.Double
    DLLLOCAL static jmethodID ctorDouble;                                         // Double(double)
    DLLLOCAL static jmethodID methodDoubleDoubleValue;                            // double Double.doubleValue()

    DLLLOCAL static GlobalReference<jclass> classFloat;                           // java.lang.Float
    DLLLOCAL static jmethodID ctorFloat;                                          // Float(float)
    DLLLOCAL static jmethodID methodFloatFloatValue;                              // float Float.floatValue()

    DLLLOCAL static GlobalReference<jclass> classCharacter;                       // java.lang.Character
    DLLLOCAL static jmethodID ctorCharacter;                                      // Character(char)
    DLLLOCAL static jmethodID methodCharacterCharValue;                           // char Character.charValue()

    DLLLOCAL static void init();
    DLLLOCAL static void cleanup();
    DLLLOCAL static Type getType(jclass cls);

    DLLLOCAL static void setAlreadyInitialized() {
        assert(!already_initialized);
        already_initialized = true;
    }

    DLLLOCAL static bool getAlreadyInitialized() {
        return already_initialized;
    }

    // if already initialized, first tries to find the class, and then defines it only if not found, otherwise
    // defines the class
    DLLLOCAL static LocalReference<jclass> findDefineClass(Env& env, const char* name, jobject loader, const unsigned char* buf,
        jsize bufLen);

private:
    DLLLOCAL static bool already_initialized;
};

class QoreJniStackLocationHelper : public QoreExternalRuntimeStackLocationHelper {
public:
    DLLLOCAL QoreJniStackLocationHelper();

    //! returns the name of the function or method call
    DLLLOCAL virtual const std::string& getCallName() const;

    //! returns the call type
    DLLLOCAL virtual qore_call_t getCallType() const;

    //! returns the source location of the element
    DLLLOCAL virtual const QoreProgramLocation& getLocation() const;

    //! returns the next location in the stack or nullptr if there is none
    DLLLOCAL virtual const QoreStackLocation* getNext() const;

private:
    int tid = q_gettid();
    mutable jsize current = 0;

    mutable std::vector<std::string> stack_call;
    mutable std::vector<bool> stack_native;
    mutable std::vector<QoreExternalProgramLocationWrapper> stack_loc;

    mutable bool init = false;

    DLLLOCAL static std::string jni_no_call_name;
    DLLLOCAL static QoreExternalProgramLocationWrapper jni_loc_builtin;

    DLLLOCAL size_t size() const {
        return stack_call.size();
    }

    DLLLOCAL void checkInit() const;
};

} // namespace jni

#endif // QORE_JNI_GLOBALS_H_
