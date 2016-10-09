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
#include "Functions.h"

using namespace jni;

extern QoreJniClassMap qjcm;

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
      ns = jns.findCreateNamespacePath(nsp.getBuffer(), true);
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
      // find/create parent namespace
      const char* sn;
      QoreNamespace* ns = qjni_find_create_namespace(jns, name, sn);
      qc->ref();
      // save class in namespace
      ns->addSystemClass(qc);

      printd(LogLevel, "QoreJniClassMap::findCreateClass() qc: %p ns: %p '%s::%s'\n", qc, ns, ns->getName(), sn);

      return qc;
   }
   else
      return loadCreateClass(jns, jpath.c_str());
}
QoreClass* QoreJniClassMap::loadCreateClass(QoreNamespace& jns, const char* cstr) {
   assert(cstr && cstr[0]);

   QoreString jstr(cstr);
   jstr.replaceAll(".", "/");

   //printd(LogLevel, "QoreJniClassMap::loadClass() cstr: '%s'\n", jstr.c_str());

   jni::Class* jcls = jni::Functions::loadClass(jstr);
   return createQoreClass(jns, cstr, jstr.c_str(), jcls);
}

// creates a QoreClass and adds it in the appropriate namespace
QoreClass* QoreJniClassMap::createQoreClass(QoreNamespace& jns, const char* name, const char* jpath, jni::Class* jc) {
   assert(!find(jpath));

   // find/create parent namespace
   const char* sn;
   QoreNamespace* ns = qjni_find_create_namespace(jns, name, sn);

   QoreClass* qc = new QoreClass(sn);
   // save pointer to java class info in QoreClass
   qc->setUserData(jc);

   // save class in namespace
   ns->addSystemClass(qc);

   printd(LogLevel, "QoreJniClassMap::createQoreClass() qc: %p ns: %p '%s::%s'\n", qc, ns, ns->getName(), sn);

   // add to class maps
   addIntern(jpath, jc, qc);

   Class* parent = jc->getSuperClass();
   printd(LogLevel, "QoreJniClassMap::createQoreClass() '%s' parent: %p\n", sn, parent);

   // add superclass
   if (parent)
      addSuperClass(jns, *qc, parent);

   /*
   // get and process superclass
   java::lang::Class *jsc = jc->getSuperclass();

   // add superclass
   if (jsc)
      addSuperClass(*qc, jsc);

   // add interface classes as superclasses
   JArray<jclass>* ifc = jc->getInterfaces();
   if (ifc && ifc->length) {
      for (int i = 0; i < ifc->length; ++i)
	 addSuperClass(*qc, elements(ifc)[i]);
   }
   */

   //populateQoreClass(*qc, jc);

   return qc;
}

void QoreJniClassMap::addSuperClass(QoreNamespace& jns, QoreClass& qc, jni::Class* parent) {
   Env env;
   LocalReference<jstring> clsName = env.callObjectMethod(parent->getJavaObject(), Globals::methodClassGetName, nullptr).as<jstring>();
   Env::GetStringUtfChars chars(env, clsName);
   printd(LogLevel, "QoreJniClassMap::addSuperClass() qc: '%s' parent: '%s'\n", qc.getName(), chars.c_str());

   QoreString jpath(chars.c_str());
   jpath.replaceAll(".", "/");
   QoreClass* pc = find(jpath.c_str());
   if (pc) {
      // find/create parent namespace
      const char* sn;
      QoreNamespace* ns = qjni_find_create_namespace(jns, chars.c_str(), sn);
      if (!ns->findLocalClass(sn)) {
         pc->ref();
         // save class in namespace
         ns->addSystemClass(pc);
      }
   }
   else
      pc = loadCreateClass(jns, jpath.c_str());

   qc.addBuiltinVirtualBaseClass(pc);
}

