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
#include "Class.h"
#include "Env.h"
#include "Field.h"
#include "Globals.h"
#include "ModifiedUtf8String.h"
#include "Method.h"

namespace jni {
Class::~Class() {
   printd(LogLevel, "Class::~Class(), this: %p, cls: %p\n", this, static_cast<jclass>(this->cls));
   for (auto& i : mlist)
      delete i;
}

void Class::trackMethod(BaseMethod* m) {
   mlist.push_back(m);
}

Field* Class::getField(const QoreStringNode* name, const QoreStringNode* descriptor) {
   Env env;
   ModifiedUtf8String nameUtf8(*name);
   ModifiedUtf8String descUtf8(*descriptor);
   printd(LogLevel, "getField %s %s\n", nameUtf8.c_str(), descUtf8.c_str());
   return new Field(this, env.getField(cls, nameUtf8.c_str(), descUtf8.c_str()), false);
}

Field* Class::getStaticField(const QoreStringNode* name, const QoreStringNode* descriptor) {
   Env env;
   ModifiedUtf8String nameUtf8(*name);
   ModifiedUtf8String descUtf8(*descriptor);
   printd(LogLevel, "getStaticField %s %s\n", nameUtf8.c_str(), descUtf8.c_str());
   return new Field(this, env.getStaticField(cls, nameUtf8.c_str(), descUtf8.c_str()), true);
}

Method* Class::getMethod(const QoreStringNode* name, const QoreStringNode* descriptor) {
   Env env;
   ModifiedUtf8String nameUtf8(*name);
   ModifiedUtf8String descUtf8(*descriptor);
   printd(LogLevel, "getMethod %s %s\n", nameUtf8.c_str(), descUtf8.c_str());
   return new Method(this, env.getMethod(cls, nameUtf8.c_str(), descUtf8.c_str()), false);
}

Method* Class::getStaticMethod(const QoreStringNode* name, const QoreStringNode* descriptor) {
   Env env;
   ModifiedUtf8String nameUtf8(*name);
   ModifiedUtf8String descUtf8(*descriptor);
   return new Method(this, env.getStaticMethod(cls, nameUtf8.c_str(), descUtf8.c_str()), true);
}

Method* Class::getConstructor(const QoreStringNode* descriptor) {
   Env env;
   ModifiedUtf8String descUtf8(*descriptor);
   printd(LogLevel, "getConstructor %s\n", descUtf8.c_str());
   return new Method(this, env.getMethod(cls, "<init>", descUtf8.c_str()), false);
}

Class* Class::getSuperClass() {
   Env env;

   LocalReference<jclass> parentClass = env.callObjectMethod(cls, Globals::methodClassGetSuperClass, nullptr).as<jclass>();
   if (!parentClass)
      return 0;

   return new Class(parentClass);
}

LocalReference<jobjectArray> Class::getInterfaces() {
   Env env;
   return env.callObjectMethod(cls, Globals::methodClassGetInterfaces, nullptr).as<jobjectArray>();
}

LocalReference<jobjectArray> Class::getDeclaredConstructors() {
   Env env;
   return env.callObjectMethod(cls, Globals::methodClassGetDeclaredConstructors, nullptr).as<jobjectArray>();
}

LocalReference<jobjectArray> Class::getDeclaredMethods() {
   Env env;
   return env.callObjectMethod(cls, Globals::methodClassGetDeclaredMethods, nullptr).as<jobjectArray>();
}

LocalReference<jobjectArray> Class::getDeclaredFields() {
   Env env;
   return env.callObjectMethod(cls, Globals::methodClassGetDeclaredFields, nullptr).as<jobjectArray>();
}

int Class::getModifiers() {
   Env env;
   return env.callIntMethod(cls, Globals::methodClassGetModifiers, nullptr);
}

bool Class::isInstance(const ObjectBase* obj) {
   Env env;
   return env.isInstanceOf(obj->getJavaObject(), cls) == JNI_TRUE;
}

} // namespace jni
