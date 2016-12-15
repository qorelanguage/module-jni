cmake_minimum_required(VERSION 2.8.3)

project(qore-jni-module)

set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
set (VERSION_PATCH 0)

set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

find_package(Qore 0.8.13 REQUIRED)
find_package(JNI REQUIRED)

# Check for C++11.
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(QPP_SRC
    src/ql_jni.qpp
    src/QC_JavaArray.qpp
    src/QC_QoreInvocationHandler.qpp
)

set(CPP_SRC
    src/jni-module.cpp
    src/QoreJniClassMap.cpp
    src/defs.cpp
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
