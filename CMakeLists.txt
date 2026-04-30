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
# Global Windows / MSVC Settings
# ---------------------------------------------------------
if(MSVC)
    # Force MSVC to use the standard-compliant preprocessor GLOBALLY
    add_compile_options(/Zc:preprocessor)
endif()

if(WIN32)
    # Globally disable Microsoft's min/max macros to protect std::max
    add_compile_definitions(NOMINMAX)
endif()

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

if(WIN32)
    # Link the Windows Socket library
    target_link_libraries(http_server PRIVATE ws2_32)
endif()

# Apply compiler warnings
target_compile_options(http_server PRIVATE
    # If the compiler is GCC or Clang (Linux/Mac)
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
        -Wall -Wextra -Wpedantic
    >
    # If the compiler is MSVC (Windows / Visual Studio)
    $<$<CXX_COMPILER_ID:MSVC>:
        /W3
    >
)

# ---------------------------------------------------------
# Setup Testing
# ---------------------------------------------------------
enable_testing()
add_subdirectory(tests EXCLUDE_FROM_ALL)

# ---------------------------------------------------------
# Setup Examples
# ---------------------------------------------------------
add_subdirectory(examples EXCLUDE_FROM_ALL)
