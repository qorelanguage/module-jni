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
jmethodID Globals::methodQoreJavaApiGetStackTrace;

GlobalReference<jclass> Globals::classQoreExceptionWrapper;
jmethodID Globals::ctorQoreExceptionWrapper;
jmethodID Globals::methodQoreExceptionWrapperGet;

GlobalReference<jclass> Globals::classQoreObject;
jmethodID Globals::ctorQoreObject;
jmethodID Globals::methodQoreObjectGet;

GlobalReference<jclass> Globals::classQoreObjectWrapper;

GlobalReference<jclass> Globals::classQoreClosureMarker;

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
jmethodID Globals::methodQoreURLClassLoaderAddPendingClass;

GlobalReference<jclass> Globals::classThread;
jmethodID Globals::methodThreadCurrentThread;
jmethodID Globals::methodThreadGetContextClassLoader;

GlobalReference<jclass> Globals::classHashMap;
jmethodID Globals::ctorHashMap;
jmethodID Globals::methodHashMapPut;
jmethodID Globals::methodHashMapEntrySet;

GlobalReference<jclass> Globals::classSet;
jmethodID Globals::methodSetIterator;

GlobalReference<jclass> Globals::classEntry;
jmethodID Globals::methodEntryGetKey;
jmethodID Globals::methodEntryGetValue;

GlobalReference<jclass> Globals::classIterator;
jmethodID Globals::methodIteratorHasNext;
jmethodID Globals::methodIteratorNext;

GlobalReference<jclass> Globals::classZonedDateTime;
jmethodID Globals::methodZonedDateTimeParse;
jmethodID Globals::methodZonedDateTimeToString;

GlobalReference<jclass> Globals::classQoreRelativeTime;
jmethodID Globals::ctorQoreRelativeTime;
jfieldID Globals::fieldQoreRelativeTimeYear;
jfieldID Globals::fieldQoreRelativeTimeMonth;
jfieldID Globals::fieldQoreRelativeTimeDay;
jfieldID Globals::fieldQoreRelativeTimeHour;
jfieldID Globals::fieldQoreRelativeTimeMinute;
jfieldID Globals::fieldQoreRelativeTimeSecond;
jfieldID Globals::fieldQoreRelativeTimeUs;

GlobalReference<jclass> Globals::classBigDecimal;
jmethodID Globals::ctorBigDecimal;
jmethodID Globals::methodBigDecimalToString;

GlobalReference<jclass> Globals::classArrays;
jmethodID Globals::methodArraysToString;
jmethodID Globals::methodArraysDeepToString;

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

std::string QoreJniStackLocationHelper::jni_no_call_name = "<jni_module_java_no_runtime_stack_info>";
QoreExternalProgramLocationWrapper QoreJniStackLocationHelper::jni_loc_builtin("<jni_module_unknown>", -1, -1);

static void JNICALL invocation_handler_finalize(JNIEnv *, jclass, jlong ptr) {
   delete reinterpret_cast<Dispatcher*>(ptr);
}

static jobject JNICALL invocation_handler_invoke(JNIEnv* jenv, jobject, jlong ptr, jobject proxy, jobject method, jobjectArray args) {
   Env env(jenv);
   Dispatcher* dispatcher = reinterpret_cast<Dispatcher*>(ptr);
   return dispatcher->dispatch(env, proxy, method, args);
}

