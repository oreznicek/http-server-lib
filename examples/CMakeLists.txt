#
# examples/CMakeLists.txt
#

add_custom_target(examples)

file(GLOB EXAMPLE_MAINS CONFIGURE_DEPENDS "*/main.cpp")

foreach(MAIN_CPP IN LISTS EXAMPLE_MAINS)
    get_filename_component(EXAMPLE_DIR "${MAIN_CPP}" DIRECTORY)
    get_filename_component(TARGET_NAME "${EXAMPLE_DIR}" NAME)
    add_executable(${TARGET_NAME} "${MAIN_CPP}")
    target_link_libraries(${TARGET_NAME} PRIVATE http_server)
    add_dependencies(examples ${TARGET_NAME})
endforeach()
