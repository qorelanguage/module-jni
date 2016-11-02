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
#include "Array.h"
#include "JavaToQore.h"
#include "QoreToJava.h"

namespace jni {

Array::Array(jclass ecls, int size) {
   LocalReference<jclass> cls(ecls);
   if (size < 1) {
      QoreStringMaker desc("cannot instantiate an array with size %d; must be greater than 0", size);
      throw BasicException(desc.c_str());
   }

   elementClass = cls.makeGlobal();
   elementType = Globals::getType(elementClass);

   array = Array::getNew(elementType, elementClass, (jsize)size).makeGlobal();
}

Array::Array(jarray array) : array(GlobalReference<jarray>::fromLocal(array)) {
   printd(LogLevel, "Array::Array(), this: %p, object: %p\n", this, static_cast<jarray>(this->array));

   Env env;
   LocalReference<jclass> arrayClass = env.getObjectClass(array);
   elementClass = env.callObjectMethod(arrayClass, Globals::methodClassGetComponentType, nullptr).as<jclass>().makeGlobal();
   elementType = Globals::getType(elementClass);
}

int64 Array::length() {
   Env env;
   return env.getArrayLength(array);
}

LocalReference<jarray> Array::getNew(Type elementType, jclass elementClass, jsize size) {
   Env env;

   switch (elementType) {
      case Type::Boolean: return env.newBooleanArray(size).as<jarray>();
      case Type::Byte: return env.newByteArray(size).as<jarray>();
      case Type::Char: return env.newCharArray(size).as<jarray>();
      case Type::Short: return env.newShortArray(size).as<jarray>();
      case Type::Int: return env.newIntArray(size).as<jarray>();
      case Type::Long: return env.newLongArray(size).as<jarray>();
      case Type::Float: return env.newFloatArray(size).as<jarray>();
      case Type::Double: return env.newDoubleArray(size).as<jarray>();
      case Type::Reference:
      default:
         assert(elementType == Type::Reference);
         return env.newObjectArray(size, elementClass).as<jarray>();
   }
}

QoreListNode* Array::getList(Env& env, jarray array, jclass arrayClass) {
   LocalReference<jclass> elementClass = env.callObjectMethod(arrayClass, Globals::methodClassGetComponentType, nullptr).as<jclass>();
   Type elementType = Globals::getType(elementClass);

   ExceptionSink xsink;
   ReferenceHolder<QoreListNode> l(new QoreListNode, &xsink);

   for (jsize i = 0, e = env.getArrayLength(array); i < e; ++i) {
      l->push(get(env, array, elementType, elementClass, i).takeNode());
   }

   return l.release();
}

QoreValue Array::get(Env& env, jarray array, Type elementType, jclass elementClass, int64 index) {
   switch (elementType) {
      case Type::Boolean:
         return JavaToQore::convert(env.getBooleanArrayElement(static_cast<jbooleanArray>(array), index));
      case Type::Byte:
         return JavaToQore::convert(env.getByteArrayElement(static_cast<jbyteArray>(array), index));
      case Type::Char:
         return JavaToQore::convert(env.getCharArrayElement(static_cast<jcharArray>(array), index));
      case Type::Short:
         return JavaToQore::convert(env.getShortArrayElement(static_cast<jshortArray>(array), index));
      case Type::Int:
         return JavaToQore::convert(env.getIntArrayElement(static_cast<jintArray>(array), index));
      case Type::Long:
         return JavaToQore::convert(env.getLongArrayElement(static_cast<jlongArray>(array), index));
      case Type::Float:
         return JavaToQore::convert(env.getFloatArrayElement(static_cast<jfloatArray>(array), index));
      case Type::Double:
         return JavaToQore::convert(env.getDoubleArrayElement(static_cast<jdoubleArray>(array), index));
      case Type::Reference:
      default:
         assert(elementType == Type::Reference);
         return JavaToQore::convertToQore(env.getObjectArrayElement(static_cast<jobjectArray>(array), index));
   }
}

QoreValue Array::get(int64 index) {
   Env env;
   return get(env, array, elementType, elementClass, index);
}

void Array::set(int64 index, const QoreValue &value) {
   set(array, elementType, elementClass, index, value);
}

void Array::set(jarray array, Type elementType, jclass elementClass, int64 index, const QoreValue &value) {
   Env env;
   switch (elementType) {
      case Type::Boolean:
         env.setBooleanArrayElement((jbooleanArray)array, index, QoreToJava::toBoolean(value));
         break;
      case Type::Byte:
         env.setByteArrayElement((jbyteArray)array, index, QoreToJava::toByte(value));
         break;
      case Type::Char:
         env.setCharArrayElement((jcharArray)array, index, QoreToJava::toChar(value));
         break;
      case Type::Short:
         env.setShortArrayElement((jshortArray)array, index, QoreToJava::toShort(value));
         break;
      case Type::Int:
         env.setIntArrayElement((jintArray)array, index, QoreToJava::toInt(value));
         break;
      case Type::Long:
         env.setLongArrayElement((jlongArray)array, index, QoreToJava::toLong(value));
         break;
      case Type::Float:
         env.setFloatArrayElement((jfloatArray)array, index, QoreToJava::toFloat(value));
         break;
      case Type::Double:
         env.setDoubleArrayElement((jdoubleArray)array, index, QoreToJava::toDouble(value));
         break;
      case Type::Reference:
      default:
         assert(elementType == Type::Reference);
         env.setObjectArrayElement((jobjectArray)array, index, QoreToJava::toObject(value, elementClass));
   }
}

jclass Array::getClassForValue(QoreValue v) {
   switch (v.getType()) {
      case NT_INT: return Globals::classPrimitiveInt.toLocal();
      case NT_FLOAT: return Globals::classPrimitiveDouble.toLocal();
      case NT_BOOLEAN: return Globals::classPrimitiveBoolean.toLocal();
      case NT_STRING: return Globals::classString.toLocal();
      case NT_OBJECT: {
         QoreObject* o = v.get<QoreObject>();
         ExceptionSink xsink;
         SimpleRefHolder<QoreJniPrivateData> obj(static_cast<QoreJniPrivateData*>(o->getReferencedPrivateData(CID_OBJECT, &xsink)));
         if (!obj) {
            if (xsink)
               throw XsinkException(xsink);
            QoreStringMaker desc("cannot create a Java array from an object of class '%s'", o->getClassName());
            throw BasicException(desc.c_str());
         }
         Env env;
         return env.getObjectClass(obj->getObject()).release();
      }
   }
   QoreStringMaker desc("cannot create a Java array from a list that contains values of type '%s'", v.getTypeName());
   throw BasicException(desc.c_str());
}

LocalReference<jarray> Array::toObjectArray(const QoreListNode* l, jclass elementClass) {
   Type elementType = Globals::getType(elementClass);

   LocalReference<jarray> jarray = getNew(elementType, elementClass, l->size());
   for (unsigned i = 0, e = l->size(); i != e; ++i) {
      set(jarray, elementType, elementClass, i, l->retrieve_entry(i));
   }

   return jarray.release();
}

LocalReference<jarray> Array::toJava(const QoreListNode* l) {
   if (l->empty())
      return nullptr;

   LocalReference<jclass> elementClass = 0;

   // check list to see if we have a unique type
   for (unsigned i = 0, e = l->size(); i != e; ++i) {
      // get this element's target Java class
      QoreValue v(l->retrieve_entry(i));
      if (v.isNullOrNothing())
         continue;
      LocalReference<jclass> eclass = getClassForValue(v);
      if (!elementClass)
         elementClass = eclass.release();
      else if (elementClass != eclass) {
         elementClass = Globals::classObject.toLocal();
         break;
      }
   }

   if (!elementClass)
      elementClass = Globals::classObject.toLocal();

   return toObjectArray(l, elementClass).release();
}

} // namespace jni
