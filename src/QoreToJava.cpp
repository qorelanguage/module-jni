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

#include "QoreToJava.h"

namespace jni {

LocalReference<jobject> QoreToJava::toObject(const QoreValue &value, jclass cls) {
   if (value.isNullOrNothing())
      return nullptr;

   LocalReference<jobject> javaObjectRef;
   switch (value.getType()) {
      case NT_STRING: {
	 ModifiedUtf8String str(*value.get<QoreStringNode>());
	 Env env;
	 javaObjectRef = env.newString(str.c_str()).release();
	 break;
      }
      case NT_LIST: {
	 javaObjectRef = qjcm.getJavaArray(value.get<QoreListNode>(), cls).as<jobject>();
	 break;
      }
      case NT_OBJECT: {
	 const QoreObject* o = value.get<QoreObject>();

	 javaObjectRef = qjcm.getJavaObject(o);
	 if (!javaObjectRef) {
	    ExceptionSink xsink;
	    TryPrivateDataRefHolder<ObjectBase> jo(o, CID_JAVAOBJECT, &xsink);
	    if (!jo) {
	       if (xsink)
		  throw XsinkException(xsink);
	       QoreStringMaker desc("A Java object argument expected; got object of class '%s' instead", o->getClassName());
	       throw BasicException(desc.c_str());
	    }

	    javaObjectRef = jo->getLocalReference();
	 }
	 break;
      }
      default: {
	 QoreStringMaker desc("A Java object argument expected; got type '%s' instead", value.getTypeName());
	 throw BasicException(desc.c_str());
      }
   }

   if (cls) {
      Env env;
      if (!env.isInstanceOf(javaObjectRef, cls)) {
	 LocalReference<jstring> clsName = env.callObjectMethod(cls, Globals::methodClassGetCanonicalName, nullptr).as<jstring>();
	 Env::GetStringUtfChars cname(env, clsName);

	 LocalReference<jclass> ocls = env.getObjectClass(javaObjectRef);

	 LocalReference<jstring> oclsName = env.callObjectMethod(ocls, Globals::methodClassGetCanonicalName, nullptr).as<jstring>();
	 Env::GetStringUtfChars ocname(env, oclsName);

	 QoreStringMaker str("expected class '%s'; instead got an object of class '%s'", cname.c_str(), ocname.c_str());
	 throw BasicException(str.c_str());
      }
   }
   return javaObjectRef;
}

}
