@echo off
ECHO "Setup Windows build environment"

SET TARGET_PROCESSOR_BITS=64
SET BUILD_WIN_PORTABLE=OFF

:GETOPTS
IF /I "%1" == "-b" SET TARGET_PROCESSOR_BITS=%2 & SHIFT
IF /I "%1" == "--portable" SET BUILD_WIN_PORTABLE=%2 & SHIFT
SHIFT
IF NOT "%1" == "" GOTO GETOPTS

IF NOT %TARGET_PROCESSOR_BITS% == 64 (
    IF NOT %TARGET_PROCESSOR_BITS% == 32 (
        ECHO "Error: TARGET_PROCESSOR_BITS not set, must be 32 or 64, current TARGET_PROCESSOR_BITS: %TARGET_PROCESSOR_BITS%"
        EXIT /b 1
    )
)

IF %TARGET_PROCESSOR_BITS% == 32 (
    ECHO "Error: Build 32-bit not available"
    EXIT /b 1
)

:: Install tools
where /q git
IF ERRORLEVEL 1 ( choco install -y git.install )

where /q 7z
IF ERRORLEVEL 1 ( choco install -y 7zip.install )

:: Set temp dir
SET TEMP_DIR="c:\TEMP\musescore"
MKDIR %TEMP_DIR%

IF %BUILD_WIN_PORTABLE% == ON (
ECHO "=== Installing PortableApps.com Tools ==="
:: portableappslauncher is a vanilla installation of PortableApps.com Launcher https://portableapps.com/apps/development/portableapps.com_launcher
CALL "curl.exe" -f -o %TEMP_DIR%\portableappslauncher.zip "https://s3.amazonaws.com/utils.musescore.org/portableappslauncher.zip"
CALL "7z" x -y %TEMP_DIR%\portableappslauncher.zip "-oC:\portableappslauncher"

:: portableappsinstaller is a vanilla installation of PortableApps.com Installer https://portableapps.com/apps/development/portableapps.com_installer
CALL "curl.exe" -f -o %TEMP_DIR%\portableappsinstaller.zip "https://s3.amazonaws.com/utils.musescore.org/portableappsinstaller.zip"
CALL "7z" x -y %TEMP_DIR%\portableappsinstaller.zip "-oC:\portableappsinstaller"
)


:: Clean
ECHO "=== Clean ==="
RMDIR /Q /S "C:\TEMP"

ECHO "Setup script done"
