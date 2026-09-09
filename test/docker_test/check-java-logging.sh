#!/bin/sh

# Verify each Java-backed provider in a fresh process so that a provider loaded earlier cannot mask a missing,
# duplicate, or version-incompatible logging implementation in another module's isolated classloader.

set -eu

# JVM launcher option variables produce their own stderr diagnostics before Java or
# any provider code runs.  Keep this check scoped to output from the providers.
unset JAVA_TOOL_OPTIONS _JAVA_OPTIONS JDK_JAVA_OPTIONS

module_src_dir=${MODULE_SRC_DIR:-$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)}
profiles=${JAVA_PROVIDER_PROFILES:-$module_src_dir/build/java-provider-profiles.tsv}
validator=$module_src_dir/test/docker_test/validate-java-provider-profiles.py
source_root=$module_src_dir/qlib
aot_root=${JAVA_PROVIDER_AOT_ROOT:-$module_src_dir/build/qlib-qmod}
install_root=${JAVA_PROVIDER_INSTALL_ROOT:-${INSTALL_PREFIX:-/usr}/share/qore-modules}
qualification_dir=${JAVA_PROVIDER_QUALIFICATION_DIR:-$module_src_dir/qualification}
qualification_suffix=${CI_JOB_NAME_SLUG:-local}
mkdir -p "$qualification_dir"

python3 "$validator" --profiles "$profiles" --root "$source_root" \
    --checksums "$source_root/java-provider-dependencies.sha256" --require-committed \
    --scope source --report "$qualification_dir/java-provider-source-$qualification_suffix.json"
python3 "$validator" --profiles "$profiles" --root "$aot_root" --reference-root "$source_root" \
    --scope aot --report "$qualification_dir/java-provider-aot-$qualification_suffix.json"
python3 "$validator" --profiles "$profiles" --root "$install_root" --reference-root "$source_root" \
    --probe-installed --scope installed \
    --report "$qualification_dir/java-provider-installed-$qualification_suffix.json"
