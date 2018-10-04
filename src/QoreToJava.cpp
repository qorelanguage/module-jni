//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2016 - 2017 Qore Technologies, s.r.o.
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

jobject QoreToJava::toAnyObject(const QoreValue& value) {
    Env env;
    switch (value.getType()) {
        case NT_BOOLEAN: {
            jvalue arg;
            arg.z = value.getAsBool();
            return env.newObject(Globals::classBoolean, Globals::ctorBoolean, &arg).release();
        }
        case NT_INT: {
            jvalue arg;
            arg.i = value.getAsBigInt();
            return env.newObject(Globals::classInteger, Globals::ctorInteger, &arg).release();
        }
        case NT_FLOAT: {
            jvalue arg;
            arg.d = value.getAsFloat();
            return env.newObject(Globals::classDouble, Globals::ctorDouble, &arg).release();
        }
        case NT_STRING: {
            ModifiedUtf8String str(*value.get<QoreStringNode>());
            Env env;
            return env.newString(str.c_str()).release();
        }
        case NT_DATE: {
            const DateTimeNode* d = value.get<const DateTimeNode>();
            if (d->isAbsolute()) {
                QoreString str;
                d->format(str, "YYYY-MM-DDTHH:mm:SS.xxZ");

                LocalReference<jstring> date_str = env.newString(str.c_str());
                std::vector<jvalue> jargs(1);
                jargs[0].l = date_str;
                return env.callStaticObjectMethod(Globals::classZonedDateTime, Globals::methodZonedDateTimeParse, &jargs[0]).release();
            } else {
                QoreStringMaker desc("cannot convert a relative date/time value to a Java object (expecting an absolute date/time value)", value.getTypeName());
                throw BasicException(desc.c_str());
            }
        }
        case NT_OBJECT: {
            const QoreObject* o = value.get<QoreObject>();

            jobject javaObjectRef = qjcm.getJavaObject(o);
            if (javaObjectRef) {
                return javaObjectRef;
            }
            break;
        }
        case NT_HASH: {
            return makeHashMap(*value.get<QoreHashNode>());
        }
        case NT_BINARY: {
            return makeByteArray(*value.get<BinaryNode>());
        }

        case NT_NOTHING:
        case NT_NULL:
            return nullptr;
        case NT_LIST:
            return Array::toJava(value.get<QoreListNode>()).release();
    }
    QoreStringMaker desc("don't know how to convert a value of type '%s' to a Java object (expecting 'java.lang.Object')", value.getTypeName());
    throw BasicException(desc.c_str());
}

jobject QoreToJava::toObject(const QoreValue& value, jclass cls) {
    if (value.isNullOrNothing())
        return nullptr;

    if (cls) {
        Env env;
        if (env.isSameObject(cls, Globals::classObject))
            return toAnyObject(value);
    }

    LocalReference<jobject> javaObjectRef;
    switch (value.getType()) {
        case NT_STRING: {
            ModifiedUtf8String str(*value.get<QoreStringNode>());
            Env env;
            javaObjectRef = env.newString(str.c_str()).release();
            break;
        }
        case NT_LIST: {
            javaObjectRef = static_cast<jobject>(qjcm.getJavaArray(value.get<QoreListNode>(), cls));
            break;
        }
        case NT_OBJECT: {
            const QoreObject* o = value.get<QoreObject>();

            javaObjectRef = qjcm.getJavaObject(o);
            if (!javaObjectRef) {
                ExceptionSink xsink;
                TryPrivateDataRefHolder<ObjectBase> jo(o, CID_JAVAARRAY, &xsink);
                if (!jo) {
                    if (xsink)
                        throw XsinkException(xsink);

                    if (cls) {
                        Env env;
                        LocalReference<jstring> clsName = env.callObjectMethod(cls, Globals::methodClassGetCanonicalName, nullptr).as<jstring>();
                        Env::GetStringUtfChars cname(env, clsName);
                        QoreStringMaker desc("A Java object argument of class '%s' expected; got object of class '%s' instead", cname.c_str(), o->getClassName());
                        throw BasicException(desc.c_str());
                    }

                    QoreStringMaker desc("A Java object argument expected; got object of class '%s' instead", o->getClassName());
                    throw BasicException(desc.c_str());
                }
                javaObjectRef = jo->makeLocal();
            }
            break;
        }
        default: {
            if (cls) {
                Env env;
                LocalReference<jstring> clsName = env.callObjectMethod(cls, Globals::methodClassGetCanonicalName, nullptr).as<jstring>();
                Env::GetStringUtfChars cname(env, clsName);
                QoreStringMaker desc("A Java object argument of class '%s' expected; got type '%s' instead", cname.c_str(), value.getTypeName());
                throw BasicException(desc.c_str());
            }

            // convert primitive types to java objects if possible
            return toAnyObject(value);
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
    return javaObjectRef.release();
}

jobject QoreToJava::makeHashMap(const QoreHashNode& h) {
    Env env;
    LocalReference<jobject> hm = env.newObject(Globals::classHashMap, Globals::ctorHashMap, nullptr);

    ConstHashIterator i(h);
    while (i.next()) {
        LocalReference<jstring> key = env.newString(i.getKey());
        QoreValue v(i.get());
        LocalReference<jobject> value = toAnyObject(v);

        std::vector<jvalue> jargs(2);
        jargs[0].l = key;
        jargs[1].l = value;

        env.callObjectMethod(hm, Globals::methodHashMapPut, &jargs[0]);
    }

    return hm.release();
}

jbyteArray QoreToJava::makeByteArray(const BinaryNode& b) {
    Env env;
    LocalReference<jbyteArray> array = env.newByteArray(b.size()).as<jbyteArray>();
    for (jsize i = 0; i < b.size(); ++i) {
        env.setByteArrayElement(array, i, ((const char*)b.getPtr())[i]);
    }

    return array.release();
}

}
