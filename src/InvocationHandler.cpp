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
#include "InvocationHandler.h"

namespace jni {

GlobalReference<jclass> InvocationHandler::clazz;
jmethodID InvocationHandler::ctorId;
jmethodID InvocationHandler::derefId;

static void JNICALL invocation_handler_finalize(JNIEnv *, jclass, jlong ptr) {
   delete reinterpret_cast<Dispatcher *>(ptr);
}

static jobject JNICALL invocation_handler_invoke(JNIEnv *jenv, jobject, jlong ptr, jobject proxy, jobject method, jobjectArray args) {
   Env env(jenv);
   Dispatcher *dispatcher = reinterpret_cast<Dispatcher *>(ptr);
   return dispatcher->dispatch(env, proxy, method, args);
}

static JNINativeMethod invocationHandlerNativeMethods[2] = {
      {
            const_cast<char *>("finalize0"),
            const_cast<char *>("(J)V"),
            reinterpret_cast<void*>(invocation_handler_finalize)
      },
      {
            const_cast<char *>("invoke0"),
            const_cast<char *>("(JLjava/lang/Object;Ljava/lang/reflect/Method;[Ljava/lang/Object;)Ljava/lang/Object;"),
            reinterpret_cast<void*>(invocation_handler_invoke)
      }
};

void InvocationHandler::init() {
   Env env;
   clazz = env.findClass("org/qore/jni/InvocationHandlerImpl").makeGlobal();
   env.registerNatives(clazz, invocationHandlerNativeMethods, 2);
   ctorId = env.getMethod(clazz, "<init>", "(J)V");
   derefId = env.getMethod(clazz, "deref", "()V");
}

void InvocationHandler::cleanup() {
   clazz = nullptr;
}

InvocationHandler::InvocationHandler(std::unique_ptr<Dispatcher> dispatcher) {
   Env env;
   jvalue arg;
   arg.j = reinterpret_cast<jlong>(dispatcher.get());
   LocalReference<jobject> obj = env.newObject(clazz, ctorId, &arg);
   dispatcher.release();      //from now on, the Java instance of InvocationHandlerImpl is responsible for the dispatcher
   handler = obj.makeGlobal();
}

InvocationHandler::InvocationHandler(const ResolvedCallReferenceNode *callback)
      : InvocationHandler(std::unique_ptr<Dispatcher>(new QoreCodeDispatcher(callback))) {
}

void InvocationHandler::destroy() {
   Env env;
   env.callVoidMethod(handler, derefId, nullptr);
}

} // namespace jni
