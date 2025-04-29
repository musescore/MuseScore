@echo off
setlocal ENABLEDELAYEDEXPANSION
ECHO "MuseScore sign"

SET SIGN_SECRET=""
SET SIGN_KEY=""
SET FILE_PATH=""

:GETOPTS
IF /I "%1" == "--file" SET FILE_PATH=%2& SHIFT
IF /I "%1" == "--secret" SET SIGN_SECRET=%2& SHIFT
IF /I "%1" == "--key" SET SIGN_KEY=%2& SHIFT
SHIFT
IF NOT "%1" == "" GOTO GETOPTS

ECHO "Try sign FILE_PATH: %FILE_PATH%"

bash ./buildscripts/ci/windows/sign_service_aws.sh --s3_key %SIGN_KEY% --s3_secret %SIGN_SECRET% --file_path %FILE_PATH%