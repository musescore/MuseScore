@echo off
setlocal EnableDelayedExpansion

rem  Please only use this file to do things that absolutely cannot be done within build.cmake,
rem  such as CALL a batch file to set environment variables in the current process. This is not
rem  the same as executing a batch file, which would set environment variables in a subprocess.

echo "Setup VS Environment"
set VC_TOOLS_COMPONENT=Microsoft.VisualStudio.Component.VC.Tools.x86.x64
set VCVARS=vcvars64.bat
if /I "%PROCESSOR_ARCHITECTURE%" == "ARM64" (
    set VC_TOOLS_COMPONENT=Microsoft.VisualStudio.Component.VC.Tools.ARM64
    set VCVARS=vcvarsarm64.bat
)
set VSWHERE="C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires %VC_TOOLS_COMPONENT% -property installationPath`) do (
    set VS_INSTALL_DIR=%%i
)
echo "VS_INSTALL_DIR: %VS_INSTALL_DIR%"
echo "PATH: %PATH%"

CALL "%VS_INSTALL_DIR%\VC\Auxiliary\Build\%VCVARS%"

cmake %*
