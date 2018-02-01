cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project(doxypress)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckTypeSize)

find_package(CopperSpice REQUIRED)

if(WIN32)
   set(CLANG_PATH "../clang_lib")

   # lib clang dll
   set(CLANG_LIBS ${CLANG_PATH}/bin/libclang.dll)

   # lib clang hdr
   set(CLANG_INC ${CLANG_PATH}/include/3.8.1)

   set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
   include(${CopperSpice_DIR}/InstallMinGW.cmake)

else()
   # find_package(LibClang REQUIRED)
   # find_package(LibTooling REQUIRED)

endif()

# file locations for installing
if(WIN32)
   set(CMAKE_INSTALL_BINDIR bin)
   set(CMAKE_INSTALL_INCLUDEDIR include)

else()
   include(GNUInstallDirs)

endif()

set(BUILD_MAJOR "1")
set(BUILD_MINOR "2")
set(BUILD_MICRO "12")
set(BUILD_COMPONENTS "doxypress")

set(PACKAGE "doxypress")
set(PACKAGE_NAME "doxypress")
set(PACKAGE_VERSION "${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_STRING "doxypress ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_TARNAME "doxypress")
set(PACKAGE_BUGREPORT "info@copperspice.com")
set(PACKAGE_URL "http://www.copperspice.com/")
set(VERSION "${PACKAGE_VERSION}")
set(prefix "${CMAKE_INSTALL_PREFIX}")

set(CPACK_PACKAGE_NAME ${PROJECT_NAME} )
set(CPACK_PACKAGE_VENDOR "CopperSpice")
set(CPACK_PACKAGE_VERSION_MAJOR ${BUILD_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BUILD_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BUILD_MICRO})

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Generate documentation for a variety of programming languages")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")

set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git;${CPACK_SOURCE_IGNORE_FILES}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CMAKE_INSTALL_PREFIX})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

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
