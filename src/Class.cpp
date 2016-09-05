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
#include "Class.h"
#include "Env.h"
#include "ModifiedUtf8String.h"
#include "Method.h"

namespace jni {

Method *Class::getMethod(const QoreStringNode *name, const QoreStringNode *descriptor) {
   Env env;
   ModifiedUtf8String nameUtf8(name);
   ModifiedUtf8String descUtf8(descriptor);
   printd(LogLevel, "getMethod %s %s\n", nameUtf8.c_str(), descUtf8.c_str());
   ref();
   return new Method(this, env.getMethod(clazz, nameUtf8.c_str(), descUtf8.c_str()));
}

Method *Class::getStaticMethod(const QoreStringNode *name, const QoreStringNode *descriptor) {
   Env env;
   ModifiedUtf8String nameUtf8(name);
   ModifiedUtf8String descUtf8(descriptor);
   printd(LogLevel, "getStaticMethod %s %s\n", nameUtf8.c_str(), descUtf8.c_str());
   ref();
   return new Method(this, env.getStaticMethod(clazz, nameUtf8.c_str(), descUtf8.c_str()));
}

} // namespace jni
