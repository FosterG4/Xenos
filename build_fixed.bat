 @echo off
echo ========================================
echo Building Xenos with DLL Encryption - FIXED
echo ========================================

REM Kill any running processes that might be using the DLL files
taskkill /f /im Xenos.exe 2>nul
taskkill /f /im Xenos64.exe 2>nul

REM Clean up any locked DLL files
del /f /q "build\Win32\Release\msdia140.dll" 2>nul
del /f /q "build\Win32\Release\symsrv.dll" 2>nul
del /f /q "build\x64\Release\msdia140.dll" 2>nul
del /f /q "build\x64\Release\symsrv.dll" 2>nul

echo.
echo Building 32-bit version...
msbuild Xenos.sln /p:Configuration=Release /p:Platform=Win32 /verbosity:minimal /p:BuildInParallel=false
if %ERRORLEVEL% neq 0 (
    echo ERROR: 32-bit build failed! Continuing with 64-bit...
) else (
    echo 32-bit build completed successfully!
)

echo.
echo Building 64-bit version...
msbuild Xenos.sln /p:Configuration=Release /p:Platform=x64 /verbosity:minimal /p:BuildInParallel=false
if %ERRORLEVEL% neq 0 (
    echo ERROR: 64-bit build failed! Trying individual project build...
    msbuild src\Xenos.vcxproj /p:Configuration=Release /p:Platform=x64 /verbosity:minimal
    if %ERRORLEVEL% neq 0 (
        echo ERROR: 64-bit build completely failed!
    ) else (
        echo 64-bit build completed successfully (individual project)!
    )
) else (
    echo 64-bit build completed successfully!
)

echo.
echo ========================================
echo Build Summary:
echo ========================================

REM Check for 32-bit executable
if exist "build\Win32\Release\Xenos.exe" (
    for %%A in ("build\Win32\Release\Xenos.exe") do (
        echo [OK] 32-bit: build\Win32\Release\Xenos.exe
        echo     Size: %%~zA bytes
    )
) else (
    echo [FAIL] 32-bit: build\Win32\Release\Xenos.exe - NOT FOUND
)

REM Check for 64-bit executable (try multiple possible locations)
if exist "build\x64\Release\Xenos64.exe" (
    for %%A in ("build\x64\Release\Xenos64.exe") do (
        echo [OK] 64-bit: build\x64\Release\Xenos64.exe
        echo     Size: %%~zA bytes
    )
) else if exist "src\build\x64\Release\Xenos64.exe" (
    for %%A in ("src\build\x64\Release\Xenos64.exe") do (
        echo [OK] 64-bit: src\build\x64\Release\Xenos64.exe
        echo     Size: %%~zA bytes
    )
) else (
    echo [FAIL] 64-bit: Xenos64.exe - NOT FOUND
)

echo.
echo Features included:
echo - DLL Encryption (XOR-based, no password required)
echo - Professional UI/UX (320x420) with proper footer spacing
echo - Manual and automatic injection modes
echo - BlackBone library integration
echo - Both 32-bit and 64-bit support
echo.
echo Build completed! Press any key to exit...
pause >nul 