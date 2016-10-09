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
#include "Globals.h"
#include "Env.h"

extern QoreClass* QC_METHOD;
extern QoreClass* QC_STATICMETHOD;
extern QoreClass* QC_CONSTRUCTOR;

namespace jni {

/**
 * \brief Represents a Java method.
 */
class Method : public ObjectBase {

public:
   /**
    * \brief Constructor.
    * \param cls the class associated with the method id
    * \param id the method id
    * \param isStatic true if the method is static
    */
   Method(Class *cls, jmethodID id, bool isStatic) : cls(cls), id(id) {
      printd(LogLevel, "Method::Method(), this: %p, cls: %p, id: %p\n", this, cls, id);
      Env env;
      method = env.toReflectedMethod(cls->getJavaObject(), id, isStatic).makeGlobal();
      init(env);
      cls->ref();
   }

   /**
    * \brief Constructor.
    * \param method an instance of java.lang.reflect.Method
    */
   Method(jobject method) {
      Env env;
      cls = new Class(env.callObjectMethod(method, Globals::methodMethodGetDeclaringClass, nullptr).as<jclass>());
      id = env.fromReflectedMethod(method);
      this->method = GlobalReference<jobject>::fromLocal(method);
      printd(LogLevel, "Method::Method(), this: %p, cls: %p, id: %p\n", this, *cls, id);
      init(env);
   }

   ~Method() {
      printd(LogLevel, "Method::~Method(), this: %p, cls: %p, id: %p\n", this, *cls, id);
   }

   /**
    * \brief Invokes an instance method.
    * \param object the instance
    * \param args the arguments
    * \return the return value
    * \throws Exception if the arguments do not match the descriptor or if the method throws
    */
   QoreValue invoke(jobject object, const QoreValueList* args);

   /**
    * \brief Invokes an instance method non-virtually.
    * \param object the instance
    * \param args the arguments
    * \return the return value
    * \throws Exception if the arguments do not match the descriptor or if the method throws
    */
   QoreValue invokeNonvirtual(jobject object, const QoreValueList* args);

   /**
    * \brief Invokes a static method.
    * \param args the arguments
    * \return the return value
    * \throws Exception if the arguments do not match the descriptor or if the method throws
    */
   QoreValue invokeStatic(const QoreValueList* args);

   /**
    * \brief Creates a new object by invoking a constructor.
    * \param args the arguments
    * \return the return value
    * \throws Exception if the arguments do not match the descriptor or if the method throws
    */
   QoreValue newInstance(const QoreValueList* args);

   jobject getJavaObject() const override {
      return method;
   }

private:
   std::vector<jvalue> convertArgs(const QoreValueList* args, size_t base = 0);

   void init(Env &env) {
      retValType = Globals::getType(env.callObjectMethod(method, Globals::methodMethodGetReturnType, nullptr).as<jclass>());

      LocalReference<jobjectArray> paramTypesArray = env.callObjectMethod(method, Globals::methodMethodGetParameterTypes, nullptr).as<jobjectArray>();
      jsize paramCount = env.getArrayLength(paramTypesArray);
      paramTypes.reserve(paramCount);
      for (jsize p = 0; p < paramCount; ++p) {
         LocalReference<jclass> paramType = env.getObjectArrayElement(paramTypesArray, p).as<jclass>();
         paramTypes.emplace_back(Globals::getType(paramType), paramType.makeGlobal());
      }
   }

private:
   SimpleRefHolder<Class> cls;
   jmethodID id;
   GlobalReference<jobject> method;             // the instance of java.lang.reflect.Method
   Type retValType;
   std::vector<std::pair<Type, GlobalReference<jclass>>> paramTypes;
};

} // namespace jni

#endif // QORE_JNI_METHOD_H_
