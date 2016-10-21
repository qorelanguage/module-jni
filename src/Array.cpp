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

QoreValue Array::get(int64 index, bool to_qore) {
   Env env;
   switch (elementType) {
      case Type::Boolean:
         return JavaToQore::convert(env.getBooleanArrayElement(array.cast<jbooleanArray>(), index));
      case Type::Byte:
         return JavaToQore::convert(env.getByteArrayElement(array.cast<jbyteArray>(), index));
      case Type::Char:
         return JavaToQore::convert(env.getCharArrayElement(array.cast<jcharArray>(), index));
      case Type::Short:
         return JavaToQore::convert(env.getShortArrayElement(array.cast<jshortArray>(), index));
      case Type::Int:
         return JavaToQore::convert(env.getIntArrayElement(array.cast<jintArray>(), index));
      case Type::Long:
         return JavaToQore::convert(env.getLongArrayElement(array.cast<jlongArray>(), index));
      case Type::Float:
         return JavaToQore::convert(env.getFloatArrayElement(array.cast<jfloatArray>(), index));
      case Type::Double:
         return JavaToQore::convert(env.getDoubleArrayElement(array.cast<jdoubleArray>(), index));
      case Type::Reference:
      default:
         assert(elementType == Type::Reference);
         return to_qore
	    ? JavaToQore::convertToQore(env.getObjectArrayElement(array.cast<jobjectArray>(), index))
	    : JavaToQore::convert(env.getObjectArrayElement(array.cast<jobjectArray>(), index));
   }
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
         env.setObjectArrayElement((jobjectArray)array, index, QoreToJava::toObject(value, elementClass).release());
   }
}

} // namespace jni