/*
int QoreJniClassMap::getArgTypes(type_vec_t &argTypeInfo, JArray<jclass>* params) {
   argTypeInfo.reserve(params->length);

   for (int i = 0; i < params->length; ++i) {
      java::lang::Class *jc = elements(params)[i];

      bool err;
      const QoreTypeInfo *typeInfo = getQoreType(jc, err);
      if (err)
	 return -1;

      //printd(0, "QoreJniClassMap::getArgTypes() jc: %p (%s), qore: %s\n", );

      argTypeInfo.push_back(typeInfo);
   }
   return 0;
}

void QoreJniClassMap::doConstructors(QoreClass &qc, java::lang::Class *jc, ExceptionSink *xsink) {
   try {
      // get constructor methods
      JArray<java::lang::reflect::Constructor*>* methods = jc->getDeclaredConstructors(false);

      for (size_t i = 0; i < (size_t)methods->length; ++i) {
	 java::lang::reflect::Constructor *m = elements(methods)[i];

#ifdef DEBUG
	 QoreString mstr;
	 getQoreString(m->toString(), mstr);
	 //if (!strcmp(qc.getName(), "URL"))
	 //printd(0, "  + adding %s.constructor() (%s) m: %p\n", qc.getName(), mstr.getBuffer(), m);
#endif

	 // get parameter type array
	 JArray<jclass>* params = m->getParameterTypes();

	 // get method's parameter types
	 type_vec_t argTypeInfo;
	 if (getArgTypes(argTypeInfo, params)) {
	    printd(0, "  + skipping %s.constructor() (%s); unsupported parameter type for arg %d\n", qc.getName(), mstr.getBuffer(), i + 1);
	    continue;
	 }

	 bool priv = m->getModifiers() & java::lang::reflect::Modifier::PRIVATE;
	 int64 flags = QC_NO_FLAGS;
	 if (m->isVarArgs())
	    flags |= QC_USES_EXTRA_ARGS;

	 const QoreMethod *qm = qc.getConstructor();
	 if (qm && qm->existsVariant(argTypeInfo)) {
	    //printd(0, "QoreJniClassMap::doConstructors() skipping already-created variant %s::constructor()\n", qc.getName());
	    continue;
	 }

	 qc.setConstructorExtendedList3((void*)i, (q_constructor3_t)exec_java_constructor, priv, flags, QDOM_DEFAULT, argTypeInfo);
      }
   }
   catch (java::lang::Throwable *t) {
      if (xsink)
	 getQoreException(t, *xsink);
   }
}

void QoreJniClassMap::doMethods(QoreClass &qc, java::lang::Class *jc, ExceptionSink *xsink) {
   //printd(0, "QoreJniClassMap::doMethods() %s qc: %p jc: %p\n", name, qc, jc);

   try {
      JArray<java::lang::reflect::Method*>* methods = jc->getDeclaredMethods();
      for (size_t i = 0; i < (size_t)methods->length; ++i) {
	 java::lang::reflect::Method *m = elements(methods)[i];

	 QoreString mname;
	 getQoreString(m->getName(), mname);
	 assert(!mname.empty());

#ifdef DEBUG
	 QoreString mstr;
	 getQoreString(m->toString(), mstr);
	 //printd(5, "  + adding %s.%s() (%s)\n", qc.getName(), mname.getBuffer(), mstr.getBuffer());
#endif

	 // get and map method's return type
	 bool err;
	 const QoreTypeInfo *returnTypeInfo = getQoreType(m->getReturnType(), err);
	 if (err) {
	    printd(0, "  + skipping %s.%s() (%s); unsupported return type\n", qc.getName(), mname.getBuffer(), mstr.getBuffer());
	    continue;
	 }

	 // get method's parameter types
	 type_vec_t argTypeInfo;
	 if (getArgTypes(argTypeInfo, m->getParameterTypes())) {
	    printd(0, "  + skipping %s.%s() (%s); unsupported parameter type for arg %d\n", qc.getName(), mname.getBuffer(), mstr.getBuffer(), i + 1);
	    continue;
	 }

	 bool priv = m->getModifiers() & java::lang::reflect::Modifier::PRIVATE;
	 int64 flags = QC_NO_FLAGS;
	 if (m->isVarArgs())
	    flags |= QC_USES_EXTRA_ARGS;

	 if ((m->getModifiers() & java::lang::reflect::Modifier::STATIC)) {
	    const QoreMethod *qm = qc.findLocalStaticMethod(mname.getBuffer());
	    if (qm && qm->existsVariant(argTypeInfo)) {
	       //printd(0, "QoreJniClassMap::doMethods() skipping already-created variant %s::%s()\n", qc.getName(), mname.getBuffer());
	       continue;
	    }

	    printd(5, "  + adding static %s%s::%s() (%s) qc: %p\n", priv ? "private " : "", qc.getName(), mname.getBuffer(), mstr.getBuffer(), &qc);
	    qc.addStaticMethodExtendedList3((void *)i, mname.getBuffer(), (q_static_method3_t)exec_java_static, priv, flags, QDOM_DEFAULT, returnTypeInfo, argTypeInfo);
	 }
	 else {
	    if (mname == "copy")
	       mname.prepend("java_");
	    const QoreMethod *qm = qc.findLocalMethod(mname.getBuffer());
	    if (qm && qm->existsVariant(argTypeInfo)) {
	       //printd(0, "QoreJniClassMap::doMethods() skipping already-created variant %s::%s()\n", qc.getName(), mname.getBuffer());
	       continue;
	    }

	    printd(5, "  + adding %s%s::%s() (%s) qc: %p\n", priv ? "private " : "", qc.getName(), mname.getBuffer(), mstr.getBuffer(), &qc);
	    qc.addMethodExtendedList3((void *)i, mname.getBuffer(), (q_method3_t)exec_java, priv, flags, QDOM_DEFAULT, returnTypeInfo, argTypeInfo);
	 }
      }
   }
   catch (java::lang::Throwable *t) {
      if (xsink)
	 getQoreException(t, *xsink);
   }
}

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

void QoreJniClassMap::populateQoreClass(QoreClass &qc, java::lang::Class *jc, ExceptionSink *xsink) {
   // do constructors
   doConstructors(qc, jc, xsink);

   // do methods
   doMethods(qc, jc, xsink);

   // do fields
   doFields(qc, jc, xsink);
}

const QoreTypeInfo *QoreJniClassMap::getQoreType(java::lang::Class *jc, bool &err) {
   err = false;
   if (jc->isArray())
      return listTypeInfo;

   if (jc == JvPrimClass(void)
       || jc == &java::lang::Void::class$)
      return nothingTypeInfo;

   if (jc == JvPrimClass(char)
       || jc == &java::lang::String::class$)
      return stringTypeInfo;

   if (jc == JvPrimClass(byte)
       || jc == JvPrimClass(short)
       || jc == JvPrimClass(int)
       || jc == JvPrimClass(long)
       || jc == &java::lang::Byte::class$
       || jc == &java::lang::Short::class$
       || jc == &java::lang::Integer::class$
       || jc == &java::lang::Long::class$
      )
      return bigIntTypeInfo;

   if (jc == JvPrimClass(float)
       || jc == JvPrimClass(double)
       || jc == &java::lang::Float::class$
       || jc == &java::lang::Double::class$
       || jc == &java::lang::Number::class$
      )
      return floatTypeInfo;

   if (jc == JvPrimClass(boolean)
       || jc == &java::lang::Boolean::class$)
      return boolTypeInfo;

   if (jc == &java::lang::Object::class$)
      return 0;

   return findCreate(jc)->getTypeInfo();
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
