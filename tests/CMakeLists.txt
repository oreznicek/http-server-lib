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

# Build standalone test executable and link it with test_main
add_executable(parser_test parser_test.cpp)
target_link_libraries(parser_test PRIVATE http_server test_main test_utils)
add_dependencies(tests parser_test)
add_test(NAME http_request_parser COMMAND parser_test)
