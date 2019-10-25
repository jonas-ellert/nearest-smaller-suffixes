print(STATUS "")
print(STATUS "Entering directory test...")

include(util/testsuite.cmake)

file(GLOB files "*.cpp")
foreach(file ${files})
    string(REGEX REPLACE ".*/" "" file "${file}")
    string(REPLACE ".cpp" "" file "${file}")
    print(STATUS "    Adding target for test: ${file}")
    run_test(${file})
endforeach()

