//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2015 Qore Technologies
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
/// \brief Defines the Method class.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_METHOD_H_
#define QORE_JNI_METHOD_H_

#include <qore/Qore.h>
#include "Class.h"

extern QoreClass* QC_METHOD;
extern QoreClass* QC_STATICMETHOD;

namespace jni {

/**
 * \brief Represents a Java method.
 */
class Method : public AbstractPrivateData {

public:
   /**
    * \brief Constructor.
    * \param clazz the class associated with the method id
    * \param id the method id
    * \param desc the descriptor
    */
   Method(Class *clazz, jmethodID id, std::string desc) : clazz(clazz), id(id), descriptor(std::move(desc)) {
      printd(LogLevel, "Method::Method(), this: %p, clazz: %p, id: %p\n", this, clazz, id);
   }

   ~Method() {
      printd(LogLevel, "Method::~Method(), this: %p, clazz: %p, id: %p\n", this, *clazz, id);
   }

   /**
    * \brief Invokes a static method.
    * \param args the arguments
    * \return the return value
    * \throws Exception if the arguments do not match the descriptor or if the method throws
    */
   QoreValue invokeStatic(const QoreValueList* args);

private:
   SimpleRefHolder<Class> clazz;
   jmethodID id;
   std::string descriptor;
};

} // namespace jni

#endif // QORE_JNI_METHOD_H_
