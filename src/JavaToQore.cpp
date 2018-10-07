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

#include <qore/Qore.h>

#include "QoreJniClassMap.h"
#include "Globals.h"
#include "JavaToQore.h"

namespace jni {

QoreValue JavaToQore::convertToQore(LocalReference<jobject> v) {
    if (!v) {
        return QoreValue();
    }

    Env env;

    // convert to Qore value if possible
    if (env.isInstanceOf(v, Globals::classString)) {
        Env::GetStringUtfChars chars(env, v.as<jstring>());
        return QoreValue(new QoreStringNode(chars.c_str(), QCS_UTF8));
    }

    if (env.isInstanceOf(v, Globals::classZonedDateTime)) {
        LocalReference<jstring> date_str = env.callObjectMethod(v,
            Globals::methodZonedDateTimeToString, nullptr).as<jstring>();
        Env::GetStringUtfChars chars(env, date_str);
        return QoreValue(new DateTimeNode(chars.c_str()));
    }

    if (env.isInstanceOf(v, Globals::classQoreObject)) {
        return reinterpret_cast<QoreObject*>(env.callLongMethod(v,
            Globals::methodQoreObjectGet, nullptr));
    }

    return qjcm.getValue(v);
}

} // namespace jni
