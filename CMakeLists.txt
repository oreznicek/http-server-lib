#
# This is the cmake project file
#

cmake_minimum_required (VERSION 3.20)

project("http_server")

set(MY_EXE "main")

# Specify the path to the include folder
include_directories(${CMAKE_SOURCE_DIR}/include)

# Collect all source files in the ./src directory
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(${MY_EXE} ${SOURCES})

set_property(TARGET ${MY_EXE} PROPERTY CXX_STANDARD 23)

# Set compiler to output all warnings
target_compile_options(${MY_EXE} PRIVATE
  -Wall -Wextra -Wpedantic
)
