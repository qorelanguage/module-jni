#!/bin/sh

# Verify each Java-backed provider in a fresh process so that a provider loaded earlier cannot mask a missing,
# duplicate, or version-incompatible logging implementation in another module's isolated classloader.

set -eu

log_dir=$(mktemp -d)
trap 'rm -rf "$log_dir"' EXIT HUP INT TERM

modules="
AvroDataProvider
IcsDataProvider
CamelDataProvider
TikaDataProvider
OdsDataProvider
OdtDataProvider
OdpDataProvider
ExcelDataProvider
PowerPointDataProvider
EmailDataProvider
WordDataProvider
VisioDataProvider
"

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

echo "Java logging provider check passed for $(echo "$modules" | wc -w) modules"
