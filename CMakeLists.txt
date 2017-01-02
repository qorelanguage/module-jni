cmake_minimum_required(VERSION 2.8.3)

project(qore-jni-module)

set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
set (VERSION_PATCH 0)

set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

find_package(Qore 0.8.13 REQUIRED)
find_package(JNI REQUIRED)
find_package(Java REQUIRED)
include(UseJava)

# Check for C++11.
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

MACRO(GENERATE_JAVA_INC _inc_files)
    foreach (it ${ARGN})
        GET_FILENAME_COMPONENT(_base ${it} NAME_WE)
        SET(_infile ${_classDir}/org/qore/jni/${_base}.class)
        SET(_outfile ${CMAKE_CURRENT_BINARY_DIR}/include/${_base}.inc)

        add_custom_command(
            OUTPUT ${_outfile}
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include
            COMMAND ${CMAKE_SOURCE_DIR}/make-inc ${_infile} ${_outfile}
            MAIN_DEPENDENCY ${_infile}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            VERBATIM
        )
        SET(${_inc_files} ${${_inc_files}} ${_outfile})
    endforeach()
ENDMACRO(GENERATE_JAVA_INC)

set(JAVA_JAR_SRC
    src/java/org/qore/jni/QoreExceptionWrapper.java
    src/java/org/qore/jni/QoreHashMap.java
    src/java/org/qore/jni/QoreInvocationHandler.java
    src/java/org/qore/jni/QoreJavaApi.java
    src/java/org/qore/jni/QoreURLClassLoader.java
)

add_jar(qore-jni ${JAVA_JAR_SRC})

set(JAVA_TEST_JAR_SRC
    test/java/src/org/qore/jni/test/Callbacks.java
    test/java/src/org/qore/jni/test/Fields.java
    test/java/src/org/qore/jni/test/FloatConversions.java
    test/java/src/org/qore/jni/test/Methods.java
    test/java/src/org/qore/jni/test/QoreJavaApiTest.java
    test/java/src/org/qore/jni/test/StaticFields.java
    test/java/src/org/qore/jni/test/StaticMethods.java
    test/java/src/org/qore/jni/test/StringFactory.java
)

add_jar(qore-jni-test ${JAVA_TEST_JAR_SRC}
    INCLUDE_JARS qore-jni
    OUTPUT_DIR test
)

get_target_property(_classDir qore-jni CLASSDIR)

message(STATUS "MESSAGE Class compiled to ${_classDir}")

set(QPP_SRC
    src/ql_jni.qpp
    src/QC_JavaArray.qpp
    src/QC_QoreInvocationHandler.qpp
)

set(CPP_SRC
    src/jni-module.cpp
    src/QoreJniClassMap.cpp
    src/defs.cpp
    src/Env.cpp
    src/GlobalReference.cpp
    src/Jvm.cpp
    src/Array.cpp
    src/Class.cpp
    src/Dispatcher.cpp
    src/Field.cpp
    src/Globals.cpp
    src/InvocationHandler.cpp
    src/Method.cpp
    src/JavaToQore.cpp
    src/QoreToJava.cpp
)

qore_wrap_qpp_value(QPP_SOURCES ${QPP_SRC})

set(module_name "jni")

set(QORE_DOX_TMPL_SRC
  docs/mainpage.dox.tmpl
)

#add_dependencies(src/Globals.cpp JavaClassQoreExceptionWrapper.inc)

add_library(${module_name} SHARED ${CPP_SRC} ${QPP_SOURCES})
include_directories(${JNI_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)
target_include_directories(${module_name} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)

generate_java_inc(QORE_INC_SRC ${JAVA_JAR_SRC} "QoreURLClassLoader\$1.java")
add_custom_target(QORE_INC_FILES DEPENDS ${QORE_INC_SRC})
add_dependencies(${module_name} QORE_INC_FILES)

target_link_libraries(${module_name} ${JNI_LIBRARIES})

qore_binary_module(${module_name} ${PROJECT_VERSION})

qore_dist(${PROJECT_VERSION})

qore_config_info()

FIND_PACKAGE(Doxygen)
if (DOXYGEN_FOUND)
  qore_wrap_dox(QORE_DOX_SRC ${QORE_DOX_TMPL_SRC})
  add_custom_target(QORE_MOD_DOX_FILES DEPENDS ${QORE_DOX_SRC})
  add_dependencies(docs QORE_MOD_DOX_FILES)
endif()
