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

# adds a make rule for java -> class generation annd class -> inc generation
MACRO(GENERATE_JAVA _input)
    GET_FILENAME_COMPONENT(_base ${_input} NAME_WE)
    GET_FILENAME_COMPONENT(_pkgdir ${_input} PATH)
    SET(_class_file ${CMAKE_CURRENT_BINARY_DIR}/${_pkgdir}/${_base}.class)

    # generate rules to generate include files for primary classes
    SET(_inc_outfile ${CMAKE_CURRENT_BINARY_DIR}/JavaClass${_base}.inc)
    add_custom_command(
        OUTPUT ${_inc_outfile}
        COMMAND ${CMAKE_SOURCE_DIR}/make-inc ${_class_file} ${_inc_outfile}
        DEPENDS ${_class_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    SET(JAVA_JAR_CLASSES ${JAVA_JAR_CLASSES} ${_class_file})
    SET(JAVA_JAR_TARGETS ${JAVA_JAR_TARGETS} ${_class_file})
    SET(JAVA_JAR_SRC ${JAVA_JAR_SRC} ${CMAKE_SOURCE_DIR}/src/java/${_input})
    SET(QORE_INC_FILES ${QORE_INC_FILES} ${_inc_outfile})

    # generate rules for include files for inner classes (if any)
    foreach (it ${ARGN})
        #message(STATUS "IT: ${it}")
        SET(_inc_infile ${CMAKE_CURRENT_BINARY_DIR}/${_pkgdir}/${_base}$${it}.class)
        # we need cmake to replace the "$" signs in inner class filenames with "$$" so they will be valid Makefile targets
        SET(_inc_infile_target ${CMAKE_CURRENT_BINARY_DIR}/${_pkgdir}/${_base}$$${it}.class)
        SET(_inc_outfile ${CMAKE_CURRENT_BINARY_DIR}/JavaClass${_base}_${it}.inc)
        add_custom_command(
            OUTPUT ${_inc_outfile}
            COMMAND ${CMAKE_SOURCE_DIR}/make-inc ${_inc_infile} ${_inc_outfile}
            DEPENDS ${_inc_infile}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            VERBATIM
        )
        SET(JAVA_JAR_CLASSES ${JAVA_JAR_CLASSES} ${_inc_infile})
        SET(JAVA_JAR_TARGETS ${JAVA_JAR_TARGETS} ${_inc_infile_target})
        SET(QORE_INC_FILES ${QORE_INC_FILES} ${_inc_outfile})
    endforeach()
ENDMACRO(GENERATE_JAVA)

# each file in qore-jni.jar is listed here, along with an indication of inner classes if present
generate_java(org/qore/jni/QoreJavaApi.java)
generate_java(org/qore/jni/QoreExceptionWrapper.java)
generate_java(org/qore/jni/QoreHashMap.java)
generate_java(org/qore/jni/QoreInvocationHandler.java)
generate_java(org/qore/jni/QoreURLClassLoader.java 1)

#message(STATUS "MESSAGE Src: ${JAVA_JAR_SRC}")
#message(STATUS "MESSAGE Classes: ${JAVA_JAR_CLASSES}")
#message(STATUS "MESSAGE Inc: ${QORE_INC_FILES}")

add_custom_command(
    OUTPUT ${JAVA_JAR_TARGETS}
    COMMAND ${Java_JAVAC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR} ${JAVA_JAR_SRC}
    DEPENDS ${JAVA_JAR_SRC}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

# generate jar manually
add_custom_command(
    OUTPUT qore-jni.jar
    COMMAND ${Java_JAR_EXECUTABLE} cf qore-jni.jar ${JAVA_JAR_CLASSES}
    DEPENDS ${JAVA_JAR_TARGETS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

add_custom_target(qore-jni ALL DEPENDS qore-jni.jar)

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
    INCLUDE_JARS qore-jni.jar
)

add_dependencies(qore-jni-test qore-jni)

add_custom_command(
    TARGET qore-jni-test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/qore-jni-test.jar ${CMAKE_SOURCE_DIR}/test
)

#get_target_property(_classDir qore-jni CLASSDIR)

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

add_library(${module_name} SHARED ${CPP_SRC} ${QPP_SOURCES})
include_directories(${JNI_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)
target_include_directories(${module_name} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)

# add include file dependencies to Globals.cpp
set_source_files_properties(${CMAKE_SOURCE_DIR}/src/Globals.cpp PROPERTIES OBJECT_DEPENDS "${QORE_INC_FILES}")

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
