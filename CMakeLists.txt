cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(xss CXX C)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)

function(print TYPE TEXT)
    string(ASCII 27 Esc)
    message(${TYPE} "${Esc}[1m${Esc}[36m[XSS]${Esc}[m ${Esc}[1m${TEXT}${Esc}[m")
endfunction()
print(STATUS "")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

# Default to Release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
print(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

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

# Compiler flags         -Winline \ -Wno-error=inline \
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        -fopenmp \
        -Wall \
        -Wextra \
        -Wpedantic \
        -fdiagnostics-color=auto \
        --param large-function-growth=10000 \
        --param inline-unit-growth=1000 \
        -fconstexpr-loop-limit=4194304")
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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/benchmark/include)

add_subdirectory(benchmark/test)
add_subdirectory(benchmark/src)

print(STATUS "")
print(STATUS "DONE!")
print(STATUS "")