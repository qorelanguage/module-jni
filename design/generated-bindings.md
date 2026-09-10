# Generated Java binding lifetime

Copyright (C) 2026 Qore Technologies, s.r.o.

Generated Java API classes serialize opaque negative handles instead of native
program, class, method, variant, function, or constant pointers. Program arguments
carry `QoreProgram::getProgramId()` and are compared without narrowing against the
program held by the binding. The handle registry holds the native metadata.

Handles start at a process-random 63-bit origin and advance without reuse. Their
negative representation distinguishes them from legacy user-space pointers.
Deduplication by program, binding kind, and metadata preserves identical bytecode
when the same class is generated repeatedly. The randomized origin prevents a
fresh process from accepting a saved jar merely because its program IDs and
binding sequence restart. These handles are lifetime identifiers, not an
authorization boundary against malicious native callers.

Each binding holds weak references to its execution program and the program
owning its metadata (which can differ for an imported class). A lookup racing
either program's destruction cannot access a freed allocation. `GeneratedBindingContext`
first acquires leases on both programs, then enters their
`QoreExternalProgramContextHelper` scopes, leaving the execution program current.
Destruction unwinds the program contexts before releasing the leases and weak
references. Native class lookups return a lease so their callers retain the
metadata throughout inheritance checks, type queries, and construction.
JNI entry points translate Qore, JNI,
allocation, and other C++ exceptions to Java exceptions.

The program cleanup callback closes lease admission and drains active leases
before removing every registry entry depending on that program. This additional drain covers the interval
between Qore's initial thread drain and its deletion marker. Counters allow
reentrant and concurrent calls; no registry mutex is held while entering program
contexts, running user code, or waiting for leases. The registry mutex precedes
the short per-program state mutex whenever both are needed. Shutdown clears the
registry before tearing down the global Java context and closes generation so
shutdown callbacks cannot repopulate it.

`$qore_cls_handle` identifies generated classes for construction and native type
queries. Only a field declared on the Java class itself is considered; user Java
subclasses must receive their own Qore wrapper. The canonical program ID and Qore
namespace path fields remain descriptive metadata. Legacy raw class fields are
never dereferenced. Constructor method/variant slots were unused by native
construction and are now reserved zeros; method/function variant pointers remain
in the registry, preserving overload selection.

Construction publishes the Java wrapper's weak object reference only after
private-data setup and the object-retention callback have succeeded. A failing
retention callback therefore leaves the Java cleanup pointer at zero and releases
the temporary Qore object through its value holder.

Generated jars describe signatures but do not contain portable executable Qore
bindings. `Class.forName(name, false, loader)` supports signature inspection without
constant initialization. Execution requires regeneration in a live receiving
program. Unknown, expired, wrong-kind, and legacy bindings fail with a diagnostic
that asks for regeneration.

Validation covers registry identity, negative inputs, reentrant and foreign-thread
contexts, explicit cleanup, live Java dispatch, program destruction, legacy and
malformed native arguments, and saved-jar loading in a fresh Qore process. The
native registry test runs without a JVM, allowing focused Valgrind verification.
