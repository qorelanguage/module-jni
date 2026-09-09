# Copyright 2026 Qore Technologies, s.r.o.
# SPDX-License-Identifier: MIT

foreach(_required IN ITEMS CASE HELPER WORK)
    if(NOT DEFINED ${_required})
        message(FATAL_ERROR "${_required} is required")
    endif()
endforeach()

if(NOT CASE STREQUAL "undeclared" AND NOT CASE STREQUAL "duplicate")
    message(FATAL_ERROR "unsupported boundary case: ${CASE}")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/src/qlib/Declared")
file(WRITE "${WORK}/src/qlib/Declared/Declared.qm"
    "%module-cmd(jni) add-relative-classpath ./jar/declared.jar\n")

set(_extra_source "")
set(_extra_profile "")
set(_expected "")
if(CASE STREQUAL "undeclared")
    file(MAKE_DIRECTORY "${WORK}/src/qlib/Undeclared")
    file(WRITE "${WORK}/src/qlib/Undeclared/Undeclared.qm"
        "%module-cmd(jni) add-relative-classpath ./jar/undeclared.jar\n")
    set(_expected "runtime classpath directives but no")
else()
    set(_extra_profile "qore_java_provider_profile(Declared none)\n")
    set(_expected "dependency profile was declared more than once")
endif()

file(WRITE "${WORK}/src/CMakeLists.txt"
    "cmake_minimum_required(VERSION 3.16)\n"
    "project(JavaProviderProfileBoundary NONE)\n"
    "include(\"${HELPER}\")\n"
    "add_custom_target(Declared-qmod)\n"
    "qore_java_provider_profile(Declared none)\n"
    "${_extra_profile}"
    "qore_finalize_java_provider_profiles()\n")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -S "${WORK}/src" -B "${WORK}/build"
    RESULT_VARIABLE _result
    OUTPUT_VARIABLE _stdout
    ERROR_VARIABLE _stderr)
set(_output "${_stdout}\n${_stderr}")
if(_result EQUAL 0)
    message(FATAL_ERROR "${CASE}: invalid profile boundary unexpectedly configured")
endif()
string(FIND "${_output}" "${_expected}" _expected_offset)
if(_expected_offset EQUAL -1)
    message(FATAL_ERROR
        "${CASE}: expected diagnostic '${_expected}' was not emitted:\n${_output}")
endif()
