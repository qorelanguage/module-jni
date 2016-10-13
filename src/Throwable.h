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
/// \brief Defines the Throwable class.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_THROWABLE_H_
#define QORE_JNI_THROWABLE_H_

#include <qore/Qore.h>
#include "LocalReference.h"
#include "Object.h"

extern QoreClass* QC_JAVATHROWABLE;
extern qore_classid_t CID_JAVATHROWABLE;

namespace jni {

/**
 * \brief Represents a Java Throwable.
 */
class Throwable : public ObjectBase {

public:
   /**
    * \brief Constructor.
    * \param throwable a local reference to a Java Throwable instance
    * \throws JavaException if a global reference cannot be created
    */
   Throwable(jthrowable throwable) : throwable(GlobalReference<jthrowable>::fromLocal(throwable)) {
      printd(LogLevel, "Throwable::Throwable(), this: %p, object: %p\n", this, static_cast<jthrowable>(this->throwable));
   }

   ~Throwable() {
      printd(LogLevel, "Throwable::~Throwable(), this: %p, object: %p\n", this, static_cast<jthrowable>(this->throwable));
   }

   jthrowable getJavaObject() const override {
      return throwable;
   }

private:
   GlobalReference<jthrowable> throwable;
};

} // namespace jni

#endif // QORE_JNI_THROWABLE_H_
