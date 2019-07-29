/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
    JniQoreClass.h

    Qore Programming Language JNI Module

    Copyright (C) 2016 - 2019 Qore Technologies, s.r.o.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <qore/Qore.h>

#include <string.h>

#include "defs.h"
#include "Jvm.h"
#include "Env.h"
#include "QoreJniPrivateData.h"
#include "JavaToQore.h"
#include "JniQoreClass.h"
#include "ModifiedUtf8String.h"
#include "QoreJniClassMap.h"

#include <classfile_constants.h>

namespace jni {

type_vec_t JniQoreClass::paramTypeInfo = { stringTypeInfo, boolTypeInfo };

// static method
QoreValue JniQoreClass::memberGate(const QoreMethod& meth, void* m, QoreObject* self, QoreJniPrivateData* jd,
    const QoreListNode* args, q_rt_flags_t rtflags, ExceptionSink* xsink) {
    if (!args || args->size() != 2)
        return QoreValue();

    bool cls_access;
    const QoreStringNode* mname;
    {
        QoreValue qv = args->retrieveEntry(0);
        if (qv.getType() != NT_STRING)
            return QoreValue();
        mname = qv.get<QoreStringNode>();
        qv = args->retrieveEntry(1);
        if (qv.getType() != NT_BOOLEAN)
            return QoreValue();
        cls_access = qv.getAsBool();
    }

    //printd(LogLevel, "JniQoreClass::memberGate: '%s'\n", mname->c_str());

    jobject jobj = jd->getObject();
    try {
        Env env;
        LocalReference<jobject> cls = env.callObjectMethod(jobj, Globals::methodObjectGetClass, nullptr);

        ModifiedUtf8String str(*mname);
        LocalReference<jstring> fname = env.newString(str.c_str());

        std::vector<jvalue> jargs(2);
        jargs[0].l = fname;

        LocalReference<jobject> field = env.callObjectMethod(cls, Globals::methodClassGetDeclaredField, &jargs[0]);

        // check access here
        int mods = env.callIntMethod(field, Globals::methodFieldGetModifiers, nullptr);
        if (mods & JVM_ACC_PROTECTED) {
            if (!cls_access) {
                LocalReference<jstring> clsName =
                    env.callObjectMethod(cls, Globals::methodClassGetName, nullptr).as<jstring>();
                jni::Env::GetStringUtfChars cname(env, clsName);
                xsink->raiseException("JNI-ACCESS-ERROR",
                    "cannot access private (Java 'protected') member '%s' of class '%s'",
                    mname->c_str(), cname.c_str());
                return QoreValue();
            }
        } else if (mods & JVM_ACC_PRIVATE) {
            LocalReference<jstring> clsName =
                env.callObjectMethod(cls, Globals::methodClassGetName, nullptr).as<jstring>();
            jni::Env::GetStringUtfChars cname(env, clsName);
            xsink->raiseException("JNI-ACCESS-ERROR",
                "cannot access private:internal (Java 'private') member '%s' of class '%s'",
                mname->c_str(), cname.c_str());
            return QoreValue();
        }

        jargs[0].l = field;
        jargs[1].l = jobj;

        QoreProgram* pgm = getProgram();
        assert(pgm);
        JniExternalProgramData* jpc = static_cast<JniExternalProgramData*>(pgm->getExternalData("jni"));
        assert(jpc);

        return JavaToQore::convertToQore(env.callStaticObjectMethod(jpc->getDynamicApi(), jpc->getFieldId(), &jargs[0]));
        //return JavaToQore::convertToQore(env.callObjectMethod(field, Globals::methodFieldGet, &jargs[0]));
    } catch (jni::JavaException& e) {
        e.convert(xsink);
        return QoreValue();
    }
}
}
