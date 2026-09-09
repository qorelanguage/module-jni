# Java Provider Dependency Profiles

## Purpose

Each Java-backed data provider runs in a classloader whose dependencies must be complete and internally
compatible. A dependency available to another provider or to a previous process can otherwise hide a missing JAR,
an incompatible SLF4J provider, or a logging bridge cycle.

Dependency qualification is initialization and release work. It does not add checks to provider action execution.

## Canonical declaration

The `%module-cmd(jni) add-relative-classpath` and `global-add-relative-classpath` directives in each provider's
`.qm` source are the canonical runtime-JAR inventory. `qore_java_provider_profile()` reads those directives and
attaches exactly one logging policy:

- `none`: the classpath contains no SLF4J API or provider;
- `slf4j-nop`: the classpath contains one compatible `slf4j-api` and `slf4j-nop` pair.

The helper emits `java-provider-profiles.tsv`. Source-tree, AOT-tree, install-tree, and fresh-process checks all
consume this inventory, so adding a runtime JAR cannot require four independently maintained lists.
Finalization independently scans every shipped `.qm` for JNI relative-classpath directives and rejects any module
without exactly one profile, including a new module that contains only a generated first-party JAR and would
therefore be invisible to the committed third-party checksum inventory.

## Static qualification

`validate-java-provider-profiles.py` resolves every path relative to the declaring module and fails for:

- a missing, escaping, duplicate, uncommitted, or undeclared JAR;
- a missing or stale checksum in `qlib/java-provider-dependencies.sha256`;
- multiple SLF4J APIs/providers, mismatched API/provider versions, missing provider classes named by service entries,
  or incompatible 1.x/2.x service mechanisms;
- a logging artifact under the `none` policy;
- a two-way Log4j, JUL, or Commons Logging bridge cycle;
- a build/AOT/install copy whose SHA-256 differs from the source inventory.

Generated first-party `qore-dataprovider-*.jar` files are compared across staging roots but are intentionally omitted
from the committed checksum file because ZIP timestamps make rebuilt output non-reproducible. Every third-party JAR
is committed and checksummed.

## Dynamic qualification

Every declared provider is loaded in a fresh Qore process after installation. Both stdout and stderr must remain
empty and the process must succeed. There are no output allowlists: an SLF4J warning is a dependency-profile defect.

Source, AOT, and installed-tree checks each write an atomic JSON qualification report. The installed report also
records every fresh-process outcome without embedding process output in a successful artifact. CI retains all three
reports even when a check fails, together with the source revision and qore-test-base provenance, so release tooling
never has to infer qualification from logs.

When a provider dependency changes, update its `.qm` classpath directive, update the committed JAR, regenerate the
checksum inventory, rebuild all provider qmods, and run `check-java-logging.sh` against an empty install prefix.
