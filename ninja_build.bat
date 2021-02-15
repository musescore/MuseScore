@echo off

ECHO "Setup VS Environment"
SET VSWHERE="C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  SET VS_INSTALL_DIR=%%i
)
ECHO "VS_INSTALL_DIR: %VS_INSTALL_DIR%"
CALL "%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvars64.bat"

bash ./ninja_build.sh %*