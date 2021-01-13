//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2016 - 2021 Qore Technologies, s.r.o.
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

void BaseMethod::init(Env &env) {
    retValClass = env.callObjectMethod(method, Globals::methodMethodGetReturnType, nullptr).as<jclass>().makeGlobal();
    retValType = Globals::getType(retValClass);

    LocalReference<jobjectArray> paramTypesArray = env.callObjectMethod(method, Globals::methodMethodGetParameterTypes, nullptr).as<jobjectArray>();
    jsize paramCount = env.getArrayLength(paramTypesArray);
    paramTypes.reserve(paramCount);
    for (jsize p = 0; p < paramCount; ++p) {
        LocalReference<jclass> paramType = env.getObjectArrayElement(paramTypesArray, p).as<jclass>();
        if (p == (paramCount - 1) && env.callBooleanMethod(paramType, Globals::methodClassIsArray, nullptr)) {
            doVarArgs = true;
        }
        paramTypes.emplace_back(Globals::getType(paramType), paramType.makeGlobal());
    }

    mods = env.callIntMethod(method, Globals::methodMethodGetModifiers, nullptr);
}

std::vector<jvalue> BaseMethod::convertArgs(const QoreListNode* args, size_t arg_offset) const {
    assert(arg_offset == 0 || (args != nullptr && args->size() >= arg_offset));

    size_t paramCount = paramTypes.size();
    // missing arguments are treated as null
    size_t argCount = args == nullptr ? 0 : (args->size() - arg_offset);

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
        // process varargs with remaining arguments or with a single argument if appropriate
        if (doVarArgs && (index == (paramCount - 1))
            && ((argCount > paramCount)
                || (argCount == paramCount && !index && argCount == 1 && args->retrieveEntry(0).getType() != NT_LIST))) {
            // get array component type
            Env env;
            LocalReference<jclass> ccls = env.callObjectMethod(paramTypes[index].second,
                Globals::methodClassGetComponentType, nullptr).as<jclass>();

            jargs[index].l = Array::toObjectArray(args, ccls, index).release();
            break;
        }
        assert(!args || args->empty() || (index < argCount));
        QoreValue qv = args ? args->retrieveEntry(index + arg_offset) : QoreValue();

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

    return jargs;
}

