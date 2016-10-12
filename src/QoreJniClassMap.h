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

DLLLOCAL void init_jni_functions(QoreNamespace& ns);
DLLLOCAL QoreClass* initObjectClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initArrayClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initThrowableClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initClassClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initFieldClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initStaticFieldClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initMethodClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initStaticMethodClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initConstructorClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initInvocationHandlerClass(QoreNamespace& ns);

DLLLOCAL QoreClass* jni_class_handler(QoreNamespace* ns, const char* cname);

namespace jni {
   // forward reference
   class Class;
}

class QoreJniClassMap {
protected:
   // map of java class names (ex 'java/lang/object') to QoreClass ptrs
   typedef std::map<std::string, QoreClass*> jcmap_t;

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
   QoreNamespace default_jns;
   jcmap_t jcmap;
   bool init_done;

   DLLLOCAL void addIntern(const char* name, jni::Class* jc, QoreClass* qc) {
      //printd(0, "QoreJniClassMap::addIntern() name: %s jc: %p qc: %p (%s)\n", name, jc, qc, qc->getName());

      if (jcmap.find(name) != jcmap.end())
         printd(0, "QoreJniClassMap::addIntern() name: %s jc: %p qc: %p (%s)\n", name, jc, qc, qc->getName());

      assert(jcmap.find(name) == jcmap.end());
      jcmap[name] = qc;
   }

   /*

   DLLLOCAL void doMethods(QoreClass& qc, java::lang::Class *jc, ExceptionSink *xsink = 0);
   DLLLOCAL void doFields(QoreClass& qc, java::lang::Class *jc, ExceptionSink *xsink = 0);

   DLLLOCAL void addQoreClass();
   */

   DLLLOCAL int getParamTypes(type_vec_t& argTypeInfo, jni::LocalReference<jobjectArray>& params, QoreString& desc);

   DLLLOCAL void doConstructors(QoreClass& qc, jni::Class* jc);

   DLLLOCAL void populateQoreClass(QoreClass& qc, jni::Class* jc);

   DLLLOCAL void addSuperClass(QoreClass& qc, jni::Class* parent);

public:
   mutable QoreThreadLock m;

   DLLLOCAL QoreJniClassMap() : default_jns("Jni"), init_done(false) {
   }

   DLLLOCAL void init();

   DLLLOCAL QoreClass* createClass(QoreNamespace& jns, const char* name, const char* jpath, jni::Class* jc);

   DLLLOCAL QoreClass* find(const char* jpath) const {
      jcmap_t::const_iterator i = jcmap.find(jpath);
      return i == jcmap.end() ? 0 : i->second;
   }

   DLLLOCAL QoreClass* findCreateClass(QoreNamespace& jns, const char* name);

   DLLLOCAL const QoreTypeInfo* getQoreType(jni::LocalReference<jclass>& cls, QoreString& desc);

   DLLLOCAL void initDone() {
      assert(!init_done);
      init_done = true;
   }

   DLLLOCAL QoreNamespace& getRootNS() {
      return default_jns;
   }

   /*
   DLLLOCAL java::lang::Object *toJava(java::lang::Class *jc, const AbstractQoreNode *n, ExceptionSink *xsink);
   DLLLOCAL const QoreTypeInfo *toTypeInfo(java::lang::Class *jc);
   DLLLOCAL AbstractQoreNode *toQore(java::lang::Object *jobj, ExceptionSink *xsink);
   */

   DLLLOCAL QoreClass* loadCreateClass(QoreNamespace& jns, const char* cstr);
};

class QoreJniPrivateData : public AbstractPrivateData {
private:
   jni::GlobalReference<jobject> jobj;

public:
   DLLLOCAL QoreJniPrivateData(jni::LocalReference<jobject> n_jobj) : jobj(n_jobj.makeGlobal()) {
   }

   DLLLOCAL jobject getObject() const {
      return jobj;
   }
};

#endif
