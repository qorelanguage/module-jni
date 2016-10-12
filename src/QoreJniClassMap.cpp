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

#include "defs.h"
#include "Jvm.h"
#include "QoreJniClassMap.h"
#include "Class.h"
#include "Method.h"
#include "Functions.h"

using namespace jni;

extern QoreJniClassMap qjcm;

static void exec_java_constructor(const QoreClass& qc, BaseMethod* m, QoreObject* self, const QoreValueList* args, q_rt_flags_t rtflags, ExceptionSink* xsink);
static QoreValue exec_java_static_method(const QoreMethod& meth, BaseMethod* m, const QoreValueList* args, q_rt_flags_t rtflags, ExceptionSink* xsink);
static QoreValue exec_java_method(const QoreMethod& meth, BaseMethod* m, QoreObject* self, QoreJniPrivateData* jd, const QoreValueList* args, q_rt_flags_t rtflags, ExceptionSink* xsink);

QoreJniClassMap::jtmap_t QoreJniClassMap::jtmap = {
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
   QoreNamespace* qorejni = new QoreNamespace("QoreJni");
   qorejni->addSystemClass(initObjectClass(*qorejni));
   qorejni->addSystemClass(initInvocationHandlerClass(*qorejni));
   qorejni->addSystemClass(initArrayClass(*qorejni));
   qorejni->addSystemClass(initThrowableClass(*qorejni));
   qorejni->addSystemClass(initFieldClass(*qorejni));
   qorejni->addSystemClass(initStaticFieldClass(*qorejni));
   qorejni->addSystemClass(initMethodClass(*qorejni));
   qorejni->addSystemClass(initStaticMethodClass(*qorejni));
   qorejni->addSystemClass(initConstructorClass(*qorejni));
   qorejni->addSystemClass(initClassClass(*qorejni));
   init_jni_functions(*qorejni);

   // add to "QoreJni" namespace
   default_jns.addInitialNamespace(qorejni);

   // create java.lang namespace with automatic class loader handler
   QoreNamespace* javans = new QoreNamespace("java");
   QoreNamespace* langns = new QoreNamespace("lang");
   langns->setClassHandler(jni_class_handler);
   javans->addInitialNamespace(langns);

   // add to "Jni" namespace
   default_jns.addInitialNamespace(javans);

   // add "Object" class
   loadCreateClass(default_jns, "java.lang.Object");
}

