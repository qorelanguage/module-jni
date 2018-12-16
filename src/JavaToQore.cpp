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

#include <qore/Qore.h>

#include "QoreJniClassMap.h"
#include "Globals.h"
#include "JavaToQore.h"
#include "QoreJniFunctionalInterface.h"

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

    if (env.isInstanceOf(v, Globals::classBigDecimal)) {
        LocalReference<jstring> num_str = env.callObjectMethod(v,
            Globals::methodBigDecimalToString, nullptr).as<jstring>();
        Env::GetStringUtfChars chars(env, num_str);
        return QoreValue(new QoreNumberNode(chars.c_str()));
    }

    if (env.isInstanceOf(v, Globals::classQoreObject)) {
        QoreObject* obj = reinterpret_cast<QoreObject*>(env.callLongMethod(v,
            Globals::methodQoreObjectGet, nullptr));
        return obj->refSelf();
    }

    if (env.isInstanceOf(v, Globals::classHashMap) && !JniExternalProgramData::compatTypes()) {
        // create hash from HashMap
        LocalReference<jobject> set = env.callObjectMethod(v,
            Globals::methodHashMapEntrySet, nullptr);
        if (!set) {
            return QoreValue();
        }
        LocalReference<jobject> i = env.callObjectMethod(set,
            Globals::methodSetIterator, nullptr);
        if (!i) {
            return QoreValue();
        }

        ExceptionSink xsink;
        ReferenceHolder<QoreHashNode> rv(new QoreHashNode(autoTypeInfo), &xsink);
        while (true) {
            if (!env.callBooleanMethod(i, Globals::methodIteratorHasNext, nullptr)) {
                break;
            }

            LocalReference<jobject> element = env.callObjectMethod(i,
                Globals::methodIteratorNext, nullptr);
            if (element) {
                LocalReference<jstring> key = env.callObjectMethod(element,
                    Globals::methodEntryGetKey, nullptr).as<jstring>();

                LocalReference<jobject> value = env.callObjectMethod(element,
                    Globals::methodEntryGetValue, nullptr);

                ValueHolder val(convertToQore(value.release()), &xsink);
                if (xsink) {
                    break;
                }

                Env::GetStringUtfChars key_str(env, key);
                rv->setKeyValue(key_str.c_str(), val.release(), &xsink);
                if (xsink) {
                    break;
                }
            }
        }

        if (xsink) {
            throw XsinkException(xsink);
        }

        return rv.release();
    }

    // for relative date/time values
    if (env.isInstanceOf(v, Globals::classQoreRelativeTime)) {
        int year = env.getIntField(v, Globals::fieldQoreRelativeTimeYear),
            month = env.getIntField(v, Globals::fieldQoreRelativeTimeMonth),
            day = env.getIntField(v, Globals::fieldQoreRelativeTimeDay),
            hour = env.getIntField(v, Globals::fieldQoreRelativeTimeHour),
            minute = env.getIntField(v, Globals::fieldQoreRelativeTimeMinute),
            second = env.getIntField(v, Globals::fieldQoreRelativeTimeSecond),
            us = env.getIntField(v, Globals::fieldQoreRelativeTimeUs);

        return QoreValue(DateTimeNode::makeRelative(year, month, day, hour, minute, second, us));
    }

    // for Qore closure / call references
    if (env.isInstanceOf(v, Globals::classQoreClosureMarker)) {
        return new QoreJniFunctionalInterface(v);
    }

    return qjcm.getValue(v);
}

} // namespace jni
