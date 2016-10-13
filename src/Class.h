//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2016 Qore Technologies, s.r.o.
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//------------------------------------------------------------------------------
///
/// \file
/// \brief Defines the Class class.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_CLASS_H_
#define QORE_JNI_CLASS_H_

#include <qore/Qore.h>

#include <vector>

#include "LocalReference.h"
#include "Object.h"

extern QoreClass* QC_JAVACLASS;
extern qore_classid_t CID_JAVACLASS;

namespace jni {

class Field;
class Method;
class BaseMethod;

/**
 * \brief Represents a Java class.
 */
class Class : public ObjectBase, public AbstractQoreClassUserData {
private:
   DLLLOCAL ~Class();

public:
   /**
    * \brief Constructor.
    * \param cls a local reference to a Java class
    * \throws JavaException if a global reference cannot be created
    */
   DLLLOCAL Class(const LocalReference<jclass>& cls) : cls(cls.makeGlobal()) {
      printd(LogLevel, "Class::Class(), this: %p, cls: %p\n", this, static_cast<jclass>(this->cls));
      assert(static_cast<jclass>(this->cls));
   }

   DLLLOCAL jclass getJavaObject() const override {
      return cls;
   }

   DLLLOCAL virtual Class* copy() const override {
      const_cast<Class*>(this)->ref();
      return const_cast<Class*>(this);
   }

   DLLLOCAL virtual void doDeref() override {
      deref();
   }

   DLLLOCAL Field* getField(const QoreStringNode* name, const QoreStringNode* descriptor);
   DLLLOCAL Field* getStaticField(const QoreStringNode* name, const QoreStringNode* descriptor);
   DLLLOCAL Method* getMethod(const QoreStringNode* name, const QoreStringNode* descriptor);
   DLLLOCAL Method* getStaticMethod(const QoreStringNode* name, const QoreStringNode* descriptor);
   DLLLOCAL Method* getConstructor(const QoreStringNode* descriptor);
   DLLLOCAL bool isInstance(const ObjectBase* obj);

   // returns the parent class or nullptr if there is none
   DLLLOCAL Class* getSuperClass();

   // returns an array of interface classes
   DLLLOCAL LocalReference<jobjectArray> getInterfaces();

   // returns an array of constructors
   DLLLOCAL LocalReference<jobjectArray> getDeclaredConstructors();

   // returns an array of methods
   DLLLOCAL LocalReference<jobjectArray> getDeclaredMethods();

   // returns an arry of fields
   DLLLOCAL LocalReference<jobjectArray> getDeclaredFields();

   // returns class modifiers as an integer
   DLLLOCAL int getModifiers();

   DLLLOCAL void trackMethod(BaseMethod* m);

private:
   GlobalReference<jclass> cls;
   // for tracking Method objects associated with this Class
   typedef std::vector<BaseMethod*> mlist_t;
   mlist_t mlist;
};

} // namespace jni

#endif // QORE_JNI_CLASS_H_
