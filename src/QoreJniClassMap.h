/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
    QoreJniClassMap.h

    Qore Programming Language JNI Module

    Copyright (C) 2016 - 2020 Qore Technologies, s.r.o.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _QORE_JNI_QOREJAVACLASSMAP_H

#define _QORE_JNI_QOREJAVACLASSMAP_H

#include "QoreJniPrivateData.h"
#include "QoreJniThreads.h"
#include "Env.h"
#include "Class.h"
#include "JniQoreClass.h"

#include <map>
#include <mutex>
#include <condition_variable>

DLLLOCAL QoreClass* initJavaArrayClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initQoreInvocationHandlerClass(QoreNamespace& ns);

DLLLOCAL void init_jni_functions(QoreNamespace& ns);
DLLLOCAL QoreClass* jni_class_handler(QoreNamespace* ns, const char* cname);

DLLLOCAL extern bool jni_compat_types;

namespace jni {

// the JniQoreClass for java::lang::Object
extern JniQoreClass* QC_OBJECT;
// the Qore class ID for java::lang::Object
extern qore_classid_t CID_OBJECT;
// the JniQoreClass for java::lang::Class
extern JniQoreClass* QC_CLASS;
// the Qore class ID for java::lang::Class
extern qore_classid_t CID_CLASS;
// the JniQoreClass for java::lang::reflect::Method
extern JniQoreClass* QC_METHOD;
// the Qore class ID for java::reflect::Method
extern qore_classid_t CID_METHOD;
// the JniQoreClass for java::lang::ClassLoader
extern JniQoreClass* QC_CLASSLOADER;
// the Qore class ID for java::lang::ClassLoader
extern qore_classid_t CID_CLASSLOADER;
// the JniQoreClass for java::lang::Throwable
extern JniQoreClass* QC_THROWABLE;
// the Qore class ID for java::lang::Throwable
extern qore_classid_t CID_THROWABLE;
// the JniQoreClass for java::lang::reflect::InvocationHandler
extern JniQoreClass* QC_INVOCATIONHANDLER;
// the Qore class ID for java::lang::reflect::InvocationHandler
extern qore_classid_t CID_INVOCATIONHANDLER;

// forward references
class Class;
class JniExternalProgramData;

class QoreJniClassMapBase {
protected:
    // map of java class names (ex 'java/lang/Object') to JniQoreClass ptrs
    typedef std::map<std::string, JniQoreClass*> jcmap_t;
    // map of java class names (ex 'java/lang/Object') to JniQoreClass ptrs
    jcmap_t jcmap;

public:
    DLLLOCAL void add(const char* name, JniQoreClass* qc) {
        printd(LogLevel, "QoreJniClassMapBase::add() this: %p name: %s qc: %p (%s)\n", this, name, qc, qc->getName());

#ifdef DEBUG
        if (jcmap.find(name) != jcmap.end())
            printd(0, "QoreJniClassMapBase::add() name: %s qc: %p (%s)\n", name, qc, qc->getName());
#endif

        assert(jcmap.find(name) == jcmap.end());
        jcmap[name] = qc;
    }

    // accepts either a dotted name (ex: "java.lang.Object)") or an internal name ("java/lang/Object") as argument
    DLLLOCAL JniQoreClass* find(const char* jpath) const {
        if (strchr(jpath, '.')) {
            QoreString str(jpath);
            str.replaceAll(".", "/");
            jcmap_t::const_iterator i = jcmap.find(str.c_str());
            return i == jcmap.end() ? nullptr : i->second;
        }
        jcmap_t::const_iterator i = jcmap.find(jpath);
        return i == jcmap.end() ? nullptr : i->second;
    }

    // accepts an internal name as argument (ex: "java/lang/Object")
    DLLLOCAL JniQoreClass* findInternal(const char* jpath) const {
        jcmap_t::const_iterator i = jcmap.find(jpath);
        return i == jcmap.end() ? nullptr : i->second;
    }
};

class QoreJniClassMap : public QoreJniClassMapBase {
public:
    static QoreJniThreadLock m;

    DLLLOCAL void init(bool already_initialized);

    DLLLOCAL void destroy(ExceptionSink& xsink);

    DLLLOCAL QoreValue getValue(LocalReference<jobject>& jobj);

    DLLLOCAL const QoreTypeInfo* getQoreType(jclass cls, const QoreTypeInfo*& altType);

    DLLLOCAL const QoreTypeInfo* getQoreType(jclass cls) {
        const QoreTypeInfo* altType = nullptr;
        return getQoreType(cls, altType);
    }

