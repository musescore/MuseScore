@echo off
ECHO "MuseScore build"

SET ARTIFACTS_DIR=build.artifacts
SET BUILD_NUMBER=""
SET TELEMETRY_TRACK_ID=""
SET CRASH_LOG_SERVER_URL=""
SET TARGET_PROCESSOR_BITS=64
SET BUILD_WIN_PORTABLE=OFF

:GETOPTS
IF /I "%1" == "-n" SET BUILD_NUMBER=%2& SHIFT
IF /I "%1" == "-b" SET TARGET_PROCESSOR_BITS=%2& SHIFT
IF /I "%1" == "--telemetry" SET TELEMETRY_TRACK_ID=%2& SHIFT
IF /I "%1" == "--crashurl" SET CRASH_LOG_SERVER_URL=%2& SHIFT
IF /I "%1" == "--portable" SET BUILD_WIN_PORTABLE=%2& SHIFT
SHIFT
IF NOT "%1" == "" GOTO GETOPTS

IF %BUILD_NUMBER% == "" ( ECHO "error: not set BUILD_NUMBER" & EXIT /b 1)
IF NOT %TARGET_PROCESSOR_BITS% == 64 (
    IF NOT %TARGET_PROCESSOR_BITS% == 32 (
        ECHO "error: not set TARGET_PROCESSOR_BITS, must be 32 or 64, current TARGET_PROCESSOR_BITS: %TARGET_PROCESSOR_BITS%"
        EXIT /b 1
    )
)

SET /p BUILD_MODE=<%ARTIFACTS_DIR%\env\build_mode.env
SET "MUSESCORE_BUILD_CONFIG=dev"
IF %BUILD_MODE% == devel_build   ( SET "MUSESCORE_BUILD_CONFIG=dev" ) ELSE (
IF %BUILD_MODE% == nightly_build ( SET "MUSESCORE_BUILD_CONFIG=dev" ) ELSE (
IF %BUILD_MODE% == testing_build ( SET "MUSESCORE_BUILD_CONFIG=testing" ) ELSE (
IF %BUILD_MODE% == stable_build  ( SET "MUSESCORE_BUILD_CONFIG=release" ) ELSE (
    ECHO "error: unknown BUILD_MODE: %BUILD_MODE%"
    EXIT /b 1
))))

ECHO "MUSESCORE_BUILD_CONFIG: %MUSESCORE_BUILD_CONFIG%"
ECHO "BUILD_NUMBER: %BUILD_NUMBER%"
ECHO "TARGET_PROCESSOR_BITS: %TARGET_PROCESSOR_BITS%"
ECHO "TELEMETRY_TRACK_ID: %TELEMETRY_TRACK_ID%"
ECHO "CRASH_LOG_SERVER_URL: %CRASH_LOG_SERVER_URL%"
ECHO "BUILD_WIN_PORTABLE: %BUILD_WIN_PORTABLE%"

XCOPY "C:\musescore_dependencies" %CD% /E /I /Y
ECHO "Finished copy dependencies"

SET GENERATOR_NAME=Visual Studio 16 2019
SET MSCORE_STABLE_BUILD="TRUE"

:: TODO We need define paths during image creation
SET "JACK_DIR=C:\Program Files (x86)\Jack"
SET "QT_DIR=C:\Qt\5.9.9"

IF %TARGET_PROCESSOR_BITS% == 32 ( 
    SET "PATH=%QT_DIR%\msvc2015\bin;%JACK_DIR%;%PATH%"
) ELSE (
    SET "PATH=%QT_DIR%\msvc2017_64\bin;%JACK_DIR%;%PATH%"
)

:: Undefined CRASH_LOG_SERVER_URL if is it empty
IF %CRASH_LOG_SERVER_URL% == "" ( SET CRASH_LOG_SERVER_URL=)
IF %CRASH_LOG_SERVER_URL% == "''" ( SET CRASH_LOG_SERVER_URL=)

CALL msvc_build.bat revision || exit \b 1
CALL msvc_build.bat relwithdebinfo %TARGET_PROCESSOR_BITS% %BUILD_NUMBER% || exit \b 1
CALL msvc_build.bat installrelwithdebinfo %TARGET_PROCESSOR_BITS% %BUILD_NUMBER% || exit \b 1


bash ./build/ci/tools/make_release_channel_env.sh 
bash ./build/ci/tools/make_version_env.sh %BUILD_NUMBER%
bash ./build/ci/tools/make_revision_env.sh
bash ./build/ci/tools/make_branch_env.sh
bash ./build/ci/tools/make_datetime_env.sh
