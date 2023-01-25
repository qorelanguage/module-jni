//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2016 - 2022 Qore Technologies, s.r.o.
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

QoreValue JavaToQore::convertToQore(LocalReference<jobject> v, QoreProgram* pgm, bool compat_types) {
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

    if (env.isInstanceOf(v, Globals::classTimestamp)) {
        LocalReference<jstring> date_str = env.callObjectMethod(v,
            Globals::methodTimestampToString, nullptr).as<jstring>();
        Env::GetStringUtfChars chars(env, date_str);
        return QoreValue(new DateTimeNode(chars.c_str()));
    }

    if (env.isInstanceOf(v, Globals::classBigDecimal)) {
        LocalReference<jstring> num_str = env.callObjectMethod(v,
            Globals::methodBigDecimalToString, nullptr).as<jstring>();
        Env::GetStringUtfChars chars(env, num_str);
        return QoreValue(new QoreNumberNode(chars.c_str()));
    }

    if (env.isInstanceOf(v, Globals::classQoreObjectBase)) {
        QoreObject* obj = reinterpret_cast<QoreObject*>(env.callLongMethod(v,
            Globals::methodQoreObjectBaseGet, nullptr));
        return obj->refSelf();
    }

    if (env.isInstanceOf(v, Globals::classQoreClosure)) {
        ResolvedCallReferenceNode* call = reinterpret_cast<ResolvedCallReferenceNode*>(env.callLongMethod(v,
            Globals::methodQoreClosureGet, nullptr));
        return call->refRefSelf();
    }

    if (env.isInstanceOf(v, Globals::classMap) && !JniExternalProgramData::compatTypes()) {
        // create hash from Map
        LocalReference<jobject> set = env.callObjectMethod(v,
            Globals::methodMapEntrySet, nullptr);
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
                LocalReference<jobject> key = env.callObjectMethod(element,
                    Globals::methodEntryGetKey, nullptr);

                // if key is not a string, then we cannot convert it to Qore
                if (!env.isInstanceOf(key, Globals::classString)) {
                    return qjcm.getValue(v, pgm, compat_types);
                }

                LocalReference<jobject> value = env.callObjectMethod(element,
                    Globals::methodEntryGetValue, nullptr);

                ValueHolder val(convertToQore(value.release(), pgm, compat_types), &xsink);
                if (xsink) {
                    break;
                }

                Env::GetStringUtfChars key_str(env, key.as<jstring>());
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

    if (env.isInstanceOf(v, Globals::classList)) {
        // create list from List
        jint size = env.callIntMethod(v, Globals::methodListSize, nullptr);

        ExceptionSink xsink;
        ReferenceHolder<QoreListNode> rv(new QoreListNode(autoTypeInfo), &xsink);

        jint pos = 0;
        while (pos < size) {
            std::vector<jvalue> jargs(1);
            jargs[0].i = pos++;

            LocalReference<jobject> value = env.callObjectMethod(v,
                Globals::methodListGet, &jargs[0]);

            ValueHolder val(convertToQore(value.release(), pgm, compat_types), &xsink);
            if (xsink) {
                break;
            }

            rv->push(val.release(), &xsink);
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

    return qjcm.getValue(v, pgm, compat_types);
}

} // namespace jni
