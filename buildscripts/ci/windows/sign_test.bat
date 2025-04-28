@echo off
ECHO MuseScore sign test

SET SIGN_SECRET=""
SET SIGN_KEY=""
SET FILE_PATH=""
SET SIGN="buildscripts\ci\windows\sign.bat"
SET TEST_FILE="tools\codestyle\scan_files\bin\windows\scan_files.exe"

:GETOPTS
IF /I "%1" == "--file" SET FILE_PATH=%2& SHIFT
IF /I "%1" == "--secret" SET SIGN_SECRET=%2& SHIFT
IF /I "%1" == "--key" SET SIGN_KEY=%2& SHIFT
SHIFT
IF NOT "%1" == "" GOTO GETOPTS


CALL %SIGN% --secret %SIGN_SECRET% --key %SIGN_KEY% --file %TEST_FILE%