QoreClass* QoreJniClassMap::findCreateClass(QoreNamespace& jns, const char* name) {
   QoreString jpath(name);
   jpath.replaceAll(".", "/");
   QoreClass *qc = find(jpath.c_str());

   if (qc) {
      assert(&jns != &default_jns);
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
   return createClass(jns, name, jpath.c_str(), jni::Functions::loadClass(jpath));
}

QoreClass* QoreJniClassMap::loadCreateClass(QoreNamespace& jns, const char* cstr) {
   assert(cstr && cstr[0]);

   QoreString jstr(cstr);
   jstr.replaceAll(".", "/");

   printd(LogLevel, "QoreJniClassMap::loadCreateClass() name: '%s' jstr: '%s'\n", cstr, jstr.c_str());
   return createClass(jns, cstr, jstr.c_str(), jni::Functions::loadClass(jstr));
}

// creates a QoreClass and adds it in the appropriate namespace
QoreClass* QoreJniClassMap::createClass(QoreNamespace& jns, const char* name, const char* jpath, jni::Class* jc) {
   assert(!find(jpath));

   // find/create parent namespace in default / master Jni namespace first
   const char* sn;
   QoreNamespace* ns = qjni_find_create_namespace(default_jns, name, sn);

   QoreClass* qc = new QoreClass(sn);
   //std::unique_ptr<QoreClass> qc_holder(qc);
   // save pointer to java class info in QoreClass
   qc->setManagedUserData(jc);

   // save class in namespace
   ns->addSystemClass(qc);

   printd(LogLevel, "QoreJniClassMap::createClass() qc: %p ns: %p '%s::%s'\n", qc, ns, ns->getName(), sn);

   // add to class maps
   //addIntern(jpath, jc, qc_holder.release());
   addIntern(jpath, jc, qc);

   populateQoreClass(*qc, jc);

   Class* parent = jc->getSuperClass();
   printd(LogLevel, "QoreJniClassMap::createClass() '%s' parent: %p\n", sn, parent);

   // add superclass
   if (parent)
      addSuperClass(*qc, parent);

   // get and process interfaces
   Env env;
   LocalReference<jobjectArray> interfaceArray = jc->getInterfaces();
   for (jsize i = 0, e = env.getArrayLength(interfaceArray); i < e; ++i) {
      addSuperClass(*qc, new Class(env.getObjectArrayElement(interfaceArray, i).as<jclass>()));
   }

   // add to target namespace if not default
   if (&jns != &default_jns) {
      ns = qjni_find_create_namespace(jns, name, sn);

      // copy class for assignment
      qc = new QoreClass(*qc);

      printd(LogLevel, "QoreJniClassMap::createClass() qc: %p ns: %p '%s::%s'\n", qc, ns, ns->getName(), sn);

      // save class in namespace
      ns->addSystemClass(qc);
   }

   printd(LogLevel, "QoreJniClassMap::createClass() returning qc: %p ns: %p '%s' -> '%s::%s'\n", qc, ns, jpath, ns->getName(), sn);

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
   if (pc)
      parent->deref();
   else
      pc = createClass(default_jns, chars.c_str(), jpath.c_str(), jni::Functions::loadClass(jpath));

   qc.addBuiltinVirtualBaseClass(pc);
}

void QoreJniClassMap::populateQoreClass(QoreClass& qc, jni::Class* jc) {
   // do constructors
   doConstructors(qc, jc);

   // do methods
   doMethods(qc, jc);

   // do fields
   //doFields(qc, jc);
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
      return listTypeInfo;
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
      qc = createClass(default_jns, cname.c_str(), jname.c_str(), jni::Functions::loadClass(jname.c_str()));
   }

   return qc->getTypeInfo();
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
      xsink->raiseException("ERR", "SM");
      return QoreValue();
   }
   catch (jni::Exception& e) {
      e.convert(xsink);
      return QoreValue();
   }
}

static QoreValue exec_java_method(const QoreMethod& meth, BaseMethod* m, QoreObject* self, QoreJniPrivateData* jd, const QoreValueList* args, q_rt_flags_t rtflags, ExceptionSink* xsink) {
   try {
      xsink->raiseException("ERR", "M");
      return QoreValue();
   }
   catch (jni::Exception& e) {
      e.convert(xsink);
      return QoreValue();
   }
}

