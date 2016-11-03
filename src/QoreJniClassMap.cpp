/* -*- indent-tabs-mode: nil -*- */
/*
  QoreJniClassMap.cpp

  Qore Programming Language JNI Module

  Copyright (C) 2016 Qore Technologies, s.r.o.

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
QoreClass* QC_OBJECT;
// the Qore class ID for java::lang::Object
qore_classid_t CID_OBJECT;
// the QoreClass for java::lang::Class
QoreClass* QC_CLASS;
// the Qore class ID for java::lang::Class
qore_classid_t CID_CLASS;
// the QoreClass for java::lang::reflect::Method
QoreClass* QC_METHOD;
// the Qore class ID for java::reflect::Method
qore_classid_t CID_METHOD;
// the QoreClass for java::lang::ClassLoader
QoreClass* QC_CLASSLOADER;
// the Qore class ID for java::lang::ClassLoader
qore_classid_t CID_CLASSLOADER;
// the QoreClass for java::lang::Throwable
QoreClass* QC_THROWABLE;
// the Qore class ID for java::lang::Throwable
qore_classid_t CID_THROWABLE;
// the QoreClass for java::lang::reflect::InvocationHandler
QoreClass* QC_INVOCATIONHANDLER;
// the Qore class ID for java::lang::reflect::InvocationHandler
qore_classid_t CID_INVOCATIONHANDLER;

QoreJniClassMap qjcm;
static void exec_java_constructor(const QoreClass& qc, BaseMethod* m, QoreObject* self, const QoreValueList* args, q_rt_flags_t rtflags, ExceptionSink* xsink);
static QoreValue exec_java_static_method(const QoreMethod& meth, BaseMethod* m, const QoreValueList* args, q_rt_flags_t rtflags, ExceptionSink* xsink);
static QoreValue exec_java_method(const QoreMethod& meth, BaseMethod* m, QoreObject* self, QoreJniPrivateData* jd, const QoreValueList* args, q_rt_flags_t rtflags, ExceptionSink* xsink);

QoreJniClassMap::jtmap_t QoreJniClassMap::jtmap = {
   {"java.lang.Object", anyTypeInfo},
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

static QoreNamespace* qjni_find_create_namespace(QoreNamespace& jns, const char* name, const char*& sn) {
   printd(LogLevel, "qjni_find_create_namespace() jns: %p '%s'\n", &jns, name);

   sn = rindex(name, '.');

   QoreNamespace* ns;
   // find parent namespace
   if (!sn) {
      ns = &jns;
      sn = name;
      printd(LogLevel, "qjni_find_create_namespace() same namespace\n");
   }
   else {
      QoreString nsp(name);
      nsp.replaceAll(".", "::");
      ++sn;
      ns = jns.findCreateNamespacePath(nsp.getBuffer());
      printd(LogLevel, "qjni_find_create_namespace() jns target: %p '%s' nsp: '%s' ns: %p '%s' new: '%s'\n", &jns, jns.getName(), nsp.c_str(), ns, ns->getName(), sn);
   }

   return ns;
}

void QoreJniClassMap::init() {
   Env env;
   // get our classloader
   classLoader = env.newObject(Globals::classQoreURLClassLoader, Globals::ctorQoreURLClassLoader, nullptr).makeGlobal();

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
   QoreNamespace* ns = qjni_find_create_namespace(*default_jns, "java.lang.Object", sn);

   QC_OBJECT = new QoreClass("Object");
   CID_OBJECT = QC_OBJECT->getID();
   createClassIntern(ns, *default_jns, "java.lang.Object", "java/lang/Object", loadClass("java/lang/Object"), QC_OBJECT);

   QC_CLASS = find("java/lang/Class");
   CID_CLASS = QC_CLASS->getID();
   QC_METHOD = find("java/lang/reflect/Method");
   CID_METHOD = QC_METHOD->getID();
   QC_CLASSLOADER = find("java/lang/ClassLoader");
   CID_CLASSLOADER = QC_CLASSLOADER->getID();
   QC_THROWABLE = find("java/lang/Throwable");
   CID_THROWABLE = QC_THROWABLE->getID();
   QC_INVOCATIONHANDLER = findCreateClass(*default_jns, "java/lang/reflect/InvocationHandler");
   CID_INVOCATIONHANDLER = QC_INVOCATIONHANDLER->getID();

   // rescan all classes
   for (auto& i : jcmap)
      i.second->rescanParents();

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
   classLoader = nullptr;
   default_jns->clear(&xsink);
   delete default_jns;
   default_jns = 0;
}

void QoreJniClassMap::addClasspath(const char* path) {
   Env env;
   LocalReference<jstring> jname = env.newString(path);
   jvalue jarg;
   jarg.l = jname;
   env.callVoidMethod(classLoader, Globals::methodQoreURLClassLoaderAddPath, &jarg);
}

Class* QoreJniClassMap::loadClass(const QoreString& name) {
   ModifiedUtf8String nameUtf8(name);
   return loadClass(nameUtf8.c_str());
}

Class* QoreJniClassMap::loadClass(const char* name) {
   Env env;
   try {
      // first we try to load with the builtin classloader
      return Functions::loadClass(name);
   }
   catch (jni::Exception& e) {
      // if this fails, then we try our classloader that supports dynamic classpaths
      e.ignore();
      QoreString nname(name);
      nname.replaceAll("/", ".");
      printd(LogLevel, "QoreJniClassMap::loadClass() '%s'\n", nname.c_str());
      LocalReference<jstring> jname = env.newString(nname.c_str());
      jvalue jarg;
      jarg.l = jname;
      LocalReference<jclass> c = env.callObjectMethod(classLoader, Globals::methodQoreURLClassLoaderLoadClass, &jarg).as<jclass>();
      printd(LogLevel, "QoreJniClassMap::loadClass() '%s': %p\n", nname.c_str(), *c);
      return new Class(c.release());
   }
}

jclass QoreJniClassMap::findLoadClass(const QoreString& name) {
   ModifiedUtf8String nameUtf8(name);
   return findLoadClass(nameUtf8.c_str());
}

jclass QoreJniClassMap::findLoadClass(const char* jpath) {
   QoreClass* qc;
   {
      QoreJniAutoLocker al(m);
      jcmap_t::iterator i = jcmap.find(jpath);
      if (i != jcmap.end()) {
         qc = i->second;
         //printd(LogLevel, "findLoadClass() '%s': %p (cached)\n", jpath, qc);
      }
      else {
         SimpleRefHolder<Class> cls(loadClass(jpath));

         QoreString cpath(jpath);
         cpath.replaceAll("/", ".");
         qc = createClass(*default_jns, cpath.c_str(), jpath, cls.release());
         //printd(LogLevel, "findLoadClass() '%s': %p (created)\n", jpath, qc);
      }
   }

   return static_cast<Class*>(qc->getManagedUserData())->toLocal();
}

QoreValue QoreJniClassMap::getValue(LocalReference<jobject>& obj) {
   Env env;

   // see if object is an array
   LocalReference<jclass> jc = env.getObjectClass(obj);

   if (env.callBooleanMethod(jc, Globals::methodClassIsArray, nullptr))
      return Array::getList(env, obj.cast<jarray>(), jc);

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

   return new QoreObject(qjcm.findCreateClass(jc), getProgram(), new QoreJniPrivateData(obj));
}

QoreClass* QoreJniClassMap::findCreateClass(jclass jc) {
   QoreClass* qc;

   Env env;
   LocalReference<jstring> clsName = env.callObjectMethod(jc, Globals::methodClassGetName, nullptr).as<jstring>();
   Env::GetStringUtfChars cname(env, clsName);

   printd(LogLevel, "QoreJniClassMap::findCreateClass() looking up: '%s'\n", cname.c_str());

   QoreString jpath(cname.c_str());
   jpath.replaceAll(".", "/");
   {
      // we need to protect access to the class map with a lock
      QoreJniAutoLocker al(m);

      qc = find(jpath.c_str());
      if (!qc) {
         printd(LogLevel, "QoreJniClassMap::findCreateClass() creating '%s' '%s'\n", cname.c_str(), jpath.c_str());

         Class* jc;
         try {
            //jc = jni::Functions::loadClass(jpath);
            jc = loadClass(jpath);
         }
         catch (jni::Exception& e) {
#ifdef DEBUG_1
            // display exception info on the console as an unhandled exception
            {
               ExceptionSink xsink;
               e.convert(&xsink);
            }
#else
            e.ignore();
#endif
            return QC_OBJECT;
         }

         qc = createClass(*default_jns, cname.c_str(), jpath.c_str(), jc);
      }
   }
   return qc;
}

QoreClass* QoreJniClassMap::findCreateClass(QoreNamespace& jns, const char* name) {
   QoreString jpath(name);
   jpath.replaceAll(".", "/");

   // we need to protect access to the class map with a lock
   QoreJniAutoLocker al(m);

   QoreClass *qc = find(jpath.c_str());

   if (qc) {
      assert(&jns != default_jns);
      // find/create parent namespace
      const char* sn;
      QoreNamespace* ns = qjni_find_create_namespace(jns, name, sn);
      qc = new QoreClass(*qc);
      // save class in namespace
      ns->addSystemClass(qc);

      printd(LogLevel, "QoreJniClassMap::findCreateClass() qc: %p ns: %p '%s::%s'\n", qc, ns, ns->getName(), sn);

      return qc;
   }

   printd(LogLevel, "QoreJniClassMap::findCreateClass() name: '%s' jstr: '%s'\n", name, jpath.c_str());
   //return createClass(jns, name, jpath.c_str(), jni::Functions::loadClass(jpath));
   return createClass(jns, name, jpath.c_str(), loadClass(jpath));
}

QoreClass* QoreJniClassMap::loadCreateClass(QoreNamespace& jns, const char* cstr) {
   assert(cstr && cstr[0]);

   QoreString jstr(cstr);
   jstr.replaceAll(".", "/");

   printd(LogLevel, "QoreJniClassMap::loadCreateClass() name: '%s' jstr: '%s'\n", cstr, jstr.c_str());
   //return createClass(jns, cstr, jstr.c_str(), jni::Functions::loadClass(jstr));
   return createClass(jns, cstr, jstr.c_str(), loadClass(jstr));
}

// creates a QoreClass and adds it in the appropriate namespace
QoreClass* QoreJniClassMap::createClass(QoreNamespace& jns, const char* name, const char* jpath, Class* jc) {
   assert(!find(jpath));

   // find/create parent namespace in default / master Jni namespace first
   const char* sn;
   QoreNamespace* ns = qjni_find_create_namespace(*default_jns, name, sn);

   QoreClass* qc = new QoreClass(sn);

   return createClassIntern(ns, jns, name, jpath, jc, qc);
}

QoreClass* QoreJniClassMap::createClassIntern(QoreNamespace* ns, QoreNamespace& jns, const char* name, const char* jpath, Class* jc, QoreClass* qc) {
   QoreClassHolder qc_holder(qc);
   // save pointer to java class info in QoreClass
   qc->setManagedUserData(jc);

   int mods = jc->getModifiers();
   if (mods & JVM_ACC_FINAL)
      qc->setFinal();

   printd(LogLevel, "QoreJniClassMap::createClass() qc: %p ns: %p '%s::%s'\n", qc, ns, ns->getName(), qc->getName());

   // add to class maps
   addIntern(jpath, qc_holder.release());

   Class* parent = jc->getSuperClass();

   printd(LogLevel, "QoreJniClassMap::createClass() '%s' parent: %p\n", jpath, parent);

   // add superclass
   if (parent)
      addSuperClass(*qc, parent);
   else if (qc != QC_OBJECT) // make interface classes at least inherit Object
      qc->addBuiltinVirtualBaseClass(QC_OBJECT);

   // get and process interfaces
   Env env;
   LocalReference<jobjectArray> interfaceArray = jc->getInterfaces();

   for (jsize i = 0, e = env.getArrayLength(interfaceArray); i < e; ++i) {
      addSuperClass(*qc, new Class(env.getObjectArrayElement(interfaceArray, i).as<jclass>()));
   }

   populateQoreClass(*qc, jc);

   // add to target namespace if not default
   if (&jns != default_jns) {
      const char* sn;
      ns = qjni_find_create_namespace(jns, name, sn);

      // copy class for assignment
      qc = new QoreClass(*qc);

      printd(LogLevel, "QoreJniClassMap::createClass() '%s' qc: %p ns: %p '%s::%s'\n", jpath, qc, ns, ns->getName(), qc->getName());
   }

   printd(LogLevel, "QoreJniClassMap::createClass() '%s' returning qc: %p ns: %p -> '%s::%s'\n", jpath, qc, ns, ns->getName(), qc->getName());

   // save class in namespace
   ns->addSystemClass(qc);

   return qc;
}

void QoreJniClassMap::addSuperClass(QoreClass& qc, jni::Class* parent) {
   Env env;
   LocalReference<jstring> clsName = env.callObjectMethod(parent->getJavaObject(), Globals::methodClassGetName, nullptr).as<jstring>();
   Env::GetStringUtfChars chars(env, clsName);

   printd(LogLevel, "QoreJniClassMap::addSuperClass() qc: '%s' parent: '%s'\n", qc.getName(), chars.c_str());

   QoreString jpath(chars.c_str());
   jpath.replaceAll(".", "/");
   QoreClass* pc = find(jpath.c_str());
   if (pc) {
      parent->deref();
   }
   else {
      pc = createClass(*default_jns, chars.c_str(), jpath.c_str(), loadClass(jpath));
   }

   qc.addBuiltinVirtualBaseClass(pc);
}

void QoreJniClassMap::populateQoreClass(QoreClass& qc, jni::Class* jc) {
   // do constructors
   doConstructors(qc, jc);

   // do methods
   doMethods(qc, jc);

   // do fields
   doFields(qc, jc);
}

void QoreJniClassMap::doConstructors(QoreClass& qc, jni::Class* jc) {
   Env env;

   // get constructor methods
   LocalReference<jobjectArray> conArray = jc->getDeclaredConstructors();

   for (jsize i = 0, e = env.getArrayLength(conArray); i < e; ++i) {
      // get Constructor object
      LocalReference<jobject> c = env.getObjectArrayElement(conArray, i);

      SimpleRefHolder<BaseMethod> meth(new BaseMethod(c, jc));

#ifdef DEBUG
      LocalReference<jstring> conStr = env.callObjectMethod(c, Globals::methodConstructorToString, nullptr).as<jstring>();
      Env::GetStringUtfChars chars(env, conStr);
      QoreString mstr(chars.c_str());
#endif

      // get method's parameter types
      type_vec_t paramTypeInfo;
      if (meth->getParamTypes(paramTypeInfo, *this)) {
         printd(LogLevel, "+ skipping %s.constructor() (%s); unsupported parameter type for variant %d\n", qc.getName(), mstr.c_str(), i + 1);
         continue;
      }

      // check for duplicate signature
      const QoreMethod* qm = qc.getConstructor();
      if (qm && qm->existsVariant(paramTypeInfo)) {
         printd(LogLevel, "QoreJniClassMap::doConstructors() skipping already-created variant %s::constructor()\n", qc.getName());
         continue;
      }

      qc.addConstructor((void*)*meth, (q_external_constructor_t)exec_java_constructor, meth->getAccess(), meth->getFlags(), QDOM_DEFAULT, paramTypeInfo);
      jc->trackMethod(meth.release());
   }
}

const QoreTypeInfo* QoreJniClassMap::getQoreType(jclass cls) {
   Env env;

   LocalReference<jstring> clsName = env.callObjectMethod(cls, Globals::methodClassGetName, nullptr).as<jstring>();
   Env::GetStringUtfChars cname(env, clsName);

   QoreString jname(cname.c_str());
   jname.replaceAll(".", "/");

   printd(LogLevel, "QoreJniClassMap::getQoreType() class: '%s' jname: '%s'\n", cname.c_str(), jname.c_str());

   if (env.callBooleanMethod(cls, Globals::methodClassIsArray, nullptr)) {
      return listOrNothingTypeInfo;
   }

   // do primitive types
   if (env.callBooleanMethod(cls, Globals::methodClassIsPrimitive, nullptr)) {
      jpmap_t::const_iterator i = jpmap.find(cname.c_str());
      assert(i != jpmap.end());
      return i->second.typeInfo;
   }

   // find static mapping
   jtmap_t::const_iterator i = jtmap.find(cname.c_str());
   if (i != jtmap.end())
      return i->second;

   // find or create a class for the type
   QoreClass* qc = find(jname.c_str());
   if (!qc) {
      printd(LogLevel, "QoreJniClassMap::getQoreType() creating cname: '%s' jname: '%s'\n", cname.c_str(), jname.c_str());
      //qc = createClass(*default_jns, cname.c_str(), jname.c_str(), jni::Functions::loadClass(jname.c_str()));
      qc = createClass(*default_jns, cname.c_str(), jname.c_str(), loadClass(jname.c_str()));
   }

   return qc->getOrNothingTypeInfo();
}

void QoreJniClassMap::doMethods(QoreClass& qc, jni::Class* jc) {
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
      if (meth->getParamTypes(paramTypeInfo, *this)) {
         printd(LogLevel, "+ skipping %s.%s(); unsupported parameter type for variant %d\n", qc.getName(), mname.c_str(), i + 1);
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
         qc.addStaticMethod((void*)*meth, mname.c_str(), (q_external_static_method_t)exec_java_static_method, meth->getAccess(), meth->getFlags(), QDOM_DEFAULT, returnTypeInfo, paramTypeInfo);
      }
      else {
         if (mname == "copy" || mname == "constructor" || mname == "destructor" || mname == "methodGate" || mname == "memberNotification" || mname == "memberGate")
            mname.prepend("java_");

         // check for duplicate signature
         const QoreMethod* qm = qc.findLocalMethod(mname.c_str());
         if (qm && qm->existsVariant(paramTypeInfo)) {
            printd(LogLevel, "QoreJniClassMap::doMethods() skipping already-created variant %s::%s()\n", qc.getName(), mname.c_str());
            continue;
         }

         if (meth->isAbstract())
            qc.addAbstractMethodVariant(mname.c_str(), meth->getAccess(), meth->getFlags(), returnTypeInfo, paramTypeInfo);
         else
            qc.addMethod((void*)*meth, mname.c_str(), (q_external_method_t)exec_java_method, meth->getAccess(), meth->getFlags(), QDOM_DEFAULT, returnTypeInfo, paramTypeInfo);
      }
      jc->trackMethod(meth.release());
   }
}

jobject QoreJniClassMap::getJavaObject(const QoreObject* o) {
   if (!o->isValid())
      return nullptr;
   ExceptionSink xsink;
   TryPrivateDataRefHolder<QoreJniPrivateData> jo(o, CID_OBJECT, &xsink);
   return jo ? jo->makeLocal().release() : nullptr;
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
   for (jsize i = 0; i < l->size(); ++i) {
      Array::set(array, elementType, cls, i, l->retrieve_entry(i));
   }

   return array.release();
}

/*
QoreObject* QoreJniClassMap::newArray(jclass cls, int size) {
   if (size < 1) {
      QoreStringMaker desc("cannot instantiate an array with size %d; must be greater than 0", size);
      throw BasicException(desc.c_str());
   }

   LocalReference<jarray> array = Array::getNew(Globals::getType(cls), cls, (jsize)size);
   Env env;
   LocalReference<jclass> acls = env.getObjectClass(array);
   QoreClass* qc = findCreateClass(acls);
   return new QoreObject(qc, getProgram(), new QoreJniPrivateData(array.release()));
}
*/

