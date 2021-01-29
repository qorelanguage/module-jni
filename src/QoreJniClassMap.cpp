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
#include <set>

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
typedef std::set<std::string> strset_t;

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
//QoreJniClassMap::q2jmap_t QoreJniClassMap::q2jmap;

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

static std::string get_class_hash(const QoreClass& qc) {
    SimpleRefHolder<BinaryNode> b(qc.getBinaryHash());
    std::string rv(reinterpret_cast<const char*>(b->getPtr()), b->size());
    assert(rv.size() == 20);

    /*
    QoreString hex;
    hex.concatHex(*b);
    printd(5, "get_class_hash() '%s': <%s>\n", qc.getName(), hex.c_str());
    */

    /*
    std::string name = qc.getNamespacePath();
    rv.insert(0, name);
    */

    return rv;
}

QoreProgram* jni_get_program_context() {
    QoreProgram* pgm;
    jni_get_context(pgm);
    return pgm;
}

JniExternalProgramData* jni_get_context() {
    QoreProgram* pgm;
    return jni_get_context(pgm);
}

JniExternalProgramData* jni_get_context(QoreProgram*& pgm) {
    JniExternalProgramData* jpc;

    // first try to get the actual Program context
    pgm = qore_get_call_program_context();
    //pgm = getProgram();
    if (pgm) {
        jpc = static_cast<JniExternalProgramData*>(pgm->getExternalData("jni"));
        if (jpc) {
            return jpc;
        }
    }
    pgm = getProgram();
    //pgm = qore_get_call_program_context();
    if (pgm) {
        jpc = static_cast<JniExternalProgramData*>(pgm->getExternalData("jni"));
        if (jpc) {
            return jpc;
        }
    }
    pgm = nullptr;
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

void QoreJniClassMap::staticInitBackground(ExceptionSink* xsink, void* pgm_ptr) {
    QoreProgram* pgm = static_cast<QoreProgram*>(pgm_ptr);
    // set program context for initialization
    QoreProgramContextHelper pgm_ctx(pgm);

    // ensure that the parent thread is signaled on exit
    InitSignaler signaler;
    try {
        qjcm.initBackground(pgm);
    } catch (jni::Exception& e) {
        e.convert(xsink);
    }
}

void QoreJniClassMap::init(bool already_initialized) {
    QoreProgram* pgm = jni_get_program_context();
    assert(pgm);
    if (already_initialized) {
        qjcm.initBackground(pgm);
        return;
    }
    // grab init mutex
    std::unique_lock<std::mutex> init_lock(init_mutex);

    // issue #3199: perform initialization in the background
    ExceptionSink xsink;
    q_start_thread(&xsink, &staticInitBackground, pgm);

    // wait for initialization to complete
    init_cond.wait(init_lock);

    // if the background thread threw an exception, then rethrow it here
    if (xsink) {
        throw XsinkException(xsink);
    }
}

void QoreJniClassMap::initBackground(QoreProgram* pgm) {
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

    QC_OBJECT = new JniQoreClass(pgm, "Object", "java.lang.Object");
    CID_OBJECT = QC_OBJECT->getID();
    createClassInNamespace(ns, *default_jns, "java/lang/Object", Functions::loadClass("java/lang/Object"), QC_OBJECT, *this, pgm);

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
            populateQoreClass(*i, static_cast<Class*>(i->getManagedUserData()), pgm);
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
    qt2jmap[NT_INT] = GlobalReference<jclass>((jclass)Globals::classPrimitiveInt);
    qt2jmap[NT_FLOAT] = GlobalReference<jclass>((jclass)Globals::classPrimitiveDouble);
    qt2jmap[NT_BOOLEAN] = GlobalReference<jclass>((jclass)Globals::classPrimitiveBoolean);
    qt2jmap[NT_STRING] = env.findClass("java/lang/String").makeGlobal();
    qt2jmap[NT_DATE] = env.findClass("java/time/ZonedDateTime").makeGlobal();
    qt2jmap[NT_NUMBER] = env.findClass("java/math/BigDecimal").makeGlobal();
    qt2jmap[NT_BINARY] = GlobalReference<jclass>((jclass)Globals::arrayClassByte);
    qt2jmap[NT_HASH] = GlobalReference<jclass>((jclass)Globals::classHash);
    qt2jmap[NT_LIST] = env.findClass("[Ljava/lang/Object;").makeGlobal();
    qt2jmap[NT_NOTHING] = GlobalReference<jclass>((jclass)Globals::classPrimitiveVoid);
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
            printd(5, "failed to load class '%s' with default classloader; no program-specific classloader present", name);
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
        LocalReference<jclass> c = env.callObjectMethod(jpc->getClassLoader(),
            Globals::methodQoreURLClassLoaderLoadResolveClass, &jarg).as<jclass>();

        return new Class(c.release());
    } catch (jni::JavaException& e) {
        printd(LogLevel, "QoreJniClassMap::loadProgramClass() '%s' LOCAL FAILED\n", name);
        LocalReference<jthrowable> je = e.save();
        // try to load from any thread context class loader
        LocalReference<jobject> thread = env.callStaticObjectMethod(Globals::classThread,
            Globals::methodThreadCurrentThread, nullptr);
        LocalReference<jobject> cl = env.callObjectMethod(thread, Globals::methodThreadGetContextClassLoader,
            nullptr);
        printd(LogLevel, "QoreJniClassMap::loadProgramClass() '%s' thread local class loader: %d\n", name,
            cl ? 1 : 0);
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
    bool base = (!baseClassLoader && !cl) || (cl && baseClassLoader && env.isSameObject(baseClassLoader, cl));
    printd(5, "QoreJniClassMap::findCreateQoreClass() '%s' base: %d\n", jpath.c_str(), base);
    return findCreateQoreClass(cname, jpath.c_str(), new Class(jc), base);
}

JniQoreClass* QoreJniClassMap::findCreateQoreClassInProgram(QoreString& name, const char* jpath, Class* c, QoreProgram* pgm) {
    SimpleRefHolder<Class> cls(c);

    // we always grab the global JNI lock first because we might need to add base classes
    // while setting up the class loaded with the jni module's classloader, and we need to
    // ensure that these locks are always acquired in order
    QoreJniAutoLocker al(m);

    // check current Program's namespace
    JniExternalProgramData* jpc;
    if (!pgm) {
        jpc = jni_get_context(pgm);
        if (!jpc) {
            throw BasicException("no Java context to create Qore class");
        }
    } else {
        jpc = static_cast<JniExternalProgramData*>(pgm->getExternalData("jni"));
        if (!jpc) {
            throw BasicException("no JNI context in current Program");
        }
    }

    printd(LogLevel, "QoreJniClassMap::findCreateQoreClassInProgram() this: %p jpc: %p looking up: '%s'\n", this, jpc,
        jpath);

    JniQoreClass* qc = jpc->find(jpath);
    if (qc)
        return qc;

    printd(LogLevel, "QoreJniClassMap::findCreateQoreClassInProgram() this: %p jpc: %p '%s' not found\n", this, jpc,
        jpath);

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
    assert(!ns->findLocalClass(sn));

    qc = new JniQoreClass(pgm, sn, name.c_str());
    assert(qc->isSystem());
    createClassInNamespace(ns, *jpc->getJniNamespace(), jpath, cls.release(), qc, *jpc, pgm);

    return qc;
}

JniQoreClass* QoreJniClassMap::findCreateQoreClass(const char* name, QoreProgram* pgm) {
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
    return findCreateQoreClass(cname, jpath.c_str(), cls.release(), base, pgm);
}

JniQoreClass* QoreJniClassMap::findCreateQoreClassInBase(QoreString& name, const char* jpath, Class* c) {
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

    QoreProgram* pgm;
    JniExternalProgramData* jpc = jni_get_context(pgm);
    JniQoreClass* qc = new JniQoreClass(pgm, sn, name.c_str());
    assert(qc->isSystem());
    // createClassInNamespace() will "save" qc in the namespace
    createClassInNamespace(ns, *default_jns, jpath, cls.release(), qc, *this);

    // now add to the current Program's namespace
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

        printd(LogLevel, "QoreJniClassMap::findCreateQoreClassInBase() jpc: %p '%s' qc: %p ns: %p '%s::%s'\n", jpc,
            jpath, new_qc.get(), ns, ns->getName(), qc->getName());

        assert(new_qc->getManagedUserData());

        // create entry for class in map
        jpc->add(jpath, new_qc.get());

        // save class in namespace
        qc = new_qc.release();
        ns->addSystemClass(qc);
    }

    return qc;
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

JniQoreClass* QoreJniClassMap::createClassInNamespace(QoreNamespace* ns, QoreNamespace& jns, const char* jpath,
        Class* jc, JniQoreClass* qc, QoreJniClassMapBase& map, QoreProgram* pgm) {
    QoreClassHolder qc_holder(qc);
    // save pointer to java class info in JniQoreClass
    qc->setManagedUserData(jc);

    int mods = jc->getModifiers();
    if (mods & JVM_ACC_FINAL) {
        qc->setFinal();
    }

    printd(LogLevel, "QoreJniClassMap::createClassInNamespace() qc: %p ns: %p '%s::%s'\n", qc, ns, ns->getName(),
        qc->getName());

    assert(qc->getManagedUserData());

    // add to class maps
    map.add(jpath, static_cast<JniQoreClass*>(qc_holder.release()));

    addSuperClasses(qc, jc, jpath, pgm);

    // add methods after parents
    if (init_done) {
        populateQoreClass(*qc, jc, pgm);
    }

    // save class in namespace
    ns->addSystemClass(qc);

    // FIXME: DELETE!
    // Java should not ask us to generate classes that already exist in Java
    /*
    std::string cls_hash = get_class_hash(*qc);
    q2jmap_t::iterator i = q2jmap.lower_bound(cls_hash);
    if (i == q2jmap.end() || i->first != cls_hash) {
        q2jmap.insert(i, q2jmap_t::value_type(cls_hash, jc->getJavaObject()));
    }
    */

    printd(LogLevel, "QoreJniClassMap::createClassInNamespace() '%s' returning qc: %p ns: %p -> '%s::%s'\n", jpath,
        qc, ns, ns->getName(), qc->getName());

    return qc;
}

void QoreJniClassMap::addSuperClasses(JniQoreClass* qc, Class* jc, const char* jpath, QoreProgram* pgm) {
    Class* parent = jc->getSuperClass();

    printd(5, "QoreJniClassMap::createClassInNamespace() '%s' parent: %p\n", jpath, parent);

    Env env;

    // add superclass
    if (parent) {
        addSuperClass(env, *qc, parent, false, pgm);
    } else if (qc == QC_OBJECT) {
        // set base class loader: the return value for Class.getClassLoader() with classes loaded by the bootstrap
        // class loader is implementation-dependent; it's possible that this will be nullptr
        LocalReference<jobject> cl = env.callObjectMethod(jc->getJavaObject(), Globals::methodClassGetClassLoader,
            nullptr);
        if (cl) {
            baseClassLoader = cl.makeGlobal();
        }
    } else { // make interface classes at least inherit Object
        qc->addBuiltinVirtualBaseClass(QC_OBJECT);
    }

    // get and process interfaces
    LocalReference<jobjectArray> interfaceArray = jc->getInterfaces();

    for (jsize i = 0, e = env.getArrayLength(interfaceArray); i < e; ++i) {
        addSuperClass(env, *qc, new Class(env.getObjectArrayElement(interfaceArray, i).as<jclass>()), true, pgm);
    }
}

void QoreJniClassMap::addSuperClass(Env& env, JniQoreClass& qc, jni::Class* parent, bool interface, QoreProgram* pgm) {
    // see if the parent class wraps a Qore class
    jclass qoreJavaClassBase = (jclass)Globals::classQoreJavaClassBase;

    if (!interface && parent->getJavaObject() != qoreJavaClassBase) {
        jvalue jarg;
        jarg.l = parent->getJavaObject();
        if (env.callBooleanMethod(qoreJavaClassBase, Globals::methodClassIsAssignableFrom, &jarg)) {
            // get class field
            bool throw_exception = false;
            QoreClass* qore_parent;
            try {
                jfieldID class_field = env.getStaticField(parent->getJavaObject(), "qore_cls_ptr", "J");
                qore_parent = reinterpret_cast<QoreClass*>(
                    env.getStaticLongField(parent->getJavaObject(), class_field)
                );
                printd(5, "QoreJniClassMap::addSuperClass() Java class '%s' (%d) has Qore parent '%s' (%d)\n",
                    qc.getName(), qc.getID(), qore_parent->getName(), qore_parent->getID());

                if (qore_parent->isFinal()) {
                    throw_exception = true;
                } else {
                    qc.addBaseClass(qore_parent, true);
                    qc.addBuiltinVirtualBaseClass(QC_OBJECT);
                    return;
                }
            } catch (jni::Exception& e) {
                // ignore exceptions when the field is not found
                e.ignore();
            }
            if (throw_exception) {
                throw QoreJniException("FINAL-ERROR", "Java class '%s' cannot inherit final Qore class '%s'",
                    qc.getName(), qore_parent->getName());
            }
        }

        //assert(!env.callBooleanMethod(Globals::classQoreObjectBase, Globals::methodClassIsAssignableFrom, &jarg));
    }

    LocalReference<jstring> clsName = env.callObjectMethod(parent->getJavaObject(), Globals::methodClassGetName,
        nullptr).as<jstring>();
    Env::GetStringUtfChars chars(env, clsName);

    printd(5, "QoreJniClassMap::addSuperClass() qc: '%s' parent: '%s'\n", qc.getName(), chars.c_str());

    QoreString jpath(chars.c_str());
    jpath.replaceAll(".", "/");
    JniQoreClass* pc = find(jpath.c_str());
    if (pc) {
        parent->deref();
    } else {
        bool base;
        SimpleRefHolder<Class> cls(loadClass(jpath.c_str(), base));

        QoreString cstr(chars.c_str());
        pc = findCreateQoreClass(cstr, jpath.c_str(), cls.release(), base, pgm);
    }

    // only add if no other parent class already inherits the interface
    if (interface) {
        bool priv;
        if (qc.getClass(*pc, priv))
            return;
    }

    qc.addBuiltinVirtualBaseClass(pc);
}

void QoreJniClassMap::populateQoreClass(JniQoreClass& qc, jni::Class* jc, QoreProgram* pgm) {
    // do constructors
    doConstructors(qc, jc, pgm);

    // do methods
    doMethods(qc, jc, pgm);

    // do fields
    doFields(qc, jc, pgm);
}

void QoreJniClassMap::doConstructors(JniQoreClass& qc, jni::Class* jc, QoreProgram* pgm) {
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
        if (meth->getParamTypes(paramTypeInfo, altParamTypeInfo, *this, pgm)) {
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
                printd(LogLevel, "QoreJniClassMap::doConstructors() skipping already-created variant " \
                    "%s::constructor()\n", qc.getName());
                continue;
            }
            qc.addConstructor((void*)*meth, (q_external_constructor_t)exec_java_constructor, meth->getAccess(),
                meth->getFlags(), QDOM_UNCONTROLLED_API, altParamTypeInfo);
        }

        jc->trackMethod(meth.release());
    }
}

