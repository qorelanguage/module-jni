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
/// \brief Defines the Method class.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_METHOD_H_
#define QORE_JNI_METHOD_H_

#include <qore/Qore.h>
#include "Class.h"
#include "Globals.h"
#include "Env.h"

#include <classfile_constants.h>

namespace jni {

class QoreJniClassMap;

/**
 * \brief Represents a Java method.
 */
class BaseMethod : public ObjectBase {
public:
   /**
    * \brief Constructor.
    * \param cls the class associated with the method id
    * \param id the method id
    * \param isStatic true if the method is static
    */
   BaseMethod(Class *cls, jmethodID id, bool isStatic) : cls(cls), id(id) {
      printd(LogLevel, "BaseMethod::BaseMethod(), this: %p, cls: %p, id: %p\n", this, cls, id);
      Env env;
      method = env.toReflectedMethod(cls->getJavaObject(), id, isStatic).makeGlobal();
      init(env);
      cls->ref();
   }

   /**
    * \brief Constructor.
    * \param method an instance of java.lang.reflect.Method
    */
   DLLLOCAL BaseMethod(jobject method, Class* cls) : cls(cls) {
      Env env;
      id = env.fromReflectedMethod(method);
      this->method = GlobalReference<jobject>::fromLocal(method);
      printd(LogLevel, "Method::Method(), this: %p, cls: %p, id: %p\n", this, cls, id);
      init(env);
   }

   ~BaseMethod() {
      printd(LogLevel, "BaseMethod::~BaseMethod(), this: %p, cls: %p, id: %p\n", this, cls, id);
   }

   /**
    * \brief throws a BasicException when the passed object's class does not match the expected class
    */
   DLLLOCAL void doObjectException(Env& env, jobject object);

   /**
    * \brief Invokes an instance method.
    * \param object the instance
    * \param args the arguments
    * \param base the offset in args for the arguments
    * \return the return value
    * \throws Exception if the arguments do not match the descriptor or if the method throws
    */
   QoreValue invoke(jobject object, const QoreValueList* args, int offset = 0);

   /**
    * \brief Invokes an instance method non-virtually.
    * \param object the instance
    * \param args the arguments
    * \param base the offset in args for the arguments
    * \return the return value
    * \throws Exception if the arguments do not match the descriptor or if the method throws
    */
   QoreValue invokeNonvirtual(jobject object, const QoreValueList* args, int offset = 0);

   /**
    * \brief Invokes a static method.
    * \param args the arguments
    * \return the return value
    * \throws Exception if the arguments do not match the descriptor or if the method throws
    */
   QoreValue invokeStatic(const QoreValueList* args, int offset = 0);

   /**
    * \brief Creates a new object by invoking a constructor.
    * \param args the arguments
    * \return the return value
    * \throws Exception if the arguments do not match the descriptor or if the method throws
    */
   QoreValue newInstance(const QoreValueList* args);

   /**
    * \brief Creates a new Qore object by invoking a constructor.
    * \param args the arguments
    * \return the return value
    * \throws Exception if the arguments do not match the descriptor or if the method throws
    */
   LocalReference<jobject> newQoreInstance(const QoreValueList* args);

   void getName(QoreString& str);

   int isStatic() const {
      return mods & JVM_ACC_STATIC;
   }

   jobject getJavaObject() const override {
      return method;
   }

   ClassAccess getAccess() const {
      ClassAccess access = Public;

      if (mods & JVM_ACC_PRIVATE)
         access = Internal;
      else if (mods & JVM_ACC_PROTECTED)
         access = Private;

      return access;
   }

   bool isAbstract() const {
      return mods & JVM_ACC_ABSTRACT;
   }

   int64 getFlags() const {
      Env env;
      int64 flags = QC_NO_FLAGS;
      if (env.callBooleanMethod(method, Globals::methodMethodIsVarArgs, nullptr))
         flags |= QC_USES_EXTRA_ARGS;
      return flags;
   }

   DLLLOCAL int getParamTypes(type_vec_t& paramTypeInfo, QoreJniClassMap& clsmap);

   DLLLOCAL const QoreTypeInfo* getReturnTypeInfo(QoreJniClassMap& clsmap);

protected:
   DLLLOCAL BaseMethod() {
   }

   std::vector<jvalue> convertArgs(const QoreValueList* args, size_t base = 0);

   void init(Env &env) {
      retValClass = env.callObjectMethod(method, Globals::methodMethodGetReturnType, nullptr).as<jclass>().makeGlobal();
      retValType = Globals::getType(retValClass);

      LocalReference<jobjectArray> paramTypesArray = env.callObjectMethod(method, Globals::methodMethodGetParameterTypes, nullptr).as<jobjectArray>();
      jsize paramCount = env.getArrayLength(paramTypesArray);
      paramTypes.reserve(paramCount);
      for (jsize p = 0; p < paramCount; ++p) {
         LocalReference<jclass> paramType = env.getObjectArrayElement(paramTypesArray, p).as<jclass>();
         paramTypes.emplace_back(Globals::getType(paramType), paramType.makeGlobal());
      }

      mods = env.callIntMethod(method, Globals::methodMethodGetModifiers, nullptr);
   }

protected:
   Class* cls;
   jmethodID id;
   GlobalReference<jobject> method;             // the instance of java.lang.reflect.Method
   GlobalReference<jclass> retValClass;
   Type retValType;
   std::vector<std::pair<Type, GlobalReference<jclass>>> paramTypes;
   // method modifiers
   int mods;
};

class Method : public BaseMethod {
public:
   /**
    * \brief Constructor.
    * \param cls the class associated with the method id
    * \param id the method id
    * \param isStatic true if the method is static
    */
   DLLLOCAL Method(Class *cls, jmethodID id, bool isStatic) : BaseMethod(cls, id, isStatic), cls_holder(cls) {
      cls->ref();
      cls_holder = cls;
   }

   /**
    * \brief Constructor.
    * \param method an instance of java.lang.reflect.Method
    */
   DLLLOCAL Method(jobject method) {
      Env env;
      cls = new Class(env.callObjectMethod(method, Globals::methodMethodGetDeclaringClass, nullptr).as<jclass>());
      cls_holder = cls;
      id = env.fromReflectedMethod(method);
      this->method = GlobalReference<jobject>::fromLocal(method);
      printd(LogLevel, "Method::Method(), this: %p, cls: %p, id: %p\n", this, cls, id);
      init(env);
   }

private:
   SimpleRefHolder<Class> cls_holder;
};

} // namespace jni

#endif // QORE_JNI_METHOD_H_
