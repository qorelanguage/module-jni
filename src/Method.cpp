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
#include "Method.h"
#include <memory>
#include "Env.h"
#include "Object.h"

namespace jni {

//FIXME WIP

class MethodDescriptorParser {

public:
   MethodDescriptorParser(const std::string &descriptor) : descriptor(descriptor), pos(1) {
      //we assume that the descriptor is well-formed since it was successfully used to get the jmethodID
      assert(descriptor[0] == '(');
   }

   bool hasNextArg() {
      return descriptor[pos] != ')';
   }

   char nextArgType() {
      return descriptor[pos++];
   }

   char retValType() {
      assert(descriptor[pos] == ')');
      return descriptor[++pos];
   }

   void skipClassName() {
      while (descriptor[pos++] != ';')
         ; // nop
   }

private:
   const std::string descriptor;
   std::string::size_type pos;
};

/**
 *
 */
class InvalidArgumentException : public Exception {

public:
   /**
    * \brief Constructor.
    */
   InvalidArgumentException(std::string message) : message(std::move(message)) {
   }

   void convert(ExceptionSink *xsink) override {
      xsink->raiseException("JNI-ERROR", message.c_str());
   }

private:
   std::string message;
};

/**
 *
 */
class XsinkException : public Exception {

public:
   /**
    * \brief Constructor.
    */
   XsinkException(ExceptionSink &xsink) : sink(new ExceptionSink()) {
      sink->assimilate(xsink);
   }

   void convert(ExceptionSink *xsink) override {
      xsink->assimilate(*sink);
   }

private:
   std::unique_ptr<ExceptionSink> sink;
};

std::vector<jvalue> convertArgs(MethodDescriptorParser &descParser, const QoreValueList* args) {
   std::vector<jvalue> jargs(args == nullptr ? 0 : args->size());

   size_t index = 0;
   while (descParser.hasNextArg()) {
      if (index >= jargs.size()) {
         throw InvalidArgumentException("Too few arguments in a Java method invocation");
      }
      QoreValue qv = args->retrieveEntry(index);

      switch (descParser.nextArgType()) {
         case 'Z':
            jargs[index].z = qv.getAsBool() ? JNI_TRUE : JNI_FALSE;
            break;
         case 'B':
            jargs[index].b = static_cast<jbyte>(qv.getAsBigInt());
            break;
         case 'C':
            jargs[index].c = static_cast<jchar>(qv.getAsBigInt());
            break;
         case 'S':
            jargs[index].s = static_cast<jshort>(qv.getAsBigInt());
            break;
         case 'I':
            jargs[index].i = static_cast<jint>(qv.getAsBigInt());
            break;
         case 'J':
            jargs[index].j = static_cast<jlong>(qv.getAsBigInt());
            break;
         case 'F':
            jargs[index].f = static_cast<jfloat>(qv.getAsFloat());
            break;
         case 'D':
            jargs[index].d = static_cast<jdouble>(qv.getAsFloat());
            break;
         case 'L': {
            descParser.skipClassName();
            if (qv.getType() == NT_NOTHING) {
               jargs[index].l = nullptr;
               break;
            }
            if (qv.getType() != NT_OBJECT) {
               //TODO string, autobox ptimitives?
               throw InvalidArgumentException("A Java object argument expected");
            }
            QoreObject *o = qv.get<QoreObject>();
            if (o->getClass() != QC_OBJECT) {
               //TODO class, throwable, arrays?
               throw InvalidArgumentException("A Java object argument expected");
            }
            ExceptionSink xsink;
            SimpleRefHolder<Object> obj(static_cast<Object *>(o->getReferencedPrivateData(CID_OBJECT, &xsink)));
            if (xsink) {
               throw XsinkException(xsink);
            }
            jargs[index].l = obj->getRef();
            break;
         }
//         case '[':
         default:
            assert(false);      //invalid descriptor - should not happen
      }
      ++index;
   }

   if (index != jargs.size()) {
      throw InvalidArgumentException("Too many arguments in a Java method invocation");
   }
   return std::move(jargs);
}

QoreValue Method::invokeStatic(const QoreValueList* args) {
   MethodDescriptorParser descParser(descriptor);
   std::vector<jvalue> jargs = convertArgs(descParser, args);

   Env env;
   switch (descParser.retValType()) {
      case 'V':
         env.callStaticVoidMethod(clazz->getRef(), id, &jargs[0]);
         return QoreValue();
      case 'Z':
         return QoreValue(env.callStaticBooleanMethod(clazz->getRef(), id, &jargs[0]) == JNI_TRUE);
      case 'B':
         return QoreValue(env.callStaticByteMethod(clazz->getRef(), id, &jargs[0]));
      case 'C':
         return QoreValue(env.callStaticCharMethod(clazz->getRef(), id, &jargs[0]));
      case 'S':
         return QoreValue(env.callStaticShortMethod(clazz->getRef(), id, &jargs[0]));
      case 'I':
         return QoreValue(env.callStaticIntMethod(clazz->getRef(), id, &jargs[0]));
      case 'J':
         return QoreValue(env.callStaticLongMethod(clazz->getRef(), id, &jargs[0]));
      case 'F':
         return QoreValue(env.callStaticFloatMethod(clazz->getRef(), id, &jargs[0]));
      case 'D':
         return QoreValue(env.callStaticDoubleMethod(clazz->getRef(), id, &jargs[0]));
      case 'L': {
         LocalReference<jobject> obj = env.callStaticObjectMethod(clazz->getRef(), id, &jargs[0]);
         //handle strings, throwables?, class?
         return QoreValue(new QoreObject(QC_OBJECT, getProgram(), new Object(std::move(obj))));
      }
//      case '[':
      default:
         assert(false);         //invalid descriptor - should not happen
         return QoreValue();
   }
}

} // namespace jni
