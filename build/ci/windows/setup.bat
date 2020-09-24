ECHO "Setup Windows build environment"

:: Install tools
where /q git
IF ERRORLEVEL 1 ( choco install -y git.install )

where /q wget
IF ERRORLEVEL 1 ( choco install -y wget )

where /q 7z
IF ERRORLEVEL 1 ( choco install -y wget )

:: Set temp dir
SET TEMP_DIR="c:\TEMP\musescore"
MKDIR %TEMP_DIR%

:: Install Qt
SET "Qt_ARCHIVE=qt599_msvc2017_64.7z"
SET "QT_URL=https://s3.amazonaws.com/utils.musescore.org/%Qt_ARCHIVE%"
SET "QT_DIR=C:\Qt\5.9.9"

CALL "wget.exe" -q --show-progress --no-check-certificate "%QT_URL%" -O "%TEMP_DIR%\%Qt_ARCHIVE%"
CALL "7z" x -y "%TEMP_DIR%\%Qt_ARCHIVE%" "-o%QT_DIR%"

SET PATH=%QT_DIR%\msvc2019_64\bin;%PATH% 

:: Install dependency
CALL "wget.exe" -q --show-progress --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/musescore_dependencies_win32.7z" -O %TEMP_DIR%\musescore_dependencies_win32.7z
CALL "7z" x -y %TEMP_DIR%\musescore_dependencies_win32.7z "-o%TEMP_DIR%\musescore_dependencies_win32"
SET JACK_DIR="C:\Program Files (x86)\Jack"
XCOPY %TEMP_DIR%\musescore_dependencies_win32\dependencies\Jack %JACK_DIR% /E /I /Y

SET PATH=%JACK_DIR%;%PATH% 

CALL "wget.exe" -q --show-progress --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/dependencies.7z" -O  %TEMP_DIR%\dependencies.7z
CALL "7z" x -y %TEMP_DIR%\dependencies.7z "-oC:\musescore_dependencies"

:: breakpad_tools
CALL "wget.exe" --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/dump_syms.7z" -O %TEMP_DIR%\dump_syms.7z
CALL "7z" x -y %TEMP_DIR%\dump_syms.7z "-oC:\breakpad_tools"

xcopy msdia140.dll %systemroot%\system32
:: regsvr32 %systemroot%\system32\msdia140.dll

:: Clean
RMDIR /Q /S "C:\TEMP"

ECHO "Setup script done"