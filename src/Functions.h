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
/// \brief Defines the Functions class.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_FUNCTIONS_H_
#define QORE_JNI_FUNCTIONS_H_

#include "Env.h"
#include "Class.h"
#include "ModifiedUtf8String.h"

namespace jni {

/**
 * \brief Implementation of the global functions declared in ql_jni.qpp.
 */
class Functions {

public:
   static QoreStringNode *getVersion() {
      Env env;
      jint v = env.getVersion();
      QoreStringNode *str = new QoreStringNode();
      str->sprintf("%d.%d", v >> 16, v & 0xFFFF);
      return str;
   }

   static Class *loadClass(const QoreStringNode *name) {
      Env env;
      ModifiedUtf8String nameUtf8(name);
      printd(LogLevel, "loadClass %s\n", nameUtf8.c_str());
      return new Class(env.findClass(nameUtf8.c_str()));
   }

private:
   Functions() = delete;
};

} // namespace jni

#endif // QORE_JNI_FUNCTIONS_H_
