cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

cmake_policy(VERSION 3.16..3.21)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.20.0")
   # enable RTTI on MSVC
   cmake_policy(SET CMP0117 OLD)
endif()

project(doxypress)

set(BUILD_MAJOR "1")
set(BUILD_MINOR "4")
set(BUILD_MICRO "1")

set(BUILD_COMPONENTS "doxypress")

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckTypeSize)

find_package(CopperSpice REQUIRED)

# file locations for installing
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   include(GNUInstallDirs)
   set(CMAKE_INSTALL_RPATH "@executable_path")

elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
   include(GNUInstallDirs)
   set(CMAKE_INSTALL_RPATH "\$ORIGIN")

elseif(MSVC)
   # use defaults

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
   set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
   include(${CopperSpice_DIR}/InstallMinGW.cmake)

endif()

# locate the files for libClang and libTooling
set(CLANG_PATH "../clang_lib")

# lib clang hdr
set(CLANG_INC ${CLANG_PATH}/include/10.0.1)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")

   # lib clang dll
   list(APPEND CLANG_LIBS
      ${CLANG_PATH}/lib/libclang.dylib
      ${CLANG_PATH}/lib/libclang.dylib
   )

elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")

   # lib clang dll
   list(APPEND CLANG_LIBS
      ${CLANG_PATH}/lib/libclang.so
      ${CLANG_PATH}/lib/libclang.so.10
   )

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")

   # lib clang dll
   list(APPEND CLANG_LIBS
      ${CLANG_PATH}/bin/libclang.dll
   )

endif()

set(PACKAGE           "doxypress")
set(PACKAGE_NAME      "DoxyPress")
set(PACKAGE_VERSION   "${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_STRING    "doxypress ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_TARNAME   "doxypress")
set(PACKAGE_BUGREPORT "info@copperspice.com")
set(PACKAGE_URL       "https://www.copperspice.com/")

set(CPACK_PACKAGE_NAME    ${PROJECT_NAME} )
set(CPACK_PACKAGE_VENDOR  "CopperSpice")

set(CPACK_PACKAGE_VERSION_MAJOR ${BUILD_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BUILD_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BUILD_MICRO})

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Generate documentation for a variety of programming languages")
set(CPACK_PACKAGE_DESCRIPTION_FILE    "${CMAKE_SOURCE_DIR}/README.md")

set(CPACK_PACKAGE_INSTALL_DIRECTORY   ${CMAKE_INSTALL_PREFIX})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY  OFF)

include(CPack)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

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

# location for building binary files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)

if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
   set(TARGETBITS 32)
else()
   set(TARGETBITS 64)
endif()

message("")
message("DoxyPress configured to run on:  ${CMAKE_SYSTEM_NAME} ${TARGETBITS} bit, ${CMAKE_BUILD_TYPE} Mode")
message("DoxyPress will be built in:      ${CMAKE_BINARY_DIR}")
message("DoxyPress will be installed in:  ${CMAKE_INSTALL_PREFIX}")
message("\n")
