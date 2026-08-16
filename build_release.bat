@echo off
setlocal

cd /d "%~dp0"

if not exist VERSION (
    echo ERROR: VERSION file is missing.
    exit /b 1
)

set /p VERSION=<VERSION
if "%VERSION%"=="" (
    echo ERROR: VERSION is empty.
    exit /b 1
)

if "%VCPKG_ROOT%"=="" (
    echo ERROR: VCPKG_ROOT is not defined.
    echo Set VCPKG_ROOT to your vcpkg folder first.
    exit /b 1
)

REM Clean previous build and package
if exist build rmdir /s /q build
if exist package rmdir /s /q package

REM Configure
cmake -S . -B build ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"

if errorlevel 1 exit /b %errorlevel%

REM Build
cmake --build build --config Release
if errorlevel 1 exit /b %errorlevel%

REM Prepare Vortex package
if not exist "package\SKSE\Plugins" mkdir "package\SKSE\Plugins"

if exist "build\Release\DisableWaitRuntime.dll" (
    copy /Y "build\Release\DisableWaitRuntime.dll" "package\SKSE\Plugins\DisableWaitRuntime.dll" >nul
) else if exist "build\DisableWaitRuntime.dll" (
    copy /Y "build\DisableWaitRuntime.dll" "package\SKSE\Plugins\DisableWaitRuntime.dll" >nul
) else (
    echo ERROR: DisableWaitRuntime.dll was not found after build.
    exit /b 1
)

REM Prepare distribution folder
if not exist "dist" mkdir "dist"

set "ZIP_NAME=Disable-Wait-Runtime-v%VERSION%.zip"
set "ZIP_PATH=dist\%ZIP_NAME%"

REM Create Vortex-ready ZIP
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "if (Test-Path '%ZIP_PATH%') { Remove-Item '%ZIP_PATH%' }; Compress-Archive -Path 'package\SKSE' -DestinationPath '%ZIP_PATH%'"

if errorlevel 1 exit /b %errorlevel%

echo.
echo Build complete: v%VERSION%
echo.
echo Package:
echo   package\SKSE\Plugins\DisableWaitRuntime.dll
echo.
echo Release:
echo   %ZIP_PATH%

endlocal
