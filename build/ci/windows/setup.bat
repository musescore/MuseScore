ECHO "Setup Windows build environment"

SET TARGET_PROCESSOR_BITS=64

:GETOPTS
IF /I "%1" == "-b" SET TARGET_PROCESSOR_BITS=%2 & SHIFT
SHIFT
IF NOT "%1" == "" GOTO GETOPTS

IF NOT %TARGET_PROCESSOR_BITS% == 64 (
    IF NOT %TARGET_PROCESSOR_BITS% == 32 (
        ECHO "error: not set TARGET_PROCESSOR_BITS, must be 32 or 64, current TARGET_PROCESSOR_BITS: %TARGET_PROCESSOR_BITS%"
        EXIT /b 1
    )
)

:: Install tools
where /q git
IF ERRORLEVEL 1 ( choco install -y git.install )

where /q wget
IF ERRORLEVEL 1 ( choco install -y wget )

where /q 7z
IF ERRORLEVEL 1 ( choco install -y 7z )

:: Set temp dir
SET TEMP_DIR="c:\TEMP\musescore"
MKDIR %TEMP_DIR%

:: Install Qt
:: Default for x64
SET "Qt_ARCHIVE=qt599_msvc2017_64.7z"

IF %TARGET_PROCESSOR_BITS% == 32 (
    SET "Qt_ARCHIVE=qt599_msvc2015.7z"
)

SET "QT_URL=https://s3.amazonaws.com/utils.musescore.org/%Qt_ARCHIVE%"
SET "QT_DIR=C:\Qt\5.9.9"

CALL "wget.exe" -q --show-progress --no-check-certificate "%QT_URL%" -O "%TEMP_DIR%\%Qt_ARCHIVE%"
CALL "7z" x -y "%TEMP_DIR%\%Qt_ARCHIVE%" "-o%QT_DIR%"

:: Install dependency
CALL "wget.exe" -q --show-progress --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/musescore_dependencies_win32.7z" -O %TEMP_DIR%\musescore_dependencies_win32.7z
CALL "7z" x -y %TEMP_DIR%\musescore_dependencies_win32.7z "-o%TEMP_DIR%\musescore_dependencies_win32"
SET JACK_DIR="C:\Program Files (x86)\Jack"
XCOPY %TEMP_DIR%\musescore_dependencies_win32\dependencies\Jack %JACK_DIR% /E /I /Y

SET PATH=%JACK_DIR%;%PATH% 

CALL "wget.exe" -q --show-progress --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/dependencies.7z" -O  %TEMP_DIR%\dependencies.7z
CALL "7z" x -y %TEMP_DIR%\dependencies.7z "-oC:\musescore_dependencies"

:: breakpad_tools
CALL "wget.exe" --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/dump_syms_32.7z" -O %TEMP_DIR%\dump_syms_32.7z
CALL "7z" x -y %TEMP_DIR%\dump_syms_32.7z "-oC:\breakpad_tools"

:: Clean
RMDIR /Q /S "C:\TEMP"

ECHO "Setup script done"