@echo off
setlocal

cd /d "%~dp0"

if "%VCPKG_ROOT%"=="" (
    echo ERROR: VCPKG_ROOT is not defined.
    echo Set VCPKG_ROOT to your vcpkg folder first.
    exit /b 1
)

if exist build rmdir /s /q build

cmake -S . -B build ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"

if errorlevel 1 exit /b %errorlevel%

cmake --build build --config Release
if errorlevel 1 exit /b %errorlevel%

if not exist "dist\SKSE\Plugins" mkdir "dist\SKSE\Plugins"

if exist "build\Release\DisableWaitRuntime.dll" (
    copy /Y "build\Release\DisableWaitRuntime.dll" "dist\SKSE\Plugins\DisableWaitRuntime.dll" >nul
) else if exist "build\DisableWaitRuntime.dll" (
    copy /Y "build\DisableWaitRuntime.dll" "dist\SKSE\Plugins\DisableWaitRuntime.dll" >nul
) else (
    echo ERROR: DisableWaitRuntime.dll was not found after build.
    exit /b 1
)

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "if (Test-Path 'dist\Disable-Wait-Runtime-v0.2.0.zip') { Remove-Item 'dist\Disable-Wait-Runtime-v0.2.0.zip' }; Compress-Archive -Path 'dist\SKSE' -DestinationPath 'dist\Disable-Wait-Runtime-v0.2.0.zip'"

echo.
echo Build complete:
echo   dist\Disable-Wait-Runtime-v0.2.0.zip