const QoreTypeInfo* QoreJniClassMap::getQoreType(jclass cls, const QoreTypeInfo*& altType, QoreProgram* pgm) {
    assert(!altType);
    Env env;

    // get class name
    LocalReference<jstring> clsName = env.callObjectMethod(cls, Globals::methodClassGetName, nullptr).as<jstring>();
    Env::GetStringUtfChars tname(env, clsName);

    // check for byte[]
    if (!strcmp(tname.c_str(), "[B")) {
        return binaryTypeInfo;
    }

     // do primitive types
    if (env.callBooleanMethod(cls, Globals::methodClassIsPrimitive, nullptr)) {
        jpmap_t::const_iterator i = jpmap.find(tname.c_str());
        assert(i != jpmap.end());
        return i->second.typeInfo;
    }

   // process array types
    if (env.callBooleanMethod(cls, Globals::methodClassIsArray, nullptr)) {
        return softAutoListTypeInfo;
    }

    QoreString cname(tname.c_str());
    QoreString jname(tname.c_str());
    jname.replaceAll(".", "/");

    printd(LogLevel, "QoreJniClassMap::getQoreType() class: '%s' jname: '%s'\n", cname.c_str(), jname.c_str());

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
            printd(LogLevel, "QoreJniClassMap::getQoreType() creating cname: '%s' jname: '%s'\n", cname.c_str(),
                jname.c_str());
            bool base;
            SimpleRefHolder<Class> cls(loadClass(jname.c_str(), base));
            qc = findCreateQoreClass(cname, jname.c_str(), cls.release(), base, pgm);
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
        const QoreClass& pqc = hierarchy_iterator.getParentClass();
        const JniQoreClass* jpqc = dynamic_cast<const JniQoreClass*>(&pqc);
        if (!jpqc) {
            continue;
        }
        const std::string jcname = jpqc->getJavaName();
        // do not return a generic type for the base object class
        if (jcname == "java.lang.Object") {
            continue;
        }
        i = jtmap.find(jcname.c_str());
        if (i != jtmap.end()) {
            // add to jtmap
            jtmap.insert(jtmap_t::value_type(tname.c_str(), i->second));
            //printd(LogLevel, "returning %s (%s) -> %s\n", tname.c_str(), jcname.c_str(),
            //  typeInfoGetName(i->second));
            altType = qc->getOrNothingTypeInfo();
            return i->second;
        }
    }

    return qc->getOrNothingTypeInfo();
}

