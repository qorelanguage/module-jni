# JNI exception-location and class-wrapper ownership audit

Copyright (C) 2026 Qore Technologies, s.r.o.

Reviewed on 2026-09-08 for the outstanding changes on `develop`.

`src/defs.cpp` normalizes Java stack-frame line numbers below -1 before constructing
Qore exception locations. Java native frames use -2, while Qore's location constructor
asserts that lines are at least -1. This brings exception conversion into agreement
with the existing `QoreJniStackLocationHelper` conversion. Exception identity, source
filenames, available line numbers, frame functions, language and native/user type
are preserved.

`src/QoreJniClassMap.cpp` guards the incoming temporary Java `Class` wrapper until
`setManagedUserData()` transfers ownership to the new Qore class. Previously,
duplicate and same-thread recursive lookups returned without releasing the unused
wrapper. Both the wrapper and the uncommitted Qore class now have independent
ownership guards before any fallible work.

`test/jni-exception-location.qtest` and its Java fixture provide deterministic
boundary and metadata coverage in addition to real native and ordinary Java
exceptions. CMake includes the fixture in the existing test JAR. The JNI 2.7.0
release notes and exception documentation describe both fixes. Ignore rules cover
the existing generated Debug build and provider test data.

The full JNI suite also exposed a stale MIME test expectation: the current core
serializer quotes multipart boundary parameters. The test now accepts both valid
parameter forms and continues comparing the complete message, including headers,
body, attachment encoding and delimiters.

Validation uses the local JNI Debug build, the local Qore Debug executable and
library, and `--enable-debug`. Both build prefixes are `/usr`, matching
`/usr/bin/qore`; no installation is required. Rebuilding stale core qmod dependencies
resolves source-hash warnings without changing runtime options or suppressing warnings.

| Validation | Result |
| --- | --- |
| Debug JNI and test-JAR build | Pass; no compiler or CMake warnings/errors |
| Exception-location regression | 6 cases, 201 assertions |
| Full JNI suite | 57 cases, 466 assertions |
| Callback suite | 2 cases, 3 assertions |
| Concurrent import suite | 2 cases, 2,882 assertions |
| Concurrent code-generation suite | 2 cases, 721 assertions |
| Whitespace and executable test permissions | Pass |

Example commands, run from the repository root after building Qore Debug and JNI:

```bash
cmake --build build-debug --target jni qore-jni-test -j4
export LD_LIBRARY_PATH=../qore/build-debug
export QORE_MODULE_DIR=build-debug:../qore/build-debug/modules/reflection:../qore/build-debug/qlib-qmod
../qore/build-debug/qore -b --enable-debug test/jni-exception-location.qtest -vv
../qore/build-debug/qore -b --enable-debug test/jni.qtest -vv
../qore/build-debug/qore -b --enable-debug test/jni-callback.qtest -vv
../qore/build-debug/qore -b --enable-debug -l jni test/jni-concurrent-import.qtest -vv
../qore/build-debug/qore -b --enable-debug -l jni test/jni-concurrent-codegen.qtest -vv
```

Logs from this review are `/tmp/jni-develop-build.log`,
`/tmp/jni-develop-locations.log`, `/tmp/jni-develop-main.log`,
`/tmp/jni-develop-callback.log`, `/tmp/jni-develop-concurrent-import.log` and
`/tmp/jni-develop-concurrent-codegen.log`.

Valgrind was run with `qore -b --enable-debug`. The regression passed all 201
assertions; the completed raw run returned 99 from `--error-exitcode=99` because
of JVM diagnostics, so it is not a clean-memory result. It reports 89 bytes in
two definitely lost blocks and 3,795 indirectly lost bytes, allocated by JVM
PerfMemory initialization and WatcherThread startup. An independent program
linking only the JVM reproduces the same definite and indirect totals. Earlier
before/after import-only runs identified 843 leaked temporary `Class` wrappers
(53,952 bytes); those lost-allocation records disappear with the ownership guard.
Possible-loss totals vary with the JVM workload and are not claimed equivalent.
Logs: `/tmp/jni-develop-valgrind.log`, `/tmp/jni-develop-valgrind-test.log`,
`/tmp/jni-develop-jvm-baseline-valgrind.log`. On 2026-09-08 the user clarified that
Valgrind is not effective for JNI memory management. JVM diagnostics are therefore
recorded as a tooling limitation, not a clean-memory acceptance requirement. An
additional diagnostic run was stopped following that clarification. No suppression
or JVM-mode change was added.

