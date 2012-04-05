# Copyright 2006-2010 The FLWOR Foundation.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT (zorba_schema-tools_module)

# The util-jvm tool finds Java and JNI, so all standard cached JAVA_
# variables will be set. util-jvm also caches Java_FOUND and JNI_FOUND
# for us.
FIND_PACKAGE (zorba_util-jvm_module REQUIRED)

IF (JNI_FOUND)
  INCLUDE_DIRECTORIES (${zorba_util-jvm_module_INCLUDE_DIRS})

  IF (WIN32)
    SET (XMLBEANS_PATH $ENV{XMLBEANS_HOME}\\lib)
    FIND_FILE( XMLBEANS_JAR NAMES xmlbeans.jar xbean.jar
      PATHS ${XMLBEANS_PATH} )
    MESSAGE (STATUS "xmlbeans.jar looking path: " ${XMLBEANS_PATH} )
  ELSE (WIN32)
    FIND_FILE( XMLBEANS_JAR NAMES xmlbeans.jar xbean.jar
      PATHS /usr/share/java/ /usr/share/java/xmlbeans/ )
  ENDIF (WIN32)
  IF( EXISTS ${XMLBEANS_JAR} )
      
    MESSAGE (STATUS "xmlbeans.jar found at: " ${XMLBEANS_JAR} )
    # if everything is found build this module
    SET (SCHEMA_TOOLS_VERSION 1.0)

    ENABLE_TESTING ()
    INCLUDE (CTest)

    SET_CMAKE_MODULE_PATH ()

    FIND_PACKAGE (Zorba REQUIRED HINTS "${ZORBA_BUILD_DIR}")
    INCLUDE ("${Zorba_USE_FILE}")

    ADD_SUBDIRECTORY ("src")
    ADD_SUBDIRECTORY ("srcJava")
    ADD_TEST_DIRECTORY("${PROJECT_SOURCE_DIR}/test")
    DONE_DECLARING_ZORBA_URIS ()

  ELSE( EXISTS ${XMLBEANS_JAR} )
    MESSAGE (STATUS "xmlbeans.jar not found; skipping schema-tools module.")
  ENDIF( EXISTS ${XMLBEANS_JAR} )

ELSE (JNI_FOUND)
  MESSAGE (STATUS "Java and/or JNI not found; skipping schema-tools module.")
ENDIF (JNI_FOUND)
