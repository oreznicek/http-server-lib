#
# tests/CMakeLists.txt
#

add_custom_target(tests)

add_library(test_main OBJECT test_main.cpp)

add_library(test_utils STATIC
    response_parser.cpp
    test_helpers.cpp
)
target_link_libraries(test_utils PRIVATE http_server)

file(GLOB TEST_SOURCES "*_test.cpp")

foreach(TEST_SOURCE IN LISTS TEST_SOURCES)
    # Extract the base name to use as the target name
    get_filename_component(TEST_NAME "${TEST_SOURCE}" NAME_WE)

    add_executable(${TEST_NAME} "${TEST_SOURCE}")
    target_link_libraries(${TEST_NAME} PRIVATE http_server test_main test_utils)
    add_dependencies(tests ${TEST_NAME})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
