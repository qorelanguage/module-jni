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
#include "Globals.h"
#include "Env.h"

extern QoreClass* QC_FIELD;
extern QoreClass* QC_STATICFIELD;

namespace jni {

/**
 * \brief Represents a Java field.
 */
class Field : public ObjectBase {

public:
   /**
    * \brief Constructor.
    * \param clazz the class associated with the field id
    * \param id the field id
    * \param isStatic true if the field is static
    */
   Field(Class *clazz, jfieldID id, bool isStatic) : clazz(clazz), id(id) {
      printd(LogLevel, "Field::Field(), this: %p, clazz: %p, id: %p\n", this, clazz, id);
      clazz->ref();
      Env env;
      field = env.toReflectedField(clazz->getJavaObject(), id, isStatic).makeGlobal();
      init(env);
   }

   /**
    * \brief Constructor.
    * \param field an instance of java.lang.reflect.Field
    */
   Field(jobject field) {
      Env env;
      clazz = new Class(env.callObjectMethod(field, Globals::methodFieldGetDeclaringClass, nullptr).as<jclass>());
      id = env.fromReflectedField(field);
      this->field = GlobalReference<jobject>::fromLocal(field);
      printd(LogLevel, "Field::Field(), this: %p, clazz: %p, id: %p\n", this, *clazz, id);
      init(env);
   }

   ~Field() {
      printd(LogLevel, "Field::~Field(), this: %p, clazz: %p, id: %p\n", this, *clazz, id);
   }

   /**
    * \brief Gets the value of an instance field.
    * \param object the instance
    * \return the value of the static field
    * \throws Exception if the value cannot be retrieved
    */
   QoreValue get(jobject object);

   /**
    * \brief Sets the value of an instance field.
    * \param object the instance
    * \param value the new value
    * \throws Exception if the value cannot be set
    */
   void set(jobject object, const QoreValue &value);

   /**
    * \brief Gets the value of a static field.
    * \return the value of the static field
    * \throws Exception if the value cannot be retrieved
    */
   QoreValue getStatic();

   /**
    * \brief Sets the value of a static field.
    * \param value the new value
    * \throws Exception if the value cannot be set
    */
   void setStatic(const QoreValue &value);

   jobject getJavaObject() const override {
      return field;
   }

private:
   void init(Env &env) {
      typeClass = env.callObjectMethod(field, Globals::methodFieldGetType, nullptr).as<jclass>().makeGlobal();
      type = Globals::getType(typeClass);
   }

private:
   SimpleRefHolder<Class> clazz;
   jfieldID id;
   GlobalReference<jobject> field;              // the instance of java.lang.reflect.Field
   GlobalReference<jclass> typeClass;           // the type of the field
   Type type;
};

} // namespace jni

#endif // QORE_JNI_FIELD_H_