/*
void QoreJniClassMap::doFields(QoreClass &qc, java::lang::Class *jc, ExceptionSink *xsink) {
   printd(5, "QoreJniClassMap::doFields() %s qc: %p jc: %p\n", qc.getName(), &qc, jc);

   try {
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
         const QoreTypeInfo *type = toTypeInfo(typec);

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
   }
   catch (java::lang::Throwable *t) {
      if (xsink)
         getQoreException(t, *xsink);
   }
}

void QoreJniClassMap::addQoreClass() {
   // get class for java::lang::Object
   QoreClass *joc = find(&java::lang::Object::class$);
   assert(joc);

   CID_OBJECT = joc->getID();

   QoreClass *qc = new QoreClass("Qore");
   qc->addStaticMethodExtended("toQore", f_toQore, false, QC_NO_FLAGS, QDOM_DEFAULT, anyTypeInfo, 1, joc->getTypeInfo(), QORE_PARAM_NO_ARG);

   default_jns.addSystemClass(qc);
}

const QoreTypeInfo *QoreJniClassMap::toTypeInfo(java::lang::Class *jc) {
   if (jc == &java::lang::String::class$
       || jc == &java::lang::Character::class$)
      return stringTypeInfo;

   if (jc == &java::lang::Long::class$
       || jc == &java::lang::Integer::class$
       || jc == &java::lang::Short::class$
       || jc == &java::lang::Byte::class$)
      return bigIntTypeInfo;

   if (jc == &java::lang::Boolean::class$)
      return boolTypeInfo;

   if (jc == &java::lang::Double::class$
       || jc == &java::lang::Float::class$)
      return floatTypeInfo;

   QoreClass *qc = find(jc);
   return qc ? qc->getTypeInfo() : 0;
}

AbstractQoreNode *QoreJniClassMap::toQore(java::lang::Object *jobj, ExceptionSink *xsink) {
   if (!jobj)
      return 0;

   jclass jc = jobj->getClass();

   if (jc->isArray()) {
      ReferenceHolder<QoreListNode> rv(new QoreListNode, xsink);

      jint len = java::lang::reflect::Array::getLength(jobj);
      for (jint i = 0; i < len; ++i) {
         java::lang::Object* elem = java::lang::reflect::Array::get(jobj, i);
         AbstractQoreNode* qe = toQore(elem, xsink);
         if (*xsink) {
            assert(!qe);
            return 0;
         }
         rv->push(qe);
      }

      return rv.release();
   }

   if (jc == &java::util::Vector::class$) {
      java::util::Vector* vec = reinterpret_cast<java::util::Vector*>(jobj);
      ReferenceHolder<QoreListNode> rv(new QoreListNode, xsink);

      jint len = vec->size();
      for (jint i = 0; i < len; ++i) {
         java::lang::Object* elem = vec->elementAt(i);
         AbstractQoreNode* qe = toQore(elem, xsink);
         if (*xsink) {
            assert(!qe);
            return 0;
         }
         rv->push(qe);
      }

      return rv.release();
   }

   if (jc == &java::lang::String::class$)
      return javaToQore((jstring)jobj);

   if (jc == &java::lang::Long::class$)
      return javaToQore(((java::lang::Long *)jobj)->longValue());

   if (jc == &java::lang::Integer::class$)
      return javaToQore(((java::lang::Integer *)jobj)->longValue());

   if (jc == &java::lang::Short::class$)
      return javaToQore(((java::lang::Short *)jobj)->longValue());

   if (jc == &java::lang::Byte::class$)
      return javaToQore(((java::lang::Byte *)jobj)->longValue());

   if (jc == &java::lang::Boolean::class$)
      return javaToQore(((java::lang::Boolean *)jobj)->booleanValue());

   if (jc == &java::lang::Double::class$)
      return javaToQore(((java::lang::Double *)jobj)->doubleValue());

   if (jc == &java::lang::Float::class$)
      return javaToQore(((java::lang::Float *)jobj)->doubleValue());

   if (jc == &java::lang::Character::class$)
      return javaToQore(((java::lang::Character *)jobj)->charValue());

   QoreClass *qc = find(jc);
   if (qc) // set Program to NULL as java objects should not depend on code in the current Program object
      return new QoreObject(qc, 0, new QoreJniPrivateData(jobj));

   if (!xsink)
      return 0;

   QoreString cname;
   getQoreString(jobj->getClass()->getName(), cname);

   xsink->raiseException("JAVA-UNSUPPORTED-TYPE", "cannot convert from Java class '%s' to a Qore value", cname.getBuffer());
   return 0;
}

java::lang::Object *QoreJniClassMap::toJava(java::lang::Class *jc, const AbstractQoreNode *n, ExceptionSink *xsink) {
#ifdef DEBUG
   QoreString pname;
   getQoreString(jc->getName(), pname);
   printd(5, "QoreJniClassMap::toJava() jc: %p %s n: %p %s\n", jc, pname.getBuffer(), n, get_type_name(n));
#endif

   // handle NULL pointers first
   if (!n || jc == JvPrimClass(void))
      return 0;

   if (jc->isArray()) {
      jclass cc = jc->getComponentType();
      if (n && n->getType() == NT_LIST) {
         const QoreListNode *l = reinterpret_cast<const QoreListNode *>(n);
         jobjectArray array = JvNewObjectArray(l->size(), cc, NULL);
         ConstListIterator li(l);
         while (li.next()) {
            elements(array)[li.index()] = toJava(cc, li.getValue(), xsink);
            if (*xsink)
               return 0;
         }
         //printd(5, "QoreJniClassMap::toJava() returning array of size %lld\n", l->size());
         return array;
      }
      else {
         jobjectArray array = JvNewObjectArray(is_nothing(n) ? 0 : 1, cc, NULL);
         if (!is_nothing(n)) {
            elements(array)[0] = toJava(cc, n, xsink);
            if (*xsink)
               return 0;
         }
         return array;
      }
   }

   if (jc == &java::lang::String::class$) {
      QoreStringValueHelper str(n, QCS_UTF8, xsink);
      if (*xsink)
         return 0;

      return JvNewStringUTF(str->getBuffer());
   }

   if (jc == &java::lang::Long::class$ || jc == JvPrimClass(long))
      return ::toJava((jlong)n->getAsBigInt());

   if (jc == &java::lang::Integer::class$ || jc == JvPrimClass(int))
      return new java::lang::Integer((jint)n->getAsInt());

   if (jc == &java::lang::Short::class$ || jc == JvPrimClass(short))
      return new java::lang::Short((jshort)n->getAsInt());

   if (jc == &java::lang::Byte::class$ || jc == JvPrimClass(byte))
      return new java::lang::Byte((jbyte)n->getAsInt());

   if (jc == &java::lang::Boolean::class$ || jc == JvPrimClass(boolean))
      return ::toJava((jboolean)n->getAsBool());

   if (jc == &java::lang::Double::class$ || jc == JvPrimClass(double))
      return ::toJava((jdouble)n->getAsFloat());

   if (jc == &java::lang::Float::class$ || jc == JvPrimClass(float))
      return ::toJava((jfloat)n->getAsFloat());

   if (jc == &java::lang::Character::class$ || jc == JvPrimClass(char)) {
      QoreStringValueHelper str(n, QCS_UTF8, xsink);
      if (*xsink)
         return 0;

      return new java::lang::Character((jchar)str->getUnicodePointFromUTF8(0));
   }

   if (jc == &java::lang::Object::class$)
      return ::toJava(n, xsink);

   // find corresponding QoreClass
   QoreClass *qc = find(jc);
   if (qc && n && n->getType() == NT_OBJECT) {
      const QoreObject *o = reinterpret_cast<const QoreObject *>(n);
      PrivateDataRefHolder<QoreJniPrivateData> jpd(o, qc->getID(), xsink);
      if (!jpd) {
         if (!*xsink) {
            QoreString cname;
            getQoreString(jc->getName(), cname);
            xsink->raiseException("JAVA-TYPE-ERROR", "java class '%s' expected, but Qore class '%s' supplied", cname.getBuffer(), o->getClassName());
         }
         return 0;
      }
      return jpd->getObject();
   }

   QoreString cname;
   getQoreString(jc->getName(), cname);

   if (n && n->getType() == NT_OBJECT)
      xsink->raiseException("JAVA-UNSUPPORTED-TYPE", "cannot convert from Qore class '%s' to Java '%s'", reinterpret_cast<const QoreObject *>(n)->getClassName(), cname.getBuffer());
   else
      xsink->raiseException("JAVA-UNSUPPORTED-TYPE", "cannot convert from Qore '%s' to Java '%s'", get_type_name(n), cname.getBuffer());
   return 0;
}
*/
