#!/bin/sh

# Verify each Java-backed provider in a fresh process so that a provider loaded earlier cannot mask a missing,
# duplicate, or version-incompatible logging implementation in another module's isolated classloader.

set -eu

# JVM launcher option variables produce their own stderr diagnostics before Java or
# any provider code runs.  Keep this check scoped to output from the providers.
unset JAVA_TOOL_OPTIONS _JAVA_OPTIONS JDK_JAVA_OPTIONS

log_dir=$(mktemp -d)
trap 'rm -rf "$log_dir"' EXIT HUP INT TERM

module_src_dir=${MODULE_SRC_DIR:-$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)}
profiles=${JAVA_PROVIDER_PROFILES:-$module_src_dir/build/java-provider-profiles.tsv}
validator=$module_src_dir/test/docker_test/validate-java-provider-profiles.py
source_root=$module_src_dir/qlib
aot_root=${JAVA_PROVIDER_AOT_ROOT:-$module_src_dir/build/qlib-qmod}
install_root=${JAVA_PROVIDER_INSTALL_ROOT:-${INSTALL_PREFIX:-/usr}/share/qore-modules}

python3 "$validator" --profiles "$profiles" --root "$source_root" \
    --checksums "$source_root/java-provider-dependencies.sha256" --require-committed
python3 "$validator" --profiles "$profiles" --root "$aot_root" --reference-root "$source_root"
python3 "$validator" --profiles "$profiles" --root "$install_root" --reference-root "$source_root"

modules=$(awk -F '\t' '!/^#/ { print $1 }' "$profiles" | sort -u)

for module in $modules; do
    stdout="$log_dir/$module.stdout"
    stderr="$log_dir/$module.stderr"
    if ! qore -l DataProvider -ne \
            "DataProvider::setOptions(DPO_DisableOnDemandInitialization | DPO_EnableOnDemandActions); load_module(\"$module\");" \
            >"$stdout" 2>"$stderr"; then
        echo "ERROR: $module failed to load" >&2
        cat "$stdout" >&2
        cat "$stderr" >&2
        exit 1
    fi
    if [ -s "$stdout" ] || [ -s "$stderr" ]; then
        echo "ERROR: $module emitted output during a fresh-process load" >&2
        cat "$stdout" >&2
        cat "$stderr" >&2
        exit 1
    fi
done

echo "Java provider installed-artifact check passed for $(echo "$modules" | wc -w) modules"
