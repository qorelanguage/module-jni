//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2016 - 2020 Qore Technologies, s.r.o.
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
#include "Jvm.h"

#include <qore/Qore.h>

#include "defs.h"
#include "Globals.h"
#include "QoreJniClassMap.h"

namespace jni {

JavaVM* Jvm::vm = nullptr;
thread_local JNIEnv *Jvm::env;

QoreStringNode* Jvm::createVM() {
    assert(vm == nullptr);

    JavaVMInitArgs vm_args;
    vm_args.version = JNI_VERSION_10;
    vm_args.ignoreUnrecognized = false;
    vm_args.nOptions = 0;

    size_t num_options = 1;
    bool disable_jit = false;
    // check QORE_JNI_DISABLE_JIT environment variable
    {
        QoreString val;
        if (!SystemEnvironment::get("QORE_JNI_DISABLE_JIT", val)) {
            disable_jit = q_parse_bool(val.c_str());
            if (disable_jit) {
                ++num_options;
                //printd(5, "jni module: disabling JIT\n");
            }
        }
    }
    QoreString classpath;
    if (!SystemEnvironment::get("QORE_CLASSPATH", classpath)) {
        ++num_options;
    }
    JavaVMOption options[num_options];
    // "reduced signals"
    options[vm_args.nOptions++].optionString = (char*)"-Xrs";
    if (disable_jit) {
        // disable JIT
        options[vm_args.nOptions++].optionString = (char*)"-Xint";
    }
    if (!classpath.empty()) {
        classpath.prepend("-Djava.class.path=");
        options[vm_args.nOptions++].optionString = (char*)classpath.c_str();
        printd(0, "classpath: '%s'\n", classpath.c_str());
    }

    vm_args.options = options;

    int rc = JNI_CreateJavaVM(&vm, reinterpret_cast<void**>(&env), &vm_args);
    if (rc != JNI_OK) {
        return new QoreStringNodeMaker("JNI_CreateJavaVM() failed with error code %d", rc);
    }
    return 0;
}

void Jvm::destroyVM() {
    assert(vm != nullptr);

    Globals::cleanup();
    vm->DestroyJavaVM();
    vm = nullptr;
    env = nullptr;
}

JNIEnv *Jvm::attachAndGetEnv() {
    if (!vm) {
        throw UnableToAttachException(JNI_ERR);
    }

    if (env == nullptr) {
        jint err = vm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
        if (err != JNI_OK) {
            throw UnableToAttachException(err);
        }
        printd(LogLevel, "JNI - thread %d attached, env: %p\n", q_gettid(), env);
    }
    return env;
}

JNIEnv* Jvm::attachAndGetEnv(bool& new_attach) {
    if (!vm) {
        throw UnableToAttachException(JNI_ERR);
    }

    if (env == nullptr) {
        jint err = vm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
        if (err != JNI_OK) {
            throw UnableToAttachException(err);
        }
        new_attach = true;
        printd(LogLevel, "JNI - thread %d attached, env: %p\n", q_gettid(), env);
    } else {
        new_attach = false;
    }
    return env;
}

void Jvm::threadCleanup() {
    assert(vm != nullptr);

    if (env != nullptr) {
        printd(LogLevel, "JNI - detaching thread, env: %p\n", env);
        vm->DetachCurrentThread();
        env = nullptr;
    }
}

} // namespace jni
