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

// the QoreBuiltinClass for java::lang::Object
extern QoreBuiltinClass* QC_OBJECT;
// the Qore class ID for java::lang::Object
extern qore_classid_t CID_OBJECT;
// the QoreBuiltinClass for java::lang::Class
extern QoreBuiltinClass* QC_CLASS;
// the Qore class ID for java::lang::Class
extern qore_classid_t CID_CLASS;
// the QoreBuiltinClass for java::lang::reflect::Method
extern QoreBuiltinClass* QC_METHOD;
// the Qore class ID for java::reflect::Method
extern qore_classid_t CID_METHOD;
// the QoreBuiltinClass for java::lang::ClassLoader
extern QoreBuiltinClass* QC_CLASSLOADER;
// the Qore class ID for java::lang::ClassLoader
extern qore_classid_t CID_CLASSLOADER;
// the QoreBuiltinClass for java::lang::Throwable
extern QoreBuiltinClass* QC_THROWABLE;
// the Qore class ID for java::lang::Throwable
extern qore_classid_t CID_THROWABLE;
// the QoreBuiltinClass for java::lang::reflect::InvocationHandler
extern QoreBuiltinClass* QC_INVOCATIONHANDLER;
// the Qore class ID for java::lang::reflect::InvocationHandler
extern qore_classid_t CID_INVOCATIONHANDLER;

// forward reference
class Class;

class QoreJniClassMapBase {
protected:
   // map of java class names (ex 'java/lang/Object') to QoreBuiltinClass ptrs
   typedef std::map<std::string, QoreBuiltinClass*> jcmap_t;
   // map of java class names (ex 'java/lang/Object') to QoreBuiltinClass ptrs
   jcmap_t jcmap;

public:
   DLLLOCAL void add(const char* name, QoreBuiltinClass* qc) {
      printd(LogLevel, "QoreJniClassMapBase::add() name: %s qc: %p (%s)\n", name, qc, qc->getName());

#ifdef DEBUG
      if (jcmap.find(name) != jcmap.end())
         printd(0, "QoreJniClassMapBase::add() name: %s qc: %p (%s)\n", name, qc, qc->getName());
#endif

      assert(jcmap.find(name) == jcmap.end());
      jcmap[name] = qc;
   }

   DLLLOCAL QoreBuiltinClass* find(const char* jpath) const {
      if (strchr(jpath, '.')) {
         QoreString str(jpath);
         str.replaceAll(".", "/");
         jcmap_t::const_iterator i = jcmap.find(str.c_str());
         return i == jcmap.end() ? 0 : i->second;
      }
      jcmap_t::const_iterator i = jcmap.find(jpath);
      return i == jcmap.end() ? 0 : i->second;
   }
};

class QoreJniClassMap : public QoreJniClassMapBase {
public:
   static QoreJniThreadLock m;

   DLLLOCAL void init();

   DLLLOCAL void destroy(ExceptionSink& xsink);

   DLLLOCAL QoreValue getValue(LocalReference<jobject>& jobj);

   DLLLOCAL const QoreTypeInfo* getQoreType(jclass cls);

   DLLLOCAL void initDone() {
      assert(!init_done);
      init_done = true;
   }

   DLLLOCAL QoreNamespace& getJniNs() {
      return *default_jns;
   }

   DLLLOCAL jobject getJavaObject(const QoreObject* o);

   DLLLOCAL jarray getJavaArray(const QoreListNode* l, jclass cls);

   DLLLOCAL jclass findLoadClass(const char* name);
   DLLLOCAL jclass findLoadClass(const QoreString& name);

   DLLLOCAL QoreBuiltinClass* findCreateQoreClass(LocalReference<jclass>& jc);
   DLLLOCAL QoreBuiltinClass* findCreateQoreClass(const char* name);

protected:
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
   QoreNamespace* default_jns = new QoreNamespace("Jni");
   bool init_done = false;

   // class loader
   GlobalReference<jobject> baseClassLoader;

   DLLLOCAL void doMethods(QoreBuiltinClass& qc, Class* jc);

   DLLLOCAL void doFields(QoreBuiltinClass& qc, Class* jc);

   DLLLOCAL int getParamTypes(type_vec_t& argTypeInfo, LocalReference<jobjectArray>& params, QoreString& desc);

   DLLLOCAL void doConstructors(QoreBuiltinClass& qc, Class* jc);

   DLLLOCAL void populateQoreClass(QoreBuiltinClass& qc, Class* jc);

   DLLLOCAL void addSuperClass(QoreBuiltinClass& qc, Class* parent, bool interface);

   DLLLOCAL QoreBuiltinClass* createClassInNamespace(QoreNamespace* ns, QoreNamespace& jns, const char* name, const char* jpath, Class* jc, QoreBuiltinClass* qc, QoreJniClassMapBase& map);
   DLLLOCAL QoreBuiltinClass* findCreateQoreClassInBase(const char* name, const char* jpath, Class* c);
   DLLLOCAL QoreBuiltinClass* findCreateQoreClassInProgram(const char* name, const char* jpath, Class* c);
   DLLLOCAL Class* loadClass(const char* name, bool& base);

private:
   DLLLOCAL jarray getJavaArrayIntern(Env& env, const QoreListNode* l, jclass cls);
};

extern QoreJniClassMap qjcm;

class JniExternalProgramData : public AbstractQoreProgramExternalData, public QoreJniClassMapBase {
public:
   DLLLOCAL JniExternalProgramData(QoreNamespace* n_jni);

   DLLLOCAL JniExternalProgramData(const JniExternalProgramData& parent);

   DLLLOCAL jobject getClassLoader() const {
      return classLoader;
   }

   DLLLOCAL virtual ~JniExternalProgramData() {
      classLoader = nullptr;
   }

   DLLLOCAL QoreNamespace* getJniNamespace() const {
      return jni;
   }

   DLLLOCAL void addClasspath(const char* path);

   DLLLOCAL virtual AbstractQoreProgramExternalData* copy(QoreProgram* pgm) const {
      return new JniExternalProgramData(*this);
   }

   DLLLOCAL virtual void doDeref() {
      delete this;
   }

protected:
   // Jni namespace pointer for the current Program
   QoreNamespace* jni;
   // class loader
   GlobalReference<jobject> classLoader;
};

}

#endif