void QoreJniClassMap::doMethods(JniQoreClass& qc, jni::Class* jc, QoreProgram* pgm) {
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
        if (meth->getParamTypes(paramTypeInfo, altParamTypeInfo, *this, pgm)) {
            printd(LogLevel, "+ skipping %s.%s(); unsupported parameter type for variant %d\n", qc.getName(),
                mname.c_str(), i + 1);
            continue;
        }

        // get method's return type
        const QoreTypeInfo* returnTypeInfo = meth->getReturnTypeInfo(*this, pgm);

        if (meth->isStatic()) {
            // check for duplicate signature
            const QoreMethod* qm = qc.findLocalStaticMethod(mname.c_str());
            if (qm && qm->existsVariant(paramTypeInfo)) {
                printd(LogLevel, "QoreJniClassMap::doMethods() skipping already-created static variant %s::%s()\n",
                    qc.getName(), mname.c_str());
                continue;
            }
            qc.addStaticMethod((void*)*meth, mname.c_str(), (q_external_static_method_t)exec_java_static_method,
                meth->getAccess(), meth->getFlags(), QDOM_UNCONTROLLED_API, returnTypeInfo, paramTypeInfo);

            if (!altParamTypeInfo.empty()) {
                if (qm && qm->existsVariant(altParamTypeInfo)) {
                    printd(LogLevel, "QoreJniClassMap::doMethods() skipping already-created static variant " \
                        "%s::%s()\n", qc.getName(), mname.c_str());
                    continue;
                }
                qc.addStaticMethod((void*)*meth, mname.c_str(), (q_external_static_method_t)exec_java_static_method,
                    meth->getAccess(), meth->getFlags(), QDOM_UNCONTROLLED_API, returnTypeInfo, altParamTypeInfo);
            }
        } else {
            if (mname == "copy" || mname == "constructor" || mname == "destructor" || mname == "methodGate"
                || mname == "memberNotification" || mname == "memberGate")
                mname.prepend("java_");

            // check for duplicate signature
            const QoreMethod* qm = qc.findLocalMethod(mname.c_str());
            if (qm && qm->existsVariant(paramTypeInfo)) {
                printd(LogLevel, "QoreJniClassMap::doMethods() skipping already-created variant %s::%s()\n",
                    qc.getName(), mname.c_str());
                continue;
            }

            if (meth->isAbstract()) {
                qc.addAbstractMethod(mname.c_str(), meth->getAccess(), meth->getFlags(), returnTypeInfo,
                    paramTypeInfo);
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
                    qc.addMethod((void*)*meth, mname.c_str(), (q_external_method_t)exec_java_method,
                        meth->getAccess(), meth->getFlags(), QDOM_UNCONTROLLED_API, returnTypeInfo, altParamTypeInfo);
                }
            }
        }
        jc->trackMethod(meth.release());
    }
}

static int qore_jni_get_acc_visibility(const QoreMethod& m, const QoreExternalMethodVariant& v) {
    switch (v.getAccess()) {
        case Internal: return ACC_PRIVATE;
        case Private: return ACC_PROTECTED;
        default:
            break;
    }
    return ACC_PUBLIC;
}

static LocalReference<jobject> get_type_def_from_class(Env& env, jclass jcls) {
    //printd(5, "get_type_def_from_class() jcls: %p\n", jcls);
    jvalue arg;
    arg.l = jcls;
    return env.callStaticObjectMethod(Globals::classJavaClassBuilder,
        Globals::methodJavaClassBuilderGetTypeDescriptionCls, &arg).release();
}

