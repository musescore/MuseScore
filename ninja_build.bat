@echo off

ECHO "Setup VS Environment"
SET VC_TOOLS_COMPONENT=Microsoft.VisualStudio.Component.VC.Tools.x86.x64
SET VCVARS=vcvars64.bat
IF /I "%PROCESSOR_ARCHITECTURE%" == "ARM64" (
  SET VC_TOOLS_COMPONENT=Microsoft.VisualStudio.Component.VC.Tools.ARM64
  SET VCVARS=vcvarsarm64.bat
)
SET VSWHERE="C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires %VC_TOOLS_COMPONENT% -property installationPath`) do (
  SET VS_INSTALL_DIR=%%i
)
ECHO "VS_INSTALL_DIR: %VS_INSTALL_DIR%"
CALL "%VS_INSTALL_DIR%\VC\Auxiliary\Build\%VCVARS%"

bash ./ninja_build.sh %*