static int save_object(Env& env, const QoreValue& rv, QoreProgram* pgm, ExceptionSink& xsink) {
    // save object in thread-local data if relevant
    if (rv.getType() != NT_OBJECT) {
        return 0;
    }

    QoreHashNode* data = pgm->getThreadData();
    assert(data);
    const char* domain_name;
    // get key name where to save the data if possible
    QoreValue v = data->getKeyValue("_jni_save");
    if (v.getType() != NT_STRING) {
        domain_name = "_jni_save";
    } else {
        domain_name = v.get<const QoreStringNode>()->c_str();
    }

    QoreValue kv = data->getKeyValue(domain_name);
    // ignore operation if domain exists but is not a list
    if (!kv || kv.getType() == NT_LIST) {
        QoreListNode* list;
        ReferenceHolder<QoreListNode> list_holder(&xsink);
        if (!kv) {
            // we need to assign list in data *after* we prepend the object to the list
            // in order to manage object counts
            list = new QoreListNode(autoTypeInfo);
            list_holder = list;
        } else {
            list = kv.get<QoreListNode>();
        }

        // prepend to list to ensure FILO destruction order
        list->splice(0, 0, rv, &xsink);
        if (!xsink && list_holder) {
             data->setKeyValue(domain_name, list_holder.release(), &xsink);
        }
        if (xsink) {
            QoreToJava::wrapException(xsink);
            return -1;
        }
        //printd(5, "save_object() domain: '%s' obj: %p %s\n", domain_name, rv.get<QoreObject>(), rv.get<QoreObject>()->getClassName());
    } else {
        //printd(5, "save_object() NOT SAVING domain: '%s' HAS KEY v: %s (kv: %s)\n", domain_name, rv.getFullTypeName(), kv.getFullTypeName());
    }
    return 0;
}

