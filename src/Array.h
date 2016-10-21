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
/// \brief Defines the Array class.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_ARRAY_H_
#define QORE_JNI_ARRAY_H_

#include <qore/Qore.h>
#include "LocalReference.h"
#include "Object.h"
#include "Globals.h"

extern QoreClass* QC_JAVAARRAY;
extern qore_classid_t CID_JAVAARRAY;

namespace jni {

/**
 * \brief Represents a Java array instance.
 */
class Array : public ObjectBase {

public:
   /**
    * \brief Constructor.
    * \param array a local reference to a Java array instance
    * \throws JavaException if a global reference cannot be created
    */
   DLLLOCAL Array(jarray array);

   DLLLOCAL ~Array() {
      printd(LogLevel, "Array::~Array(), this: %p, object: %p\n", this, static_cast<jarray>(this->array));
   }

   DLLLOCAL jarray getJavaObject() const override {
      return array;
   }

   DLLLOCAL int64 length();
   DLLLOCAL QoreValue get(int64 index, bool to_qore = false);
   DLLLOCAL void set(int64 index, const QoreValue &value);

   DLLLOCAL static void set(jarray array, Type elementType, jclass elementClass, int64 index, const QoreValue &value);

   DLLLOCAL static LocalReference<jarray> getNew(Type elementType, jclass elementClass, jsize size);

private:
   GlobalReference<jarray> array;
   GlobalReference<jclass> elementClass;
   Type elementType;
};

} // namespace jni

#endif // QORE_JNI_ARRAY_H_