// returns an ArrayList<Type> object or null
LocalReference<jobject> JniExternalProgramData::getJavaParamList(Env& env, jobject class_loader, const QoreMethod& m,
        const QoreExternalMethodVariant& v, QoreProgram* pgm, unsigned& len) {
    const type_vec_t& params = v.getParamTypeList();
    len = params.size();
    if (params.empty() && (v.isAbstract() || !(v.getCodeFlags() & QCF_USES_EXTRA_ARGS))) {
        return nullptr;
    }

    printd(5, "JniExternalProgramData::getJavaParamList() %s::%s() %d param(s)\n", m.getClassName(), m.getName(),
        (int)params.size());

    // create parameter list
    LocalReference<jobject> plist = env.newObject(Globals::classArrayList, Globals::ctorArrayList, nullptr);

    for (const QoreTypeInfo* i : params) {
        LocalReference<jobject> ptype = getJavaTypeDefinition(env, class_loader, i, pgm);
        printd(5, "%s::%s(): adding %s -> %p\n", m.getClassName(), m.getName(), type_get_name(i), *ptype);

        jvalue jarg;
        jarg.l = ptype;
        env.callBooleanMethod(plist, Globals::methodArrayListAdd, &jarg);
    }
    if (v.getCodeFlags() & QCF_USES_EXTRA_ARGS) {
        // add Object... as the final parameter
        jvalue jarg;
        LocalReference<jobject> jtype(get_type_def_from_class(env, (jclass)Globals::arrayClassObject));
        jarg.l = jtype;
        env.callBooleanMethod(plist, Globals::methodArrayListAdd, &jarg);
    }

    return plist;
}

