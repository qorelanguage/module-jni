# Declarative qualification for Java-backed Qore data providers.
#
# The runtime classpath remains declared exactly once by each module's
# %module-cmd(jni) add-relative-classpath directives.  This helper attaches one
# logging policy to that declaration and emits the common inventory consumed by
# source-tree, AOT-tree, install-tree, and fresh-process checks.

set(QORE_JAVA_PROVIDER_PROFILE_FILE
    "${CMAKE_CURRENT_BINARY_DIR}/java-provider-profiles.tsv")
file(WRITE "${QORE_JAVA_PROVIDER_PROFILE_FILE}" "# module\tlogging-policy\truntime-jar\n")

function(qore_java_provider_profile _module _logging_policy)
    if(NOT _logging_policy STREQUAL "none" AND
            NOT _logging_policy STREQUAL "slf4j-nop")
        message(FATAL_ERROR
            "Java provider ${_module} has unsupported logging policy '${_logging_policy}'")
    endif()

    set(_module_source "${CMAKE_SOURCE_DIR}/qlib/${_module}/${_module}.qm")
    if(NOT EXISTS "${_module_source}")
        message(FATAL_ERROR "Java provider source not found: ${_module_source}")
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_module_source}")

    file(STRINGS "${_module_source}" _classpath_lines
        REGEX "^[ \t]*%module-cmd\\(jni\\)[ \t]+(global-)?add-relative-classpath[ \t]+")
    if(NOT _classpath_lines)
        message(FATAL_ERROR "Java provider ${_module} declares no runtime JARs")
    endif()

    set(_seen_paths "")
    foreach(_line IN LISTS _classpath_lines)
        string(REGEX REPLACE
            "^[ \t]*%module-cmd\\(jni\\)[ \t]+(global-)?add-relative-classpath[ \t]+([^ \t]+).*$"
            "\\2" _runtime_jar "${_line}")
        if(_runtime_jar IN_LIST _seen_paths)
            message(FATAL_ERROR
                "Java provider ${_module} declares runtime JAR ${_runtime_jar} more than once")
        endif()
        list(APPEND _seen_paths "${_runtime_jar}")
        file(APPEND "${QORE_JAVA_PROVIDER_PROFILE_FILE}"
            "${_module}\t${_logging_policy}\t${_runtime_jar}\n")
    endforeach()

    set_property(GLOBAL APPEND PROPERTY QORE_JAVA_PROVIDER_PROFILE_MODULES "${_module}")
endfunction()

function(qore_finalize_java_provider_profiles)
    get_property(_modules GLOBAL PROPERTY QORE_JAVA_PROVIDER_PROFILE_MODULES)
    if(NOT _modules)
        message(FATAL_ERROR "No Java provider profiles were declared")
    endif()

    set(_qmod_targets "")
    foreach(_module IN LISTS _modules)
        if(NOT TARGET ${_module}-qmod)
            message(FATAL_ERROR "Java provider ${_module} has no qmod build target")
        endif()
        list(APPEND _qmod_targets ${_module}-qmod)
    endforeach()

    find_program(QORE_JAVA_PROVIDER_PYTHON NAMES python3)
    if(NOT QORE_JAVA_PROVIDER_PYTHON)
        message(FATAL_ERROR "python3 is required to validate Java provider profiles")
    endif()
    add_custom_target(validate-java-provider-profiles ALL
        COMMAND "${QORE_JAVA_PROVIDER_PYTHON}"
            "${CMAKE_SOURCE_DIR}/test/docker_test/validate-java-provider-profiles.py"
            --profiles "${QORE_JAVA_PROVIDER_PROFILE_FILE}"
            --root "${CMAKE_SOURCE_DIR}/qlib"
            --checksums "${CMAKE_SOURCE_DIR}/qlib/java-provider-dependencies.sha256"
            --require-committed
        DEPENDS ${_qmod_targets}
        VERBATIM
    )

    install(FILES "${QORE_JAVA_PROVIDER_PROFILE_FILE}"
        DESTINATION "share/qore-jni")
endfunction()
