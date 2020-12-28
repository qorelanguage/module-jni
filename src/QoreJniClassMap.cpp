/* -*- indent-tabs-mode: nil -*- */
/*
    QoreJniClassMap.cpp

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

#include <qore/Qore.h>

#include <string.h>

#include <memory>

#include "defs.h"
#include "Jvm.h"
#include "QoreJniClassMap.h"
#include "Class.h"
#include "Method.h"
#include "Functions.h"
#include "JavaToQore.h"
#include "QoreJniThreads.h"
#include "ModifiedUtf8String.h"

#include "JavaClassQoreJavaDynamicApi.inc"

namespace jni {

// the QoreClass for java::lang::Object
JniQoreClass* QC_OBJECT;
// the Qore class ID for java::lang::Object
qore_classid_t CID_OBJECT;
// the QoreClass for java::lang::Class
JniQoreClass* QC_CLASS;
// the Qore class ID for java::lang::Class
qore_classid_t CID_CLASS;
// the QoreClass for java::lang::reflect::Method
JniQoreClass* QC_METHOD;
// the Qore class ID for java::reflect::Method
qore_classid_t CID_METHOD;
// the QoreClass for java::lang::ClassLoader
JniQoreClass* QC_CLASSLOADER;
// the Qore class ID for java::lang::ClassLoader
qore_classid_t CID_CLASSLOADER;
// the QoreClass for java::lang::Throwable
JniQoreClass* QC_THROWABLE;
// the Qore class ID for java::lang::Throwable
qore_classid_t CID_THROWABLE;
// the QoreClass for java::lang::reflect::InvocationHandler
JniQoreClass* QC_INVOCATIONHANDLER;
// the Qore class ID for java::lang::reflect::InvocationHandler
qore_classid_t CID_INVOCATIONHANDLER;
// the QoreClass for java::time::ZonedDateTime
JniQoreClass* QC_ZONEDDATETIME;
// the Qore class ID for java::time::ZonedDateTime
qore_classid_t CID_ZONEDDATETIME;

bool QoreJniClassMap::init_done = false;
std::mutex QoreJniClassMap::init_mutex;
std::condition_variable QoreJniClassMap::init_cond;

QoreJniClassMap qjcm;
static void exec_java_constructor(const QoreMethod& meth, BaseMethod* m, QoreObject* self, const QoreListNode* args, q_rt_flags_t rtflags, ExceptionSink* xsink);
static QoreValue exec_java_static_method(const QoreMethod& meth, BaseMethod* m, const QoreListNode* args, q_rt_flags_t rtflags, ExceptionSink* xsink);
static QoreValue exec_java_method(const QoreMethod& meth, BaseMethod* m, QoreObject* self, QoreJniPrivateData* jd, const QoreListNode* args, q_rt_flags_t rtflags, ExceptionSink* xsink);

QoreJniThreadLock QoreJniClassMap::m;

QoreJniClassMap::jtmap_t QoreJniClassMap::jtmap = {
    {"java.lang.Object", autoTypeInfo},
    // because of automatic array conversions, we do not use "or nothing" types for simple types
    {"java.lang.String", stringTypeInfo},
    {"java.lang.Float", floatTypeInfo},
    {"java.lang.Double", floatTypeInfo},
    {"java.lang.Boolean", boolTypeInfo},
    {"java.lang.Byte", bigIntTypeInfo},
    {"java.lang.Short", bigIntTypeInfo},
    {"java.lang.Integer", bigIntTypeInfo},
    {"java.lang.Long", bigIntTypeInfo},
    {"java.lang.Void", nothingTypeInfo},
    // for complex types, we use "or nothing" types to allow NULL to be passed explicitly
    {"java.time.ZonedDateTime", dateOrNothingTypeInfo},
    {"org.qore.jni.QoreRelativeTime", dateOrNothingTypeInfo},
    {"java.math.BigDecimal", numberOrNothingTypeInfo},
    {"java.util.Map", autoHashOrNothingTypeInfo},
    {"java.util.AbstractMap", autoHashOrNothingTypeInfo},
    {"java.util.HashMap", autoHashOrNothingTypeInfo},
    {"java.util.LinkedHashMap", autoHashOrNothingTypeInfo},
    {"org.qore.jni.Hash", autoHashOrNothingTypeInfo},
    {"java.util.List", autoListOrNothingTypeInfo},
    {"org.qore.jni.QoreObject", objectOrNothingTypeInfo},
    {"org.qore.jni.QoreClosureMarker", codeOrNothingTypeInfo},
    {"org.qore.jni.QoreClosure", codeOrNothingTypeInfo},
};

QoreJniClassMap::qt2jmap_t QoreJniClassMap::qt2jmap;
QoreJniClassMap::q2jmap_t QoreJniClassMap::q2jmap;

QoreJniClassMap::jpmap_t QoreJniClassMap::jpmap = {
    {"byte", {bigIntTypeInfo, "B"}},
    {"char", {bigIntTypeInfo, "C"}},
    {"int", {bigIntTypeInfo, "I"}},
    {"long", {bigIntTypeInfo, "J"}},
    {"short", {bigIntTypeInfo, "S"}},
    {"double", {floatTypeInfo, "D"}},
    {"float", {floatTypeInfo, "F"}},
    {"void", {nothingTypeInfo, "V"}},
    {"boolean", {boolTypeInfo, "Z"}},
};

QoreProgram* jni_get_program_context() {
    return getProgram();
}

JniExternalProgramData* jni_get_context() {
    JniExternalProgramData* jpc;

    // first try to get the actual Program context
    QoreProgram* pgm = getProgram();
    if (pgm) {
        jpc = static_cast<JniExternalProgramData*>(pgm->getExternalData("jni"));
        if (jpc) {
            return jpc;
        }
    }
    pgm = qore_get_call_program_context();
    if (pgm) {
        jpc = static_cast<JniExternalProgramData*>(pgm->getExternalData("jni"));
        if (jpc) {
            return jpc;
        }
    }
    return nullptr;
}

static QoreNamespace* jni_find_create_namespace(QoreNamespace& jns, const char* name, const char*& sn) {
    printd(LogLevel, "jni_find_create_namespace() jns: %p '%s'\n", &jns, name);

    sn = rindex(name, '.');

    QoreNamespace* ns;
    // find parent namespace
    if (!sn) {
        ns = &jns;
        sn = name;
        printd(LogLevel, "jni_find_create_namespace() same namespace\n");
    } else {
        QoreString nsp(name);
        nsp.replaceAll(".", "::");
        ++sn;
        ns = jns.findCreateNamespacePath(nsp.getBuffer());
        printd(LogLevel, "jni_find_create_namespace() jns target: %p '%s' nsp: '%s' ns: %p '%s' new: '%s'\n", &jns, jns.getName(), nsp.c_str(), ns, ns->getName(), sn);
    }

    return ns;
}

void QoreJniClassMap::staticInitBackground(ExceptionSink* xsink, void* pgm) {
    // set program context for initialization
    QoreProgramContextHelper pgm_ctx(static_cast<QoreProgram*>(pgm));

    // ensure that the parent thread is signaled on exit
    InitSignaler signaler;
    try {
        qjcm.initBackground();
    } catch (jni::Exception& e) {
        e.convert(xsink);
    }
}

void QoreJniClassMap::init(bool already_initialized) {
    if (already_initialized) {
        qjcm.initBackground();
        return;
    }
    // grab init mutex
    std::unique_lock<std::mutex> init_lock(init_mutex);

    // issue #3199: perform initialization in the background
    ExceptionSink xsink;
    QoreProgram* pgm = jni_get_program_context();
    q_start_thread(&xsink, &staticInitBackground, pgm);

    // wait for initialization to complete
    init_cond.wait(init_lock);

    // if the background thread threw an exception, then rethrow it here
    if (xsink) {
        throw XsinkException(xsink);
    }
}

void QoreJniClassMap::initBackground() {
    // create java.lang namespace with automatic class loader handler
    QoreNamespace* javans = new QoreNamespace("java");
    QoreNamespace* langns = new QoreNamespace("lang");
    langns->setClassHandler(jni_class_handler);
    javans->addInitialNamespace(langns);

    // add "java" to "Jni" namespace
    default_jns->addInitialNamespace(javans);

    // add "Object" class
    // find/create parent namespace in default / master Jni namespace first
    const char* sn;
    QoreNamespace* ns = jni_find_create_namespace(*default_jns, "java.lang.Object", sn);

    QC_OBJECT = new JniQoreClass("Object", "java.lang.Object");
    CID_OBJECT = QC_OBJECT->getID();
    createClassInNamespace(ns, *default_jns, "java/lang/Object", Functions::loadClass("java/lang/Object"), QC_OBJECT, *this);

    QC_CLASS = findCreateQoreClass("java.lang.Class");
    CID_CLASS = QC_CLASS->getID();
    QC_METHOD = findCreateQoreClass("java.lang.reflect.Method");
    CID_METHOD = QC_METHOD->getID();
    QC_CLASSLOADER = findCreateQoreClass("java.lang.ClassLoader");
    CID_CLASSLOADER = QC_CLASSLOADER->getID();
    QC_THROWABLE = findCreateQoreClass("java.lang.Throwable");
    CID_THROWABLE = QC_THROWABLE->getID();
    QC_INVOCATIONHANDLER = findCreateQoreClass("java.lang.reflect.InvocationHandler");
    CID_INVOCATIONHANDLER = QC_INVOCATIONHANDLER->getID();

    QC_ZONEDDATETIME = findCreateQoreClass("java.time.ZonedDateTime");
    CID_ZONEDDATETIME = QC_ZONEDDATETIME->getID();

    // populate classes after initial hierarchy done
    init_done = true;

    // now populate all classes created up until now
    {
        // copy all classes to a vector
        std::vector<JniQoreClass*> cvec;
        cvec.reserve(jcmap.size());
        for (auto& i : jcmap) {
            cvec.push_back(i.second);
        }
        // populate all classes in the vector
        for (auto& i : cvec) {
            populateQoreClass(*i, static_cast<Class*>(i->getManagedUserData()));
        }
    }

    // rescan all classes
    for (auto& i : jcmap) {
        i.second->rescanParents();
    }

    // add low-level API classes
    {
        QoreNamespace* org = new QoreNamespace("org");
        QoreNamespace* qore = new QoreNamespace("qore");
        QoreNamespace* jni = new QoreNamespace("jni");

        jni->addSystemClass(initQoreInvocationHandlerClass(*jni));
        jni->addSystemClass(initJavaArrayClass(*jni));

        // add low-level API functions
        init_jni_functions(*jni);

        org->addInitialNamespace(qore);
        qore->addInitialNamespace(jni);

        default_jns->addInitialNamespace(org);
    }

    // initialize Qore base type -> java class map
    Env env;
    qt2jmap[NT_INT] = env.findClass("java/lang/Integer").makeGlobal();
    qt2jmap[NT_FLOAT] = env.findClass("java/lang/Double").makeGlobal();
    qt2jmap[NT_BOOLEAN] = env.findClass("java/lang/Boolean").makeGlobal();
    qt2jmap[NT_STRING] = env.findClass("java/lang/String").makeGlobal();
    qt2jmap[NT_DATE] = env.findClass("java/time/ZonedDateTime").makeGlobal();
    qt2jmap[NT_NUMBER] = env.findClass("java/math/BigDecimal").makeGlobal();
    qt2jmap[NT_HASH] = env.findClass("java/util/AbstractMap").makeGlobal();
    qt2jmap[NT_LIST] = env.findClass("java/util/List").makeGlobal();
    qt2jmap[NT_NOTHING] = env.findClass("java/lang/Void").makeGlobal();
}

void QoreJniClassMap::destroy(ExceptionSink& xsink) {
    default_jns->clear(&xsink);
    delete default_jns;
    default_jns = nullptr;
}

jclass QoreJniClassMap::findLoadClass(const QoreString& name) {
   ModifiedUtf8String nameUtf8(name);
   return findLoadClass(nameUtf8.c_str());
}

jclass QoreJniClassMap::findLoadClass(const char* jpath) {
    JniQoreClass* qc;
    {
        QoreJniAutoLocker al(m);
        jcmap_t::iterator i = jcmap.find(jpath);
        if (i != jcmap.end()) {
            qc = i->second;
            //printd(LogLevel, "findLoadClass() '%s': %p (cached)\n", jpath, qc);
        } else {
            JniExternalProgramData* jpc = jni_get_context();
            if (jpc) {
                assert(static_cast<QoreJniClassMapBase*>(jpc) != static_cast<QoreJniClassMapBase*>(this));
                qc = jpc->find(jpath);
            }

            if (!qc) {
                bool base;
                SimpleRefHolder<Class> cls(loadClass(jpath, base));

                QoreString cpath(jpath);
                cpath.replaceAll("/", ".");
                //cpath.replaceAll("$", "__");
                qc = findCreateQoreClass(cpath, jpath, cls.release(), base);
                //printd(LogLevel, "findLoadClass() '%s': %p (created)\n", jpath, qc);
            } else {
                //printd(LogLevel, "findLoadClass() '%s': %p (cached 2)\n", jpath, qc);
            }
        }
    }

    return static_cast<Class*>(qc->getManagedUserData())->toLocal();
}

QoreValue QoreJniClassMap::getValue(LocalReference<jobject>& obj) {
    Env env;

    // see if object is an array
    LocalReference<jclass> jc = env.getObjectClass(obj);

    if (env.callBooleanMethod(jc, Globals::methodClassIsArray, nullptr)) {
        ReferenceHolder<> return_value(nullptr);
        Array::getList(return_value, env, obj.cast<jarray>(), jc);
        return return_value.release();
    }

    if (env.isSameObject(jc, Globals::classInteger)) {
        return env.callIntMethod(obj, Globals::methodIntegerIntValue, nullptr);
    }

    if (env.isSameObject(jc, Globals::classLong))
        return env.callLongMethod(obj, Globals::methodLongLongValue, nullptr);

    if (env.isSameObject(jc, Globals::classShort))
        return env.callShortMethod(obj, Globals::methodShortShortValue, nullptr);

    if (env.isSameObject(jc, Globals::classByte))
        return env.callByteMethod(obj, Globals::methodByteByteValue, nullptr);

    if (env.isSameObject(jc, Globals::classBoolean)) {
        return (bool)env.callBooleanMethod(obj, Globals::methodBooleanBooleanValue, nullptr);
    }

    if (env.isSameObject(jc, Globals::classDouble))
        return (double)env.callDoubleMethod(obj, Globals::methodDoubleDoubleValue, nullptr);

    if (env.isSameObject(jc, Globals::classFloat))
        return (double)env.callFloatMethod(obj, Globals::methodFloatFloatValue, nullptr);

    if (env.isSameObject(jc, Globals::classCharacter))
        return (int64)env.callCharMethod(obj, Globals::methodCharacterCharValue, nullptr);

    QoreProgram* pgm = jni_get_program_context();
    return new QoreObject(qjcm.findCreateQoreClass(jc), pgm, new QoreJniPrivateData(obj));
}

static LocalReference<jstring> get_dot_name(Env& env, const char* name) {
    QoreString nname(name);
    nname.replaceAll("/", ".");
    //printd(LogLevel, "using '%s' -> '%s'\n", name, nname.c_str());
    return env.newString(nname.c_str());
}

Class* QoreJniClassMap::loadClass(const char* name, bool& base) {
    try {
        base = true;

        // first we try to load with the builtin classloader
        return Functions::loadClass(name);
    } catch (jni::JavaException& e) {
        base = false;
        //printd(LogLevel, "QoreJniClassMap::loadClass() '%s' BASE FAILED\n", name);
        JniExternalProgramData* jpc = jni_get_context();
        if (!jpc) {
            printd(LogLevel, "failed to load class '%s' with default classloader; no program-specific classloader present", name);
            throw;
        }
        e.ignore();
        return loadProgramClass(name, jpc);
    }
}

Class* QoreJniClassMap::loadProgramClass(const char* name, JniExternalProgramData* jpc) {
    Env env;
    LocalReference<jstring> jname = get_dot_name(env, name).release();
    jvalue jarg;
    jarg.l = jname;

    try {
        LocalReference<jclass> c = env.callObjectMethod(jpc->getClassLoader(), Globals::methodQoreURLClassLoaderLoadClass, &jarg).as<jclass>();
        return new Class(c.release());
    } catch (jni::JavaException& e) {
        printd(LogLevel, "QoreJniClassMap::loadProgramClass() '%s' LOCAL FAILED\n", name);
        LocalReference<jthrowable> je = e.save();
        // try to load from any thread context class loader
        LocalReference<jobject> thread = env.callStaticObjectMethod(Globals::classThread, Globals::methodThreadCurrentThread, nullptr);
        LocalReference<jobject> cl = env.callObjectMethod(thread, Globals::methodThreadGetContextClassLoader, nullptr);
        printd(LogLevel, "QoreJniClassMap::loadProgramClass() '%s' thread local class loader: %d\n", name, cl ? 1 : 0);
        if (!cl) {
            printd(LogLevel, "QoreJniClassMap::loadProgramClass() FINAL FAILURE '%s'\n", name);
            e.restore(je.release());
            throw;
        }
        LocalReference<jclass> c = env.callObjectMethod(cl, Globals::methodClassLoaderLoadClass, &jarg).as<jclass>();
        printd(LogLevel, "QoreJniClassMap::loadProgramClass() thread-local '%s': %p\n", name, *c);
        return new Class(c.release());
    }
}

JniQoreClass* QoreJniClassMap::findCreateQoreClass(LocalReference<jclass>& jc, const QoreClass* qore_parent) {
    Env env;
    LocalReference<jstring> clsName = env.callObjectMethod(jc, Globals::methodClassGetName, nullptr).as<jstring>();
    Env::GetStringUtfChars tname(env, clsName);

    printd(LogLevel, "QoreJniClassMap::findCreateQoreClass() looking up: '%s'\n", tname.c_str());

    QoreString cname(tname.c_str());
    //cname.replaceAll("$", "__");

    QoreString jpath(tname.c_str());
    jpath.replaceAll(".", "/");

    // see if class is a builtin class or loaded by our custom classloader
    LocalReference<jobject> cl = env.callObjectMethod(jc, Globals::methodClassGetClassLoader, nullptr);
    bool base = (!baseClassLoader && !cl) || (cl && baseClassLoader && env.isSameObject(baseClassLoader, cl));
    //printd(LogLevel, "QoreJniClassMap::findCreateQoreClass() '%s' base: %d\n", jpath.c_str(), base);
    return findCreateQoreClass(cname, jpath.c_str(), new Class(jc), base, qore_parent);
}

JniQoreClass* QoreJniClassMap::findCreateQoreClassInProgram(QoreString& name, const char* jpath, Class* c,
        const QoreClass* qore_parent) {
    SimpleRefHolder<Class> cls(c);

    // we always grab the global JNI lock first because we might need to add base classes
    // while setting up the class loaded with the jni module's classloader, and we need to
    // ensure that these locks are always acquired in order
    QoreJniAutoLocker al(m);

    // check current Program's namespace
    JniExternalProgramData* jpc = jni_get_context();
    if (!jpc) {
        throw BasicException("1: could not attach to deleted Qore Program");
    }

    printd(LogLevel, "QoreJniClassMap::findCreateQoreClassInProgram() this: %p jpc: %p looking up: '%s'\n", this, jpc, jpath);

    JniQoreClass* qc = jpc->find(jpath);
    if (qc)
        return qc;

    printd(LogLevel, "QoreJniClassMap::findCreateQoreClassInProgram() this: %p jpc: %p '%s' not found\n", this, jpc, jpath);

    // grab current Program's parse lock before manipulating namespaces
    CurrentProgramRuntimeExternalParseContextHelper pch;
    if (!pch)
        throw BasicException("2: could not attach to deleted Qore Program");

    // see if we have an inner class
    int ic_idx = name.rfind('$');
    if (ic_idx != -1) {
        name.replaceChar(ic_idx, '_');
        name.insertch('_', ic_idx + 1, 1);
    }

    // find/create parent namespace in default / master Jni namespace first
    const char* sn;
    QoreNamespace* ns = jni_find_create_namespace(*jpc->getJniNamespace(), name.c_str(), sn);
    if (ic_idx != -1) {
        // get last '.'
        int dot = name.rfind('.');

        // check for name conflict
        while (ns->findLocalClass(sn)) {
            // add an underscore
            name.insertch('_', ic_idx + 2, 1);
            sn = name.c_str() + (dot != -1 ? dot + 1 : 0);
        }
    }
    assert(!ns->findLocalClass(sn));

    qc = new JniQoreClass(sn, name.c_str());
    assert(qc->isSystem());
    if (qore_parent) {
        qc->addBaseClass(const_cast<QoreClass*>(qore_parent));
    }
    createClassInNamespace(ns, *jpc->getJniNamespace(), jpath, cls.release(), qc, *jpc);

    return qc;
}

JniQoreClass* QoreJniClassMap::findCreateQoreClass(const char* name) {
    QoreString jpath(name);
    jpath.replaceAll(".", "/");
    jpath.replaceAll("__", "$");

    // first try to find class
    JniQoreClass* rv = findInternal(jpath.c_str());
    if (rv) {
        //printd(LogLevel, "QoreJniClassMap::findCreateQoreClass() '%s': %p\n", name, rv);
        return rv;
    }
    //printd(LogLevel, "QoreJniClassMap::findCreateQoreClass() '%s' not cached\n", name);

    // first try to load the class if possible
    bool base;
    SimpleRefHolder<Class> cls(loadClass(jpath.c_str(), base));

    QoreString cname(name);
    // create the class in the correct namespace
    return findCreateQoreClass(cname, jpath.c_str(), cls.release(), base);
}

JniQoreClass* QoreJniClassMap::findCreateQoreClassInBase(QoreString& name, const char* jpath, Class* c,
        const QoreClass* qore_parent) {
    SimpleRefHolder<Class> cls(c);

    printd(LogLevel, "QoreJniClassMap::findCreateQoreClassInBase() looking up: '%s'\n", jpath);

    // we need to protect access to the default namespace and class map with a lock
    QoreJniAutoLocker al(m);

    // if we have the QoreClass already, then return it
    {
        JniQoreClass* qc = find(jpath);
        if (qc)
            return qc;
    }

    // see if we have an inner class
    int ic_idx = name.rfind('$');
    if (ic_idx != -1) {
        name.replaceChar(ic_idx, '_');
        name.insertch('_', ic_idx + 1, 1);
    }

    // find/create parent namespace in default / master Jni namespace first
    const char* sn;
    QoreNamespace* ns = jni_find_create_namespace(*default_jns, name.c_str(), sn);
    if (ic_idx != -1) {
        // get last '.'
        int dot = name.rfind('.');

        // check for name conflict
        while (ns->findLocalClass(sn)) {
            // add an underscore
            name.insertch('_', ic_idx + 2, 1);
            sn = name.c_str() + (dot != -1 ? dot + 1 : 0);
        }
    }

    JniQoreClass* qc = new JniQoreClass(sn, name.c_str());
    assert(qc->isSystem());
    if (qore_parent) {
        qc->addBaseClass(const_cast<QoreClass*>(qore_parent));
    }
    // createClassInNamespace() will "save" qc in the namespace
    createClassInNamespace(ns, *default_jns, jpath, cls.release(), qc, *this);

    // now add to the current Program's namespace
    JniExternalProgramData* jpc = jni_get_context();
    if (jpc) {
        // grab current Program's parse lock before manipulating namespaces
        CurrentProgramRuntimeExternalParseContextHelper pch;
        if (!pch) {
            throw BasicException("0: could not attach to deleted Qore Program");
        }

        {
            JniQoreClass* qc0 = jpc->find(jpath);
            if (qc0) {
                return qc0;
            }
        }

        ns = jni_find_create_namespace(*jpc->getJniNamespace(), name.c_str(), sn);

        // copy class for assignment
        std::unique_ptr<JniQoreClass> new_qc(new JniQoreClass(*qc));
        assert(new_qc->isSystem());

        printd(LogLevel, "QoreJniClassMap::findCreateQoreClassInBase() jpc: %p '%s' qc: %p ns: %p '%s::%s'\n", jpc, jpath, new_qc.get(), ns, ns->getName(), qc->getName());

        assert(new_qc->getManagedUserData());

        // create entry for class in map
        jpc->add(jpath, new_qc.get());

        // save class in namespace
        qc = new_qc.release();
        ns->addSystemClass(qc);
    }

    return qc;
}

JniQoreClass* QoreJniClassMap::createClassInNamespace(QoreNamespace* ns, QoreNamespace& jns, const char* jpath,
        Class* jc, JniQoreClass* qc, QoreJniClassMapBase& map) {
    QoreClassHolder qc_holder(qc);
    // save pointer to java class info in JniQoreClass
    qc->setManagedUserData(jc);

    int mods = jc->getModifiers();
    if (mods & JVM_ACC_FINAL) {
        qc->setFinal();
    }

    printd(LogLevel, "QoreJniClassMap::createClassInNamespace() qc: %p ns: %p '%s::%s'\n", qc, ns, ns->getName(), qc->getName());

    assert(qc->getManagedUserData());

    // add to class maps
    map.add(jpath, static_cast<JniQoreClass*>(qc_holder.release()));

    addSuperClasses(qc, jc, jpath);

    // add methods after parents
    if (init_done) {
        populateQoreClass(*qc, jc);
    }

    // save class in namespace
    ns->addSystemClass(qc);

    assert(q2jmap.find(qc) == q2jmap.end());
    q2jmap[qc] = jc->getJavaObject();

    printd(LogLevel, "QoreJniClassMap::createClassInNamespace() '%s' returning qc: %p ns: %p -> '%s::%s'\n", jpath, qc, ns, ns->getName(), qc->getName());

    return qc;
}

void QoreJniClassMap::addSuperClasses(JniQoreClass* qc, Class* jc, const char* jpath) {
    Class* parent = jc->getSuperClass();

    printd(LogLevel, "QoreJniClassMap::createClassInNamespace() '%s' parent: %p\n", jpath, parent);

    Env env;

    // add superclass
    if (parent) {
        addSuperClass(*qc, parent, false);
    } else if (qc == QC_OBJECT) {
        // set base class loader: the return value for Class.getClassLoader() with classes loaded by the bootstrap
        // class loader is implementation-dependent; it's possible that this will be nullptr
        LocalReference<jobject> cl = env.callObjectMethod(jc->getJavaObject(), Globals::methodClassGetClassLoader, nullptr);
        if (cl) {
            baseClassLoader = cl.makeGlobal();
        }
    } else { // make interface classes at least inherit Object
        qc->addBuiltinVirtualBaseClass(QC_OBJECT);
    }

    // get and process interfaces
    LocalReference<jobjectArray> interfaceArray = jc->getInterfaces();

    for (jsize i = 0, e = env.getArrayLength(interfaceArray); i < e; ++i) {
        addSuperClass(*qc, new Class(env.getObjectArrayElement(interfaceArray, i).as<jclass>()), true);
    }
}

void QoreJniClassMap::addSuperClass(JniQoreClass& qc, jni::Class* parent, bool interface) {
    Env env;
    LocalReference<jstring> clsName = env.callObjectMethod(parent->getJavaObject(), Globals::methodClassGetName, nullptr).as<jstring>();
    Env::GetStringUtfChars chars(env, clsName);

    printd(LogLevel, "QoreJniClassMap::addSuperClass() qc: '%s' parent: '%s'\n", qc.getName(), chars.c_str());

    QoreString jpath(chars.c_str());
    jpath.replaceAll(".", "/");
    JniQoreClass* pc = find(jpath.c_str());
    if (pc) {
        parent->deref();
    }
    else {
        bool base;
        SimpleRefHolder<Class> cls(loadClass(jpath.c_str(), base));

        QoreString cstr(chars.c_str());
        pc = findCreateQoreClass(cstr, jpath.c_str(), cls.release(), base);
    }

    // only add if no other parent class already inherits the interface
    if (interface) {
        bool priv;
        if (qc.getClass(*pc, priv))
            return;
    }

    qc.addBuiltinVirtualBaseClass(pc);
}

void QoreJniClassMap::populateQoreClass(JniQoreClass& qc, jni::Class* jc) {
    // do constructors
    doConstructors(qc, jc);

    // do methods
    doMethods(qc, jc);

    // do fields
    doFields(qc, jc);
}

void QoreJniClassMap::doConstructors(JniQoreClass& qc, jni::Class* jc) {
    Env env;

    // get constructor methods
    LocalReference<jobjectArray> conArray = jc->getDeclaredConstructors();

    for (jsize i = 0, e = env.getArrayLength(conArray); i < e; ++i) {
        // get Constructor object
        LocalReference<jobject> c = env.getObjectArrayElement(conArray, i);

        SimpleRefHolder<BaseMethod> meth(new BaseMethod(c, jc));

#ifdef DEBUG
        LocalReference<jstring> conStr = env.callObjectMethod(c,
            Globals::methodConstructorToString, nullptr).as<jstring>();
        Env::GetStringUtfChars chars(env, conStr);
        QoreString mstr(chars.c_str());
#endif

        // get method's parameter types
        type_vec_t paramTypeInfo;
        type_vec_t altParamTypeInfo;
        if (meth->getParamTypes(paramTypeInfo, altParamTypeInfo, *this)) {
            printd(LogLevel, "+ skipping %s.constructor() (%s); unsupported parameter type for variant %d\n",
                qc.getName(), mstr.c_str(), i + 1);
            continue;
        }

        // check for duplicate signature
        const QoreMethod* qm = qc.getConstructor();
        if (qm && qm->existsVariant(paramTypeInfo)) {
            printd(LogLevel, "QoreJniClassMap::doConstructors() skipping already-created variant %s::constructor()\n",
                qc.getName());
            continue;
        }

        qc.addConstructor((void*)*meth, (q_external_constructor_t)exec_java_constructor, meth->getAccess(),
            meth->getFlags(), QDOM_UNCONTROLLED_API, paramTypeInfo);

        // add native alternatives for hash and list arg types, if any
        if (!altParamTypeInfo.empty()) {
            if (qm && qm->existsVariant(altParamTypeInfo)) {
                printd(LogLevel, "QoreJniClassMap::doConstructors() skipping already-created variant %s::constructor()\n",
                    qc.getName());
                continue;
            }
            qc.addConstructor((void*)*meth, (q_external_constructor_t)exec_java_constructor, meth->getAccess(),
                meth->getFlags(), QDOM_UNCONTROLLED_API, altParamTypeInfo);
        }

        jc->trackMethod(meth.release());
    }
}

const QoreTypeInfo* QoreJniClassMap::getQoreType(jclass cls, const QoreTypeInfo*& altType) {
    assert(!altType);
    Env env;

    LocalReference<jstring> clsName = env.callObjectMethod(cls, Globals::methodClassGetName, nullptr).as<jstring>();
    Env::GetStringUtfChars tname(env, clsName);

    // substitute "$" with "__"
    QoreString cname(tname.c_str());
    //cname.replaceAll("$", "__");

    QoreString jname(tname.c_str());
    jname.replaceAll(".", "/");

    printd(LogLevel, "QoreJniClassMap::getQoreType() class: '%s' jname: '%s'\n", cname.c_str(), jname.c_str());

    // process array types
    if (env.callBooleanMethod(cls, Globals::methodClassIsArray, nullptr)) {
        return softAutoListTypeInfo;
    }

    // do primitive types
    if (env.callBooleanMethod(cls, Globals::methodClassIsPrimitive, nullptr)) {
        jpmap_t::const_iterator i = jpmap.find(tname.c_str());
        assert(i != jpmap.end());
        return i->second.typeInfo;
    }

    // find or create a class for the type
    JniQoreClass* qc = find(jname.c_str());
    if (!qc) {
        // try to find mapping in Program-specific class map
        JniExternalProgramData* jpc = jni_get_context();
        if (jpc) {
            assert(static_cast<QoreJniClassMapBase*>(jpc) != static_cast<QoreJniClassMapBase*>(this));
            qc = jpc->find(jname.c_str());
        }

        if (!qc) {
            printd(LogLevel, "QoreJniClassMap::getQoreType() creating cname: '%s' jname: '%s'\n", cname.c_str(), jname.c_str());
            bool base;
            SimpleRefHolder<Class> cls(loadClass(jname.c_str(), base));
            qc = findCreateQoreClass(cname, jname.c_str(), cls.release(), base);
        }
    }

    // find static mapping
    jtmap_t::const_iterator i = jtmap.find(tname.c_str());
    if (i != jtmap.end()) {
        altType = qc->getOrNothingTypeInfo();
        return i->second;
    }

    // try all parents to see if a static mapping matches
    QoreParentClassIterator hierarchy_iterator(*qc);
    while (hierarchy_iterator.next()) {
        const std::string jcname = static_cast<const JniQoreClass&>(hierarchy_iterator.getParentClass()).getJavaName();
        // do not return a generic type for the base object class
        if (jcname == "java.lang.Object") {
            continue;
        }
        i = jtmap.find(jcname.c_str());
        if (i != jtmap.end()) {
            // add to jtmap
            jtmap.insert(jtmap_t::value_type(tname.c_str(), i->second));
            //printd(LogLevel, "returning %s (%s) -> %s\n", tname.c_str(), jcname.c_str(), typeInfoGetName(i->second));
            altType = qc->getOrNothingTypeInfo();
            return i->second;
        }
    }

    return qc->getOrNothingTypeInfo();
}

void QoreJniClassMap::doMethods(JniQoreClass& qc, jni::Class* jc) {
    Env env;
    //printd(LogLevel, "QoreJniClassMap::doMethods() qc: %p jc: %p\n", qc, jc);

    LocalReference<jobjectArray> mArray = jc->getDeclaredMethods();

    for (jsize i = 0, e = env.getArrayLength(mArray); i < e; ++i) {
        // get Method object
        LocalReference<jobject> m = env.getObjectArrayElement(mArray, i);

        SimpleRefHolder<BaseMethod> meth(new BaseMethod(m, jc));

        QoreString mname;
        meth->getName(mname);

        printd(LogLevel, "+ adding method %s.%s()\n", qc.getName(), mname.c_str());

        // get method's parameter types
        type_vec_t paramTypeInfo;
        type_vec_t altParamTypeInfo;
        if (meth->getParamTypes(paramTypeInfo, altParamTypeInfo, *this)) {
            printd(LogLevel, "+ skipping %s.%s(); unsupported parameter type for variant %d\n", qc.getName(),
                mname.c_str(), i + 1);
            continue;
        }

        // get method's return type
        const QoreTypeInfo* returnTypeInfo = meth->getReturnTypeInfo(*this);

        if (meth->isStatic()) {
            // check for duplicate signature
            const QoreMethod* qm = qc.findLocalStaticMethod(mname.c_str());
            if (qm && qm->existsVariant(paramTypeInfo)) {
                printd(LogLevel, "QoreJniClassMap::doMethods() skipping already-created static variant %s::%s()\n", qc.getName(), mname.c_str());
                continue;
            }
            qc.addStaticMethod((void*)*meth, mname.c_str(), (q_external_static_method_t)exec_java_static_method, meth->getAccess(), meth->getFlags(), QDOM_UNCONTROLLED_API, returnTypeInfo, paramTypeInfo);

            if (!altParamTypeInfo.empty()) {
                if (qm && qm->existsVariant(altParamTypeInfo)) {
                    printd(LogLevel, "QoreJniClassMap::doMethods() skipping already-created static variant %s::%s()\n",
                        qc.getName(), mname.c_str());
                    continue;
                }
                qc.addStaticMethod((void*)*meth, mname.c_str(), (q_external_static_method_t)exec_java_static_method,
                    meth->getAccess(), meth->getFlags(), QDOM_UNCONTROLLED_API, returnTypeInfo, altParamTypeInfo);
            }
        } else {
            if (mname == "copy" || mname == "constructor" || mname == "destructor" || mname == "methodGate" || mname == "memberNotification" || mname == "memberGate")
                mname.prepend("java_");

            // check for duplicate signature
            const QoreMethod* qm = qc.findLocalMethod(mname.c_str());
            if (qm && qm->existsVariant(paramTypeInfo)) {
                printd(LogLevel, "QoreJniClassMap::doMethods() skipping already-created variant %s::%s()\n", qc.getName(), mname.c_str());
                continue;
            }

            if (meth->isAbstract()) {
                qc.addAbstractMethod(mname.c_str(), meth->getAccess(), meth->getFlags(), returnTypeInfo, paramTypeInfo);
                // do not add additional abstract variants for alternate parameter types
            } else {
                qc.addMethod((void*)*meth, mname.c_str(), (q_external_method_t)exec_java_method, meth->getAccess(),
                    meth->getFlags(), QDOM_UNCONTROLLED_API, returnTypeInfo, paramTypeInfo);
                if (!altParamTypeInfo.empty()) {
                    if (qm && qm->existsVariant(altParamTypeInfo)) {
                        printd(LogLevel, "QoreJniClassMap::doMethods() skipping already-created variant %s::%s()\n",
                            qc.getName(), mname.c_str());
                        continue;
                    }
                    qc.addMethod((void*)*meth, mname.c_str(), (q_external_method_t)exec_java_method, meth->getAccess(),
                        meth->getFlags(), QDOM_UNCONTROLLED_API, returnTypeInfo, altParamTypeInfo);
                }
            }
        }
        jc->trackMethod(meth.release());
    }
}

// ACC opcodes
constexpr int ACC_PUBLIC = 0x0001; // class, field, method
constexpr int ACC_PRIVATE = 0x0002; // class, field, method
constexpr int ACC_PROTECTED = 0x0004; // class, field, method
constexpr int ACC_STATIC = 0x0008; // field, method
constexpr int ACC_FINAL = 0x0010; // class, field, method, parameter
constexpr int ACC_SUPER = 0x0020; // class
constexpr int ACC_SYNCHRONIZED = 0x0020; // method
constexpr int ACC_VOLATILE = 0x0040; // field
constexpr int ACC_BRIDGE = 0x0040; // method
constexpr int ACC_VARARGS = 0x0080; // method
constexpr int ACC_TRANSIENT = 0x0080; // field
constexpr int ACC_NATIVE = 0x0100; // method
constexpr int ACC_INTERFACE = 0x0200; // class
constexpr int ACC_ABSTRACT = 0x0400; // class, method
constexpr int ACC_STRICT = 0x0800; // method
constexpr int ACC_SYNTHETIC = 0x1000; // class, field, method, parameter
constexpr int ACC_ANNOTATION = 0x2000; // class
constexpr int ACC_ENUM = 0x4000; // class(?) field inner
constexpr int ACC_MANDATED = 0x8000; // parameter

static int qore_jni_get_acc_visibility(const QoreMethod& m, const QoreExternalMethodVariant& v) {
    switch (v.getAccess()) {
        case Internal: return ACC_PRIVATE;
        case Private: return ACC_PROTECTED;
        default:
            break;
    }
    return ACC_PUBLIC;
}

static LocalReference<jclass> qore_jni_get_return_class(Env& env, const QoreMethod& m, const QoreExternalMethodVariant& v, QoreProgram* pgm) {
    const QoreTypeInfo* ti = v.getReturnTypeInfo();
    return QoreJniClassMap::getJavaType(env, ti, pgm);
}

// returns an ArrayList<Type> object or null
static LocalReference<jobject> qore_jni_get_java_params(Env& env, const QoreMethod& m, const QoreExternalMethodVariant& v,
        QoreProgram* pgm, unsigned& len) {
    const type_vec_t& params = v.getParamTypeList();
    len = params.size();
    if (params.empty()) {
        return nullptr;
    }

    printd(5, "qore_jni_get_java_params() %s::%s() %d param(s)\n", m.getClassName(), m.getName(), (int)params.size());

    // create parameter list
    LocalReference<jobject> plist = env.newObject(Globals::classArrayList, Globals::ctorArrayList, nullptr);

    for (const QoreTypeInfo* i : params) {
        LocalReference<jclass> ptype = QoreJniClassMap::getJavaType(env, i, pgm);
        printd(5, "%s::%s(): adding %s -> %p\n", m.getClassName(), m.getName(), type_get_name(i), *ptype);

        jvalue jarg;
        jarg.l = ptype;
        env.callBooleanMethod(plist, Globals::methodArrayListAdd, &jarg);
    }

    return plist.release();
}

static void shorten_params(Env& env, LocalReference<jobject>& params, unsigned len) {
    jvalue jarg;
    jarg.i = (int)len;
    env.callObjectMethod(Globals::classArrayList, Globals::methodArrayListRemove, &jarg);
}

static bool check_optional_last_param(Env& env, const QoreExternalMethodVariant& v, LocalReference<jobject>& params,
        unsigned& len) {
    --len;
    const type_vec_t& qore_params = v.getParamTypeList();
    assert(qore_params.size() > len);
    // if the type accepts NOTHING, then it's optional
    if (qore_type_is_assignable_from(qore_params[len], QoreValue())) {
        shorten_params(env, params, len);
        return true;
    }

    const arg_vec_t& def_args = v.getDefaultArgList();
    if (def_args.size() > len && def_args[len]) {
        shorten_params(env, params, len);
        return true;
    }
    return false;
}

static int qore_url_classloader_create_java_qore_class_add_constructor(Env& env, const QoreClass& qcls,
        LocalReference<jobject>& bb, const QoreMethod& m, const QoreExternalMethodVariant& v, QoreProgram* pgm) {
    printd(5, "qore_url_classloader_create_java_qore_class_add_constructor() adding Java constructor %s %s::constructor(%s) {}\n",
        v.getAccessString(), qcls.getName(), v.getSignatureText());

    // first get the params
    unsigned len;
    LocalReference<jobject> params = qore_jni_get_java_params(env, m, v, pgm, len).release();

    while (true) {
        std::vector<jvalue> jargs(4);
        jargs[0].l = bb;
        jargs[1].l = Globals::classQoreObjectBase;
        jargs[2].i = qore_jni_get_acc_visibility(m, v);
        jargs[3].l = params;

        bb = env.callStaticObjectMethod(Globals::classJavaClassBuilder, Globals::methodJavaClassBuilderAddConstructor,
            &jargs[0]);
        printd(5, "qore_url_classloader_create_java_qore_class_add_constructor() bb: %p\n", (jobject)bb);

        if (!params || !len || !check_optional_last_param(env, v, params, len)) {
            break;
        }
    }

    return 0;
}

static int qore_url_classloader_create_java_qore_class_add_normal_method(Env& env, const QoreClass& qcls,
        LocalReference<jobject>& bb, const QoreMethod& m, const QoreExternalMethodVariant& v, QoreProgram* pgm) {
    printd(5, "qore_url_classloader_create_java_qore_class_add_normal_method() adding Java normal method %s %s::%s(%s)\n",
        qore_type_get_name(v.getReturnTypeInfo()), qcls.getName(), m.getName(), v.getSignatureText());

    // first get the params
    unsigned len;
    LocalReference<jobject> params = qore_jni_get_java_params(env, m, v, pgm, len).release();

    while (true) {
        std::vector<jvalue> jargs(6);
        jargs[0].l = bb;
        LocalReference<jstring> mname = env.newString(m.getName());
        jargs[1].l = mname;
        jargs[2].i = qore_jni_get_acc_visibility(m, v);
        LocalReference<jclass> return_type = qore_jni_get_return_class(env, m, v, pgm);
        jargs[3].l = return_type;
        jargs[4].l = params;
        jargs[5].z = v.isAbstract();

        printd(5, "qore_url_classloader_create_java_qore_class_add_normal_method() %s %s::%s(%s): about to call bb: %p\n", qore_type_get_name(v.getReturnTypeInfo()), qcls.getName(), m.getName(), v.getSignatureText(), (jobject)bb);
        bb = env.callStaticObjectMethod(Globals::classJavaClassBuilder,
            Globals::methodJavaClassBuilderAddNormalMethod, &jargs[0]);
        printd(5, "qore_url_classloader_create_java_qore_class_add_normal_method() bb: %p\n", (jobject)bb);

        if (!params || !len || !check_optional_last_param(env, v, params, len)) {
            break;
        }
    }

    return 0;
}

static int qore_url_classloader_create_java_qore_class_add_static_method(Env& env, const QoreClass& qcls,
        LocalReference<jobject>& bb, const QoreMethod& m, const QoreExternalMethodVariant& v, QoreProgram* pgm) {
    printd(5, "qore_url_classloader_create_java_qore_class_add_static_method() adding Java static method %s %s::%s(%s)\n",
        qore_type_get_name(v.getReturnTypeInfo()), qcls.getName(), m.getName(), v.getSignatureText());

    // first get the params
    unsigned len;
    LocalReference<jobject> params = qore_jni_get_java_params(env, m, v, pgm, len).release();

    while (true) {
        std::vector<jvalue> jargs(5);
        jargs[0].l = bb;
        LocalReference<jstring> mname = env.newString(m.getName());
        jargs[1].l = mname;
        jargs[2].i = qore_jni_get_acc_visibility(m, v);
        LocalReference<jclass> return_type = qore_jni_get_return_class(env, m, v, pgm);
        jargs[3].l = return_type;
        jargs[4].l = params;

        printd(5, "qore_url_classloader_create_java_qore_class_add_static_method() about to call bb: %p\n", (jobject)bb);
        bb = env.callStaticObjectMethod(Globals::classJavaClassBuilder,
            Globals::methodJavaClassBuilderAddStaticMethod, &jargs[0]);
        printd(5, "qore_url_classloader_create_java_qore_class_add_static_method() bb: %p\n", (jobject)bb);

        if (!params || !len || !check_optional_last_param(env, v, params, len)) {
            break;
        }
    }

    return 0;
}

static int qore_url_classloader_create_java_qore_class_add_methods(Env& env, const QoreClass& qcls,
        LocalReference<jobject>& bb, QoreProgram* pgm) {
    QoreMethodIterator i(qcls);
    unsigned constructor_count = 0;
    while (i.next()) {
        const QoreMethod* m = i.getMethod();
        QoreExternalFunctionIterator vi(*m->getFunction());

        while (vi.next()) {
            const QoreExternalMethodVariant* v = reinterpret_cast<const QoreExternalMethodVariant*>(vi.getVariant());
            printd(5, "qore_url_classloader_create_java_qore_class_add_methods() %s::%s(%s)\n",
                qcls.getName(), m->getName(), v->getSignatureText());
            switch (m->getMethodType()) {
                case MT_Constructor: {
                    if (qore_url_classloader_create_java_qore_class_add_constructor(env, qcls, bb, *m, *v, pgm)) {
                        return -1;
                    }
                    ++constructor_count;
                    break;
                };

                case MT_Normal: {
                    if (qore_url_classloader_create_java_qore_class_add_normal_method(env, qcls, bb, *m, *v, pgm)) {
                        return -1;
                    }
                    break;
                }

                case MT_Static: {
                    if (qore_url_classloader_create_java_qore_class_add_normal_method(env, qcls, bb, *m, *v, pgm)) {
                        return -1;
                    }
                    break;
                }

                default: {
                    printd(5, "qore_url_classloader_create_java_qore_class() ignoring method %s::%s(%s)\n",
                        qcls.getName(), m->getName(), v->getSignatureText());
                    break;
                }
            }
        }
    }

    // add default constructor if necessary
    if (!constructor_count) {
        std::vector<jvalue> jargs(4);
        jargs[0].l = bb;
        jargs[1].l = Globals::classQoreObjectBase;
        jargs[2].i = ACC_PROTECTED;
        jargs[3].l = nullptr;

        bb = env.callStaticObjectMethod(Globals::classJavaClassBuilder, Globals::methodJavaClassBuilderAddConstructor,
            &jargs[0]);
        printd(5, "qore_url_classloader_create_java_qore_class_add_methods() %p: %s: bb: %p (default constructor)\n", &qcls, qcls.getName(), (jobject)bb);
    }

    return 0;
}

LocalReference<jobject> QoreJniClassMap::getCreateJavaClass(Env& env, jobject class_loader,
        const Env::GetStringUtfChars& qpath, QoreProgram* pgm, jstring jname, jboolean need_byte_code) {
    ExceptionSink xsink;
    // set program context (and read lock) before calling QoreProgram::findClass()
    QoreExternalProgramContextHelper pch(&xsink, pgm);
    const QoreClass* qcls;
    if (!xsink) {
        qcls = pgm->findClass(qpath.c_str(), &xsink);
    }
    if (xsink) {
        assert(!qcls);
        throw XsinkException(xsink);
    }
    printd(5, "QoreJniClassMap::getCreateJavaClass() qpath: '%s'\n", qpath.c_str());
    if (!qcls) {
        QoreStringMaker desc("%s: no such Qore class exists", qpath.c_str());
        env.throwNew(env.findClass("java/lang/RuntimeException"), desc.c_str());
        return nullptr;
    }

    // ensure exclusive access while creating java classes
    QoreJniAutoLocker al(m);
    return getCreateJavaClassIntern(env, class_loader, qcls, pgm, jname, need_byte_code);
}

template<typename T>
static LocalReference<jobject> get_qore_java_dynamic_class_data(Env& env, T& cls) {
    // build QoreJavaDynamicClassData object
    jvalue jargs[2];
    jargs[0].l = cls;
    jargs[1].l = nullptr;
    return env.newObject(Globals::classQoreJavaDynamicClassData, Globals::ctorQoreJavaDynamicClassData, &jargs[0]).release();
}

LocalReference<jobject> QoreJniClassMap::getCreateJavaClassIntern(Env& env, jobject class_loader,
        const QoreClass* qcls, QoreProgram* pgm, jstring jname, jboolean need_byte_code) {
    // look in q2jmap first if byte code not needed
    bool found = false;
    q2jmap_t::iterator i = q2jmap.lower_bound(qcls);
    if (i != q2jmap.end() && i->first == qcls) {
        if (!need_byte_code) {
            // build QoreJavaDynamicClassData object
            return get_qore_java_dynamic_class_data<GlobalReference<jclass>>(env, i->second);
        }
        found = true;
    }

    // insert a placeholder for the current class
    LocalReference<jclass> tmp = Globals::classObject.toLocal();
    if (!found) {
        i = q2jmap.insert(i, q2jmap_t::value_type(qcls, tmp.makeGlobal()));
    }
    try {
        jlong cptr = reinterpret_cast<jlong>(qcls);

        printd(5, "QoreJniClassMap::getCreateJavaClassIntern() p: %p path: '%s': %p (abstract: %d) qob: %p\n", pgm, qcls->getName(), cptr, qcls->isAbstract(), (jclass)Globals::classQoreObjectBase);

        LocalReference<jstring> njname;
        if (!jname) {
            std::string pname = qcls->getNamespacePath(true);
            size_t start_pos = 0;
            while ((start_pos = pname.find("::", start_pos)) != std::string::npos) {
                pname.replace(start_pos, 2, ".");
                ++start_pos;
            }
            pname.insert(0, "qore.");
            printd(5, "QoreJniClassMap::getCreateJavaClassIntern() cls '%s' -> java '%s' (generated)\n", qcls->getName(), pname.c_str());

            njname = env.newString(pname.c_str());
            jname = njname;
        }

        std::vector<jvalue> jargs(4);
        jargs[0].l = jname;
        jargs[1].l = Globals::classQoreObjectBase;
        jargs[2].z = qcls->isAbstract();
        jargs[3].j = cptr;

        LocalReference<jobject> bb = env.callStaticObjectMethod(Globals::classJavaClassBuilder,
            Globals::methodJavaClassBuilderGetClassBuilder, &jargs[0]);
        printd(5, "QoreJniClassMap::getCreateJavaClassIntern() bb: %p\n", (jobject)bb);

        // add methods
        if (qore_url_classloader_create_java_qore_class_add_methods(env, *qcls, bb, pgm)) {
            if (!found) {
                q2jmap.erase(i);
            }
            printd(5, "QoreJniClassMap::getCreateJavaClassIntern() failed to add members\n");
            return nullptr;
        }

        printd(5, "QoreJniClassMap::getCreateJavaClassIntern() %s methods added bb: %p; building class with syscl: %p\n", qcls->getName(), (jobject)bb, (jobject)Globals::syscl);

        jargs[0].l = bb;
        jargs[1].l = class_loader;
        LocalReference<jobject> rv = env.callStaticObjectMethod(Globals::classJavaClassBuilder,
            Globals::methodJavaClassBuilderGetClassFromBuilder, &jargs[0]);

        if (!found) {
            i->second = env.getObjectField(rv, Globals::fieldQoreJavaDynamicClassDataCls).as<jclass>().makeGlobal();
        }
        printd(5, "QoreJniClassMap::getCreateJavaClassIntern() %s rv: %p\n", qcls->getName(), (jobject)rv);

        // build QoreJavaDynamicClassData object
        return rv.release();
    } catch (...) {
        if (!found) {
            q2jmap.erase(i);
        }
        throw;
    }
}

LocalReference<jclass> QoreJniClassMap::getJavaType(Env& env, const QoreTypeInfo* ti, QoreProgram* pgm) {
    QoreJniAutoLocker al(m);

    qore_type_t t = qore_type_get_base_type(ti);
    //printd(5, "QoreJniClassMap::getJavaType() looking up type '%s' (%d)\n", qore_type_get_name(ti), t);
    if (t != NT_OBJECT) {
        qt2jmap_t::const_iterator i = qt2jmap.find(t);
        LocalReference<jclass> rv = i != qt2jmap.end() ? i->second.toLocal() : Globals::classObject.toLocal();
        printd(5, "QoreJniClassMap::getJavaType() type '%s' (%d) got java cls %p\n", qore_type_get_name(ti), t, (jclass)rv);
        return rv;
    }

    const QoreClass* cls = typeInfoGetUniqueReturnClass(ti);
    if (!cls) {
        return nullptr;
    }

    q2jmap_t::const_iterator i = q2jmap.find(cls);
    if (i != q2jmap.end()) {
        return i->second.toLocal();
    }

    //printd(5, "QoreJniClassMap::getJavaType() type '%s' (%d) creating Java class for '%s'\n", qore_type_get_name(ti), t, cls->getName());
    LocalReference<jobject> rv = getCreateJavaClassIntern(env, Globals::syscl, cls, pgm);
    return env.getObjectField(rv, Globals::fieldQoreJavaDynamicClassDataCls).as<jclass>();
}

jobject QoreJniClassMap::getJavaObject(const QoreObject* o) {
    if (!o->isValid()) {
        return nullptr;
    }
    ExceptionSink xsink;
    TryPrivateDataRefHolder<QoreJniPrivateData> jo(o, CID_OBJECT, &xsink);
    if (jo) {
        return jo->makeLocal().release();
    }

    // return a new Java QoreObject with a weak reference to the actual Qore object
    o->tRef();
    jvalue arg;
    arg.j = reinterpret_cast<jlong>(o);
    try {
        Env env;
        return env.newObject(Globals::classQoreObject, Globals::ctorQoreObject, &arg).release();
    } catch (jni::Exception& e) {
        const_cast<QoreObject*>(o)->tDeref();
        throw;
    }
}

jobject QoreJniClassMap::getJavaClosure(const ResolvedCallReferenceNode* call) {
    // return a new Java QoreClosure; weak references are not needed, as ResolvedCallReferenceNode objects always
    // implement a weak reference to any captured QoreObject*s
    call->ref();
    jvalue arg;
    arg.j = reinterpret_cast<jlong>(call);
    try {
        Env env;
        return env.newObject(Globals::classQoreClosure, Globals::ctorQoreClosure, &arg).release();
    } catch (jni::Exception& e) {
        // NOTE: in the very unlikely case of a Qore exception here, the default exception handler will handle it
        ExceptionSink xsink;
        const_cast<ResolvedCallReferenceNode*>(call)->deref(&xsink);
        throw;
    }
}

jarray QoreJniClassMap::getJavaArray(const QoreListNode* l, jclass cls) {
   Env env;

   if (!cls)
      return getJavaArrayIntern(env, l, Globals::classObject);

   // get component class for array
   LocalReference<jclass> ccls = env.callObjectMethod(cls, Globals::methodClassGetComponentType, nullptr).as<jclass>();
   if (!ccls) {
      LocalReference<jstring> clsName = env.callObjectMethod(cls, Globals::methodClassGetCanonicalName, nullptr).as<jstring>();
      Env::GetStringUtfChars cname(env, clsName);
      QoreStringMaker str("cannot instantiate array from '%s'", cname.c_str());

      throw BasicException(str.c_str());
   }

   return getJavaArrayIntern(env, l, ccls);
}

jarray QoreJniClassMap::getJavaArrayIntern(Env& env, const QoreListNode* l, jclass cls) {
    Type elementType = Globals::getType(cls);

    LocalReference<jarray> array = Array::getNew(elementType, cls, l->size());

    // now populate array
    for (jsize i = 0; i < static_cast<jsize>(l->size()); ++i) {
        Array::set(array, elementType, cls, i, l->retrieveEntry(i));
    }

    return array.release();
}

static void exec_java_constructor(const QoreMethod& qmeth, BaseMethod* m, QoreObject* self, const QoreListNode* args, q_rt_flags_t rtflags, ExceptionSink* xsink) {
    try {
        // issue #3585: set context for external java threads
        JniExternalProgramData::setContext();
        self->setPrivate(qmeth.getClass()->getID(), new QoreJniPrivateData(m->newQoreInstance(args)));
    } catch (jni::Exception& e) {
        e.convert(xsink);
    }
}

static QoreValue exec_java_static_method(const QoreMethod& meth, BaseMethod* m, const QoreListNode* args, q_rt_flags_t rtflags, ExceptionSink* xsink) {
    try {
        // issue #3585: set context for external java threads
        JniExternalProgramData::setContext();
        return m->invokeStatic(args);
    } catch (jni::Exception& e) {
        e.convert(xsink);
        return QoreValue();
    }
}

static QoreValue exec_java_method(const QoreMethod& meth, BaseMethod* m, QoreObject* self, QoreJniPrivateData* jd, const QoreListNode* args, q_rt_flags_t rtflags, ExceptionSink* xsink) {
    // NOTE: Java base classes will have no Qore program context
    QoreProgramContextHelper pch(self->getProgram());

    try {
        // issue #3585: set context for external java threads
        JniExternalProgramData::setContext();
        return m->invoke(jd->getObject(), args);
    } catch (jni::Exception& e) {
        e.convert(xsink);
        return QoreValue();
    }
}

static const char* access_str(ClassAccess a) {
    switch (a) {
        case Public: return "public";
        case Private: return "private";
        default: return "private:internal";
    }
}

void QoreJniClassMap::doFields(JniQoreClass& qc, jni::Class* jc) {
    Env env;

    printd(LogLevel, "QoreJniClassMap::doFields() %s qc: %p jc: %p\n", qc.getName(), &qc, jc);

    LocalReference<jobjectArray> fArray = jc->getDeclaredFields();
    for (jsize i = 0, e = env.getArrayLength(fArray); i < e; ++i) {
        // get Field object
        LocalReference<jobject> f = env.getObjectArrayElement(fArray, i);

        SimpleRefHolder<BaseField> field(new BaseField(f, jc));

        QoreString fname;
        field->getName(fname);

        const QoreTypeInfo* fieldTypeInfo = field->getQoreTypeInfo(*this);

        if (field->isStatic()) {
            printd(LogLevel, "+ adding static field %s %s %s.%s (%s)\n", access_str(field->getAccess()), typeInfoGetName(fieldTypeInfo), qc.getName(), fname.c_str(), field->isFinal() ? "const" : "var");

            QoreValue v(field->getStatic());
            if (field->isFinal()) {
                if (v.isNothing())
                v.assign(0ll);
                qc.addBuiltinConstant(fname.c_str(), v, field->getAccess());
            } else
                qc.addBuiltinStaticVar(fname.c_str(), v, field->getAccess(), fieldTypeInfo);
        } else if (!qc.findLocalMember(fname.c_str())) {
            printd(LogLevel, "+ adding field %s %s %s.%s\n", access_str(field->getAccess()), typeInfoGetName(fieldTypeInfo), qc.getName(), fname.c_str());
            qc.addMember(fname.c_str(), field->getAccess(), fieldTypeInfo);
        }
    }
}

JniExternalProgramData::JniExternalProgramData(QoreNamespace* n_jni) : jni(n_jni) {
    assert(jni);
    Env env(false);

    // get Program context
    QoreProgram* pgm = jni_get_program_context();
    // issue #3310: if there is no Program context - for example, if we are being called from a pure Java context -
    // create one to provide Qore functionality to Java
    if (!pgm) {
        pgm = jni_get_create_program(env);
    }
    assert(pgm);

    // set up QoreURLClassLoader constructor args
    {
        std::vector<jvalue> jargs(2);
        jargs[0].j = reinterpret_cast<long>(pgm);
        jargs[1].l = Globals::syscl;

        // create our custom classloader
        classLoader = env.newObject(Globals::classQoreURLClassLoader, Globals::ctorQoreURLClassLoader, &jargs[0]).makeGlobal();
    }

    {
        // define the QoreJavaDynamicApi class using our new classloader
        LocalReference<jstring> jname = env.newString("org.qore.jni.QoreJavaDynamicApi");

        // make byte array
        LocalReference<jbyteArray> jbyte_code = env.newByteArray(java_org_qore_jni_QoreJavaDynamicApi_class_len).as<jbyteArray>();
        for (jsize i = 0; (unsigned)i < java_org_qore_jni_QoreJavaDynamicApi_class_len; ++i) {
            env.setByteArrayElement(jbyte_code, i, java_org_qore_jni_QoreJavaDynamicApi_class[i]);
        }

        std::vector<jvalue> jargs(4);
        jargs[0].l = jname;
        jargs[1].l = jbyte_code;
        jargs[2].i = 0;
        jargs[3].i = java_org_qore_jni_QoreJavaDynamicApi_class_len;

        dynamicApi = env.callObjectMethod(classLoader, Globals::methodQoreURLClassLoaderDefineResolveClass, &jargs[0]).as<jclass>().makeGlobal();
        methodQoreJavaDynamicApiInvokeMethod = env.getStaticMethod(dynamicApi, "invokeMethod", "(Ljava/lang/reflect/Method;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
        methodQoreJavaDynamicApiInvokeMethodNonvirtual = env.getStaticMethod(dynamicApi, "invokeMethodNonvirtual", "(Ljava/lang/reflect/Method;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
        methodQoreJavaDynamicApiGetField = env.getStaticMethod(dynamicApi, "getField", "(Ljava/lang/reflect/Field;Ljava/lang/Object;)Ljava/lang/Object;");

        /*
        // get lookup object
        jmethodID methodQoreJavaDynamicApiLookup = env.getStaticMethod(dynamicApi, "lookup", "()Ljava/lang/invoke/MethodHandles$Lookup;");
        lookup = env.callStaticObjectMethod(dynamicApi, methodQoreJavaDynamicApiLookup, nullptr).makeGlobal();
        */

        //printd(LogLevel, "this: %p: dynamic API created: %p classloader: %p\n", this, getDynamicApi(), getClassLoader());
    }

    // setup classpath
    TempString classpath(SystemEnvironment::get("QORE_JNI_CLASSPATH"));
    if (classpath) {
        addClasspath(classpath->c_str());
    }
}