static void shorten_params(Env& env, LocalReference<jobject>& params, unsigned len) {
    jvalue jarg;
    jarg.i = (int)len;
    env.callObjectMethod(params, Globals::methodArrayListRemove, &jarg);
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

QoreJavaParamHelper::QoreJavaParamHelper(Env& env, const char* mname, jclass parent_class) : env(env), mname(mname),
        parent_class(parent_class), plist(env.newObject(Globals::classArrayList, Globals::ctorArrayList, nullptr)) {
}

void QoreJavaParamHelper::add(LocalReference<jobject>& params) {
    jvalue jarg;
    LocalReference<jobject> params_copy;
    if (params) {
        // first we need to clone the object
        params_copy = env.callObjectMethod(params, Globals::methodObjectClone, nullptr);
        jarg.l = params_copy;
    } else {
        jarg.l = nullptr;
    }
    env.callBooleanMethod(plist, Globals::methodArrayListAdd, &jarg);
}

int QoreJavaParamHelper::checkVariant(LocalReference<jobject>& params, qore_method_type_t method_type) {
    int list_size = env.callIntMethod(plist, Globals::methodArrayListSize, nullptr);
    if (!list_size) {
        return 0;
    }

    // get size of param list
    int plen = params ? env.callIntMethod(params, Globals::methodArrayListSize, nullptr) : 0;

    for (int i = 0; i < list_size; ++i) {
        // get list element
        jvalue jarg;
        jarg.i = i;
        LocalReference<jobject> params0 = env.callObjectMethod(plist, Globals::methodArrayListGet, &jarg);

        // get size of p0
        int plen0 = params0 ? env.callIntMethod(params0, Globals::methodArrayListSize, nullptr) : 0;

        // skip comparisons if the sizes are different
        if (plen != plen0) {
            printd(5, "QoreJavaParamHelper::checkVariant() IGNORING variant with plen0: %d (plen: %d " \
                "list_size: %d)\n", plen0, plen, list_size);
            continue;
        }
        printd(5, "QoreJavaParamHelper::checkVariant() CHECKING plen: %d plen0: %d (list_size: %d)\n", plen, plen0,
            list_size);

        bool match = true;
        // compare each parameter type in order
        for (int j = 0; j < plen; ++j) {
            jarg.i = j;
            LocalReference<jobject> e = env.callObjectMethod(params, Globals::methodArrayListGet, &jarg);
            LocalReference<jobject> e0 = env.callObjectMethod(params0, Globals::methodArrayListGet, &jarg);

            jarg.l = e0;
            bool equal = env.callBooleanMethod(e, Globals::methodObjectEquals, &jarg);

            /*
            // XXX DEBUG: FIXME add TypeDescription.getCanonicalName()
            LocalReference<jstring> eName = env.callObjectMethod(e, Globals::methodTypeDescriptionGetCanonicalName,
                nullptr).as<jstring>();
            LocalReference<jstring> eName0 = env.callObjectMethod(e0, Globals::methodTypeDescriptionGetCanonicalName,
                nullptr).as<jstring>();
            Env::GetStringUtfChars c(env, eName);
            Env::GetStringUtfChars c0(env, eName0);
            printd(5, "QoreJavaParamHelper::checkVariant() %s == %s (%s)\n", c.c_str(), c0.c_str(),
                equal ? "true" : "false");
            */

            if (!equal) {
                match = false;
                break;
            }
        }
        if (match) {
            printd(5, "QoreJavaParamHelper::checkVariant() SKIPPING method with plen: %d (list_size: %d)\n", plen,
                list_size);
            return -1;
        }
    }

    // if there are no matches in the list, then check base class methods of the opposite type
    if (mname) {
        try {
            // check for a conflict in a base class with a method of the opposite type
            jvalue jargs[4];
            jargs[0].l = parent_class;
            LocalReference<jstring> jname = env.newString(mname);
            jargs[1].l = jname;
            jargs[2].l = params;
            jargs[3].z = method_type == QMT_NORMAL;
            jboolean conflict = env.callStaticBooleanMethod(Globals::classJavaClassBuilder,
                Globals::methodJavaClassBuilderFindBaseClassMethodConflict, &jargs[0]);

            if (conflict) {
                printd(5, "QoreJavaParamHelper::checkVariant() SKIPPING method '%s' with matching base method; " \
                    "plen %d (list_size: %d)\n", mname, plen, list_size);
                return -1;
            }

            /*
            // convert ArrayList to array

            jvalue jargs[2];
            LocalReference<jstring> jname = env.newString(mname);
            jargs[0].l = jname;
            LocalReference<jobject> param_types = env.callObjectMethod(params, Globals::methodArrayListToArray, nullptr);
            jargs[1].l = param_types;
            LocalReference<jobject> method = env.callObjectMethod(parent_class, Globals::methodClassGetMethod, &jargs[0]);

            int mods = env.callIntMethod(method, Globals::methodMethodGetModifiers, nullptr);
            if ((method_type == QMT_NORMAL && (mods & JVM_ACC_STATIC))
                || (method_type == QMT_STATIC && !(mods & JVM_ACC_STATIC))) {
                printd(5, "QoreJavaParamHelper::checkVariant() SKIPPING method with matching base method; " \
                    "plen %d (list_size: %d)\n", plen, list_size);
                return -1;
            }
            */
        } catch (JavaException& e) {
            e.ignore();
        }
    }

    return 0;
}

int JniExternalProgramData::addConstructorVariant(Env& env, jobject class_loader, const QoreClass& qcls,
        LocalReference<jobject>& bb, const QoreMethod& m, const QoreExternalMethodVariant& v, QoreProgram* pgm,
        jclass parent_class, QoreJavaParamHelper& jph) {
    printd(5, "JniExternalProgramData::addConstructorVariant() adding Java constructor %s %s::constructor(%s) {}\n",
        v.getAccessString(), qcls.getName(), v.getSignatureText());

    // first get the params
    unsigned len;
    LocalReference<jobject> params = getJavaParamList(env, class_loader, m, v, pgm, len).release();

    while (true) {
        if (!jph.checkVariant(params, QMT_CONSTRUCTOR)) {
            std::vector<jvalue> jargs(6);
            jargs[0].l = bb;
            jargs[1].l = parent_class;
            jargs[2].j = reinterpret_cast<jlong>(&m);
            jargs[3].i = qore_jni_get_acc_visibility(m, v);
            jargs[4].l = params;
            jargs[5].z = v.getCodeFlags() & QCF_USES_EXTRA_ARGS;

            printd(5, "JniExternalProgramData::addConstructorVariant() %s %s::constructor(%s): adding (len: %d)\n",
                v.getAccessString(), qcls.getName(), v.getSignatureText(), len);
            bb = env.callStaticObjectMethod(Globals::classJavaClassBuilder,
                Globals::methodJavaClassBuilderAddConstructor, &jargs[0]);
            printd(5, "JniExternalProgramData::addConstructorVariant() bb: %p\n", (jobject)bb);

            // add to param list
            jph.add(params);
        } else {
            printd(5, "JniExternalProgramData::addConstructorVariant() %s %s::constructor(%s): " \
                "skipping duplicate variant (len: %d)\n",
                v.getAccessString(), qcls.getName(), v.getSignatureText(), len);
        }

        if (!params || !len || !check_optional_last_param(env, v, params, len)) {
            break;
        }
    }

    return 0;
}

int JniExternalProgramData::addNormalMethodVariant(Env& env, jobject class_loader, const QoreClass& qcls,
        LocalReference<jobject>& bb, const QoreMethod& m, const QoreExternalMethodVariant& v, QoreProgram* pgm,
        QoreJavaParamHelper& jph) {
    printd(5, "JniExternalProgramData::addNormalMethodVariant() adding Java normal method %s %s::%s(%s)\n",
        qore_type_get_name(v.getReturnTypeInfo()), qcls.getName(), m.getName(), v.getSignatureText());

    // first get the params
    unsigned len;
    LocalReference<jobject> params = getJavaParamList(env, class_loader, m, v, pgm, len).release();

    QoreString jname;
    if (!strcmp(m.getName(), "getClass")) {
        jname = "getQoreClass";
    }

    while (true) {
        //printd(5, "JniExternalProgramData::addNormalMethodVariant() adding Java normal method %s %s::%s(%s) " \
        //  len: %d\n", qore_type_get_name(v.getReturnTypeInfo()), qcls.getName(), m.getName(), v.getSignatureText(),
        //  len);

        if (!jph.checkVariant(params, QMT_NORMAL)) {
            std::vector<jvalue> jargs(9);
            jargs[0].l = bb;
            // rename methods that are final in java.lang.Object()
            LocalReference<jstring> mname;
            if (!jname.empty()) {
                mname = env.newString(jname.c_str());
            } else {
                mname = env.newString(m.getName());
            }
            jargs[1].l = mname;
            jargs[2].j = reinterpret_cast<jlong>(&m);
            jargs[3].j = reinterpret_cast<jlong>(&v);
            jargs[4].i = qore_jni_get_acc_visibility(m, v);
            LocalReference<jobject> return_type = getJavaTypeDefinition(env, class_loader, v.getReturnTypeInfo(), pgm);
            jargs[5].l = return_type;
            jargs[6].l = params;
            jargs[7].z = v.isAbstract();
            jargs[8].z = !v.isAbstract() && (v.getCodeFlags() & QCF_USES_EXTRA_ARGS);

            printd(5, "JniExternalProgramData::addNormalMethodVariant() %s %s::%s(%s): adding (len: %d)\n",
                qore_type_get_name(v.getReturnTypeInfo()), qcls.getName(), m.getName(), v.getSignatureText(), len);
            bb = env.callStaticObjectMethod(Globals::classJavaClassBuilder,
                Globals::methodJavaClassBuilderAddNormalMethod, &jargs[0]);
            printd(5, "JniExternalProgramData::addNormalMethodVariant() bb: %p\n", (jobject)bb);

            // add to param list
            jph.add(params);
        } else {
            printd(5, "JniExternalProgramData::addNormalMethodVariant() %s %s::%s(%s): skipping duplicate variant\n",
                qore_type_get_name(v.getReturnTypeInfo()), qcls.getName(), m.getName(), v.getSignatureText());
        }

        if (!params || !len || !check_optional_last_param(env, v, params, len)) {
            break;
        }
    }

    return 0;
}

int JniExternalProgramData::addStaticMethodVariant(Env& env, jobject class_loader,
        const QoreClass& qcls, LocalReference<jobject>& bb, const QoreMethod& m, const QoreExternalMethodVariant& v,
        QoreProgram* pgm, QoreJavaParamHelper& jph) {
    printd(5, "JniExternalProgramData::addStaticMethodVariant() adding Java method static %s %s::%s(%s)\n",
        qore_type_get_name(v.getReturnTypeInfo()), qcls.getName(), m.getName(), v.getSignatureText());

    // first get the params
    unsigned len;
    LocalReference<jobject> params = getJavaParamList(env, class_loader, m, v, pgm, len).release();

    while (true) {
        if (!jph.checkVariant(params, QMT_STATIC)) {
            std::vector<jvalue> jargs(8);
            jargs[0].l = bb;
            LocalReference<jstring> mname = env.newString(m.getName());
            jargs[1].l = mname;
            jargs[2].j = reinterpret_cast<jlong>(&m);
            jargs[3].j = reinterpret_cast<jlong>(&v);
            jargs[4].i = qore_jni_get_acc_visibility(m, v);
            LocalReference<jobject> return_type = getJavaTypeDefinition(env, class_loader, v.getReturnTypeInfo(), pgm);
            jargs[5].l = return_type;
            jargs[6].l = params;
            jargs[7].z = v.getCodeFlags() & QCF_USES_EXTRA_ARGS;

            printd(5, "JniExternalProgramData::addStaticMethodVariant() static %s %s::%s(%s): adding (len: %d)\n",
                qore_type_get_name(v.getReturnTypeInfo()), qcls.getName(), m.getName(), v.getSignatureText(), len);
            bb = env.callStaticObjectMethod(Globals::classJavaClassBuilder,
                Globals::methodJavaClassBuilderAddStaticMethod, &jargs[0]);
            printd(5, "JniExternalProgramData::addStaticMethodVariant() bb: %p\n", (jobject)bb);

            // add to param list
            jph.add(params);
        } else {
            printd(5, "JniExternalProgramData::addStaticMethodVariant() static %s %s::%s(%s): skipping duplicate " \
                "variant (len: %d)\n", qore_type_get_name(v.getReturnTypeInfo()), qcls.getName(), m.getName(),
                v.getSignatureText(), len);
        }

        if (!params || !len || !check_optional_last_param(env, v, params, len)) {
            break;
        }
    }

    return 0;
}

int JniExternalProgramData::addStaticMethods(Env& env, jobject class_loader,
        const QoreClass& qcls, const QoreMethod& m, QoreJavaParamHelper& jph, LocalReference<jobject>& bb,
        QoreProgram* pgm) {
    QoreExternalFunctionIterator vi(*m.getFunction());
    while (vi.next()) {
        const QoreExternalMethodVariant* v = reinterpret_cast<const QoreExternalMethodVariant*>(vi.getVariant());
        printd(5, "JniExternalProgramData::addStaticMethods() %s::%s(%s)\n",
            qcls.getName(), m.getName(), v->getSignatureText());
        assert(m.getMethodType() == MT_Static);
        if (addStaticMethodVariant(env, class_loader, qcls, bb, m, *v, pgm,
            jph)) {
            return -1;
        }
    }
    return 0;
}

int JniExternalProgramData::addMethods(Env& env, jobject class_loader, const QoreClass& qcls,
        LocalReference<jobject>& bb, QoreProgram* pgm, jclass parent_class) {
    // map of static methods already provisioned
    strset_t static_methods;

    {
        QoreMethodIterator i(qcls);
        unsigned constructor_count = 0;
        while (i.next()) {
            const QoreMethod* m = i.getMethod();

            switch (m->getMethodType()) {
                case MT_Constructor: {
                    QoreExternalFunctionIterator vi(*m->getFunction());
                    QoreJavaParamHelper jph(env, nullptr, parent_class);
                    while (vi.next()) {
                        const QoreExternalMethodVariant* v =
                            reinterpret_cast<const QoreExternalMethodVariant*>(vi.getVariant());
                        printd(5, "JniExternalProgramData::addMethods() constructor: %s::%s(%s)\n",
                            qcls.getName(), m->getName(), v->getSignatureText());

                        if (addConstructorVariant(env, class_loader, qcls, bb, *m, *v, pgm, parent_class, jph)) {
                            return -1;
                        }
                        ++constructor_count;
                    }
                    break;
                };

                case MT_Normal: {
                    QoreExternalFunctionIterator vi(*m->getFunction());
                    QoreJavaParamHelper jph(env, m->getName(), parent_class);
                    while (vi.next()) {
                        const QoreExternalMethodVariant* v =
                            reinterpret_cast<const QoreExternalMethodVariant*>(vi.getVariant());
                        printd(5, "JniExternalProgramData::addMethods() normal method: %s::%s(%s)\n",
                            qcls.getName(), m->getName(), v->getSignatureText());

                        if (addNormalMethodVariant(env, class_loader, qcls, bb, *m, *v, pgm, jph)) {
                            return -1;
                        }
                    }

                    // find any static method with the same name and process here to ensure that no arguments conflict
                    const QoreMethod* sm = qcls.findStaticMethod(m->getName());
                    if (sm) {
                        if (addStaticMethods(env, class_loader, qcls, *sm, jph, bb, pgm)) {
                            return -1;
                        }
                        assert(static_methods.find(sm->getName()) == static_methods.end());
                        static_methods.insert(sm->getName());
                    }
                    break;
                }

                default: {
                    printd(5, "JniExternalProgramData::addMethods() ignoring method %s::%s(...) " \
                        "(all variants)\n", qcls.getName(), m->getName());
                    break;
                }
            }
        }

        // add default constructor if necessary
        if (!constructor_count) {
            std::vector<jvalue> jargs(6);
            jargs[0].l = bb;
            jargs[1].l = parent_class;
            jargs[2].j = 0;
            jargs[3].i = ACC_PROTECTED;
            jargs[4].l = nullptr;
            jargs[5].z = false;

            bb = env.callStaticObjectMethod(Globals::classJavaClassBuilder, Globals::methodJavaClassBuilderAddConstructor,
                &jargs[0]);
            printd(5, "JniExternalProgramData::addMethods() %p: %s: bb: %p (default constructor)\n", &qcls,
                qcls.getName(), (jobject)bb);
        }
    }

    QoreStaticMethodIterator i(qcls);
    while (i.next()) {
        const QoreMethod* m = i.getMethod();
        if (static_methods.find(m->getName()) != static_methods.end()) {
            continue;
        }

        QoreJavaParamHelper jph(env, m->getName(), parent_class);
        if (addStaticMethods(env, class_loader, qcls, *m, jph, bb, pgm)) {
            return -1;
        }
    }

    return 0;
}

LocalReference<jobject> JniExternalProgramData::getCreateJavaClass(Env& env, jobject class_loader,
        const Env::GetStringUtfChars& qpath, QoreProgram* pgm, jstring jname, jboolean need_byte_code) {
    ExceptionSink xsink;
    // set program context (and read lock) before calling QoreProgram::findClass()
    QoreExternalProgramContextHelper pch(&xsink, pgm);
    if (xsink) {
        throw XsinkException(xsink);
    }

    const QoreClass* qcls = pgm->findClass(qpath.c_str(), &xsink);
    if (xsink) {
        assert(!qcls);
        throw XsinkException(xsink);
    }
    printd(5, "JniExternalProgramData::getCreateJavaClass() qpath: '%s'\n", qpath.c_str());
    if (!qcls) {
        // get java name for error message
        Env::GetStringUtfChars java_name(env, jname);
        ReferenceHolder<QoreListNode> feature_list(pgm->getFeatureList(), &xsink);
        QoreStringMaker desc("Java class '%s' cannot be generated, because Qore class '%s' cannot be found; loaded " \
            "modules: ", java_name.c_str(), qpath.c_str());
        ConstListIterator i(*feature_list);
        while (i.next()) {
            desc.sprintf("%s, ", i.getValue().get<const QoreStringNode>()->c_str());
        }
        desc.terminate(desc.size() - 2);
        desc.concat(')');
        env.throwNew(env.findClass("java/lang/ClassNotFoundException"), desc.c_str());
        return nullptr;
    }

    //printd(5, "JniExternalProgramData::getCreateJavaClass() qpath: '%s' (%p) nbc: %d\n", qpath.c_str(), qcls,
    //    need_byte_code);
    // ensure exclusive access while creating java classes
    QoreJniAutoLocker al(QoreJniClassMap::m);
    bool pending;
    return getCreateJavaClassIntern(env, class_loader, qcls, pgm, pending, jname, need_byte_code);
}

template<typename T>
static LocalReference<jobject> get_qore_java_dynamic_class_data(Env& env, T& cls) {
    // build QoreJavaDynamicClassData object
    jvalue jargs[2];
    jargs[0].l = cls;
    jargs[1].l = nullptr;
    return env.newObject(Globals::classQoreJavaDynamicClassData, Globals::ctorQoreJavaDynamicClassData,
        &jargs[0]);
}

static LocalReference<jstring> get_binary_name_for_class(Env& env, const QoreClass& qc) {
    std::string pname = qc.getNamespacePath(true);
    size_t start_pos = 0;
    while ((start_pos = pname.find("::", start_pos)) != std::string::npos) {
        pname.replace(start_pos, 2, ".");
        ++start_pos;
    }
    pname.insert(0, "qore");
    printd(5, "get_binary_name_for_class() cls '%s' -> java '%s'\n", qc.getName(), pname.c_str());
    return env.newString(pname.c_str());
}

LocalReference<jobject> JniExternalProgramData::getCreateJavaClass(Env& env, const QoreClass& qc,
        jobject class_loader, bool need_byte_code) {
    // get binary name from class name
    jvalue jargs[2];
    LocalReference<jstring> bin_name = get_binary_name_for_class(env, qc);
    jargs[0].l = bin_name;
    jargs[1].z = need_byte_code;
    return env.callObjectMethod(class_loader,
        Globals::methodQoreURLClassLoaderCreateJavaQoreClass, &jargs[0]);
}

LocalReference<jobject> JniExternalProgramData::getCreateJavaClassIntern(Env& env, jobject class_loader,
        const QoreClass* qcls, QoreProgram* pgm, bool& pending, jstring jname, jboolean need_byte_code) {
    pending = false;
    // look in q2jmap first if byte code not needed
    bool found = false;
    std::string cls_hash = get_class_hash(*qcls);
    q2jmap_t::iterator i = q2jmap.lower_bound(cls_hash);
    if (i != q2jmap.end() && i->first == cls_hash) {
        if (!need_byte_code) {
            return get_qore_java_dynamic_class_data<GlobalReference<jclass>>(env, i->second);
        }
        found = true;
    }

    if (!found) {
        // insert a placeholder for the current class
        LocalReference<jclass> tmp = Globals::classObject.toLocal();
        i = q2jmap.insert(i, q2jmap_t::value_type(cls_hash, tmp.makeGlobal()));
    }
    try {
        // get parent class
        LocalReference<jclass> parent_class;
        jclass parent_ptr = nullptr;
        // get single base class - Java and Qore's inheritance models are not compatible
        // we can only set a single class for the Java base class
        {
            QoreParentClassIterator ci(*qcls);
            while (ci.next()) {
                if (ci.getAccess() > Private) {
                    continue;
                }

                LocalReference<jobject> parent_class_data = getCreateJavaClass(env, ci.getParentClass(), class_loader,
                    false);
                parent_class = env.getObjectField(parent_class_data,
                    Globals::fieldQoreJavaDynamicClassDataCls).as<jclass>();
                parent_ptr = (jclass)parent_class;
                printd(5, "JniExternalProgramData::getCreateJavaClassIntern() cls: '%s' parent: '%s'\n",
                    qcls->getName(), ci.getParentClass().getName());

                // XXX DEBUG
                LocalReference<jstring> clsName = env.callObjectMethod(parent_ptr, jni::Globals::methodClassGetName,
                    nullptr).as<jstring>();
                Env::GetStringUtfChars cname(env, clsName);

                // FIXME! fix recursive class resolution in generated classes
                if (!strcmp(cname.c_str(), "java.lang.Object")) {
                    printd(5, "JniExternalProgramData::getCreateJavaClassIntern() ERROR cls: '%s' " \
                        "parent: '%s' => '%s'\n", qcls->getName(), ci.getParentClass().getName(), cname.c_str());
                    //assert(false);
                    parent_ptr = (jclass)Globals::classQoreJavaClassBase;
                }

                break;
            }
        }
        if (!parent_ptr) {
            parent_ptr = (jclass)Globals::classQoreJavaClassBase;
            printd(5, "JniExternalProgramData::getCreateJavaClassIntern() cls: '%s' parent: QoreBaseClass\n",
                qcls->getName());
        }

        jlong cptr = reinterpret_cast<jlong>(qcls);

        //printd(5, "JniExternalProgramData::getCreateJavaClassIntern() p: %p path: '%s': %p (abstract: %d) " \
        //  jparent: %p\n", pgm, qcls->getName(), cptr, qcls->isAbstract(), parent_ptr);

        LocalReference<jstring> njname;
        if (!jname) {
            njname = get_binary_name_for_class(env, *qcls);
            //printd(5, "JniExternalProgramData::getCreateJavaClassIntern() cls '%s' -> java '%s' (generated)\n",
            //    qcls->getName(), pname.c_str());
            jname = njname;
        }

        std::vector<jvalue> jargs(4);
        jargs[0].l = jname;
        jargs[1].l = parent_ptr;
        jargs[2].z = qcls->isAbstract();
        jargs[3].j = cptr;

        LocalReference<jobject> bb = env.callStaticObjectMethod(Globals::classJavaClassBuilder,
            Globals::methodJavaClassBuilderGetClassBuilder, &jargs[0]);
        printd(5, "JniExternalProgramData::getCreateJavaClassIntern() bb: %p\n", (jobject)bb);

        // add methods
        if (addMethods(env, class_loader, *qcls, bb, pgm, parent_ptr)) {
            if (!found) {
                q2jmap.erase(i);
                //printd(5, "JniExternalProgramData::getCreateJavaClassIntern() DELETED TEMP FOR FAILED QORE '%s'\n",
                //  qcls->getName());
            }
            //printd(5, "JniExternalProgramData::getCreateJavaClassIntern() failed to add members\n");
            return nullptr;
        }

        //printd(5, "JniExternalProgramData::getCreateJavaClassIntern() %s methods added bb: %p; building class with " \
        //    "cl: %p\n", qcls->getName(), (jobject)bb, (jobject)class_loader);

        jargs[0].l = bb;
        jargs[1].l = class_loader;
        jargs[2].l = jname;
        LocalReference<jobject> rv = env.callStaticObjectMethod(Globals::classJavaClassBuilder,
            Globals::methodJavaClassBuilderGetClassFromBuilder, &jargs[0]);

        if (!found) {
            LocalReference<jclass> cls = env.getObjectField(rv, Globals::fieldQoreJavaDynamicClassDataCls).as<jclass>();
            i->second = cls.makeGlobal();
            //printd(5, "JniExternalProgramData::getCreateJavaClassIntern() UPDATED TEMP FOR QORE '%s'\n",
            //  qcls->getName());
        }
        //printd(5, "JniExternalProgramData::getCreateJavaClassIntern() %s rv: %p\n", qcls->getName(), (jobject)rv);

        // build QoreJavaDynamicClassData object
        return rv;
    } catch (...) {
        if (!found) {
            q2jmap.erase(i);
            //printd(5, "JniExternalProgramData::getCreateJavaClassIntern() DELETED TEMP FOR EXCEPTION: QORE '%s'\n",
            //    qcls->getName());
        }
        throw;
    }
}

LocalReference<jobject> JniExternalProgramData::getJavaTypeDefinition(Env& env, jobject class_loader, const QoreTypeInfo* ti,
        QoreProgram* pgm) {
    qore_type_t t = qore_type_get_base_type(ti);
    //printd(5, "JniExternalProgramData::getJavaTypeDefinition() looking up type '%s' (%d)\n", qore_type_get_name(ti), t);
    if (t != NT_OBJECT) {
        LocalReference<jclass> jtype(QoreJniClassMap::getPrimitiveType(t));
        return get_type_def_from_class(env, jtype);
    }

    const QoreClass* cls = type_info_get_return_class(ti);
    if (!cls) {
        printd(5, "JniExternalProgramData::getJavaTypeDefinition() no mapping for '%s'\n", qore_type_get_name(ti));
        return get_type_def_from_class(env, Globals::classObject);
    }

    QoreJniAutoLocker al(QoreJniClassMap::m);

    printd(5, "JniExternalProgramData::getJavaTypeDefinition() type '%s' (%d) creating Java class for '%s' (%p)\n",
        qore_type_get_name(ti), t, cls->getName(), cls);

    bool pending;
    LocalReference<jobject> rv = getCreateJavaClassIntern(env, class_loader, cls, pgm, pending);
    // create a future type for pending classes
    if (pending) {
        jvalue arg;
        LocalReference<jstring> jname = get_binary_name_for_class(env, *cls);
        arg.l = jname;
        return env.callStaticObjectMethod(Globals::classJavaClassBuilder,
            Globals::methodJavaClassBuilderGetTypeDescriptionStr, &arg).release();
    }

    LocalReference<jclass> jtype(env.getObjectField(rv, Globals::fieldQoreJavaDynamicClassDataCls).as<jclass>());
    return get_type_def_from_class(env, jtype);
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

LocalReference<jclass> QoreJniClassMap::getPrimitiveType(qore_type_t t) {
    qt2jmap_t::const_iterator i = qt2jmap.find(t);
    LocalReference<jclass> rv = i != qt2jmap.end() ? i->second.toLocal() : Globals::classObject.toLocal();
    printd(5, "QoreJniClassMap::getPrimitiveType() type %d -> java cls %p\n", t, (jclass)rv);
    return rv;
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
    LocalReference<jclass> ccls = env.callObjectMethod(cls, Globals::methodClassGetComponentType,
        nullptr).as<jclass>();
    if (!ccls) {
        LocalReference<jstring> clsName = env.callObjectMethod(cls, Globals::methodClassGetCanonicalName,
            nullptr).as<jstring>();
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

static void exec_java_constructor(const QoreMethod& qmeth, BaseMethod* m, QoreObject* self, const QoreListNode* args,
        q_rt_flags_t rtflags, ExceptionSink* xsink) {
    try {
        // issue #3585: set context for external java threads
        JniExternalProgramData::setContext();
        self->setPrivate(qmeth.getClass()->getID(), new QoreJniPrivateData(m->newQoreInstance(args)));
    } catch (jni::Exception& e) {
        e.convert(xsink);
    }
}

static QoreValue exec_java_static_method(const QoreMethod& meth, BaseMethod* m, const QoreListNode* args,
        q_rt_flags_t rtflags, ExceptionSink* xsink) {
    try {
        // issue #3585: set context for external java threads
        JniExternalProgramData::setContext();
        return m->invokeStatic(args);
    } catch (jni::Exception& e) {
        e.convert(xsink);
        return QoreValue();
    }
}

static QoreValue exec_java_method(const QoreMethod& meth, BaseMethod* m, QoreObject* self, QoreJniPrivateData* jd,
        const QoreListNode* args, q_rt_flags_t rtflags, ExceptionSink* xsink) {
    // NOTE: Java base classes will have no Qore program context
    QoreProgram* pgm = meth.getClass()->getProgram();
    if (!pgm) {
        pgm = self->getProgram();
    }
    QoreProgramContextHelper pch(pgm);

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

void QoreJniClassMap::doFields(JniQoreClass& qc, jni::Class* jc, QoreProgram* pgm) {
    Env env;

    printd(LogLevel, "QoreJniClassMap::doFields() %s qc: %p jc: %p\n", qc.getName(), &qc, jc);

    LocalReference<jobjectArray> fArray = jc->getDeclaredFields();
    for (jsize i = 0, e = env.getArrayLength(fArray); i < e; ++i) {
        // get Field object
        LocalReference<jobject> f = env.getObjectArrayElement(fArray, i);

        SimpleRefHolder<BaseField> field(new BaseField(f, jc));

        QoreString fname;
        field->getName(fname);

        const QoreTypeInfo* fieldTypeInfo = field->getQoreTypeInfo(*this, pgm);

        if (field->isStatic()) {
            printd(LogLevel, "+ adding static field %s %s %s.%s (%s)\n", access_str(field->getAccess()),
                typeInfoGetName(fieldTypeInfo), qc.getName(), fname.c_str(), field->isFinal() ? "const" : "var");

            QoreValue v(field->getStatic());
            if (field->isFinal()) {
                if (v.isNothing())
                v.assign(0ll);
                qc.addBuiltinConstant(fname.c_str(), v, field->getAccess());
            } else
                qc.addBuiltinStaticVar(fname.c_str(), v, field->getAccess(), fieldTypeInfo);
        } else if (!qc.findLocalMember(fname.c_str())) {
            printd(LogLevel, "+ adding field %s %s %s.%s\n", access_str(field->getAccess()),
                typeInfoGetName(fieldTypeInfo), qc.getName(), fname.c_str());
            qc.addMember(fname.c_str(), field->getAccess(), fieldTypeInfo);
        }
    }
}

JniExternalProgramData::JniExternalProgramData(QoreNamespace* n_jni, QoreProgram* pgm) : jni(n_jni) {
    assert(jni);
    Env env(false);

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
        classLoader = env.newObject(Globals::classQoreURLClassLoader, Globals::ctorQoreURLClassLoader,
            &jargs[0]).makeGlobal();
    }

    {
        // define the QoreJavaDynamicApi class using our new classloader
        LocalReference<jstring> jname = env.newString("org.qore.jni.QoreJavaDynamicApi");

        // make byte array
        LocalReference<jbyteArray> jbyte_code =
            env.newByteArray(java_org_qore_jni_QoreJavaDynamicApi_class_len).as<jbyteArray>();
        for (jsize i = 0; (unsigned)i < java_org_qore_jni_QoreJavaDynamicApi_class_len; ++i) {
            env.setByteArrayElement(jbyte_code, i, java_org_qore_jni_QoreJavaDynamicApi_class[i]);
        }

        std::vector<jvalue> jargs(4);
        jargs[0].l = jname;
        jargs[1].l = jbyte_code;
        jargs[2].i = 0;
        jargs[3].i = java_org_qore_jni_QoreJavaDynamicApi_class_len;

        dynamicApi = env.callObjectMethod(classLoader, Globals::methodQoreURLClassLoaderDefineResolveClass,
            &jargs[0]).as<jclass>().makeGlobal();
        methodQoreJavaDynamicApiInvokeMethod = env.getStaticMethod(dynamicApi, "invokeMethod",
            "(Ljava/lang/reflect/Method;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
        methodQoreJavaDynamicApiInvokeMethodNonvirtual = env.getStaticMethod(dynamicApi, "invokeMethodNonvirtual",
            "(Ljava/lang/reflect/Method;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
        methodQoreJavaDynamicApiGetField = env.getStaticMethod(dynamicApi, "getField",
            "(Ljava/lang/reflect/Field;Ljava/lang/Object;)Ljava/lang/Object;");

        //printd(LogLevel, "this: %p: dynamic API created: %p classloader: %p\n", this, getDynamicApi(), getClassLoader());
    }

    // setup classpath
    TempString classpath(SystemEnvironment::get("QORE_JNI_CLASSPATH"));
    if (classpath) {
        addClasspath(classpath->c_str());
    }
}

JniExternalProgramData::JniExternalProgramData(const JniExternalProgramData& parent, Env& env, QoreProgram* pgm) :
        classLoader(nullptr),
        // reuse the same dynamic API as the parent
        dynamicApi(GlobalReference<jclass>::fromLocal(parent.dynamicApi.toLocal())),
        methodQoreJavaDynamicApiInvokeMethod(parent.methodQoreJavaDynamicApiInvokeMethod),
        methodQoreJavaDynamicApiInvokeMethodNonvirtual(parent.methodQoreJavaDynamicApiInvokeMethodNonvirtual),
        methodQoreJavaDynamicApiGetField(parent.methodQoreJavaDynamicApiGetField),
        override_compat_types(parent.override_compat_types),
        compat_types(parent.compat_types) {
    // clone the parent's classLoader
    {
        jvalue jargs[2];
        jargs[0].j = (jlong)pgm;
        jargs[1].l = parent.classLoader;
        classLoader = env.newObject(Globals::classQoreURLClassLoader, Globals::ctorQoreURLClassLoader,
            &jargs[0]).makeGlobal();
    }

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
