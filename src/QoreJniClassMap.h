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

#include "QoreJniPrivateData.h"
#include "QoreJniThreads.h"
#include "Env.h"
#include "Class.h"

DLLLOCAL QoreClass* initJavaArrayClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initQoreInvocationHandlerClass(QoreNamespace& ns);

DLLLOCAL void init_jni_functions(QoreNamespace& ns);
DLLLOCAL QoreClass* jni_class_handler(QoreNamespace* ns, const char* cname);

namespace jni {

// the QoreClass for java::lang::Object
extern QoreClass* QC_OBJECT;
// the Qore class ID for java::lang::Object
extern qore_classid_t CID_OBJECT;
// the QoreClass for java::lang::Class
extern QoreClass* QC_CLASS;
// the Qore class ID for java::lang::Class
extern qore_classid_t CID_CLASS;
// the QoreClass for java::lang::reflect::Method
extern QoreClass* QC_METHOD;
// the Qore class ID for java::reflect::Method
extern qore_classid_t CID_METHOD;
// the QoreClass for java::lang::ClassLoader
extern QoreClass* QC_CLASSLOADER;
// the Qore class ID for java::lang::ClassLoader
extern qore_classid_t CID_CLASSLOADER;
// the QoreClass for java::lang::Throwable
extern QoreClass* QC_THROWABLE;
// the Qore class ID for java::lang::Throwable
extern qore_classid_t CID_THROWABLE;
// the QoreClass for java::lang::reflect::InvocationHandler
extern QoreClass* QC_INVOCATIONHANDLER;
// the Qore class ID for java::lang::reflect::InvocationHandler
extern qore_classid_t CID_INVOCATIONHANDLER;

// forward reference
class Class;

class QoreJniClassMap {
public:
   mutable QoreJniThreadLock m;

   DLLLOCAL QoreJniClassMap() : default_jns(new QoreNamespace("Jni")), init_done(false) {
   }

   DLLLOCAL void init();

   DLLLOCAL void destroy(ExceptionSink& xsink);

   DLLLOCAL QoreValue getValue(LocalReference<jobject>& jobj);

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

   DLLLOCAL jobject getJavaObject(const QoreObject* o);

   DLLLOCAL jarray getJavaArray(const QoreListNode* l, jclass cls);

   DLLLOCAL void addClasspath(const char* path);

   DLLLOCAL jclass findLoadClass(const char* name);

   DLLLOCAL jclass findLoadClass(const QoreString& name);

   DLLLOCAL Class* loadClass(const char* name);

   DLLLOCAL Class* loadClass(const QoreString& name);

protected:
   // class loader
   GlobalReference<jobject> classLoader;

   // map of java class names (ex 'java/lang/Object') to QoreClass ptrs
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

   DLLLOCAL void addIntern(const char* name, QoreClass* qc) {
      printd(LogLevel, "QoreJniClassMap::addIntern() name: %s qc: %p (%s)\n", name, qc, qc->getName());

#ifdef DEBUG
      if (jcmap.find(name) != jcmap.end())
         printd(0, "QoreJniClassMap::addIntern() name: %s qc: %p (%s)\n", name, qc, qc->getName());
#endif

      assert(jcmap.find(name) == jcmap.end());
      jcmap[name] = qc;
   }

   DLLLOCAL void doMethods(QoreClass& qc, Class* jc);

   DLLLOCAL void doFields(QoreClass& qc, Class* jc);

   DLLLOCAL int getParamTypes(type_vec_t& argTypeInfo, LocalReference<jobjectArray>& params, QoreString& desc);

   DLLLOCAL void doConstructors(QoreClass& qc, Class* jc);

   DLLLOCAL void populateQoreClass(QoreClass& qc, Class* jc);

   DLLLOCAL void addSuperClass(QoreClass& qc, Class* parent);

   DLLLOCAL QoreClass* find(const char* jpath) const {
      jcmap_t::const_iterator i = jcmap.find(jpath);
      return i == jcmap.end() ? 0 : i->second;
   }

   DLLLOCAL QoreClass* createClass(QoreNamespace& jns, const char* name, const char* jpath, Class* jc);
   DLLLOCAL QoreClass* createClassIntern(QoreNamespace* ns, QoreNamespace& jns, const char* name, const char* jpath, Class* jc, QoreClass* qc);

private:
   DLLLOCAL jarray getJavaArrayIntern(Env& env, const QoreListNode* l, jclass cls);
};

extern QoreJniClassMap qjcm;
}

#endif