    DLLLOCAL QoreNamespace& getJniNs() {
        return *default_jns;
    }

    DLLLOCAL jobject getJavaObject(const QoreObject* o);
    DLLLOCAL jobject getJavaClosure(const ResolvedCallReferenceNode* call);

    DLLLOCAL jarray getJavaArray(const QoreListNode* l, jclass cls);

    DLLLOCAL jclass findLoadClass(const char* name);
    DLLLOCAL jclass findLoadClass(const QoreString& name);

    DLLLOCAL JniQoreClass* findCreateQoreClass(LocalReference<jclass>& jc);

    // create the Qore class from the Java dot name (ex: java.lang.Object)
    DLLLOCAL JniQoreClass* findCreateQoreClass(const char* name);

    DLLLOCAL JniQoreClass* findCreateQoreClass(QoreString& name, const char* jpath, Class* c, bool base) {
        //printd(5, "QoreJniClassMap::findCreateQoreClass() '%s' base: %d\n", jpath, base);
        return base
            ? findCreateQoreClassInBase(name, jpath, c)
            : findCreateQoreClassInProgram(name, jpath, c);
    }

    DLLLOCAL static LocalReference<jclass> getCreateJavaClass(const Env::GetStringUtfChars& qpath, QoreProgram* pgm, jstring jname);

    DLLLOCAL static LocalReference<jbyteArray> loadQoreClass(Env& env, const Env::GetStringUtfChars& qpath, QoreProgram* pgm, jstring jname);

    DLLLOCAL static LocalReference<jclass> getJavaType(const QoreTypeInfo* ti, QoreProgram* pgm);

protected:
    // map of java class names to const QoreTypeInfo ptrs
    typedef std::map<const std::string, const QoreTypeInfo*> jtmap_t;
    // map for Qore types from Java classes
    static jtmap_t jtmap;

    // struct of type info and primitive type descriptor strings
    struct qore_java_primitive_info_t {
        const QoreTypeInfo* typeInfo;
        const char* descriptor;
    };
    // map of java primitive type names to type and descriptor info
    typedef std::map<const char*, struct qore_java_primitive_info_t, ltstr> jpmap_t;
    static jpmap_t jpmap;

    // map of Qore base types to java classes
    typedef std::map<qore_type_t, GlobalReference<jclass>> qt2jmap_t;
    static qt2jmap_t qt2jmap;

    // map of Qore classes to java classes
    typedef std::map<const QoreClass*, GlobalReference<jclass>> q2jmap_t;
    static q2jmap_t q2jmap;

    // parent namespace for jni module functionality
    QoreNamespace* default_jns = new QoreNamespace("Jni");

    // class loader
    GlobalReference<jobject> baseClassLoader;

    DLLLOCAL void doMethods(JniQoreClass& qc, Class* jc);

    DLLLOCAL void doFields(JniQoreClass& qc, Class* jc);

    DLLLOCAL int getParamTypes(type_vec_t& argTypeInfo, LocalReference<jobjectArray>& params, QoreString& desc);

    DLLLOCAL void doConstructors(JniQoreClass& qc, Class* jc);

    // add Java parent classes and interfaces as Qore parent classes
    DLLLOCAL void addSuperClasses(JniQoreClass* qc, Class* jc, const char* jpath);

    // populate the Qore class with methods and members from the Java class
    DLLLOCAL void populateQoreClass(JniQoreClass& qc, Class* jc);

    DLLLOCAL void addSuperClass(JniQoreClass& qc, Class* parent, bool interface);

    DLLLOCAL JniQoreClass* createClassInNamespace(QoreNamespace* ns, QoreNamespace& jns, const char* jpath,
        Class* jc, JniQoreClass* qc, QoreJniClassMapBase& map);
    DLLLOCAL JniQoreClass* findCreateQoreClassInBase(QoreString& name, const char* jpath, Class* c);
    DLLLOCAL Class* loadClass(const char* name, bool& base);

    /** @param name an input/output variable, on input it is the java name for the class, which could
        be an inner class (ex: MyClass$1), on output it is the Qore name for the class (ex: MyClass_1)
        @param jpath the java path to the class
        @param c the Java class object

        @return the new builtin Qore class object wrapping the Java class
    */
    DLLLOCAL JniQoreClass* findCreateQoreClassInProgram(QoreString& name, const char* jpath, Class* c);

