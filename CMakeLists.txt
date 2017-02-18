#
# Copyright (C) 2014-2017 Barbara Geller & Ansel Sermersheim
# All rights reserved.    
#
# Permission to use, copy, modify, and distribute this software and its
# documentation under the terms of the GNU General Public License version 2
# is hereby granted. No representations are made about the suitability of
# this software for any purpose. It is provided "as is" without express or
# implied warranty. See the GNU General Public License for more details.
#
# Documents produced by DoxyPress are derivative works derived from the
# input used in their production; they are not affected by this license.

cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project(doxypress)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckTypeSize)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# FIXME - find packages not working
find_package(CopperSpice REQUIRED)
# find_package(LibClang REQUIRED)
# find_package(LibTooling REQUIRED)

include(CopperSpiceMacros)

# FIXME - hard coded, I believe these should be  CopperSpice::Core
set(CopperSpiceCore "z:/cs5_x_lib/bin/libCsCore1.4.dll")
set(CopperSpiceXml  "z:/cs5_x_lib/bin/libCsXml1.4.dll")

# list of CopperSpice libraries
set(CS_LIBS ${CopperSpiceCore}  ${CopperSpiceXml})

# path to lib clang
set(CLANG_PATH "../clang_lib")

# lib clang dll
set(CLANG_LIBS ${CLANG_PATH}/bin/libclang.dll)

# lib clang hdr
set(CLANG_INC ${CLANG_PATH}/include/3.7.1)

# file locations for installing
if(DARWIN)
   include(GNUInstallDirs)

elseif(WIN32)
   set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX})   
   set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
  
   # path to gcc  FIXME
   # gccpath=$(dir $(shell which $(word 1,$(CC))))
   set(GCC_PATH "z:/w_retain/gcc_libs")

   # list of gcc libraries  sjlj or seh
   file(GLOB wild ${GCC_PATH}/libgcc_s_*-1.dll)
   set(GCC_LIBS ${wild} ${GCC_PATH}/libstdc++-6.dll  ${GCC_PATH}/libwinpthread-1.dll )

else()
   include(GNUInstallDirs)

endif()

set(BUILD_MAJOR "1")
set(BUILD_MINOR "2")
set(BUILD_MICRO "6")

set(BUILD_COMPONENTS "DOXYPRESS")

set(PACKAGE "doxypress")
set(PACKAGE_NAME "doxypress")
set(PACKAGE_VERSION "${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_STRING "doxypress ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_TARNAME "doxypress")
set(PACKAGE_BUGREPORT "info@copperspice.com")
set(PACKAGE_URL "http://www.copperspice.com/") 
set(HEX_VERSION "0x0${BUILD_MAJOR}0${BUILD_MINOR}0${BUILD_MICRO}")
set(VERSION "${PACKAGE_VERSION}")
set(prefix "${CMAKE_INSTALL_PREFIX}")

if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang|AppleClang)")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpmachine
        OUTPUT_VARIABLE DUMPMACHINE_OUTPUT
        RESULT_VARIABLE DUMPMACHINE_EXITCODE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT DUMPMACHINE_EXITCODE EQUAL 0)
        message(SEND_ERROR "failed to obtain machine from ${CMAKE_CXX_COMPILER}")
    endif()
    set(target "${DUMPMACHINE_OUTPUT}")
else()
    message(WARNING
        "Your compiler (${CMAKE_CXX_COMPILER}) is not yet handled properly. "
        "If it supports -dumpmachine argument or alternative let use know. "
        "This affects binary compatibility with plugins, the key that will be "
        "used is: ${CMAKE_SYSTEM}"
    )
    # more compiler handlers may be required, check CMake docs for CMAKE_LANG_COMPILER_ID
    set(target "${CMAKE_SYSTEM}")
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME} )
set(CPACK_PACKAGE_VENDOR "CopperSpice")
set(CPACK_PACKAGE_VERSION_MAJOR ${BUILD_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BUILD_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BUILD_MICRO})

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Generate documentation for a variety of programming languages")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")

set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git;${CPACK_SOURCE_IGNORE_FILES}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CMAKE_INSTALL_PREFIX})
include(CPack)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

if(APPLE)
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,-undefined,error")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,error")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-undefined,error")
else()
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,--no-undefined")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")
endif()

string(TIMESTAMP BUILD_DATE "%m/%d/%Y")
add_definitions(-DBUILD_DATE="${BUILD_DATE}")
configure_file(
    ${CMAKE_SOURCE_DIR}/src/doxy_build_info.h.in    
    src/doxy_build_info.h
)

# file locations for building
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# for install, not sure why this is required  FIXME?
set(CMAKE_INSTALL_LOCAL_ONLY ON)

add_subdirectory(src)
