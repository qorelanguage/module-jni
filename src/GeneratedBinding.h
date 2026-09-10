/* Copyright (C) 2026 Qore Technologies, s.r.o.
   SPDX-License-Identifier: LGPL-2.1-or-later */

#ifndef QORE_JNI_GENERATED_BINDING_H
#define QORE_JNI_GENERATED_BINDING_H

#include <qore/Qore.h>
#include <jni.h>
#include <memory>

namespace jni {

enum class GeneratedBindingKind { Class, Method, StaticMethod, Function, Constant };
struct GeneratedProgram;

// Only the opaque handle is serialized. All borrowed native metadata stays here.
struct GeneratedBinding {
    QoreProgram* const pgm;
    QoreProgram* const metadata_pgm;
    const GeneratedBindingKind kind;
    const QoreClass* cls = nullptr;
    const QoreMethod* method = nullptr;
    const QoreExternalVariant* variant = nullptr;
    const QoreExternalFunction* function = nullptr;
    const QoreExternalConstant* constant = nullptr;
    std::shared_ptr<GeneratedProgram> lifetime;
    std::shared_ptr<GeneratedProgram> metadata_lifetime;

    DLLLOCAL GeneratedBinding(QoreProgram* pgm, GeneratedBindingKind kind, QoreProgram* metadata_pgm = nullptr)
            : pgm(pgm), metadata_pgm(metadata_pgm ? metadata_pgm : pgm), kind(kind) {
        assert(pgm);
        pgm->depRef();
        if (this->metadata_pgm != pgm) {
            this->metadata_pgm->depRef();
        }
    }

    DLLLOCAL ~GeneratedBinding() {
        if (metadata_pgm != pgm) {
            metadata_pgm->depDeref();
        }
        pgm->depDeref();
    }

    GeneratedBinding(const GeneratedBinding&) = delete;
    GeneratedBinding& operator=(const GeneratedBinding&) = delete;
};

DLLLOCAL jlong register_generated_binding(std::shared_ptr<GeneratedBinding> binding);
DLLLOCAL std::shared_ptr<const GeneratedBinding> find_generated_binding(jlong handle, GeneratedBindingKind kind);
DLLLOCAL void purge_generated_bindings(QoreProgram* pgm);
DLLLOCAL void clear_generated_bindings();

class GeneratedBindingLease {
public:
    DLLLOCAL GeneratedBindingLease(jlong handle, GeneratedBindingKind kind);
    DLLLOCAL ~GeneratedBindingLease();
    GeneratedBindingLease(const GeneratedBindingLease&) = delete;
    GeneratedBindingLease& operator=(const GeneratedBindingLease&) = delete;

    DLLLOCAL const GeneratedBinding* operator->() const {
        return binding.get();
    }

private:
    std::shared_ptr<const GeneratedBinding> binding;
};

// A weak reference from the registry keeps the Program allocation valid while the
// context helper atomically enters execution or rejects a Program being torn down.
// Never hold the registry mutex while acquiring a Program context or executing code.
class GeneratedBindingContext {
public:
    DLLLOCAL GeneratedBindingContext(jlong handle, GeneratedBindingKind kind, ExceptionSink* xsink);

    DLLLOCAL const GeneratedBinding* operator->() const {
        return binding.operator->();
    }

private:
    GeneratedBindingLease binding;
    std::unique_ptr<QoreExternalProgramContextHelper> metadata_context;
    std::unique_ptr<QoreExternalProgramContextHelper> context;
};

}

#endif
