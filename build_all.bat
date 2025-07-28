@echo off
echo ========================================
echo Building Xenos with DLL Encryption
echo ========================================
echo.

echo Building 32-bit version...
msbuild Xenos.sln /p:Configuration=Release /p:Platform=Win32 /verbosity:minimal
if %ERRORLEVEL% neq 0 (
    echo ERROR: 32-bit build failed!
    pause
    exit /b 1
)
echo 32-bit build completed successfully!
echo.

echo Building 64-bit version...
msbuild Xenos.sln /p:Configuration=Release /p:Platform=x64 /verbosity:minimal
if %ERRORLEVEL% neq 0 (
    echo ERROR: 64-bit build failed!
    pause
    exit /b 1
)
echo 64-bit build completed successfully!
echo.

echo ========================================
echo Build Summary:
echo ========================================
if exist "build\Win32\Release\Xenos.exe" (
    echo [✓] 32-bit: build\Win32\Release\Xenos.exe
    for %%I in ("build\Win32\Release\Xenos.exe") do echo     Size: %%~zI bytes
) else (
    echo [✗] 32-bit build missing!
)

if exist "build\x64\Release\Xenos64.exe" (
    echo [✓] 64-bit: build\x64\Release\Xenos64.exe  
    for %%I in ("build\x64\Release\Xenos64.exe") do echo     Size: %%~zI bytes
) else (
    echo [✗] 64-bit build missing!
)
echo.

echo Features included:
echo - DLL Encryption (XOR-based, no password required)
echo - Manual and automatic injection modes
echo - BlackBone library integration
echo - Both 32-bit and 64-bit support
echo.
echo Build completed! Press any key to exit...
pause >nul 