    DLLLOCAL static LocalReference<jclass> getCreateJavaClassIntern(const QoreClass* qcls, QoreProgram* pgm,
        jstring jname = nullptr);

private:
    // initialization flag
    static bool init_done;
    static std::mutex init_mutex;
    static std::condition_variable init_cond;

    DLLLOCAL static void staticInitBackground(ExceptionSink* xsink, void* pgm);

    DLLLOCAL void initBackground();

    DLLLOCAL jarray getJavaArrayIntern(Env& env, const QoreListNode* l, jclass cls);

    DLLLOCAL Class* loadProgramClass(const char* name, JniExternalProgramData* jpc);

    class InitSignaler {
    public:
        DLLLOCAL ~InitSignaler() {
            init_cond.notify_one();
        }

        std::lock_guard<std::mutex> init_guard();
    };
};

extern QoreJniClassMap qjcm;

class JniExternalProgramData : public AbstractQoreProgramExternalData, public QoreJniClassMapBase {
public:
    DLLLOCAL JniExternalProgramData(QoreNamespace* n_jni);

    DLLLOCAL JniExternalProgramData(const JniExternalProgramData& parent, QoreProgram* pgm);

    // delete the copy constructor
    JniExternalProgramData(const JniExternalProgramData& parent) = delete;

    DLLLOCAL virtual ~JniExternalProgramData() {
        classLoader = nullptr;
    }

    DLLLOCAL jobject getClassLoader() const {
        return classLoader;
    }

    DLLLOCAL jclass getDynamicApi() const {
        assert(dynamicApi);
        return dynamicApi;
    }

    DLLLOCAL jmethodID getInvokeMethodId() const {
        assert(methodQoreJavaDynamicApiInvokeMethod);
        return methodQoreJavaDynamicApiInvokeMethod;
    }

    DLLLOCAL jmethodID getInvokeMethodNonvirtualId() const {
        assert(methodQoreJavaDynamicApiInvokeMethodNonvirtual);
        return methodQoreJavaDynamicApiInvokeMethodNonvirtual;
    }

    DLLLOCAL jmethodID getFieldId() const {
        assert(methodQoreJavaDynamicApiGetField);
        return methodQoreJavaDynamicApiGetField;
    }

    DLLLOCAL QoreNamespace* getJniNamespace() const {
        return jni;
    }

    DLLLOCAL void addClasspath(const char* path);

    DLLLOCAL void overrideCompatTypes(bool compat_types) {
        override_compat_types = true;
        this->compat_types = compat_types;
    }

    DLLLOCAL bool getCompatTypes() const {
        return override_compat_types ? compat_types : jni_compat_types;
    }

    DLLLOCAL void setSaveObjectCallback(const ResolvedCallReferenceNode* save_object_callback) {
        if (this->save_object_callback) {
            this->save_object_callback->deref(nullptr);
        }
        this->save_object_callback = save_object_callback ? save_object_callback->refRefSelf() : nullptr;
    }

    DLLLOCAL ResolvedCallReferenceNode* getSaveObjectCallback() const {
        return save_object_callback;
    }

    DLLLOCAL virtual AbstractQoreProgramExternalData* copy(QoreProgram* pgm) const {
        // issue #3862: ensure that the thread is registered for this call
        Env env;
        return new JniExternalProgramData(*this, pgm);
    }

    DLLLOCAL virtual void doDeref() {
        if (save_object_callback) {
            save_object_callback->deref(nullptr);
        }
        delete this;
    }

    DLLLOCAL static void setContext() {
        Env env;
        setContext(env);
    }

    DLLLOCAL static void setContext(Env& env);

    DLLLOCAL static bool compatTypes();

protected:
    // Jni namespace pointer for the current Program
    QoreNamespace* jni;
    // class loader
    GlobalReference<jobject> classLoader;
    // dynamic API class
    GlobalReference<jclass> dynamicApi;
    // call reference for saving object references
    ResolvedCallReferenceNode* save_object_callback = nullptr;
    // QoreJavaDynamicApi.invokeNethod()
    jmethodID methodQoreJavaDynamicApiInvokeMethod = 0;
    // QoreJavaDynamicApi.invokeNethodNonvirtual()
    jmethodID methodQoreJavaDynamicApiInvokeMethodNonvirtual = 0;
    // QoreJavaDynamicApi.getField()
    jmethodID methodQoreJavaDynamicApiGetField = 0;
    // override compat-types
    bool override_compat_types = false;
    // compat-types values
    bool compat_types = false;
};

DLLLOCAL QoreProgram* jni_get_program_context();
DLLLOCAL JniExternalProgramData* jni_get_context();

}

#endif
