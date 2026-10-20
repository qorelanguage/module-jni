#!/bin/bash
#
# usage: check-module-paths.sh [binary-module...]
#
# Checks that the tests load the modules of this tree, whatever directory they are run from and whatever is
# installed:
# - rejects %prepend-module-path and %append-module-path directives with a relative path in the test files:
#   Qore resolves a relative module path against the working directory, not against the file containing the
#   directive, so a test run from another directory (the CI scripts run the tests from the repository root)
#   silently loads an installed module instead of the one in this tree; use "${SCRIPT_DIR}/<path>" instead
# - for each binary module built by this repository (the arguments), requires the test programs that load it,
#   directly (%requires, %try-module, %module-cmd) or through a module in qlib/ that loads it, to prepend the build
#   directory: %prepend-module-path "${SCRIPT_DIR}/<relative path>/build"; loads at run time (ex: load_module(),
#   or %requires in program text that a test parses) cannot be found and are not checked
#
# Copyright 2026 Qore Technologies, s.r.o.
set -e
src_dir=$(cd "$(dirname "$0")/../.." && pwd)
rc=0

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
    rc=1
fi

if [ $# -gt 0 ]; then
    # the modules that load a binary module of this repository: the binary modules, and the modules in qlib/ that
    # load one of them (%requires, %try-module, or %module-cmd, which loads the module), directly or through other
    # such modules
    binary="$*"
    binary_names=$(echo ${binary} | tr ' ' '|')
    loaders=" $* "
    changed=1
    while [ ${changed} -eq 1 ]; do
        changed=0
        names=$(echo ${loaders} | tr ' ' '|')
        for qm in "${src_dir}"/qlib/*.qm "${src_dir}"/qlib/*/*.qm; do
            [ -f "${qm}" ] || continue
            name=$(basename "${qm}" .qm)
            dir=$(basename "$(dirname "${qm}")")
            # a module is qlib/<name>.qm or qlib/<name>/<name>.qm
            if [ "${dir}" != qlib ] && [ "${dir}" != "${name}" ]; then
                continue
            fi
            case "${loaders}" in *" ${name} "*) continue;; esac
            if grep -qE "^[[:space:]]*%(requires|try-module)([[:space:]]*\([^)]*\))?[[:space:]]+([^[:space:]]*/)?(${names})(\.qm|\.qmod)?([[:space:]]|$|[<>=])|^[[:space:]]*%module-cmd\((${binary_names})\)" \
                "${qm}"; then
                loaders="${loaders}${name} "
                changed=1
            fi
        done
    done
    names=$(echo ${loaders} | tr ' ' '|')
    missing=""
    while IFS= read -r f; do
        if ! grep -qE "^[[:space:]]*%(requires|try-module)([[:space:]]*\([^)]*\))?[[:space:]]+([^[:space:]]*/)?(${names})(\.qm|\.qmod)?([[:space:]]|$|[<>=])|^[[:space:]]*%module-cmd\((${binary_names})\)" \
            "${f}"; then
            continue
        fi
        # the build directory relative to the directory of the test
        rel_dir=$(dirname "${f#${src_dir}/}")
        up=$(echo "${rel_dir}" | sed -E 's|[^/]+|..|g')
        if ! grep -qF "%prepend-module-path \"\${SCRIPT_DIR}/${up}/build\"" "${f}"; then
            missing="${missing}  ${f#${src_dir}/}: add %prepend-module-path \"\${SCRIPT_DIR}/${up}/build\"
"
        fi
    done < <(find "${src_dir}/test" -type f \( -name '*.qtest' -o -name '*.qr' -o -name '*.q' \) | sort)
    if [ -n "${missing}" ]; then
        echo "ERROR: tests load a binary module of this repository (${binary}) without the build directory of this"
        echo "tree in their module path, so they can load an installed module instead of the one built here:"
        printf "%s" "${missing}"
        rc=1
    fi
fi

if [ ${rc} -eq 0 ]; then
    echo "module paths in test files: OK"
fi
exit ${rc}
