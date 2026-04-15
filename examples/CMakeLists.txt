#
# examples/CMakeLists.txt
#

add_custom_target(examples)

# 1)
add_executable(basic basic/main.cpp)
target_link_libraries(basic PRIVATE http_server)
add_dependencies(examples basic)
