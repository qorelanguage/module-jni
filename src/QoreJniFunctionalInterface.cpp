//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2016 - 2018 Qore Technologies, s.r.o.
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

#include "QoreJniFunctionalInterface.h"
#include "QoreToJava.h"

namespace jni {
QoreJniFunctionalInterface::QoreJniFunctionalInterface(jobject obj) : obj(GlobalReference<jobject>::fromLocal(obj)) {
    Env env;

    cls = new Class(env.callObjectMethod(obj, Globals::methodObjectGetClass, nullptr).as<jclass>());
    LocalReference<jobjectArray> methods = cls->getDeclaredMethods();

    jsize num_methods = env.getArrayLength(methods);
    if (num_methods != 1) {
        QoreStringMaker str("Java class for Java object to be used as a Qore closure has %d methods; must have 1 call() method to be used as a Qore closure", static_cast<int>(num_methods));
        throw BasicException(str.c_str());
    }

    // get Method object
    LocalReference<jobject> m = env.getObjectArrayElement(methods, 0);
    method = new BaseMethod(m, *cls);

    QoreString name;
    method->getName(name);
    if (name != "call") {
        QoreStringMaker str("Java class for Java object to be used as a Qore closure has a single method named '%s()'; it must have a method named 'call()' instead", name.c_str());
        throw BasicException(str.c_str());
    }
}

QoreValue QoreJniFunctionalInterface::execValue(const QoreListNode* args, ExceptionSink* xsink) const {
    try {
        return method->isStatic() ? method->invokeStatic(args) : method->invoke(obj, args);
    } catch (jni::Exception& e) {
        e.convert(xsink);
        QoreToJava::wrapException(*xsink);
        return QoreValue();
    }
}

int QoreJniFunctionalInterface::getAsString(QoreString& str, int foff, ExceptionSink* xsink) const {
    str.concat("java closure reference");
    return 0;
}

QoreString* QoreJniFunctionalInterface::getAsString(bool& del, int foff, ExceptionSink* xsink) const {
    del = true;
    return new QoreString("java closure reference");
}
}