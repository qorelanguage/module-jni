cmake_minimum_required(VERSION 2.8.13)

project(qore-jni-module)

set (VERSION_MAJOR 2)
set (VERSION_MINOR 0)
set (VERSION_PATCH 0)

set (BYTE_BUDDY_JAR ${CMAKE_SOURCE_DIR}/byte-buddy/byte-buddy-1.10.20.jar)

set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# enable RPATH in macos bins (OS requires this in any case)
if (POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

find_package(Qore 0.9.5 REQUIRED)
find_package(JNI REQUIRED)
find_package(Java REQUIRED)
if (${Java_VERSION_MAJOR} LESS 11)
    message(FATAL_ERROR "This module requires Java 11+; the current Java version is ${Java_VERSION}.")
endif()
add_definitions(-DJAVA_VERSION_MAJOR=${Java_VERSION_MAJOR})
include(UseJava)

find_package(BZip2 REQUIRED)

# Check for C++11.
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

SET(_qore_jni_sentinel ${CMAKE_CURRENT_BINARY_DIR}/qore-jni.sentinel)
SET(_qore_jni_compiler_sentinel ${CMAKE_CURRENT_BINARY_DIR}/qore-jni-compiler.sentinel)

# adds a make rule for java -> class generation annd class -> inc generation
MACRO(GENERATE_JAVA _input)
    GET_FILENAME_COMPONENT(_base ${_input} NAME_WE)
    GET_FILENAME_COMPONENT(_pkgdir ${_input} PATH)
    SET(_class_file ${_pkgdir}/${_base}.class)

    # generate rules to generate include files for primary classes
    SET(_inc_outfile ${CMAKE_CURRENT_BINARY_DIR}/JavaClass${_base}.inc)
    add_custom_command(
        OUTPUT ${_inc_outfile}
        COMMAND ${CMAKE_SOURCE_DIR}/make-inc ${_class_file} ${_inc_outfile}
        DEPENDS ${_qore_jni_sentinel}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    SET(JAVA_JAR_CLASSES ${JAVA_JAR_CLASSES} ${_class_file})
    SET(JAVA_JAR_SRC ${JAVA_JAR_SRC} ${CMAKE_SOURCE_DIR}/src/java/${_input})
    SET(QORE_INC_FILES ${QORE_INC_FILES} ${_inc_outfile})

    # issue #3019: get list of source files for documentation
    string (REPLACE ";" " " JAVA_JAR_SRC_STR "${JAVA_JAR_SRC}")

    # generate rules for include files for inner classes (if any)
    foreach (it ${ARGN})
        #message(STATUS "IT: ${it}")
        if (${it} MATCHES "^[0-9]+$")
            SET(_inc_infile ${CMAKE_CURRENT_BINARY_DIR}/${_pkgdir}/${_base}$${it}.class)
            SET(_inc_relinfile ${_pkgdir}/${_base}$${it}.class)
            # we need cmake to replace the "$" signs in inner class filenames with "$$" so they will be valid Makefile targets
            SET(_inc_infile_target ${CMAKE_CURRENT_BINARY_DIR}/${_pkgdir}/${_base}$$${it}.class)
            SET(_inc_outfile ${CMAKE_CURRENT_BINARY_DIR}/JavaClass${_base}_${it}.inc)
        else()
            SET(_inc_infile ${CMAKE_CURRENT_BINARY_DIR}/${_pkgdir}/${it}.class)
            SET(_inc_relinfile ${_pkgdir}/${it}.class)
            # we need cmake to replace the "$" signs in inner class filenames with "$$" so they will be valid Makefile targets
            SET(_inc_infile_target ${CMAKE_CURRENT_BINARY_DIR}/${_pkgdir}/${it}.class)
            SET(_inc_outfile ${CMAKE_CURRENT_BINARY_DIR}/JavaClass${it}.inc)
        endif()
        add_custom_command(
            OUTPUT ${_inc_outfile}
            COMMAND ${CMAKE_SOURCE_DIR}/make-inc ${_inc_infile} ${_inc_outfile}
            DEPENDS ${_qore_jni_sentinel}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            VERBATIM
        )
        SET(JAVA_JAR_CLASSES ${JAVA_JAR_CLASSES} ${_inc_relinfile})
        SET(QORE_INC_FILES ${QORE_INC_FILES} ${_inc_outfile})
    endforeach()
ENDMACRO(GENERATE_JAVA)

# adds a make rule for jar -> inc generation
MACRO(GENERATE_JAR _input _output)
    GET_FILENAME_COMPONENT(_base ${_input} NAME_WE)
    GET_FILENAME_COMPONENT(_pkgdir ${_input} PATH)

    # generate rules to generate include files for primary classes
    SET(_inc_outfile ${CMAKE_CURRENT_BINARY_DIR}/${_output}.inc)
    add_custom_command(
        OUTPUT ${_inc_outfile}
        COMMAND ${CMAKE_SOURCE_DIR}/make-inc ${_input} ${_inc_outfile}
        DEPENDS ${_qore_jni_sentinel} ${_qore_jni_compiler_sentinel}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    SET(QORE_INC_FILES ${QORE_INC_FILES} ${_inc_outfile})
ENDMACRO(GENERATE_JAR)

# adds a make rule for java -> class generation and class -> inc generation
MACRO(ADD_JAVA_SOURCE _input)
    GET_FILENAME_COMPONENT(_base ${_input} NAME_WE)
    GET_FILENAME_COMPONENT(_pkgdir ${_input} PATH)
    SET(_class_file ${_pkgdir}/${_base}.class)
    SET(JAVA_JAR_CLASSES ${JAVA_JAR_CLASSES} ${_class_file})
    SET(JAVA_JAR_SRC ${JAVA_JAR_SRC} ${CMAKE_SOURCE_DIR}/src/java/${_input})

    # issue #3158: get list of source files for documentation
    string (REPLACE ";" " " JAVA_JAR_SRC_STR "${JAVA_JAR_SRC}")

    # generate rules for inner classes (if any)
    foreach (it ${ARGN})
        SET(_inc_relinfile ${_pkgdir}/${_base}$${it}.class)
        SET(JAVA_JAR_CLASSES ${JAVA_JAR_CLASSES} ${_inc_relinfile})
    endforeach()
ENDMACRO(ADD_JAVA_SOURCE)

MACRO(ADD_JAR_TARGETS)
    foreach (it ${ARGN})
        file(GLOB _files "${CMAKE_CURRENT_BINARY_DIR}/${it}/*.class")
        foreach (_file ${_files})
            string(REPLACE "${CMAKE_CURRENT_BINARY_DIR}/" "" _class_file ${_file})
            set(_file ${_file} )
            SET(JAVA_JAR_CLASSES ${JAVA_JAR_CLASSES} ${_class_file})
            #message(STATUS "ADDED ${_class_file}")
        endforeach()
    endforeach()
ENDMACRO(ADD_JAR_TARGETS)

# adds a make rule for java -> class generation for the compiler
MACRO(ADD_JAVA_COMPILER_SOURCE _input)
    GET_FILENAME_COMPONENT(_base ${_input} NAME_WE)
    GET_FILENAME_COMPONENT(_pkgdir ${_input} PATH)
    SET(_class_file ${_pkgdir}/${_base}.class)

    # generate rules to generate include files for compiler classes
    SET(_inc_outfile ${CMAKE_CURRENT_BINARY_DIR}/JavaClass${_base}.inc)
    add_custom_command(
        OUTPUT ${_inc_outfile}
        COMMAND ${CMAKE_SOURCE_DIR}/make-inc ${_class_file} ${_inc_outfile}
        DEPENDS ${_qore_jni_compiler_sentinel}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )
    SET(QORE_INC_FILES ${QORE_INC_FILES} ${_inc_outfile})
    SET(JAVA_JAR_COMPILER_CLASSES ${JAVA_JAR_COMPILER_CLASSES} ${_class_file})
    SET(JAVA_JAR_COMPILER_SRC ${JAVA_JAR_COMPILER_SRC} ${CMAKE_SOURCE_DIR}/src/java/${_input})

    # issue #3158: get list of source files for documentation
    string (REPLACE ";" " " JAVA_JAR_COMPILER_SRC_STR "${JAVA_JAR_COMPILER_SRC}")

    # generate rules for inner classes (if any)
    foreach (it ${ARGN})
        if (${it} MATCHES "^[0-9]+$")
            SET(_inc_infile ${_pkgdir}/${_base}$${it}.class)
            SET(_inc_relinfile ${_pkgdir}/${_base}$${it}.class)
            # we need cmake to replace the "$" signs in inner class filenames with "$$" so they will be valid Makefile targets
            SET(_inc_infile_target ${CMAKE_CURRENT_BINARY_DIR}/${_pkgdir}/${_base}$$${it}.class)
            SET(_inc_outfile ${CMAKE_CURRENT_BINARY_DIR}/JavaClass${_base}_${it}.inc)
        else()
            SET(_inc_infile ${_pkgdir}/${it}.class)
            SET(_inc_relinfile ${_pkgdir}/${it}.class)
            # we need cmake to replace the "$" signs in inner class filenames with "$$" so they will be valid Makefile targets
            SET(_inc_infile_target ${_pkgdir}/${it}.class)
            SET(_inc_outfile ${CMAKE_CURRENT_BINARY_DIR}/JavaClass${it}.inc)
        endif()

        add_custom_command(
            OUTPUT ${_inc_outfile}
            COMMAND ${CMAKE_SOURCE_DIR}/make-inc ${_inc_infile} ${_inc_outfile}
            DEPENDS ${_qore_jni_compiler_sentinel}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            VERBATIM
        )
        SET(JAVA_JAR_COMPILER_CLASSES ${JAVA_JAR_COMPILER_CLASSES} ${_inc_relinfile})
        SET(QORE_INC_FILES ${QORE_INC_FILES} ${_inc_outfile})
    endforeach()
ENDMACRO(ADD_JAVA_COMPILER_SOURCE)

# Java sources built in to the binary module with an indication of inner classes, if any
generate_java(org/qore/jni/QoreJavaApi.java)
generate_java(org/qore/jni/QoreExceptionWrapper.java)
generate_java(org/qore/jni/QoreException.java)
generate_java(org/qore/jni/QoreObjectBase.java)
generate_java(org/qore/jni/QoreJavaClassBase.java)
generate_java(org/qore/jni/QoreObject.java)
generate_java(org/qore/jni/QoreClosure.java)
generate_java(org/qore/jni/QoreObjectWrapper.java)
generate_java(org/qore/jni/QoreInvocationHandler.java)
generate_java(org/qore/jni/BooleanWrapper.java)
generate_java(org/qore/jni/ClassModInfo.java)
generate_java(org/qore/jni/QoreURLClassLoader.java 1 2)
generate_java(org/qore/jni/QoreRelativeTime.java)
generate_java(org/qore/jni/QoreClosureMarker.java)
generate_java(org/qore/jni/QoreJavaDynamicApi.java)
generate_java(org/qore/jni/Hash.java 1 2 3 4 5 6 7 8 9 10)
generate_java(org/qore/jni/JavaClassBuilder.java 1 2 StaticEntry)
generate_java(org/qore/jni/QoreJavaFileObject.java)
generate_jar(${BYTE_BUDDY_JAR} JavaJarByteBuddy)

# add Java sources without native methods
add_java_source(org/qore/lang/AbstractIterator.java)
add_java_source(org/qore/lang/HashListIterator.java)
add_java_source(org/qore/lang/reflection/Type.java)
add_java_source(org/qore/lang/sqlutil/Table.java)
add_java_source(org/qore/lang/sqlutil/AbstractTable.java 1 2 3 4 5)
add_java_source(org/qore/lang/sqlutil/AbstractSqlUtilBase.java)
add_java_source(org/qore/lang/AbstractDatasource.java)
add_java_source(org/qore/lang/mapper/Mapper.java)
add_java_source(org/qore/lang/mapper/MapperOutputRecordIterator.java)
add_java_source(org/qore/lang/tablemapper/RawSqlStatementOutboundMapper.java)
add_java_source(org/qore/lang/tablemapper/InboundIdentityTableMapper.java)
add_java_source(org/qore/lang/tablemapper/AbstractSqlStatementOutboundMapper.java)
add_java_source(org/qore/lang/tablemapper/InboundTableMapper.java)
add_java_source(org/qore/lang/tablemapper/SqlStatementOutboundMapper.java)
add_java_source(org/qore/lang/bulksqlutil/AbstractBulkOperation.java)
add_java_source(org/qore/lang/bulksqlutil/BulkRowCallback.java)
add_java_source(org/qore/lang/bulksqlutil/BulkInsertOperation.java)
add_java_source(org/qore/lang/bulksqlutil/BulkUpsertOperation.java)
add_java_source(org/qore/lang/dataprovider/AbstractDataProvider.java)
add_java_source(org/qore/lang/dataprovider/AbstractDataProviderType.java)
add_java_source(org/qore/lang/dataprovider/AbstractDataProviderRecordIterator.java)
add_java_source(org/qore/lang/dataprovider/AbstractDataProviderBulkRecordInterface.java)
add_java_source(org/qore/lang/dataprovider/AbstractDataField.java)
add_java_source(org/qore/lang/dataprovider/AbstractDataProviderBulkOperation.java)
add_java_source(org/qore/lang/dataprovider/AbstractDataProcessor.java)
add_java_source(org/qore/lang/qunit/Test.java)
add_java_source(org/qore/lang/qunit/TestCode.java)
add_java_source(org/qore/lang/qunit/TestCodeArgs.java)
add_java_source(org/qore/lang/qunit/TestCondition.java)
add_java_source(org/qore/lang/qunit/TestConditionArgs.java)
add_java_source(org/qore/lang/qunit/AbstractTestResult.java)
add_java_source(org/qore/lang/qunit/TestResultValue.java)
add_java_source(org/qore/lang/qunit/TestResultSuccess.java)
add_java_source(org/qore/lang/qunit/TestResultFailure.java)
add_java_source(org/qore/lang/qunit/TestResultExceptionType.java)
add_java_source(org/qore/lang/qunit/TestResultExceptionDetail.java)
add_java_source(org/qore/lang/qunit/TestResultExceptionRegexp.java)
add_java_source(org/qore/lang/qunit/TestResultExceptionSubstring.java)
add_java_source(org/qore/lang/LogCallback.java)
add_java_source(org/qore/lang/DatasourcePool.java)
add_java_source(org/qore/lang/AbstractSQLStatement.java)
add_java_source(org/qore/lang/HTTPClient.java)
add_java_source(org/qore/lang/soapclient/SoapClient.java)
add_java_source(org/qore/lang/restclient/RestClient.java)
add_java_source(org/qore/lang/mailmessage/Message.java)
add_java_source(org/qore/lang/mailmessage/Attachment.java)
add_java_source(org/qore/lang/mailmessage/Part.java)
add_java_source(org/qore/lang/smtpclient/SmtpClient.java)

add_java_compiler_source(org/qore/jni/compiler/CustomJavaFileObject.java)
add_java_compiler_source(org/qore/jni/compiler/PackageInternalsFinder.java)
add_java_compiler_source(org/qore/jni/compiler/QoreJavaClassObject.java)
add_java_compiler_source(org/qore/jni/compiler/QoreJavaCompiler.java JavaFileObjectImpl FileManagerImpl)
add_java_compiler_source(org/qore/jni/compiler/QoreJavaCompilerException.java)
add_java_compiler_source(org/qore/jni/compiler/QoreJavaFileManager.java)
add_java_compiler_source(org/qore/jni/compiler/CompilerOutput.java)

#message(STATUS "MESSAGE Src: ${JAVA_JAR_SRC}")
#message(STATUS "STR: ${JAVA_JAR_SRC_STR}")
#message(STATUS "MESSAGE Classes: ${JAVA_JAR_CLASSES}")
#message(STATUS "MESSAGE Inc: ${QORE_INC_FILES}")

set(CMAKE_JAVA_COMPILE_FLAGS -Xlint:unchecked -Xlint:deprecation -Xdiags:verbose)

add_custom_command(
    OUTPUT ${_qore_jni_sentinel}
    COMMAND ${Java_JAVAC_EXECUTABLE} ${CMAKE_JAVA_COMPILE_FLAGS} -d ${CMAKE_CURRENT_BINARY_DIR} ${JAVA_JAR_SRC} -cp ${BYTE_BUDDY_JAR}
    COMMAND ${CMAKE_COMMAND} -E touch ${_qore_jni_sentinel}
    DEPENDS ${JAVA_JAR_SRC}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

add_custom_command(
    OUTPUT ${_qore_jni_compiler_sentinel}
    COMMAND ${Java_JAVAC_EXECUTABLE} ${CMAKE_JAVA_COMPILE_FLAGS} -d ${CMAKE_CURRENT_BINARY_DIR} ${JAVA_JAR_COMPILER_SRC} -cp qore-jni.jar
    COMMAND ${CMAKE_COMMAND} -E touch ${_qore_jni_compiler_sentinel}
    DEPENDS ${JAVA_JAR_COMPILER_SRC} qore-jni.jar
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

add_jar_targets(org/qore/lang org/qore/lang/sqlutil org/qore/lang/mapper org/qore/lang/tablemapper org/qore/jni/QoreJavaDynamicApi.java)

# generate jar manually
add_custom_command(
    OUTPUT qore-jni-qore.jar
    COMMAND ${Java_JAR_EXECUTABLE} cf qore-jni-qore.jar ${JAVA_JAR_CLASSES}
    DEPENDS ${_qore_jni_sentinel}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

# combine jars into final jar
add_custom_command(
    OUTPUT qore-jni.jar
    COMMAND ${CMAKE_SOURCE_DIR}/make-jar -o qore-jni.jar ${BYTE_BUDDY_JAR} ${CMAKE_CURRENT_BINARY_DIR}/qore-jni-qore.jar
    DEPENDS ${BYTE_BUDDY_JAR} qore-jni-qore.jar
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

# build and install this jar file so that java code using native classes can be compiled
add_custom_target(qore-jni ALL DEPENDS qore-jni.jar qore-jni-compiler.jar)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qore-jni.jar DESTINATION ${CMAKE_INSTALL_PREFIX}/share/qore/java)

add_jar(qore-jni-compiler ${JAVA_JAR_COMPILER_SRC}
    INCLUDE_JARS qore-jni.jar
    DEPENDS qore-jni.jar
)

add_dependencies(qore-jni-compiler qore-jni)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qore-jni-compiler.jar DESTINATION ${CMAKE_INSTALL_PREFIX}/share/qore/java)

set(JAVA_TEST_JAR_SRC
    test/java/src/org/qore/jni/test/Callbacks.java
    test/java/src/org/qore/jni/test/Fields.java
    test/java/src/org/qore/jni/test/FloatConversions.java
    test/java/src/org/qore/jni/test/Methods.java
    test/java/src/org/qore/jni/test/A.java
    test/java/src/org/qore/jni/test/B.java
    test/java/src/org/qore/jni/test/C.java
    test/java/src/org/qore/jni/test/QoreJavaApiTest.java
    test/java/src/org/qore/jni/test/ClosureTest1.java
    test/java/src/org/qore/jni/test/ClosureTest2.java
    test/java/src/org/qore/jni/test/ClosureTest3.java
    test/java/src/org/qore/jni/test/StaticFields.java
    test/java/src/org/qore/jni/test/StaticMethods.java
    test/java/src/org/qore/jni/test/StringFactory.java
    test/java/src/org/qore/jni/test/QoreCallback.java
    test/java/src/org/qore/lang/test/QoreJavaLangApiTest.java
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
    src/QoreJniFunctionalInterface.cpp
    src/JniQoreClass.cpp
)

qore_wrap_qpp_value(QPP_SOURCES ${QPP_SRC})

foreach (it ${QPP_SOURCES})
    GET_FILENAME_COMPONENT(_outfile ${it} NAME_WE)
    set(QPP_DOX ${QPP_DOX} ${CMAKE_CURRENT_BINARY_DIR}/${_outfile}.dox.h)
endforeach()

set(module_name "jni")

set(QORE_DOX_TMPL_SRC
    docs/mainpage.dox.tmpl
)

add_library(${module_name} MODULE ${CPP_SRC} ${QPP_SOURCES})
include_directories(${JNI_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)
target_include_directories(${module_name} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)

# add include file dependencies to Globals.cpp
set_source_files_properties(${CMAKE_SOURCE_DIR}/src/Globals.cpp PROPERTIES OBJECT_DEPENDS "${QORE_INC_FILES}")

add_custom_target(QORE_INC_FILES DEPENDS ${QORE_INC_SRC})
add_dependencies(${module_name} QORE_INC_FILES)

target_link_libraries(${module_name} ${JNI_LIBRARIES} ${BZIP2_LIBRARIES} ${QORE_LIBRARY})
#target_link_libraries(${module_name} ${JNI_LIBRARIES})

set(MODULE_DOX_INPUT ${CMAKE_CURRENT_BINARY_DIR}/mainpage.dox ${JAVA_JAR_SRC_STR} ${QPP_DOX})
string(REPLACE ";" " " MODULE_DOX_INPUT "${MODULE_DOX_INPUT}")
#message(STATUS mdi: ${MODULE_DOX_INPUT})

set(MODULE_DOX_INPUT ${CMAKE_BINARY_DIR} ${JAVA_JAR_SRC} ${JAVA_JAR_COMPILER_SRC})
qore_binary_module(${module_name} ${PROJECT_VERSION})

qore_dist(${PROJECT_VERSION})

qore_config_info()

FIND_PACKAGE(Doxygen)
if (DOXYGEN_FOUND)
    qore_wrap_dox(QORE_DOX_SRC ${QORE_DOX_TMPL_SRC})
    add_custom_target(QORE_MOD_DOX_FILES DEPENDS ${QORE_DOX_SRC})
    add_dependencies(docs QORE_MOD_DOX_FILES)
endif()
