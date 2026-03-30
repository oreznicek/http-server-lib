#
# Top-level CMakeLists.txt
#

cmake_minimum_required (VERSION 3.20)
project("http_server")

# Generate compile_commands.json for clangd/NVIM
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard globally
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------
# Define the Core Static Library
# ---------------------------------------------------------

# Collect all source files in the ./src directory
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# Create the static library
add_library(http_server STATIC ${SOURCES})

# Attach include directories PUBLICLY.
# Anything that links to this library gets this include folder automatically.
target_include_directories(http_server PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Apply compiler warnings
target_compile_options(http_server PRIVATE -Wall -Wextra -Wpedantic)

# ---------------------------------------------------------
# Setup Testing
# ---------------------------------------------------------

enable_testing()

# This MUST be at the end, so the tests can see 'http_server_core'
add_subdirectory(tests)
