@echo off
setlocal ENABLEDELAYEDEXPANSION

set PROJECT_ROOT=%~dp0..
pushd %PROJECT_ROOT%

set PRESET=windows-release

for %%I in (%*) do (
    if /I "%%~I"=="--gui" set PRESET=windows-gui-release
)

set BUILD_DIR=%PROJECT_ROOT%\build\windows-release

if not exist "%BUILD_DIR%" (
    echo Build directory not found. Run build-windows.bat first.
    exit /b 1
)

cmake --build --preset %PRESET% --target package

set PACKAGE_DIR=%BUILD_DIR%
for %%f in ("%PACKAGE_DIR%\GaelicClash-*.exe") do (
    echo NSIS installer created: %%~nxf
)
for %%f in ("%PACKAGE_DIR%\GaelicClash-*.zip") do (
    echo Portable archive created: %%~nxf
)

popd
endlocal
