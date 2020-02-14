cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(xss CXX C)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)

function(print TYPE TEXT)
    string(ASCII 27 Esc)
    message(${TYPE} "${Esc}[1m${Esc}[36m[XSS]${Esc}[m ${Esc}[1m${TEXT}${Esc}[m")
endfunction()
print(STATUS "")
print(STATUS "Executing CMakeList.txt in directory ${CMAKE_CURRENT_SOURCE_DIR}...")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# REQUIRE G++/GCC 7.3
# Require G++ version
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "7.3")
        print(FATAL_ERROR "g++ version 7.3 or greater required!")
    endif()
else()
    print(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER_ID} is not supported!")
endif()
print(STATUS "g++ version: ${CMAKE_CXX_COMPILER_VERSION}")

# Check GCC version
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    if(${CMAKE_C_COMPILER_VERSION} VERSION_LESS "7.3")
        print(FATAL_ERROR "gcc version 7.3 or greater required!")
    endif()
else()
    print(FATAL_ERROR "Compiler ${CMAKE_C_COMPILER_ID} is not supported!")
endif()
print(STATUS "gcc version: ${CMAKE_C_COMPILER_VERSION}")

# Default to Release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
print(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

# Compiler flags         -Winline \ -Wno-error=inline \
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        -fopenmp \
        -Wall \
        -Wextra \
        -Wpedantic \
        -fdiagnostics-color=auto \
        --param large-function-growth=10000 \
        --param inline-unit-growth=1000")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} \
        -O3 \
        -ffast-math \
        -funroll-loops \
        -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} \
        -O0 \
        -ggdb")

print(STATUS "")
print(STATUS "Adding xss includes...")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

print(STATUS "Adding sdsl-lite as an external project...")
include(ExternalProject)
ExternalProject_Add(
        fetch_sdsl PREFIX benchmark/sdsl
        GIT_REPOSITORY https://github.com/simongog/sdsl-lite
        GIT_TAG ddb0fbbc33bb183baa616f17eb48e261ac2a3672 # 1.8.1
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        UPDATE_COMMAND ""
        EXCLUDE_FROM_ALL 1
)
print(STATUS "Adding include directory for sdsl, divsufsort, and gtest...")
include_directories(${CMAKE_CURRENT_BINARY_DIR}/benchmark/sdsl/include)
set(SDSL_LIB_PATH ${CMAKE_CURRENT_BINARY_DIR}/benchmark/sdsl/lib)

print(STATUS "Adding libraries for sdsl, divsufsort, and gtest...")
set(gtest ${SDSL_LIB_PATH}/libgtest.a)
set(gtest_main ${SDSL_LIB_PATH}/libgtest_main.a)
set(divsufsort ${SDSL_LIB_PATH}/libdivsufsort.a)
set(divsufsort64 ${SDSL_LIB_PATH}/libdivsufsort64.a)
set(sdsl ${SDSL_LIB_PATH}/libsdsl.a)

add_subdirectory(benchmark/test)
add_subdirectory(benchmark/src)

print(STATUS "")
print(STATUS "DONE!")
print(STATUS "")