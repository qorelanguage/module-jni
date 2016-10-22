/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
  QoreJniClassMap.h

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

#ifndef _QORE_JNI_QOREJAVACLASSMAP_H

#define _QORE_JNI_QOREJAVACLASSMAP_H

#include "LocalReference.h"
#include "QoreJniThreads.h"
#include "Env.h"

DLLLOCAL void init_jni_functions(QoreNamespace& ns);

DLLLOCAL QoreClass* initJavaObjectClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initJavaArrayClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initJavaThrowableClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initJavaClassClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initJavaFieldClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initJavaStaticFieldClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initJavaMethodClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initJavaStaticMethodClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initJavaConstructorClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initJavaInvocationHandlerClass(QoreNamespace& ns);

DLLLOCAL QoreClass* jni_class_handler(QoreNamespace* ns, const char* cname);

namespace jni {
   // forward reference
   class Class;
}

// the QoreClass for java::lang::Object
extern QoreClass* QC_OBJECT;
// the Qore class ID for java::lang::Object
extern qore_classid_t CID_OBJECT;
// the QoreClass for java::lang::Class
extern QoreClass* QC_CLASS;
// the Qore class ID for java::lang::Class
extern qore_classid_t CID_CLASS;
// the QoreClass for java::lang::ClassLoader
extern QoreClass* QC_CLASSLOADER;
// the Qore class ID for java::lang::ClassLoader
extern qore_classid_t CID_CLASSLOADER;

class QoreJniClassMap {
protected:
   // map of java class names (ex 'java/lang/object') to QoreClass ptrs
   typedef std::map<std::string, QoreClass*> jcmap_t;
   jcmap_t jcmap;

   // map of java class names to const QoreTypeInfo ptrs
   typedef std::map<const char*, const QoreTypeInfo*, ltstr> jtmap_t;
   static jtmap_t jtmap;

   // struct of type info and primitive type descriptor strings
   struct qore_java_primitive_info_t {
      const QoreTypeInfo* typeInfo;
      const char* descriptor;
   };
   // map of java primitive type names to type and descriptor info
   typedef std::map<const char*, struct qore_java_primitive_info_t, ltstr> jpmap_t;
   static jpmap_t jpmap;

   // parent namespace for jni module functionality
   QoreNamespace* default_jns;
   bool init_done;

   DLLLOCAL void addIntern(const char* name, jni::Class* jc, QoreClass* qc) {
      //printd(0, "QoreJniClassMap::addIntern() name: %s jc: %p qc: %p (%s)\n", name, jc, qc, qc->getName());

#ifdef DEBUG
      if (jcmap.find(name) != jcmap.end())
         printd(0, "QoreJniClassMap::addIntern() name: %s jc: %p qc: %p (%s)\n", name, jc, qc, qc->getName());
#endif

      assert(jcmap.find(name) == jcmap.end());
      jcmap[name] = qc;
   }

   DLLLOCAL void doMethods(QoreClass& qc, jni::Class* jc);

   DLLLOCAL void doFields(QoreClass& qc, jni::Class* jc);

   DLLLOCAL int getParamTypes(type_vec_t& argTypeInfo, jni::LocalReference<jobjectArray>& params, QoreString& desc);

   DLLLOCAL void doConstructors(QoreClass& qc, jni::Class* jc);

   DLLLOCAL void populateQoreClass(QoreClass& qc, jni::Class* jc);

   DLLLOCAL void addSuperClass(QoreClass& qc, jni::Class* parent);

   DLLLOCAL QoreClass* find(const char* jpath) const {
      jcmap_t::const_iterator i = jcmap.find(jpath);
      return i == jcmap.end() ? 0 : i->second;
   }

   DLLLOCAL QoreClass* createClass(QoreNamespace& jns, const char* name, const char* jpath, jni::Class* jc);
   DLLLOCAL QoreClass* createClassIntern(QoreNamespace* ns, QoreNamespace& jns, const char* name, const char* jpath, jni::Class* jc, QoreClass* qc);

public:
   mutable QoreJniThreadLock m;

   DLLLOCAL QoreJniClassMap() : default_jns(new QoreNamespace("Jni")), init_done(false) {
   }

   DLLLOCAL void init();

   DLLLOCAL void destroy(ExceptionSink& xsink);

   DLLLOCAL QoreObject* getObject(const jni::LocalReference<jobject>& jobj);

   DLLLOCAL QoreClass* findCreateClass(jclass jc);

   DLLLOCAL QoreClass* findCreateClass(QoreNamespace& jns, const char* name);

   DLLLOCAL const QoreTypeInfo* getQoreType(jclass cls);

   DLLLOCAL void initDone() {
      assert(!init_done);
      init_done = true;
   }

   DLLLOCAL QoreNamespace& getRootNS() {
      return *default_jns;
   }

   DLLLOCAL QoreClass* loadCreateClass(QoreNamespace& jns, const char* cstr);

   DLLLOCAL jni::LocalReference<jobject> getJavaObject(const QoreObject* o);

   DLLLOCAL jni::LocalReference<jarray> getJavaArray(const QoreListNode* l, jclass cls);

private:
   DLLLOCAL jni::LocalReference<jarray> getJavaArrayIntern(jni::Env& env, const QoreListNode* l, jclass cls);
};

class QoreJniPrivateData : public AbstractPrivateData {
private:
   jni::GlobalReference<jobject> jobj;

public:
   DLLLOCAL QoreJniPrivateData(const jni::LocalReference<jobject>& n_jobj) : jobj(n_jobj.makeGlobal()) {
   }

   DLLLOCAL jobject getObject() const {
      return jobj;
   }

   DLLLOCAL jni::LocalReference<jobject> getLocalReference() const {
      return jobj.toLocal();
   }
};

extern QoreJniClassMap qjcm;

#endif
