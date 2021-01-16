@echo off
setlocal ENABLEDELAYEDEXPANSION
ECHO "MuseScore sign"

SET CERT_ENCRYPT_SECRET=""
SET CERT_PASSWORD=""
SET FILES_DIR=""
SET FILE_PATH=""
SET FILE_NAME=""

SET SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe"
SET SIGNCERT="build\ci\windows\resources\musescore.p12"
SET SIGNCERT_ENCRYPTED="build\ci\windows\resources\musescore.p12.enc"

SET TIMESERVER1="http://timestamp.globalsign.com/scripts/timstamp.dll"
SET TIMESERVER2="http://timestamp.sectigo.com"
SET TIMESERVER3="http://tsa.starfieldtech.com"

:GETOPTS
IF /I "%1" == "--dir" SET FILES_DIR=%2& SHIFT
IF /I "%1" == "--file" SET FILE_PATH=%2& SHIFT
IF /I "%1" == "--name" SET FILE_NAME=%2& SHIFT
IF /I "%1" == "--secret" SET CERT_ENCRYPT_SECRET=%2& SHIFT
IF /I "%1" == "--pass" SET CERT_PASSWORD=%2& SHIFT
SHIFT
IF NOT "%1" == "" GOTO GETOPTS

IF %FILES_DIR% == "" ( 
    SET MODE="file"
) ELSE (
    SET MODE="dir"
)

ECHO "MODE: %MODE%"
ECHO "FILES_DIR: %FILES_DIR%"
ECHO "FILE_PATH: %FILE_PATH%"
ECHO "FILE_NAME: %FILE_NAME%"

ECHO "Install secure-file"
WHERE /q secure-file
IF ERRORLEVEL 1 ( choco install -y --ignore-checksums secure-file )

ECHO "Decrypt certificate"
secure-file -decrypt %SIGNCERT_ENCRYPTED% -secret %CERT_ENCRYPT_SECRET%

:: Sign files in dir
IF %MODE% == "dir" (
    ECHO "Sign files in dir: %FILES_DIR%"
    FOR /f "delims=" %%f in ('dir /a-d /b /s "%FILES_DIR%\*.dll" "%FILES_DIR%\*.exe"') do (
        ECHO.
        ECHO "======================="
        ECHO "Signing %%f"
        %SIGNTOOL% sign /f %SIGNCERT% /t %TIMESERVER1% /p %CERT_PASSWORD% "%%f"
        IF ERRORLEVEL 1 (
            ECHO "signtool return error level: %ERRORLEVEL%, try use TIMESERVER2"
            CALL %SIGNTOOL% sign /f %SIGNCERT% /t %TIMESERVER2% /p %CERT_PASSWORD% "%%f"
        )
        IF ERRORLEVEL 1 (
            ECHO "signtool return error level: %ERRORLEVEL%, try use TIMESERVER3"
            %SIGNTOOL% sign /f %SIGNCERT% /t %TIMESERVER3% /p %CERT_PASSWORD% "%%f"
        )
        IF ERRORLEVEL 1 (
            ECHO "signtool return error level: %ERRORLEVEL%, sign failed"
            exit /b 1 
        )
    )
)

:: Sign file
IF %MODE% == "file" (
    ECHO.
    ECHO "=======================" 
    ECHO "Sign file: %FILE_PATH%"
    %SIGNTOOL% sign /debug /f %SIGNCERT% /t %TIMESERVER1% /p %CERT_PASSWORD% /d %FILE_NAME% %FILE_PATH%
    IF ERRORLEVEL 1 (
        ECHO "signtool return error level: %ERRORLEVEL%, try use TIMESERVER2"
        %SIGNTOOL% sign /debug /f %SIGNCERT% /t %TIMESERVER2% /p %CERT_PASSWORD% /d %FILE_NAME% %FILE_PATH%
    )
    IF ERRORLEVEL 1 (
        ECHO "signtool return error level: %ERRORLEVEL%, try use TIMESERVER3"
        %SIGNTOOL% sign /debug /f %SIGNCERT% /t %TIMESERVER3% /p %CERT_PASSWORD% /d %FILE_NAME% %FILE_PATH%
    )
    IF ERRORLEVEL 1 (
        ECHO "signtool return error level: %ERRORLEVEL%, sign failed"
        exit /b 1 
    )

    :: verify signature
    %SIGNTOOL% verify /pa %FILE_PATH%
    IF ERRORLEVEL 1 (
        ECHO "failed verify signature, error level: %ERRORLEVEL%"
        exit /b 1 
    )
)


