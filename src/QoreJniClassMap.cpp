/* -*- indent-tabs-mode: nil -*- */
/*
    QoreJniClassMap.cpp

    Qore Programming Language JNI Module

    Copyright (C) 2016 - 2018 Qore Technologies, s.r.o.

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

#include "defs.h"
#include "Jvm.h"
#include "QoreJniClassMap.h"
#include "Class.h"
#include "Method.h"
#include "Functions.h"
#include "JavaToQore.h"
#include "QoreJniThreads.h"
#include "ModifiedUtf8String.h"

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
    {"java.lang.String", stringTypeInfo},
    {"java.lang.Float", floatTypeInfo},
    {"java.lang.Double", floatTypeInfo},
    {"java.lang.Boolean", boolTypeInfo},
    {"java.lang.Float", floatTypeInfo},
    {"java.lang.Double", floatTypeInfo},
    {"java.lang.Byte", bigIntTypeInfo},
    {"java.lang.Short", bigIntTypeInfo},
    {"java.lang.Integer", bigIntTypeInfo},
    {"java.lang.Long", bigIntTypeInfo},
    {"java.lang.Void", nothingTypeInfo},
    {"java.time.ZonedDateTime", dateTypeInfo},
    {"org.qore.jni.QoreRelativeTime", dateTypeInfo},
    {"java.math.BigDecimal", numberTypeInfo},
    {"java.util.Map", autoHashTypeInfo},
    {"java.util.AbstractMap", autoHashTypeInfo},
    {"java.util.AbstractArray", autoListTypeInfo},
    {"org.qore.jni.QoreObject", objectOrNothingTypeInfo},
    {"org.qore.jni.QoreClosureMarker", codeTypeInfo},
};

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

static QoreNamespace* jni_find_create_namespace(QoreNamespace& jns, const char* name, const char*& sn) {
    printd(LogLevel, "jni_find_create_namespace() jns: %p '%s'\n", &jns, name);

    sn = rindex(name, '.');

    QoreNamespace* ns;
    // find parent namespace
    if (!sn) {
        ns = &jns;
        sn = name;
        printd(LogLevel, "jni_find_create_namespace() same namespace\n");
    }
    else {
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

void QoreJniClassMap::init() {
    // grab init mutex
    std::unique_lock<std::mutex> init_lock(init_mutex);

    // issue #3199: perform initialization in the background
    ExceptionSink xsink;
    q_start_thread(&xsink, &staticInitBackground, getProgram());
    //std::thread init_thread(&QoreJniClassMap::initBackground, &qjcm);

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
}

void QoreJniClassMap::destroy(ExceptionSink& xsink) {
   default_jns->clear(&xsink);
   delete default_jns;
   default_jns = 0;
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
            JniExternalProgramData* jpc = static_cast<JniExternalProgramData*>(getProgram()->getExternalData("jni"));
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

    if (env.isSameObject(jc, Globals::classInteger))
        return env.callIntMethod(obj, Globals::methodIntegerIntValue, nullptr);

    if (env.isSameObject(jc, Globals::classLong))
        return env.callLongMethod(obj, Globals::methodLongLongValue, nullptr);

    if (env.isSameObject(jc, Globals::classShort))
        return env.callShortMethod(obj, Globals::methodShortShortValue, nullptr);

    if (env.isSameObject(jc, Globals::classByte))
        return env.callByteMethod(obj, Globals::methodByteByteValue, nullptr);

    if (env.isSameObject(jc, Globals::classBoolean))
        return (bool)env.callBooleanMethod(obj, Globals::methodBooleanBooleanValue, nullptr);

    if (env.isSameObject(jc, Globals::classDouble))
        return (double)env.callDoubleMethod(obj, Globals::methodDoubleDoubleValue, nullptr);

    if (env.isSameObject(jc, Globals::classFloat))
        return (double)env.callFloatMethod(obj, Globals::methodFloatFloatValue, nullptr);

    if (env.isSameObject(jc, Globals::classCharacter))
        return (int64)env.callCharMethod(obj, Globals::methodCharacterCharValue, nullptr);

    return new QoreObject(qjcm.findCreateQoreClass(jc), getProgram(), new QoreJniPrivateData(obj));
}

Class* QoreJniClassMap::loadClass(const char* name, bool& base) {
    // issue #3171: do not assume that all classes retrieved with the JNI LoadClass function are in the base classloader
    base = !strncmp(name, "java/", 5)
        || !strncmp(name, "sun/", 4)
        || !strncmp(name, "javax/", 6);

    try {
        // first we try to load with the builtin classloader
        return Functions::loadClass(name);
    } catch (jni::JavaException& e) {
        JniExternalProgramData* jpc = static_cast<JniExternalProgramData*>(getProgram()->getExternalData("jni"));
        if (!jpc) {
            printd(LogLevel, "loadClass() '%s' base: %d failed, no program data\n", name, base);
            throw;
        }
        Env env;
        // if this fails, then we try our classloader that supports dynamic classpaths
        e.ignore();
        base = false;
        QoreString nname(name);
        nname.replaceAll("/", ".");
        printd(LogLevel, "QoreJniClassMap::loadClass() '%s'\n", nname.c_str());
        LocalReference<jstring> jname = env.newString(nname.c_str());
        jvalue jarg;
        jarg.l = jname;

        try {
            LocalReference<jclass> c = env.callObjectMethod(jpc->getClassLoader(), Globals::methodQoreURLClassLoaderLoadClass, &jarg).as<jclass>();
            //printd(LogLevel, "QoreJniClassMap::loadClass() program-specific '%s': %p\n", nname.c_str(), *c);
            return new Class(c.release());
        } catch (jni::JavaException& e) {
            LocalReference<jthrowable> je = e.save();
            // try to load from any thread context class loader
            LocalReference<jobject> thread = env.callStaticObjectMethod(Globals::classThread, Globals::methodThreadCurrentThread, nullptr);
            LocalReference<jobject> cl = env.callObjectMethod(thread, Globals::methodThreadGetContextClassLoader, nullptr);
            if (!cl) {
                e.restore(je.release());
                throw;
            }
            LocalReference<jclass> c = env.callObjectMethod(cl, Globals::methodClassLoaderLoadClass, &jarg).as<jclass>();
            printd(LogLevel, "QoreJniClassMap::loadClass() thread-local '%s': %p\n", nname.c_str(), *c);
            return new Class(c.release());
        }
    }
}

JniQoreClass* QoreJniClassMap::findCreateQoreClass(LocalReference<jclass>& jc) {
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
    bool base = (baseClassLoader && env.isSameObject(baseClassLoader, cl)) || (!baseClassLoader && !cl);
    return findCreateQoreClass(cname, jpath.c_str(), new Class(jc), base);
}

JniQoreClass* QoreJniClassMap::findCreateQoreClassInProgram(QoreString& name, const char* jpath, Class* c) {
    SimpleRefHolder<Class> cls(c);

    printd(LogLevel, "QoreJniClassMap::findCreateQoreClassInProgram() looking up: '%s'\n", jpath);

    // we always grab the global JNI lock first because we might need to add base classes
    // while setting up the class loaded with the jni module's classloader, and we need to
    // ensure that these locks are always acquired in order
    QoreJniAutoLocker al(m);

    // check current Program's namespace
    JniExternalProgramData* jpc = static_cast<JniExternalProgramData*>(getProgram()->getExternalData("jni"));
    if (!jpc)
        throw BasicException("could not attach to deleted Qore Program");

    JniQoreClass* qc = jpc->find(jpath);
    if (qc)
        return qc;

    // grab current Program's parse lock before manipulating namespaces
    CurrentProgramRuntimeExternalParseContextHelper pch;
    if (!pch)
        throw BasicException("could not attach to deleted Qore Program");

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

    qc = new JniQoreClass(sn, name.c_str());
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
        return rv;
    }

    // first try to load the class if possible
    bool base;
    SimpleRefHolder<Class> cls(loadClass(jpath.c_str(), base));

    QoreString cname(name);
    // create the class in the correct namespace
    return findCreateQoreClass(cname, jpath.c_str(), cls.release(), base);
}

JniQoreClass* QoreJniClassMap::findCreateQoreClassInBase(QoreString& name, const char* jpath, Class* c) {
    SimpleRefHolder<Class> cls(c);

    printd(LogLevel, "QoreJniClassMap::findCreateQoreClassInBase() looking up: '%s'\n", jpath);

    // we need to protect access to the default namespace and class map with a lock
    QoreJniAutoLocker al(m);

    // if we have the QoreClass already, then return it
    JniQoreClass* qc = find(jpath);
    if (qc)
        return qc;

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

    qc = new JniQoreClass(sn, name.c_str());
    assert(qc->isSystem());
    createClassInNamespace(ns, *default_jns, jpath, cls.release(), qc, *this);

    // now add to the current Program's namespace
    QoreProgram* pgm = getProgram();
    if (pgm) {
        JniExternalProgramData* jpc = static_cast<JniExternalProgramData*>(pgm->getExternalData("jni"));
        if (jpc) {
            // grab current Program's parse lock before manipulating namespaces
            CurrentProgramRuntimeExternalParseContextHelper pch;
            if (!pch)
                throw BasicException("could not attach to deleted Qore Program");

            ns = jni_find_create_namespace(*jpc->getJniNamespace(), name.c_str(), sn);

            // copy class for assignment
            qc = new JniQoreClass(*qc);
            assert(qc->isSystem());

            printd(LogLevel, "QoreJniClassMap::findCreateQoreClassInBase() '%s' qc: %p ns: %p '%s::%s'\n", jpath, qc, ns, ns->getName(), qc->getName());

            // save class in namespace
            ns->addSystemClass(qc);
        }
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

    // add to class maps
    map.add(jpath, static_cast<JniQoreClass*>(qc_holder.release()));

    addSuperClasses(qc, jc, jpath);

    // add methods after parents
    if (init_done) {
        populateQoreClass(*qc, jc);
    }

    printd(LogLevel, "QoreJniClassMap::createClassInNamespace() '%s' returning qc: %p ns: %p -> '%s::%s'\n", jpath, qc, ns, ns->getName(), qc->getName());

    // save class in namespace
    ns->addSystemClass(qc);

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
        QoreProgram* pgm = getProgram();
        if (pgm) {
            JniExternalProgramData* jpc = static_cast<JniExternalProgramData*>(getProgram()->getExternalData("jni"));
            if (jpc) {
                assert(static_cast<QoreJniClassMapBase*>(jpc) != static_cast<QoreJniClassMapBase*>(this));
                qc = jpc->find(jname.c_str());
            }
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
      self->setPrivate(qmeth.getClass()->getID(), new QoreJniPrivateData(m->newQoreInstance(args)));
   }
   catch (jni::Exception& e) {
      e.convert(xsink);
   }
}

static QoreValue exec_java_static_method(const QoreMethod& meth, BaseMethod* m, const QoreListNode* args, q_rt_flags_t rtflags, ExceptionSink* xsink) {
    try {
        return m->invokeStatic(args);
    } catch (jni::Exception& e) {
        e.convert(xsink);
        return QoreValue();
    }
}

static QoreValue exec_java_method(const QoreMethod& meth, BaseMethod* m, QoreObject* self, QoreJniPrivateData* jd, const QoreListNode* args, q_rt_flags_t rtflags, ExceptionSink* xsink) {
    // set Program context
    assert(self->getProgram());
    QoreProgramContextHelper pch(self->getProgram());

    try {
        return m->invokeNonvirtual(jd->getObject(), args);
    }
    catch (jni::Exception& e) {
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
         }
         else
            qc.addBuiltinStaticVar(fname.c_str(), v, field->getAccess(), fieldTypeInfo);
      }
      else {
         printd(LogLevel, "+ adding field %s %s %s.%s\n", access_str(field->getAccess()), typeInfoGetName(fieldTypeInfo), qc.getName(), fname.c_str());
         qc.addMember(fname.c_str(), field->getAccess(), fieldTypeInfo);
      }
   }
}

JniExternalProgramData::JniExternalProgramData(QoreNamespace* n_jni) : jni(n_jni) {
    assert(jni);
    Env env(false);

    // set up QoreURLClassLoader constructor args
    jvalue jarg;
    jarg.j = (long)getProgram();
    assert(jarg.j);

    // create our custom classloader
    classLoader = env.newObject(Globals::classQoreURLClassLoader, Globals::ctorQoreURLClassLoader, &jarg).makeGlobal();

    // setup classpath
    TempString classpath(SystemEnvironment::get("QORE_JNI_CLASSPATH"));
    if (classpath) {
        addClasspath(classpath->c_str());
    }
}

JniExternalProgramData::JniExternalProgramData(const JniExternalProgramData& parent, QoreProgram* pgm) :
    // reuse the same classLoader as the parent
    classLoader(GlobalReference<jobject>::fromLocal(parent.classLoader.toLocal())) {
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
   }
   catch (jni::Exception& e) {
      // display exception info on the console as an unhandled exception
      ExceptionSink xsink;
      e.convert(&xsink);
   }
}

void JniExternalProgramData::setContext(Env& env) {
    QoreProgram* pgm = getProgram();
    assert(pgm);
    JniExternalProgramData* jpc = static_cast<JniExternalProgramData*>(pgm->getExternalData("jni"));
    // issue #3153: no context is available when called from a static method
    if (jpc) {
        // set classloader context in new thread
        env.callVoidMethod(jpc->classLoader, Globals::methodQoreURLClassLoaderSetContext, nullptr);
        //printd(LogLevel, "JniExternalProgramData::setContext() pgm: %p jpc: %p\n", pgm, jpc);
    }
}

bool JniExternalProgramData::compatTypes() {
   QoreProgram* pgm = getProgram();
   // no program context when called from the cmd-line
   if (!pgm) {
       return jni_compat_types;
   }
   JniExternalProgramData* jpc = static_cast<JniExternalProgramData*>(pgm->getExternalData("jni"));
   return jpc ? jpc->getCompatTypes() : jni_compat_types;
}
}
