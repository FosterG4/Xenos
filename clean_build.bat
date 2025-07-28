@echo off
echo ========================================
echo Clean Build - Xenos with DLL Encryption
echo ========================================
echo.

echo Cleaning previous builds...
msbuild Xenos.sln /p:Configuration=Release /p:Platform=Win32 /t:Clean /verbosity:minimal
msbuild Xenos.sln /p:Configuration=Release /p:Platform=x64 /t:Clean /verbosity:minimal

if exist "build\Win32\Release" rmdir /s /q "build\Win32\Release"
if exist "build\x64\Release" rmdir /s /q "build\x64\Release"
if exist "obj\Win32\Release" rmdir /s /q "obj\Win32\Release"
if exist "obj\x64\Release" rmdir /s /q "obj\x64\Release"

echo Clean completed!
echo.

echo Building 32-bit version from scratch...
msbuild Xenos.sln /p:Configuration=Release /p:Platform=Win32 /verbosity:minimal
if %ERRORLEVEL% neq 0 (
    echo ERROR: 32-bit build failed!
    pause
    exit /b 1
)
echo 32-bit build completed successfully!
echo.

echo Building 64-bit version from scratch...
msbuild Xenos.sln /p:Configuration=Release /p:Platform=x64 /verbosity:minimal
if %ERRORLEVEL% neq 0 (
    echo ERROR: 64-bit build failed!
    pause
    exit /b 1
)
echo 64-bit build completed successfully!
echo.

echo ========================================
echo Clean Build Summary:
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

echo All builds completed from scratch!
echo Press any key to exit...
pause >nul 