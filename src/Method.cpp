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
#include "Method.h"
#include "Env.h"
#include "JavaToQore.h"
#include "QoreToJava.h"
#include "QoreJniClassMap.h"

namespace jni {

std::vector<jvalue> BaseMethod::convertArgs(const QoreListNode* args, size_t base) const {
    assert(base == 0 || (args != nullptr && args->size() >= base));

    size_t paramCount = paramTypes.size();
    // missing arguments are treated as null
    size_t argCount = args == nullptr ? 0 : (args->size() - base);

    if (paramCount < argCount && !doVarArgs) {
        Env env;
        // get class and method name for exception text
        LocalReference<jstring> mcname = env.callObjectMethod(cls->getJavaObject(), Globals::methodClassGetName,
            nullptr).as<jstring>();
        Env::GetStringUtfChars mcn(env, mcname);

        QoreString mname;
        getName(mname);

        QoreStringMaker err("Too many arguments (%d) in invocation to Java method %s.%s() (takes %d arg%s)",
            static_cast<int>(argCount), mcn.c_str(), mname.c_str(), static_cast<int>(paramCount),
            paramCount == 1 ? "" : "s");
        throw BasicException(err.c_str());
    }

    std::vector<jvalue> jargs(paramCount);
    for (size_t index = 0; index < paramCount; ++index) {
        // process varargs with remaining arguments if appropriate
        if (doVarArgs && (argCount > paramCount) && (index == (paramCount - 1))) {
            // get array component type
            Env env;
            LocalReference<jclass> ccls = env.callObjectMethod(paramTypes[index].second,
                Globals::methodClassGetComponentType, nullptr).as<jclass>();
            jargs[index].l = Array::toObjectArray(args, ccls, index).release();
            break;
        }
        assert(index < argCount);
        QoreValue qv = args ? args->retrieveEntry(index + base) : QoreValue();

        switch (paramTypes[index].first) {
            case Type::Boolean:
                jargs[index].z = QoreToJava::toBoolean(qv);
                break;
            case Type::Byte:
                jargs[index].b = QoreToJava::toByte(qv);
                break;
            case Type::Char:
                jargs[index].c = QoreToJava::toChar(qv);
                break;
            case Type::Short:
                jargs[index].s = QoreToJava::toShort(qv);
                break;
            case Type::Int:
                jargs[index].i = QoreToJava::toInt(qv);
                break;
            case Type::Long:
                jargs[index].j = QoreToJava::toLong(qv);
                break;
            case Type::Float:
                jargs[index].f = QoreToJava::toFloat(qv);
                break;
            case Type::Double:
                jargs[index].d = QoreToJava::toDouble(qv);
                break;
            case Type::Reference:
            default:
                assert(paramTypes[index].first == Type::Reference);
                jargs[index].l = QoreToJava::toObject(qv, paramTypes[index].second);
                break;
        }
    }

    return std::move(jargs);
}

void BaseMethod::doObjectException(Env& env, jobject object) const {
    LocalReference<jclass> ocls = env.getObjectClass(object);
    LocalReference<jstring> ocname = env.callObjectMethod(ocls, Globals::methodClassGetName, nullptr).as<jstring>();
    Env::GetStringUtfChars ocn(env, ocname);

    LocalReference<jstring> mcname = env.callObjectMethod(cls->getJavaObject(), Globals::methodClassGetName, nullptr).as<jstring>();
    Env::GetStringUtfChars mcn(env, mcname);

    QoreString mname;
    getName(mname);

    QoreStringMaker desc("cannot invoke method %s.%s() on an object of class '%s' (%p)", mcn.c_str(), mname.c_str(), ocn.c_str(), object);
    throw BasicException(desc.c_str());
}

QoreValue BaseMethod::invoke(jobject object, const QoreListNode* args, int offset) const {
    std::vector<jvalue> jargs = convertArgs(args, offset);

    Env env;
    if (!env.isInstanceOf(object, cls->getJavaObject())) {
        doObjectException(env, object);
    }
    switch (retValType) {
        case Type::Boolean:
            return JavaToQore::convert(env.callBooleanMethod(object, id, &jargs[0]));
        case Type::Byte:
            return JavaToQore::convert(env.callByteMethod(object, id, &jargs[0]));
        case Type::Char:
            return JavaToQore::convert(env.callCharMethod(object, id, &jargs[0]));
        case Type::Short:
            return JavaToQore::convert(env.callShortMethod(object, id, &jargs[0]));
        case Type::Int:
            return JavaToQore::convert(env.callIntMethod(object, id, &jargs[0]));
        case Type::Long:
            return JavaToQore::convert(env.callLongMethod(object, id, &jargs[0]));
        case Type::Float:
            return JavaToQore::convert(env.callFloatMethod(object, id, &jargs[0]));
        case Type::Double:
            return JavaToQore::convert(env.callDoubleMethod(object, id, &jargs[0]));
        case Type::Reference:
            return JavaToQore::convertToQore(env.callObjectMethod(object, id, &jargs[0]));
        case Type::Void:
        default:
            assert(retValType == Type::Void);
            env.callVoidMethod(object, id, &jargs[0]);
            return QoreValue();
    }
}

QoreValue BaseMethod::invokeNonvirtual(jobject object, const QoreListNode* args, int offset) const {
    std::vector<jvalue> jargs = convertArgs(args, offset);

    Env env;
    if (!env.isInstanceOf(object, cls->getJavaObject())) {
        doObjectException(env, object);
    }
    switch (retValType) {
        case Type::Boolean:
            return JavaToQore::convert(env.callNonvirtualBooleanMethod(object, cls->getJavaObject(), id, &jargs[0]));
        case Type::Byte:
            return JavaToQore::convert(env.callNonvirtualByteMethod(object, cls->getJavaObject(), id, &jargs[0]));
        case Type::Char:
            return JavaToQore::convert(env.callNonvirtualCharMethod(object, cls->getJavaObject(), id, &jargs[0]));
        case Type::Short:
            return JavaToQore::convert(env.callNonvirtualShortMethod(object, cls->getJavaObject(), id, &jargs[0]));
        case Type::Int:
            return JavaToQore::convert(env.callNonvirtualIntMethod(object, cls->getJavaObject(), id, &jargs[0]));
        case Type::Long:
            return JavaToQore::convert(env.callNonvirtualLongMethod(object, cls->getJavaObject(), id, &jargs[0]));
        case Type::Float:
            return JavaToQore::convert(env.callNonvirtualFloatMethod(object, cls->getJavaObject(), id, &jargs[0]));
        case Type::Double:
            return JavaToQore::convert(env.callNonvirtualDoubleMethod(object, cls->getJavaObject(), id, &jargs[0]));
        case Type::Reference:
            return JavaToQore::convertToQore(env.callNonvirtualObjectMethod(object, cls->getJavaObject(), id, &jargs[0]));
        case Type::Void:
        default:
            assert(retValType == Type::Void);
            env.callNonvirtualVoidMethod(object, cls->getJavaObject(), id, &jargs[0]);
            return QoreValue();
    }
}

QoreValue BaseMethod::invokeStatic(const QoreListNode* args, int offset) const {
    std::vector<jvalue> jargs = convertArgs(args, offset);

    Env env;
    switch (retValType) {
        case Type::Boolean:
            return JavaToQore::convert(env.callStaticBooleanMethod(cls->getJavaObject(), id, &jargs[0]));
        case Type::Byte:
            return JavaToQore::convert(env.callStaticByteMethod(cls->getJavaObject(), id, &jargs[0]));
        case Type::Char:
            return JavaToQore::convert(env.callStaticCharMethod(cls->getJavaObject(), id, &jargs[0]));
        case Type::Short:
            return JavaToQore::convert(env.callStaticShortMethod(cls->getJavaObject(), id, &jargs[0]));
        case Type::Int:
            return JavaToQore::convert(env.callStaticIntMethod(cls->getJavaObject(), id, &jargs[0]));
        case Type::Long:
            return JavaToQore::convert(env.callStaticLongMethod(cls->getJavaObject(), id, &jargs[0]));
        case Type::Float:
            return JavaToQore::convert(env.callStaticFloatMethod(cls->getJavaObject(), id, &jargs[0]));
        case Type::Double:
            return JavaToQore::convert(env.callStaticDoubleMethod(cls->getJavaObject(), id, &jargs[0]));
        case Type::Reference:
            return JavaToQore::convertToQore(env.callStaticObjectMethod(cls->getJavaObject(), id, &jargs[0]));
        case Type::Void:
        default:
            assert(retValType == Type::Void);
            env.callStaticVoidMethod(cls->getJavaObject(), id, &jargs[0]);
            return QoreValue();
    }
}

QoreValue BaseMethod::newInstance(const QoreListNode* args) {
    std::vector<jvalue> jargs = convertArgs(args);
    Env env;
    return JavaToQore::convertToQore(env.newObject(cls->getJavaObject(), id, &jargs[0]));
}

LocalReference<jobject> BaseMethod::newQoreInstance(const QoreListNode* args) {
    std::vector<jvalue> jargs = convertArgs(args);
    Env env;
    return env.newObject(cls->getJavaObject(), id, &jargs[0]);
}

void BaseMethod::getName(QoreString& str) const {
    Env env;
    // get Method name
    LocalReference<jstring> jmName = env.callObjectMethod(method, Globals::methodMethodGetName, nullptr).as<jstring>();
    Env::GetStringUtfChars mName(env, jmName);
    str.concat(mName.c_str());
}

int BaseMethod::getParamTypes(type_vec_t& paramTypeInfo, QoreJniClassMap& clsmap) {
    unsigned len = paramTypes.size();
    if (len)
        paramTypeInfo.reserve(len);

    for (auto& i : paramTypes)
        paramTypeInfo.push_back(clsmap.getQoreType(i.second));

    return 0;
}

const QoreTypeInfo* BaseMethod::getReturnTypeInfo(QoreJniClassMap& clsmap) {
    return clsmap.getQoreType(retValClass);
}

void BaseMethod::getSignature(QoreString& str) const {
    Env env;
    LocalReference<jstring> sig = env.callObjectMethod(method, Globals::methodMethodToGenericString, nullptr).as<jstring>();
    Env::GetStringUtfChars msig(env, sig);
    str.concat(msig.c_str());
}

} // namespace jni
