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
#include "Dispatcher.h"
#include "Array.h"
#include "Method.h"
#include "QoreToJava.h"

namespace jni {

QoreCodeDispatcher::QoreCodeDispatcher(const ResolvedCallReferenceNode *callback) : callback(callback->refRefSelf()) {
   printd(LogLevel, "QoreCodeDispatcher::QoreCodeDispatcher(), this: %p\n", this);
}

QoreCodeDispatcher::~QoreCodeDispatcher() {
   try {
      qoreThreadAttacher.attach();
   } catch (Exception &e) {
      printd(LogLevel, "~QoreCodeDispatcher() - unable to attach thread to Qore, this: %p", this);
      return;
   }
   printd(LogLevel, "QoreCodeDispatcher::~QoreCodeDispatcher(), this: %p\n", this);
   ExceptionSink xsink;
   callback->deref(&xsink);
   if (xsink) {
      QoreToJava::wrapException(xsink);
   }
}

jobject QoreCodeDispatcher::dispatch(Env &env, jobject proxy, jobject method, jobjectArray jargs) {
   try {
      qoreThreadAttacher.attach();
   } catch (Exception &e) {
      env.throwNew(env.findClass("java/lang/RuntimeException"), "Unable to attach thread to Qore");
      return nullptr;
   }

   printd(LogLevel, "QoreCodeDispatcher::dispatch(), this: %p\n", this);

   ExceptionSink xsink;
   {
      ReferenceHolder<QoreListNode> args(new QoreListNode(), &xsink);
      int mods = env.callIntMethod(method, Globals::methodMethodGetModifiers, nullptr);
      args->push(new QoreObject(mods & 8 ? QC_STATICMETHOD : QC_METHOD, getProgram(), new Method(method)));
      args->push(jargs == nullptr ? nullptr : new QoreObject(QC_ARRAY, getProgram(), new Array(jargs)));
      QoreValue qv = callback->execValue(*args, &xsink);
      if (xsink) {
         QoreToJava::wrapException(xsink);
         return nullptr;
      }
      return QoreToJava::toObject(qv, nullptr);
   }
}

} // namespace jni