static jobject java_api_call_function_internal(JNIEnv* jenv, jobject obj, jlong ptr, jboolean save, jstring name, jobjectArray args) {
    Env env(jenv);
    QoreThreadAttachHelper attach_helper;
    try {
        attach_helper.attach();
    } catch (Exception& e) {
        env.throwNew(env.findClass("java/lang/RuntimeException"), "Unable to attach thread to Qore");
        return nullptr;
    }
    QoreProgram* pgm = reinterpret_cast<QoreProgram*>(ptr);

    QoreProgramContextHelper pch(pgm);

    ExceptionSink xsink;

    jsize len = args ? env.getArrayLength(args) : 0;
    ReferenceHolder<QoreListNode> qore_args(&xsink);

    if (len) {
        Array::getArgList(qore_args, env, args);
    }

    Env::GetStringUtfChars fname(env, name);
    //printd(LogLevel, "java_api_call_function() '%s()' args: %p %d\n", fname.c_str(), *qore_args, len);

    QoreJniStackLocationHelper slh;

    ValueHolder rv(pgm->callFunction(fname.c_str(), *qore_args, &xsink), &xsink);

    if (xsink) {
        QoreToJava::wrapException(xsink);
        return nullptr;
    }

    if (save && save_object(env, *rv, pgm, xsink)) {
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

static jobject JNICALL java_api_call_function(JNIEnv* jenv, jobject obj, jlong ptr, jstring name,
    jobjectArray args) {
    return java_api_call_function_internal(jenv, obj, ptr, false, name, args);
}

static jobject JNICALL java_api_call_function_save(JNIEnv* jenv, jobject obj, jlong ptr, jstring name,
    jobjectArray args) {
    return java_api_call_function_internal(jenv, obj, ptr, true, name, args);
}

static jobject java_api_call_static_method_internal(JNIEnv* jenv, jobject obj, jlong ptr, jboolean save,
    jstring class_name, jstring method_name, jobjectArray args) {
    Env env(jenv);
    QoreThreadAttachHelper attach_helper;
    try {
        attach_helper.attach();
    } catch (Exception& e) {
        env.throwNew(env.findClass("java/lang/RuntimeException"), "Unable to attach thread to Qore");
        return nullptr;
    }
    QoreProgram* pgm = reinterpret_cast<QoreProgram*>(ptr);

    QoreJniStackLocationHelper slh;

    ExceptionSink xsink;
    // grab the current Program's parse lock before calling QoreProgram::findClass()
    QoreExternalProgramContextHelper epch(&xsink, pgm);
    if (xsink) {
        QoreToJava::wrapException(xsink);
        return nullptr;
    }

    jsize len = args ? env.getArrayLength(args) : 0;
    ReferenceHolder<QoreListNode> qore_args(&xsink);

    if (len) {
        Array::getArgList(qore_args, env, args);
    }

    Env::GetStringUtfChars cname(env, class_name);
    Env::GetStringUtfChars mname(env, method_name);
    //printd(LogLevel, "java_api_call_function() '%s()' args: %p %d\n", fname.c_str(), *qore_args, len);

    const QoreClass* cls = pgm->findClass(cname.c_str(), &xsink);
    if (!cls) {
        if (!xsink) {
            xsink.raiseException("UNKNOWN-CLASS", "cannot resolve class '%s'", cname.c_str());
        }
        QoreToJava::wrapException(xsink);
        return nullptr;
    }

    const QoreMethod* m = cls->findLocalStaticMethod(mname.c_str());
    if (!m) {
        xsink.raiseException("UNKNOWN-METHOD", "cannot resolve static method '%s::%s()'", cname.c_str(),
            mname.c_str());
        QoreToJava::wrapException(xsink);
        return nullptr;
    }

    ValueHolder rv(QoreObject::evalStaticMethod(*m, m->getClass(), *qore_args, &xsink), &xsink);
    if (xsink) {
        QoreToJava::wrapException(xsink);
        return nullptr;
    }

    if (save && save_object(env, *rv, pgm, xsink)) {
        return nullptr;
    }

    try {
        return QoreToJava::toAnyObject(*rv);
    } catch (jni::Exception& e) {
        e.convert(&xsink);
        QoreToJava::wrapException(xsink);
        return nullptr;
    }
}

static jobject JNICALL java_api_call_static_method(JNIEnv* jenv, jobject obj, jlong ptr, jstring class_name,
    jstring method_name, jobjectArray args) {
    return java_api_call_static_method_internal(jenv, obj, ptr, false, class_name, method_name, args);
}

static jobject JNICALL java_api_call_static_method_save(JNIEnv* jenv, jobject obj, jlong ptr, jstring class_name,
    jstring method_name, jobjectArray args) {
    return java_api_call_static_method_internal(jenv, obj, ptr, true, class_name, method_name, args);
}

// private native static QoreObject newObjectSave0(long pgm_ptr, String class_name, Object...args);
static jobject JNICALL java_api_new_object_save(JNIEnv* jenv, jobject obj, jlong ptr, jstring cname, jobjectArray args) {
    assert(ptr);
    QoreProgram* pgm = reinterpret_cast<QoreProgram*>(ptr);
    Env env(jenv);
    QoreThreadAttachHelper attach_helper;
    try {
        attach_helper.attach();
    } catch (Exception& e) {
        env.throwNew(env.findClass("java/lang/RuntimeException"), "Unable to attach thread to Qore");
        return nullptr;
    }

    QoreProgramContextHelper pch(pgm);

    QoreJniStackLocationHelper slh;

    ExceptionSink xsink;

    jsize len = args ? env.getArrayLength(args) : 0;
    ReferenceHolder<QoreListNode> qore_args(&xsink);

    if (len) {
        Array::getArgList(qore_args, env, args);
    }

    Env::GetStringUtfChars clsname(env, cname);
    //printd(LogLevel, "java_api_new_object() class '%s' args: %p %d\n", clsname.c_str(), *qore_args, len);

    const QoreClass* cls = pgm->findClass(clsname.c_str(), &xsink);
    if (cls && !xsink) {
        if (pgm->getParseOptions64() & cls->getDomain()) {
            xsink.raiseException("CREATE-OBJECT-ERROR", "Program sandboxing restrictions do not allow access to the '%s' class", cls->getName());
        } else {
            cls->runtimeCheckInstantiateClass(&xsink);
        }
    }

    if (!cls && !xsink) {
        xsink.raiseException("CREATE-OBJECT-ERROR", "class '%s' cannot be found", clsname.c_str());
    }

    if (xsink) {
        QoreToJava::wrapException(xsink);
        return nullptr;
    }

    ValueHolder rv(cls->execConstructor(*qore_args, &xsink), &xsink);
    if (xsink) {
        QoreToJava::wrapException(xsink);
        return nullptr;
    }

    if (save_object(env, *rv, pgm, xsink)) {
        return nullptr;
    }

    try {
        return QoreToJava::toAnyObject(*rv);
    } catch (jni::Exception& e) {
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

static jstring JNICALL qore_exception_wrapper_get_message(JNIEnv* jenv, jclass, jlong ptr) {
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

    Env env(jenv);
    ModifiedUtf8String str(jstr);
    return env.newString(str.c_str()).release();
}

static jstring JNICALL qore_object_class_name(JNIEnv* jenv, jclass, jlong ptr) {
    assert(ptr);
    QoreObject* obj = reinterpret_cast<QoreObject*>(ptr);

    Env env(jenv);
    return env.newString(obj->getClassName()).release();
}

static jboolean JNICALL qore_object_instance_of(JNIEnv* jenv, jclass, jlong ptr, jstring cname) {
    assert(ptr);
    QoreObject* obj = reinterpret_cast<QoreObject*>(ptr);

    Env env(jenv);
    Env::GetStringUtfChars class_name(env, cname);

    ExceptionSink xsink;
    const QoreClass* cls = getProgram()->findClass(class_name.c_str(), &xsink);
    //printd(5, "qore_object_instance_of() cls: %p (%s) xsink: %d\n", cls, class_name.c_str(), (bool)xsink);
    if (!cls) {
        xsink.clear();
        return false;
    }
    assert(!xsink);
    return obj->validInstanceOf(*cls);
}

static jobject qore_object_call_method_internal(JNIEnv* jenv, jclass, jlong pgm_ptr, jlong obj_ptr, jboolean save, jstring mname, jobjectArray args) {
    assert(pgm_ptr);
    assert(obj_ptr);
    QoreProgram* pgm = reinterpret_cast<QoreProgram*>(pgm_ptr);
    QoreObject* obj = reinterpret_cast<QoreObject*>(obj_ptr);
    // must ensure that the thread is attached before executing Qore code
    Env env(jenv);
    QoreThreadAttachHelper attach_helper;
    try {
        attach_helper.attach();
    } catch (Exception& e) {
        env.throwNew(env.findClass("java/lang/RuntimeException"), "Unable to attach thread to Qore");
        return nullptr;
    }

    ExceptionSink xsink;
    try {
        // set program context before converting arguments
        QoreExternalProgramContextHelper pch(&xsink, pgm);
        if (xsink) {
            throw XsinkException(xsink);
        }

        jsize len = args ? env.getArrayLength(args) : 0;
        ReferenceHolder<QoreListNode> qore_args(&xsink);

        if (len) {
            Array::getArgList(qore_args, env, args);
        }

        Env::GetStringUtfChars method_name(env, mname);

        ValueHolder val(obj->evalMethod(method_name.c_str(), *qore_args, &xsink), &xsink);
        if (xsink) {
            throw XsinkException(xsink);
        }

        //printd(5, "qore_object_call_method_internal() method: '%s::%s()' rv: %s\n", obj->getClassName(), method_name.c_str(), val->getFullTypeName());

        if (save && save_object(env, *val, getProgram(), xsink)) {
            return nullptr;
        }

        return QoreToJava::toAnyObject(*val);
    } catch (jni::Exception& e) {
        e.convert(&xsink);
        QoreToJava::wrapException(xsink);
    } catch (const std::bad_alloc& e) {
        // translate OOM C++ exception to a Java exception
        env.throwNew(env.findClass("java/lang/OutOfMemoryError"), e.what());
    } catch (const std::exception& e) {
        // translate unknown C++ exceptions to a Java exception
        env.throwNew(env.findClass("java/lang/Error"), e.what());
    } catch (...) {
        // translate unknown C++ exception to a Java exception
        env.throwNew(env.findClass("java/lang/Error"), "Unknown exception type");
    }
    return nullptr;
}

static jobject JNICALL qore_object_call_method(JNIEnv* jenv, jclass jcls, jlong pgm_ptr, jlong obj_ptr, jstring mname, jobjectArray args) {
    return qore_object_call_method_internal(jenv, jcls, pgm_ptr, obj_ptr, false, mname, args);
}

static jobject JNICALL qore_object_call_method_save(JNIEnv* jenv, jclass jcls, jlong pgm_ptr, jlong obj_ptr, jstring mname, jobjectArray args) {
    return qore_object_call_method_internal(jenv, jcls, pgm_ptr, obj_ptr, true, mname, args);
}

static jobject JNICALL qore_object_get_member_value(JNIEnv* jenv, jclass, jlong ptr, jstring mname) {
    assert(ptr);
    QoreObject* obj = reinterpret_cast<QoreObject*>(ptr);
    // must ensure that the thread is attached before calling QoreObject::getReferencedMemberNoMethod()
    Env env(jenv);
    QoreThreadAttachHelper attach_helper;
    try {
        attach_helper.attach();
    } catch (Exception& e) {
        env.throwNew(env.findClass("java/lang/RuntimeException"), "Unable to attach thread to Qore");
        return nullptr;
    }

    Env::GetStringUtfChars member_name(env, mname);

    ExceptionSink xsink;
    try {
        ValueHolder val(obj->getReferencedMemberNoMethod(member_name.c_str(), &xsink), &xsink);
        if (xsink) {
            throw XsinkException(xsink);
        }

        return QoreToJava::toAnyObject(*val);
    } catch (jni::Exception& e) {
        e.convert(&xsink);
        QoreToJava::wrapException(xsink);
    } catch (const std::bad_alloc& e) {
        // translate OOM C++ exception to a Java exception
        env.throwNew(env.findClass("java/lang/OutOfMemoryError"), e.what());
    } catch (const std::exception& e) {
        // translate unknown C++ exceptions to a Java exception
        env.throwNew(env.findClass("java/lang/Error"), e.what());
    } catch (...) {
        // translate unknown C++ exception to a Java exception
        env.throwNew(env.findClass("java/lang/Error"), "Unknown exception type");
    }
    return nullptr;
}

static void JNICALL qore_object_release(JNIEnv*, jclass, jlong ptr) {
    assert(ptr);
    reinterpret_cast<QoreObject*>(ptr)->tDeref();
}

static void JNICALL qore_object_destroy(JNIEnv* jenv, jclass, jlong ptr) {
    assert(ptr);
    // must ensure that the thread is attached before executing Qore code
    Env env(jenv);
    QoreThreadAttachHelper attach_helper;
    try {
        attach_helper.attach();
    } catch (Exception& e) {
        // NOTE: this results in a memory leak
        env.throwNew(env.findClass("java/lang/RuntimeException"), "Unable to attach thread to Qore");
        return;
    }

    ExceptionSink xsink;
    try {
        reinterpret_cast<QoreObject*>(ptr)->doDelete(&xsink);
        reinterpret_cast<QoreObject*>(ptr)->tDeref();
        if (xsink) {
            throw XsinkException(xsink);
        }
    } catch (jni::Exception& e) {
        e.convert(&xsink);
        QoreToJava::wrapException(xsink);
    } catch (const std::bad_alloc& e) {
        // translate OOM C++ exception to a Java exception
        env.throwNew(env.findClass("java/lang/OutOfMemoryError"), e.what());
    } catch (const std::exception& e) {
        // translate unknown C++ exceptions to a Java exception
        env.throwNew(env.findClass("java/lang/Error"), e.what());
    } catch (...) {
        // translate unknown C++ exception to a Java exception
        env.throwNew(env.findClass("java/lang/Error"), "Unknown exception type");
    }
}

static void JNICALL qore_object_finalize(JNIEnv*, jclass, jlong ptr) {
    assert(ptr);
    reinterpret_cast<QoreObject*>(ptr)->tDeref();
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

static JNINativeMethod qoreJavaApiNativeMethods[] = {
    {
        const_cast<char*>("callFunction0"),
        const_cast<char*>("(JLjava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;"),
        reinterpret_cast<void*>(java_api_call_function)
    },
    {
        const_cast<char*>("callFunctionSave0"),
        const_cast<char*>("(JLjava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;"),
        reinterpret_cast<void*>(java_api_call_function_save)
    },
    {
        const_cast<char*>("callStaticMethod0"),
        const_cast<char*>("(JLjava/lang/String;Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;"),
        reinterpret_cast<void*>(java_api_call_static_method)
    },
    {
        const_cast<char*>("callStaticMethodSave0"),
        const_cast<char*>("(JLjava/lang/String;Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;"),
        reinterpret_cast<void*>(java_api_call_static_method_save)
    },
    {
        // private native static QoreObject newObjectSave0(long pgm_ptr, String key, String class_name, Object...args);
        const_cast<char*>("newObjectSave0"),
        const_cast<char*>("(JLjava/lang/String;[Ljava/lang/Object;)Lorg/qore/jni/QoreObject;"),
        reinterpret_cast<void*>(java_api_new_object_save)
    },
};

static const size_t num_qore_java_api_native_methods = sizeof(qoreJavaApiNativeMethods) / sizeof(JNINativeMethod);

static JNINativeMethod qoreExceptionWrapperNativeMethods[] = {
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

static JNINativeMethod qoreObjectNativeMethods[] = {
    {
        const_cast<char*>("className0"),
        const_cast<char*>("(J)Ljava/lang/String;"),
        reinterpret_cast<void*>(qore_object_class_name)
    },
    {
        const_cast<char*>("instanceOf0"),
        const_cast<char*>("(JLjava/lang/String;)Z"),
        reinterpret_cast<void*>(qore_object_instance_of)
    },
    {
        const_cast<char*>("callMethod0"),
        const_cast<char*>("(JJLjava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;"),
        reinterpret_cast<void*>(qore_object_call_method)
    },
    {
        const_cast<char*>("callMethodSave0"),
        const_cast<char*>("(JJLjava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;"),
        reinterpret_cast<void*>(qore_object_call_method_save)
    },
    {
        const_cast<char*>("getMemberValue0"),
        const_cast<char*>("(JLjava/lang/String;)Ljava/lang/Object;"),
        reinterpret_cast<void*>(qore_object_get_member_value)
    },
    {
        const_cast<char*>("release0"),
        const_cast<char*>("(J)V"),
        reinterpret_cast<void*>(qore_object_release)
    },
    {
        const_cast<char*>("destroy0"),
        const_cast<char*>("(J)V"),
        reinterpret_cast<void*>(qore_object_destroy)
    },
    {
        const_cast<char*>("finalize0"),
        const_cast<char*>("(J)V"),
        reinterpret_cast<void*>(qore_object_finalize)
    },
};

static const size_t num_qore_object_native_methods = sizeof(qoreObjectNativeMethods) / sizeof(JNINativeMethod);

static GlobalReference<jclass> getPrimitiveClass(Env& env, const char* wrapperName) {
   LocalReference<jclass> wrapperClass = env.findClass(wrapperName);
   jfieldID typeFieldId = env.getStaticField(wrapperClass, "TYPE", "Ljava/lang/Class;");
   return std::move(env.getStaticObjectField(wrapperClass, typeFieldId).as<jclass>().makeGlobal());
}

#include "JavaClassQoreInvocationHandler.inc"
#include "JavaClassQoreExceptionWrapper.inc"
#include "JavaClassQoreObject.inc"
#include "JavaClassQoreObjectWrapper.inc"
#include "JavaClassQoreClosureMarker.inc"
#include "JavaClassQoreURLClassLoader.inc"
#include "JavaClassQoreURLClassLoader_1.inc"
#include "JavaClassQoreJavaApi.inc"
#include "JavaClassQoreRelativeTime.inc"

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

    classQoreObject = env.defineClass("org/qore/jni/QoreObject", nullptr, java_org_qore_jni_QoreObject_class, java_org_qore_jni_QoreObject_class_len).makeGlobal();
    env.registerNatives(classQoreObject, qoreObjectNativeMethods, num_qore_object_native_methods);
    ctorQoreObject = env.getMethod(classQoreObject, "<init>", "(J)V");
    methodQoreObjectGet = env.getMethod(classQoreObject, "get", "()J");

    classQoreObjectWrapper = env.defineClass("org/qore/jni/QoreObjectWrapper", nullptr,
        java_org_qore_jni_QoreObjectWrapper_class, java_org_qore_jni_QoreObjectWrapper_class_len).makeGlobal();

    classQoreClosureMarker = env.defineClass("org/qore/jni/QoreClosureMarker", nullptr,
        java_org_qore_jni_QoreClosureMarker_class, java_org_qore_jni_QoreClosureMarker_class_len).makeGlobal();

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
    env.registerNatives(classQoreJavaApi, qoreJavaApiNativeMethods, num_qore_java_api_native_methods);
    methodQoreJavaApiGetStackTrace = env.getStaticMethod(classQoreJavaApi, "getStackTrace", "()[Ljava/lang/StackTraceElement;");

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
    methodQoreURLClassLoaderAddPendingClass = env.getMethod(classQoreURLClassLoader, "addPendingClass", "(Ljava/lang/String;[B)V");

    env.defineClass("org/qore/jni/QoreURLClassLoader$1", nullptr, java_org_qore_jni_QoreURLClassLoader_1_class, java_org_qore_jni_QoreURLClassLoader_1_class_len);

    classThread = env.findClass("java/lang/Thread").makeGlobal();
    methodThreadCurrentThread = env.getStaticMethod(classThread, "currentThread", "()Ljava/lang/Thread;");
    methodThreadGetContextClassLoader = env.getMethod(classThread, "getContextClassLoader", "()Ljava/lang/ClassLoader;");

    classHashMap = env.findClass("java/util/HashMap").makeGlobal();
    ctorHashMap = env.getMethod(classHashMap, "<init>", "()V");
    methodHashMapPut = env.getMethod(classHashMap, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    methodHashMapEntrySet = env.getMethod(classHashMap, "entrySet", "()Ljava/util/Set;");

    classSet = env.findClass("java/util/Set").makeGlobal();
    methodSetIterator = env.getMethod(classSet, "iterator", "()Ljava/util/Iterator;");

    classEntry = env.findClass("java/util/Map$Entry").makeGlobal();
    methodEntryGetKey = env.getMethod(classEntry, "getKey", "()Ljava/lang/Object;");
    methodEntryGetValue = env.getMethod(classEntry, "getValue", "()Ljava/lang/Object;");

    classIterator = env.findClass("java/util/Iterator").makeGlobal();
    methodIteratorHasNext = env.getMethod(classIterator, "hasNext", "()Z");
    methodIteratorNext = env.getMethod(classIterator, "next", "()Ljava/lang/Object;");

    classZonedDateTime = env.findClass("java/time/ZonedDateTime").makeGlobal();
    methodZonedDateTimeParse = env.getStaticMethod(classZonedDateTime, "parse", "(Ljava/lang/CharSequence;)Ljava/time/ZonedDateTime;");
    methodZonedDateTimeToString = env.getMethod(classZonedDateTime, "toString", "()Ljava/lang/String;");

    classQoreRelativeTime = env.defineClass("org/qore/jni/QoreRelativeTime", nullptr, java_org_qore_jni_QoreRelativeTime_class, java_org_qore_jni_QoreRelativeTime_class_len).makeGlobal();
    ctorQoreRelativeTime = env.getMethod(classQoreRelativeTime, "<init>", "(IIIIIII)V");
    fieldQoreRelativeTimeYear = env.getField(classQoreRelativeTime, "year", "I");
    fieldQoreRelativeTimeMonth = env.getField(classQoreRelativeTime, "month", "I");
    fieldQoreRelativeTimeDay = env.getField(classQoreRelativeTime, "day", "I");
    fieldQoreRelativeTimeHour = env.getField(classQoreRelativeTime, "hour", "I");
    fieldQoreRelativeTimeMinute = env.getField(classQoreRelativeTime, "minute", "I");
    fieldQoreRelativeTimeSecond = env.getField(classQoreRelativeTime, "second", "I");
    fieldQoreRelativeTimeUs = env.getField(classQoreRelativeTime, "us", "I");

    classBigDecimal = env.findClass("java/math/BigDecimal").makeGlobal();
    ctorBigDecimal = env.getMethod(classBigDecimal, "<init>", "(Ljava/lang/String;)V");
    methodBigDecimalToString = env.getMethod(classBigDecimal, "toString", "()Ljava/lang/String;");

    classArrays = env.findClass("java/util/Arrays").makeGlobal();
    methodArraysToString = env.getStaticMethod(classArrays, "toString", "([Ljava/lang/Object;)Ljava/lang/String;");
    methodArraysDeepToString = env.getStaticMethod(classArrays, "deepToString", "([Ljava/lang/Object;)Ljava/lang/String;");

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
    classQoreObject = nullptr;
    classQoreObjectWrapper = nullptr;
    classQoreClosureMarker = nullptr;
    classQoreJavaApi = nullptr;
    classProxy = nullptr;
    classClassLoader = nullptr;
    classQoreURLClassLoader = nullptr;
    classThread = nullptr;
    classHashMap = nullptr;
    classSet = nullptr;
    classEntry = nullptr;
    classIterator = nullptr;
    classZonedDateTime = nullptr;
    classQoreRelativeTime = nullptr;
    classBigDecimal = nullptr;
    classArrays = nullptr;
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

QoreJniStackLocationHelper::QoreJniStackLocationHelper() {
}

const std::string& QoreJniStackLocationHelper::getCallName() const {
    if (tid != gettid()) {
        return jni_no_call_name;
    }
    checkInit();
    assert(current < size());
    //printd(5, "QoreJniStackLocationHelper::getCallName() this: %p %d/%d '%s'\n", this, (int)current, (int)size,
    //    stack_call[current].c_str());
    return stack_call[current];
}

qore_call_t QoreJniStackLocationHelper::getCallType() const {
    if (tid != gettid()) {
        return CT_BUILTIN;
    }
    checkInit();
    assert(current < size());
    return stack_native[current] ? CT_BUILTIN : CT_USER;
}

const QoreProgramLocation& QoreJniStackLocationHelper::getLocation() const {
    if (tid != gettid()) {
        return jni_loc_builtin.get();
    }
    checkInit();
    assert(current < size());
    //printd(5, "QoreJniStackLocationHelper::getLocation() %s:%d (%s)\n", stack_loc[current].getFile(), stack_loc[current].getStartLine());
    return stack_loc[current].get();
}

const QoreStackLocation* QoreJniStackLocationHelper::getNext() const {
    if (tid != gettid()) {
        return stack_next;
    }
    checkInit();
    assert(current < size());
    return (++current < size()) ? this : stack_next;
}

void QoreJniStackLocationHelper::checkInit() const {
    assert(tid == gettid());
    if (init) {
        return;
    }
    init = true;

    Env env;

    try {
        LocalReference<jobjectArray> jstack = env.callStaticObjectMethod(Globals::classQoreJavaApi,
            Globals::methodQoreJavaApiGetStackTrace, nullptr).as<jobjectArray>();

        if (jstack) {
            Type elementType = Globals::getType(Globals::classStackTraceElement);
            jsize len = env.getArrayLength(jstack);
            stack_loc.reserve(len);
            stack_native.reserve(len);
            stack_call.reserve(len);
            for (jsize i = 0; i < len; ++i) {
                LocalReference<jobject> jste = env.getObjectArrayElement(jstack, i);

                LocalReference<jstring> jcls = env.callObjectMethod(jste, Globals::methodStackTraceElementGetClassName, nullptr).as<jstring>();
                jni::Env::GetStringUtfChars cname(env, jcls);
                LocalReference<jstring> jmethod = env.callObjectMethod(jste, Globals::methodStackTraceElementGetMethodName, nullptr).as<jstring>();
                jni::Env::GetStringUtfChars mname(env, jmethod);
                LocalReference<jstring> jfilename = env.callObjectMethod(jste, Globals::methodStackTraceElementGetFileName, nullptr).as<jstring>();
                jni::Env::GetStringUtfChars file(env, jfilename);
                jint line = env.callIntMethod(jste, Globals::methodStackTraceElementGetLineNumber, nullptr);
                jboolean native = env.callBooleanMethod(jste, Globals::methodStackTraceElementIsNativeMethod, nullptr);

                QoreStringMaker code("%s.%s", cname.c_str(), mname.c_str());

                QoreExternalProgramLocationWrapper loc(file.c_str(), line, line, nullptr, 0, "Java");
                //printd(5, "QoreJniStackLocationHelper::checkInit() %d/%d %s:%d\n", (int)i, (int)len, loc.getFile(), line);
                stack_loc.push_back(loc);
                stack_call.push_back(code.c_str());
                stack_native.push_back(native);
            }
        }
    } catch (jni::Exception& e) {
        e.ignore();
    }

    if (!size()) {
        stack_call.push_back(jni_no_call_name);
        stack_native.push_back(true);
        stack_loc.push_back(jni_loc_builtin);
    }
}

} // namespace jni