static void exec_java_constructor(const QoreClass& qc, BaseMethod* m, QoreObject* self, const QoreValueList* args, q_rt_flags_t rtflags, ExceptionSink* xsink) {
   try {
      self->setPrivate(qc.getID(), new QoreJniPrivateData(m->newQoreInstance(args)));
   }
   catch (jni::Exception& e) {
      e.convert(xsink);
   }
}

static QoreValue exec_java_static_method(const QoreMethod& meth, BaseMethod* m, const QoreValueList* args, q_rt_flags_t rtflags, ExceptionSink* xsink) {
   try {
      return m->invokeStatic(args);
   }
   catch (jni::Exception& e) {
      e.convert(xsink);
      return QoreValue();
   }
}

static QoreValue exec_java_method(const QoreMethod& meth, BaseMethod* m, QoreObject* self, QoreJniPrivateData* jd, const QoreValueList* args, q_rt_flags_t rtflags, ExceptionSink* xsink) {
   try {
      return m->invokeNonvirtual(jd->getObject(), args);
   }
   catch (jni::Exception& e) {
      e.convert(xsink);
      return QoreValue();
   }
}

void QoreJniClassMap::doFields(QoreClass& qc, jni::Class* jc) {
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

      printd(LogLevel, "+ adding field %s %s.%s\n", typeInfoGetName(fieldTypeInfo), qc.getName(), fname.c_str());

      if (field->isStatic()) {
         QoreValue v(field->getStatic());
         if (v.isNothing())
            v.assign(0ll);

         if (field->isFinal())
            qc.addBuiltinConstant(fname.getBuffer(), v, field->getAccess());
         else
            qc.addBuiltinStaticVar(fname.getBuffer(), v, field->getAccess());
      }
   }

   /*
   JArray<java::lang::reflect::Field*>* fields = jc->getDeclaredFields();
   for (int i = 0; i < fields->length; ++i) {
      java::lang::reflect::Field* f = elements(fields)[i];

      f->setAccessible(true);

      int mod = f->getModifiers();

      QoreString fname;
      getQoreString(f->getName(), fname);

      assert(fname.strlen());

#ifdef DEBUG
      QoreString fstr;
      getQoreString(f->toString(), fstr);
      printd(5, "  + adding %s.%s (%s)\n", qc.getName(), fname.getBuffer(), fstr.getBuffer());
#endif

      bool priv = mod & (java::lang::reflect::Modifier::PRIVATE|java::lang::reflect::Modifier::PROTECTED);

      java::lang::Class *typec = f->getType();
      const QoreTypeInfo *type = getQoreType(typec);

      if (mod & java::lang::reflect::Modifier::STATIC) {
         AbstractQoreNode *val = toQore(f->get(0), xsink);
         if (*xsink)
            break;

         if (mod & java::lang::reflect::Modifier::FINAL) {
            if (val)
               qc.addBuiltinConstant(fname.getBuffer(), val, priv);
         }
         else
            qc.addBuiltinStaticVar(fname.getBuffer(), val, priv, type);
      }
      else {
         if (priv)
            qc.addPrivateMember(fname.getBuffer(), type);
         else
            qc.addPublicMember(fname.getBuffer(), type);
      }
   }
   */
}

}
