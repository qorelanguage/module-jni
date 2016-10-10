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

#include "LocalReference.h"
#include "Object.h"

extern QoreClass* QC_CLASS;
extern qore_classid_t CID_CLASS;

namespace jni {

class Field;
class Method;

/**
 * \brief Represents a Java class.
 */
class Class : public ObjectBase, public AbstractQoreClassUserData {
private:
   DLLLOCAL ~Class() {
      printd(LogLevel, "Class::~Class(), this: %p, cls: %p\n", this, static_cast<jclass>(this->cls));
   }

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

private:
   GlobalReference<jclass> cls;
};

} // namespace jni

#endif // QORE_JNI_CLASS_H_
