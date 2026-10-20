#!/bin/bash
#
# Rejects %prepend-module-path and %append-module-path directives with a relative path in the test files.
#
# Qore resolves a relative module path against the working directory, not against the file containing the
# directive, so a test run from another directory (the CI scripts run the tests from the repository root) silently
# loads an installed module instead of the one in this tree.  Use "${SCRIPT_DIR}/<path>" instead.
#
# Copyright 2026 Qore Technologies, s.r.o.
set -e
src_dir=$(cd "$(dirname "$0")/../.." && pwd)
# a path is not relative when it starts with "/" or with a variable (ex: ${SCRIPT_DIR}), quoted or not (also in
# escaped quotes, in program text that a test writes)
bad=$(find "${src_dir}/test" -type f \( -name '*.qtest' -o -name '*.qr' -o -name '*.q' -o -name '*.qm' \
    -o -name '*.qc' -o -name '*.ql' \) -exec grep -nHE \
    '^[[:space:]]*%(prepend|append)-module-path[[:space:]]+(\\?")?[^"\\/$[:space:]]' {} + || true)
if [ -n "${bad}" ]; then
    echo "ERROR: test files use a relative module path, which Qore resolves against the working directory, not"
    echo "against the test file, so the test can load an installed module instead of the one in this tree;"
    echo "use \"\${SCRIPT_DIR}/<path>\" instead:"
    echo "${bad}" | sed "s|^${src_dir}/|  |"
    exit 1
fi
echo "module paths in test files: OK"