LocalReference<jobjectArray> BaseMethod::convertArgsToArray(const QoreListNode* args, size_t arg_offset, size_t array_offset) const {
    assert(arg_offset == 0 || (args != nullptr && args->size() >= arg_offset));

    size_t paramCount = paramTypes.size();
    // missing arguments are treated as null
    size_t argCount = args == nullptr ? 0 : (args->size() - arg_offset);

    Env env;
    if (paramCount < argCount && !doVarArgs) {
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

    LocalReference<jobjectArray> jargs = env.newObjectArray(paramCount + array_offset, Globals::classObject).as<jobjectArray>();
    for (size_t index = 0; index < paramCount; ++index) {
        // process varargs with remaining arguments or with a single argument if appropriate
        if (doVarArgs && (index == (paramCount - 1))
            && ((argCount > paramCount)
                || (argCount == paramCount && !index && argCount == 1 && args->retrieveEntry(0).getType() != NT_LIST))) {
            // get array component type
            Env env;
            LocalReference<jclass> ccls = env.callObjectMethod(paramTypes[index].second,
                Globals::methodClassGetComponentType, nullptr).as<jclass>();

            env.setObjectArrayElement(jargs, index + array_offset, Array::toObjectArray(args, ccls, index).release());
            break;
        }
        assert(!args || args->empty() || (index < argCount));
        QoreValue qv = args ? args->retrieveEntry(index + arg_offset) : QoreValue();
        env.setObjectArrayElement(jargs, index + array_offset, QoreToJava::toObject(qv, paramTypes[index].second));
    }

    return jargs;
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
    Env env;
    if (!env.isInstanceOf(object, cls->getJavaObject())) {
        doObjectException(env, object);
    }

    // try to make a call through the dynamic API
    JniExternalProgramData* jpc = jni_get_context();

    if (!jpc) {
        // make a standard Java call; there will be no Java context for security access though
        try {
            std::vector<jvalue> jargs = convertArgs(args, offset);
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
        } catch (JavaException& e) {
            // workaround for https://bugs.openjdk.java.net/browse/JDK-8221530
            if (e.checkBug_8221530()) {
                throw;
            }
        }
    }
    assert(jpc);

    // add the object as the first argument
    LocalReference<jobjectArray> vargs = convertArgsToArray(args, offset).release();

    // public static Object invokeMethodNonvirtual(Method m, Object obj, Object... args);
    std::vector<jvalue> jargs(3);
    jargs[0].l = method;
    jargs[1].l = object;
    // process method arguments
    jargs[2].l = vargs;

    //printd(5, "BaseMethod::invoke() args: %d\n", (int)(args ? args->size() : 0));
    return JavaToQore::convertToQore(env.callStaticObjectMethod(jpc->getDynamicApi(), jpc->getInvokeMethodId(), &jargs[0]));
}

QoreValue BaseMethod::invokeNonvirtual(jobject object, const QoreListNode* args, int offset) const {
    Env env;
    if (!env.isInstanceOf(object, cls->getJavaObject())) {
        doObjectException(env, object);
    }

    JniExternalProgramData* jpc = jni_get_context();
    assert(jpc);

    // add the object as the first argument
    LocalReference<jobjectArray> vargs = convertArgsToArray(args, offset).release();

    // public static Object invokeMethodNonvirtual(Method m, Object obj, Object... args);
    std::vector<jvalue> jargs(3);
    jargs[0].l = method;
    jargs[1].l = object;
    // process method arguments
    jargs[2].l = vargs;

    //printd(0, "BaseMethod::invokeNonvirtual() args: %d\n", (int)(args ? args->size() : 0));
    return JavaToQore::convertToQore(env.callStaticObjectMethod(jpc->getDynamicApi(), jpc->getInvokeMethodNonvirtualId(), &jargs[0]));
}

QoreValue BaseMethod::invokeStatic(const QoreListNode* args, int offset) const {
    Env env;

    JniExternalProgramData* jpc = jni_get_context();
    if (jpc) {
        LocalReference<jarray> vargs = args ? convertArgsToArray(args, offset).release() : nullptr;
        std::vector<jvalue> jargs(3);
        jargs[0].l = method;
        jargs[1].l = nullptr;
        // process method arguments
        jargs[2].l = vargs;

        //printd(0, "BaseMethod::invokeStatic() with jpc context; args: %d\n", (int)(args ? args->size() : 0));
        return JavaToQore::convertToQore(env.callStaticObjectMethod(jpc->getDynamicApi(), jpc->getInvokeMethodId(), &jargs[0]));
    }

    std::vector<jvalue> jargs = convertArgs(args);
    //printd(0, "BaseMethod::invokeStatic() w/o jpc context; args: %d\n", (int)(args ? args->size() : 0));
    switch (retValType) {
        case Type::Boolean:
            return (bool)env.callStaticBooleanMethod(cls->getJavaObject(), id, &jargs[0]);
        case Type::Byte:
            return (int)env.callStaticByteMethod(cls->getJavaObject(), id, &jargs[0]);
        case Type::Char:
            return (int64)env.callStaticCharMethod(cls->getJavaObject(), id, &jargs[0]);
        case Type::Short:
            return (int64)env.callStaticShortMethod(cls->getJavaObject(), id, &jargs[0]);
        case Type::Int:
            return (int64)env.callStaticIntMethod(cls->getJavaObject(), id, &jargs[0]);
        case Type::Long:
            return (int64)env.callStaticLongMethod(cls->getJavaObject(), id, &jargs[0]);
        case Type::Float:
            return (double)env.callStaticFloatMethod(cls->getJavaObject(), id, &jargs[0]);
        case Type::Double:
            return (double)env.callStaticDoubleMethod(cls->getJavaObject(), id, &jargs[0]);
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

int BaseMethod::getParamTypes(type_vec_t& paramTypeInfo, type_vec_t& altParamTypeInfo, QoreJniClassMap& clsmap,
        QoreProgram* pgm) {
    unsigned len = paramTypes.size();
    if (len) {
        paramTypeInfo.reserve(len);
    }

    for (auto& i : paramTypes) {
        const QoreTypeInfo* altType = nullptr;
        paramTypeInfo.push_back(clsmap.getQoreType(i.second, altType, pgm));
        if (altType) {
            if (altParamTypeInfo.empty()) {
                altParamTypeInfo.reserve(len);
            }
            for (size_t i = altParamTypeInfo.size(), e = paramTypeInfo.size() - 1; i < e; ++i) {
                altParamTypeInfo.push_back(paramTypeInfo[i]);
            }
            altParamTypeInfo.push_back(altType);
        }
    }

    if (!altParamTypeInfo.empty()) {
        for (size_t i = altParamTypeInfo.size(), e = paramTypeInfo.size(); i < e; ++i) {
            altParamTypeInfo.push_back(paramTypeInfo[i]);
        }
    }

    return 0;
}

const QoreTypeInfo* BaseMethod::getReturnTypeInfo(QoreJniClassMap& clsmap, QoreProgram* pgm) {
    return clsmap.getQoreType(retValClass, pgm);
}

void BaseMethod::getSignature(QoreString& str) const {
    Env env;
    LocalReference<jstring> sig = env.callObjectMethod(method, Globals::methodMethodToGenericString, nullptr).as<jstring>();
    Env::GetStringUtfChars msig(env, sig);
    str.concat(msig.c_str());
}

} // namespace jni