JniExternalProgramData::JniExternalProgramData(const JniExternalProgramData& parent, QoreProgram* pgm) :
    // reuse the same classLoader as the parent
    classLoader(GlobalReference<jobject>::fromLocal(parent.classLoader.toLocal())),
    // reuse the same dynamic API as the parent
    dynamicApi(GlobalReference<jclass>::fromLocal(parent.dynamicApi.toLocal())),
    methodQoreJavaDynamicApiInvokeMethod(parent.methodQoreJavaDynamicApiInvokeMethod),
    methodQoreJavaDynamicApiInvokeMethodNonvirtual(parent.methodQoreJavaDynamicApiInvokeMethodNonvirtual),
    methodQoreJavaDynamicApiGetField(parent.methodQoreJavaDynamicApiGetField),
    override_compat_types(parent.override_compat_types),
    compat_types(parent.compat_types) {
    // copy the parent's class map to this one
    jcmap = parent.jcmap;
    // find Jni namespace in new Program if present
    jni = pgm->findNamespace("Jni");
    if (!jni) {
        jni = qjcm.getJniNs().copy();
        pgm->getRootNS()->addNamespace(jni);
    }
}

void JniExternalProgramData::addClasspath(const char* path) {
    Env env;
    LocalReference<jstring> jname = env.newString(path);
    jvalue jarg;
    jarg.l = jname;
    try {
        env.callVoidMethod(classLoader, Globals::methodQoreURLClassLoaderAddPath, &jarg);
    } catch (jni::Exception& e) {
        // display exception info on the console as an unhandled exception
        ExceptionSink xsink;
        e.convert(&xsink);
    }
}

void JniExternalProgramData::setContext(Env& env) {
    // issue #3199: no program is available when initializing the jni module from the command line
    // issue #3153: no context is available when called from a static method
    JniExternalProgramData* jpc = jni_get_context();
    if (jpc) {
        // set classloader context in new thread
        env.callVoidMethod(jpc->classLoader, Globals::methodQoreURLClassLoaderSetContext, nullptr);
        //printd(LogLevel, "JniExternalProgramData::setContext() pgm: %p jpc: %p\n", pgm, jpc);
    }
}

bool JniExternalProgramData::compatTypes() {
    // issue #3199: no program is available when initializing the jni module from the command line
    // issue #3153: no context is available when called from a static method
    JniExternalProgramData* jpc = jni_get_context();
    if (!jpc) {
        return jni_compat_types;
    }
    return jpc->getCompatTypes();
}
}
