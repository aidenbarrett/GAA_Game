@echo off
setlocal ENABLEDELAYEDEXPANSION

set PROJECT_ROOT=%~dp0..
pushd %PROJECT_ROOT%

set PRESET=windows-release
set PACKAGE=false

for %%I in (%*) do (
    if /I "%%~I"=="--gui" set PRESET=windows-gui-release
    if /I "%%~I"=="--package" set PACKAGE=true
)

cmake --preset %PRESET%
if errorlevel 1 goto :end

cmake --build --preset %PRESET%
if errorlevel 1 goto :end

if /I "%PACKAGE%"=="true" (
    cmake --build --preset %PRESET% --target package
)

:end
popd
endlocal
