cmake_minimum_required(VERSION 3.0.0)

project(qore-jni-module VERSION 1.0.0)

include(CheckCXXCompilerFlag)

find_package(Qore 0.8.13 REQUIRED)

# Check for C++11.
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(CPP_SRC
    src/jni-module.cpp
)

set(module_name "jni")

add_library(${module_name} SHARED ${CPP_SRC})

qore_binary_module(${module_name} ${PROJECT_VERSION})

qore_dist(${PROJECT_VERSION})

qore_config_info()
