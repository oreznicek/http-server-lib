#!/bin/bash
set -e 

echo "==============================================="
echo "========= Building HTTP Server Library ========"
echo "==============================================="

mkdir -p build
cd build

echo ">>>>>>>>>>>> Configuring with CMake <<<<<<<<<<<"
cmake .. -DCMAKE_BUILD_TYPE=Release

echo ">>>>>>>>>>>>>> Building Library <<<<<<<<<<<<<<<"
cmake --build . -j $(nproc)
echo ">>>>>>>>>>>>>>> Building Tests <<<<<<<<<<<<<<<<"
cmake --build tests -j $(nproc)
echo ">>>>>>>>>>>>>> Building Examples <<<<<<<<<<<<<<"
cmake --build examples -j $(nproc)

echo ">>>>>>>>>>>>>>>> Running Tests <<<<<<<<<<<<<<<<" 
ctest --output-on-failure

echo "=== Build and Tests Completed Successfully! ==="
