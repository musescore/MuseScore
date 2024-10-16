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

where /q wget
IF ERRORLEVEL 1 ( choco install -y wget )

where /q 7z
IF ERRORLEVEL 1 ( choco install -y 7zip.install )

:: Set temp dir
SET TEMP_DIR="c:\TEMP\musescore"
MKDIR %TEMP_DIR%

:: Install Qt
ECHO "=== Install Qt ==="

:: r2 - added websocket module
SET "Qt_ARCHIVE=Qt624_msvc2019_64_r2.7z"
SET "QT_DIR=C:\Qt\6.2.4"
SET "QT_URL=https://s3.amazonaws.com/utils.musescore.org/%Qt_ARCHIVE%"

CALL "wget.exe" -q --show-progress --no-check-certificate "%QT_URL%" -O "%TEMP_DIR%\%Qt_ARCHIVE%"
CALL "7z" x -y "%TEMP_DIR%\%Qt_ARCHIVE%" "-o%QT_DIR%"

:: Install dependencies
ECHO "=== Install dependencies ==="
CALL "wget.exe" -q --show-progress --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/musescore_dependencies_win32.7z" -O %TEMP_DIR%\musescore_dependencies_win32.7z
CALL "7z" x -y %TEMP_DIR%\musescore_dependencies_win32.7z "-o%TEMP_DIR%\musescore_dependencies_win32"
SET JACK_DIR="C:\Program Files (x86)\Jack"
XCOPY %TEMP_DIR%\musescore_dependencies_win32\dependencies\Jack %JACK_DIR% /E /I /Y
SET PATH=%JACK_DIR%;%PATH% 

CALL "wget.exe" -q --show-progress --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/dependencies.7z" -O  %TEMP_DIR%\dependencies.7z
CALL "7z" x -y %TEMP_DIR%\dependencies.7z "-oC:\musescore_dependencies"

CALL "wget.exe" -q --show-progress --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/VST3_SDK_379.7z" -O  %TEMP_DIR%\VST3_SDK.7z
CALL "7z" x -y %TEMP_DIR%\VST3_SDK.7z "-oC:\vst"

IF %BUILD_WIN_PORTABLE% == ON (
ECHO "=== Installing PortableApps.com Tools ==="
:: portableappslauncher is a vanilla installation of PortableApps.com Launcher https://portableapps.com/apps/development/portableapps.com_launcher
CALL "wget.exe" --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/portableappslauncher.zip" -O %TEMP_DIR%\portableappslauncher.zip
CALL "7z" x -y %TEMP_DIR%\portableappslauncher.zip "-oC:\portableappslauncher"

:: portableappslauncher is a vanilla installation of PortableApps.com Installer https://portableapps.com/apps/development/portableapps.com_launcher
CALL "wget.exe" --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/portableappsinstaller.zip" -O %TEMP_DIR%\portableappsinstaller.zip
CALL "7z" x -y %TEMP_DIR%\portableappsinstaller.zip "-oC:\portableappsinstaller"
)


:: Clean
ECHO "=== Clean ==="
RMDIR /Q /S "C:\TEMP"

ECHO "Setup script done"
