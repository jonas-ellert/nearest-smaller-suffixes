print(STATUS "")
print(STATUS "Entering directory src...")
get_filename_component(BENCHMARK_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
get_filename_component(BENCHMARK_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} DIRECTORY)

print(STATUS "    Adding tlx as external project...")
include(ExternalProject)
ExternalProject_Add(
        fetch_tlx PREFIX external/tlx
        GIT_REPOSITORY https://github.com/tlx/tlx
        GIT_TAG 1052dc757aad89b5d2bf442a17c22c4c0e697bf0 # 1.8.1
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        UPDATE_COMMAND ""
        EXCLUDE_FROM_ALL 1
)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/external/tlx/include)

print(STATUS "    Adding malloc_count libraries and includes...")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi")
add_library(malloc_count ${BENCHMARK_SRC_DIR}/external/malloc_count/malloc_count.c)
include_directories(${BENCHMARK_SRC_DIR}/external/malloc_count)

print(STATUS "    List of include directories:")
get_property(all_include_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${all_include_dirs})
    print(STATUS "        ${dir}")
endforeach()


print(STATUS "    Adding benchmark target...")
add_executable(benchmark ${CMAKE_CURRENT_SOURCE_DIR}/benchmark.cpp)
add_dependencies(benchmark fetch_tlx)
target_link_libraries(benchmark stdc++fs malloc_count rt dl
        ${CMAKE_CURRENT_BINARY_DIR}/external/tlx/lib/libtlx.a)