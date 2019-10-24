print(STATUS "")
print(STATUS "Entering directory src...")
print(STATUS "    Adding benchmark target...")

add_executable(benchmark ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(benchmark stdc++fs malloc_count rt dl)