print(STATUS "    List of include directories:")
get_property(all_include_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${all_include_dirs})
    print(STATUS "        ${dir}")
endforeach()

# Custom test target to run the googletest tests
add_custom_target(check)
add_custom_command(
        TARGET check
        POST_BUILD
        COMMENT "All tests were successful!" VERBATIM
)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/stamps)

# will compile and run ${test_target}.cpp
# and add all further arguments as dependencies
macro(run_test test_target)
    add_executable(${test_target}_testrunner EXCLUDE_FROM_ALL ${test_target}.cpp)

    add_dependencies(${test_target}_testrunner fetch_sdsl)

    target_link_libraries(${test_target}_testrunner stdc++fs rt dl gtest gtest_main)

    # Runs the test and generates a stamp file on success.
    add_custom_command(
            OUTPUT
            stamps/${test_target}_testrunner.stamp
            DEPENDS
            ${test_target}_testrunner
            COMMAND
            ${test_target}_testrunner
            COMMAND
            cmake -E touch ${CMAKE_CURRENT_BINARY_DIR}/stamps/${test_target}_testrunner.stamp
            WORKING_DIRECTORY
            "${CMAKE_BINARY_DIR}"
            COMMENT
            "Running test: ${test_target} ..."
            VERBATIM
    )

    # The test target. Depends on the stamp file to ensure the
    # test is only run if the source changed
    add_custom_target(
            ${test_target}
            DEPENDS
            stamps/${test_target}_testrunner.stamp
    )

    # Hook into check target
    add_custom_command(
            TARGET check
            PRE_BUILD
            COMMAND cmake --build . --target ${test_target}
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Test ${test_target}" VERBATIM
    )

    # Ensure binary deps of the testrunner are compiled first
    foreach(bin_dep ${TEST_TARGET_BIN_DEPS})
        add_custom_command(
                TARGET ${test_target}_testrunner
                PRE_BUILD
                COMMAND cmake --build . --target ${bin_dep}
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
    endforeach(bin_dep)
endmacro()