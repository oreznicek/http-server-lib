@echo off

echo ===============================================
echo ========= Building HTTP Server Library ========
echo ===============================================

if not exist build mkdir build
cd build

echo ^>^>^>^>^>^>^>^>^>^>^>^> Configuring with CMake ^<^<^<^<^<^<^<^<^<^<^<^<
cmake .. || exit /b

echo ^>^>^>^>^>^>^>^>^>^>^>^>^>^> Building Library ^<^<^<^<^<^<^<^<^<^<^<^<^<^<
cmake --build . --config Release -j %NUMBER_OF_PROCESSORS% || exit /b

echo ^>^>^>^>^>^>^>^>^>^>^>^>^>^> Building Tests ^<^<^<^<^<^<^<^<^<^<^<^<^<^<^<
cmake --build . --target tests --config Release -j %NUMBER_OF_PROCESSORS% || exit /b

echo ^>^>^>^>^>^>^>^>^>^>^>^>^> Building Examples ^<^<^<^<^<^<^<^<^<^<^<^<^<^<
cmake --build . --target examples --config Release -j %NUMBER_OF_PROCESSORS% || exit /b

echo ^>^>^>^>^>^>^>^>^>^>^>^>^>^>^> Running Tests ^<^<^<^<^<^<^<^<^<^<^<^<^<^<^<
ctest -C Release --output-on-failure || exit /b

echo === Build and Tests Completed Successfully! ===
pause
