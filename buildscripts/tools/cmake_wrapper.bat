@echo off
setlocal EnableDelayedExpansion

rem  Please only use this file to do things that absolutely cannot be done within build.cmake,
rem  such as CALL a batch file to set environment variables in the current process. This is not
rem  the same as executing a batch file, which would set environment variables in a subprocess.

set "TARGET_PROCESSOR_BITS=64"
:loop
    rem  CMD splits arguments on equals sign, so -DFOO=BAR is treated as -DFOO and BAR
    if "%~1%~2" == "" goto continue
    if "%~1=%~2" == "-DMUSE_COMPILE_BUILD_64=OFF" set "TARGET_PROCESSOR_BITS=32"
    shift
    goto loop
:continue
echo "TARGET_PROCESSOR_BITS: %TARGET_PROCESSOR_BITS%"

echo "Setup VS Environment"
set VSWHERE="C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set VS_INSTALL_DIR=%%i
)
echo "VS_INSTALL_DIR: %VS_INSTALL_DIR%"
echo "PATH: %PATH%"

CALL "%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvars%TARGET_PROCESSOR_BITS%.bat"

cmake %*