The source-line contract is documented by Java's
[StackTraceElement API](https://docs.oracle.com/en/java/javase/21/docs/api/java.base/java/lang/StackTraceElement.html#getLineNumber()).
The [Valgrind FAQ](https://valgrind.org/docs/manual/faq.html) documents limitations
with JVM behavior and PCRE2 generated code; these explain why process-wide reports
need separate interpretation from the scoped ownership review.

The full audit-changes checklist is recorded below. N/A entries identify checks
whose triggering functionality is absent from this diff.

| Check | Status | Evidence |
| --- | --- | --- |
| 1. Entry exists in `doxygen/lang/120_modules.dox.tmpl` (for modules in the Qore repo; N/A for external module repos) | N/A | No new Qore module or public QPP class; existing module registration and namespaces remain in place. |
| 2. Entry exists in `doxygen/lang/900_release_notes.dox.tmpl` (for modules in the Qore repo; external modules have release notes in their .qm) | Pass | JNI 2.7.0 release notes and the Java exception API section describe the line-number mapping. |
| 3. `qore_user_module()` or `qore_external_user_module()` call in `CMakeLists.txt` | N/A | No new Qore module or public QPP class; existing module registration and namespaces remain in place. |
| 4. Module added to QMOD list in `CMakeLists.txt` | N/A | No new Qore module or public QPP class; existing module registration and namespaces remain in place. |
| 5. `.qm` file has `@section <lowercasemodname>intro` as first doc section — **must be all lowercase** (e.g., `avrodataproviderintro`, not `AvroDataProviderintro`) | N/A | No new Qore module or public QPP class; existing module registration and namespaces remain in place. |
| 6. `%modern` in `.qm` file — no redundant `%new-style`, `%require-types`, `%strict-args`, `%enable-all-warnings` | N/A | No .qm implementation changed. |
| 7. No parse directives (`%requires`, `%modern`, `%new-style`) in separated `.qc` files (check OUTSIDE of `@code` blocks only) | N/A | No separated Qore source changed. |
| 8. No `%include` usage (deprecated for modules) | Pass | No %include added. |
| 9. Copyright 2026 on all new files | Pass | All newly authored source, tests and review/design records have 2026 copyright notices; existing third-party notices remain intact. |
| 10. Directory layout: `.qm` inside `qlib/<ModuleName>/` directory (not at `qlib/<ModuleName>.qm` for multi-file modules) | N/A | No new Qore module or public QPP class; existing module registration and namespaces remain in place. |
| 11. No second `.qm` for the same module at `qlib/<ModuleName>.qm` | N/A | No new Qore module or public QPP class; existing module registration and namespaces remain in place. |
| 12. `ns=Qore::XX` matches the QoreNamespace constructor path | N/A | No new Qore module or public QPP class; existing module registration and namespaces remain in place. |
| 13. `%modern` directive present | Pass | The new Qore regression file explicitly declares %modern. |
| 14. Executable permission set (`chmod +x`) | Pass | The new .qtest has mode 100755 (filesystem mode 755 checked). |
| 15. Uses %prepend-module-path  before %requires for in-repo modules (Qore and Qore modules only; not Qorus) | Pass | The new regression prepends repository qlib before requires; QORE_MODULE_DIR selects local Debug JNI and core reflection artifacts. The existing JNI suite retains its dependencies; concurrent suites preload the local JNI module with -l jni before their build-path directives. |
| 16. External module dependencies use `%try-module` — except modules delivered with the project itself (Qore ex: DataProvider, ConnectionProvider, QUnit, etc.) which use hard `%requires` | Pass | The tested binary module belongs to the repository and uses hard %requires; QUnit is a required core test dependency. No new optional external dependency. |
| 17. No filesystem operations (fopen, open, creat, unlink, remove, rename, mkdir, rmdir, stat, chmod) without sandbox checks | Pass | Local source-line normalization and Class reference ownership add no native filesystem operation. |
| 18. No network operations (connect, bind, socket, getaddrinfo, gethostbyname) without sandbox checks | Pass | No native network operation was added. |
| 19. If filesystem/network ops exist, verify `QoreSandboxManagerHelper` usage | N/A | No filesystem/network operation or sandbox policy change. |
| 20. No `File::`, `Dir::`, `Socket::`, `HTTPClient::` usage without justification | Pass | Tests invoke Java arraycopy and integer parsing; no production Qore I/O was added. |
| 21. All `for`/`while` loops that could iterate >100 times have `qore_check_cancel()` checks | N/A | No loop or traversal added: source-line normalization and a local ownership guard cover existing paths. |
| 22. Uses `qore_check_cancel()` (NOT deprecated `qore_check_io_interrupt()`) | N/A | No cancellation operation changed. |
| 23. Check frequency: every 100 iterations for tight loops, every 10 for expensive iterations | N/A | No new potentially long-running operation. |
| 24. No blocking operations without cancellation support | Pass | Normalization performs one comparison and assignment; no blocking operation. |
| 25. Every action has `display_name`, `short_desc` (plain text, <80 chars), `desc` (markdown) | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 26. Every action has `options` populated via `getActionOptionFromFields()` — without this, the action shows an empty, unusable form | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 27. Every action has `output_type` set to a typed data type constant (e.g., `MyResponseDataType`) — not omitted | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 28. DPAT_API actions: provider has `"supports_request": True` and implements `doRequestImpl()` | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 29. DPAT_FIND actions: every option exists in `SearchOptions`, `getRecordTypeImpl()` returns `*hash<string, AbstractDataField>` | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 30. Scheme-based apps (with `"scheme"` in registerApp): actions use `"path"` and do NOT use `"cls"` — having both `scheme` and `cls` causes a runtime error | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 31. Single-key hash slices use trailing comma: `Fields{"key",}` (without trailing comma, `Fields{"key"}` returns the value, not a hash) | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 32. **Typed data type classes exist** for request and response types — inherit `HashDataType`, have `const Fields` hash, call `addQoreFields(Fields)` in constructor, export public constant at bottom (e.g., `public const MyDataType = new MyDataType();`) | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 33. Request/input types use `public` Fields (enables `ClassName::Fields` in action registration) | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 34. Response/output types use `private` Fields | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 35. Each field in data types has `display_name`, `type`, and `desc` (markdown-formatted) | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 36. Input fields have `example_value` where useful (string fields, endpoint URIs, SQL queries, etc.) | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 37. Fields with finite allowed values use `allowed_values` with `AllowedValueInfo` containing both `value` and `display_name` (Title Case, human-readable) — never bare values, never described only in text | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 38. Password/secret fields have `"sensitive": True` | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 39. `groups` uses `AppGroup` enum values from `qlib/DataProvider/AppGroup.qc` | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 40. App `logo` stored as separate file, loaded at module level in `Priv` namespace | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 41. App `desc` uses markdown: bullet list of capabilities, links to project website, business-language explanation of value | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 42. `display_name` is user-friendly ("Apache Avro" not "avro") | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 43. `short_desc` is plain text, under 80 chars, single sentence — no markdown | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 44. `desc` uses markdown: backticks for code/field refs (`` `field_name` ``, `` `True` ``, `` `pdf` ``), `\n\n` for paragraphs, `- ` bullet lists for enumerations, `**bold**` for caveats | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 45. Descriptions use plain business language relating to common challenges — not just technical "what" but "why" and "when to use" | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 46. No bare `True`/`False`/`NOTHING` — must be backtick-wrapped in `desc` | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 47. No bare field/option names in prose — must use backticks | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 48. Long descriptions (>500 chars) use bold section headers and bullet lists | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 49. **Factory registration in Qore repo**: every factory name registered in `qlib/DataProvider/DataProvider.qc` → `FactoryMap` (without this, module loads but doesn't appear in Qorus apps) | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 50. **`getRecordTypeImpl()` signature**: must be `private *hash<string, AbstractDataField> getRecordTypeImpl(*hash<auto> search_options)` — NOT returning `*AbstractDataProviderType` | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 51. **Dependency JARs committed** (for JNI modules): JAR files in `qlib/*/jar/` may be gitignored — use `git add -f` to ensure they're tracked, otherwise CI compilation fails | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 52. JAR install rules in CMakeLists.txt for all dependency JARs | N/A | No DataProvider implementation, registration, field metadata, JNI dependency JAR or application packaging change in this increment. |
| 53. **No workarounds**: No TODOs, FIXMEs, stubs, or partially-implemented features | Pass | The implementation fixes the identified cause. No fixture-specific branch, stub, ignored regression or runtime workaround was added. Independent validation findings are recorded explicitly. |
| 54. **Exception safety**: C++ uses `ReferenceHolder` for Qore allocations, `std::unique_ptr` for C++ allocations, `*xsink` checked after every fallible operation | Pass | SimpleRefHolder owns each temporary Class wrapper through duplicate/recursive returns and exceptions until the non-throwing setManagedUserData assignment takes ownership. Debug lifecycle and concurrency tests pass. JVM-level Valgrind diagnostics are not an acceptance gate, per the user clarification on 2026-09-08. |
| 55. **Thread safety**: All mutable shared state protected by `std::lock_guard<std::mutex>` or documented as immutable-after-construction | Pass | The jint and reference holder are local; existing class-map locks, publication and recursive placeholder ownership remain unchanged. |
| 56. **Type safety**: Strongly-typed `code<return(args)>` instead of untyped `code`; `static_cast` instead of C casts; typed hashdecls for results; enums where appropriate | Pass | Retains JNI jint and the Qore sentinel; SimpleRefHolder<Class> transfers ownership explicitly with release(). No public type change. |
| 57. **Performance**: No O(n²) where O(n) is possible; no unnecessary copies; coordinate descent uses incremental residuals not full matrix multiply | Pass | Constant-time line normalization and ownership guard; no new allocation or traversal. |
| 58. **Error handling**: All inputs validated (dimensions, empty data, unfitted models); C++ I/O handles EAGAIN/EINTR if applicable | Pass | Handles every line below -1; preserves known source lines and Java exception identity. Native null-array rejection, ordinary parse rejection and successful subsequent calls pass. |
| 59. **Documentation**: Doxygen `@param`, `@return`, `@throw` on all public methods; `@par Example` with realistic business scenarios; `@note` for important caveats | Pass | No new public method. Exception-location API docs and release notes describe the conversion, with executable examples in the regression. |
| 60. **QPP flags**: `[flags=CONSTANT]` on methods that never throw; `[flags=RET_VALUE_ONLY]` on methods that throw but have no side effects | N/A | No QPP function or method declaration changes in this repository increment. |
| 61. **Security**: No user-controlled format strings; no buffer overflows; bounds checking on array indices; no credentials in code | Pass | No pointer arithmetic, format-string change, external I/O or credential. Wrapper ownership now covers early returns and exceptions. |
| 62. **Correctness**: Algorithms verified against reference implementations; edge cases tested (empty data, single sample, all-zero features) | Pass | Six location cases cover native calls, -1/-2/arbitrary negative lines, both jint limits, zero, positive lines, missing filenames, empty stacks, metadata and recovery. All 57 existing JNI cases and both callback/concurrent import/concurrent code-generation cases pass. The MIME test now accepts quoted and unquoted boundaries while comparing the complete serialized message. |
