print(STATUS "")
print(STATUS "Entering directory test...")
print(STATUS "    Adding gtest as external project...")

include(ExternalProject)
ExternalProject_Add(
        get_gtest PREFIX external/gtest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG 37f322783175a66c11785d17fc153477b0777753 # 1.8.1
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        UPDATE_COMMAND ""
        EXCLUDE_FROM_ALL 1
)


file(GLOB files "*.cpp")
foreach(file ${files})
    string(REGEX REPLACE ".*/" "" file "${file}")
    string(REPLACE ".cpp" "" file "${file}")
    print(STATUS "    Adding target for test: ${file}")

    add_executable(${file} ${CMAKE_CURRENT_SOURCE_DIR}/${file}.cpp)
    add_dependencies(${file} get_gtest)

    include_directories(${CMAKE_CURRENT_BINARY_DIR}/external/gtest/include)
    target_link_libraries(${file} stdc++fs rt dl
            ${CMAKE_CURRENT_BINARY_DIR}/external/gtest/lib/libgtest.a
            ${CMAKE_CURRENT_BINARY_DIR}/external/gtest/lib/libgtest_main.a)
endforeach()

