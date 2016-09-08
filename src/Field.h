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
/// \brief Defines the Field class.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_FIELD_H_
#define QORE_JNI_FIELD_H_

#include <qore/Qore.h>
#include "Class.h"

extern QoreClass* QC_FIELD;

namespace jni {

/**
 * \brief Represents a Java field.
 */
class Field : public AbstractPrivateData {

public:
   /**
    * \brief Constructor.
    * \param clazz the class associated with the field id
    * \param id the field id
    * \param desc the descriptor
    */
   Field(Class *clazz, jfieldID id, std::string desc) : clazz(clazz), id(id), descriptor(std::move(desc)) {
      printd(LogLevel, "Field::Field(), this: %p, clazz: %p, id: %p\n", this, clazz, id);
   }

   ~Field() {
      printd(LogLevel, "Field::~Field(), this: %p, clazz: %p, id: %p\n", this, *clazz, id);
   }

   /**
    * \brief Gets the value of a static field.
    * \return the value of the static field
    * \throws Exception if the value cannot be retrieved
    */
   QoreValue getStatic();

private:
   SimpleRefHolder<Class> clazz;
   jfieldID id;
   std::string descriptor;
};

} // namespace jni

#endif // QORE_JNI_FIELD_H_
