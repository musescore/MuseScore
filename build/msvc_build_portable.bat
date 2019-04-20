@echo off

REM CLEAN:
REM    "msvc_build.bat clean" remove all files in msvc.build* and MuseScorePortable folders and the folders themselves

REM BUILD_64 and BUILD_FOR_WINSTORE are used in CMakeLists.txt
SET BUILD_FOR_WINSTORE=OFF
SET BUILD_64=OFF

SET BUILD_FOLDER=msvc.build
SET INSTALL_FOLDER=MuseScorePortable\App\MuseScore
SET ARCH=x86
SET GENERATOR_NAME="Visual Studio 15 2017"
SET BUILD_AUTOUPDATE="OFF"

IF NOT "%3"=="" (
   SET BUILD_NUMBER="%3"
   )

IF /I "%1"=="release" (
   SET CONFIGURATION_STR="release"
   GOTO :BUILD
   )

IF /I "%1"=="relwithdebinfo" (
   SET CONFIGURATION_STR="relwithdebinfo"
   GOTO :BUILD
   )

IF /I "%1"=="install" (
   SET BUILD_FOLDER=%BUILD_FOLDER%_%ARCH%
   SET CONFIGURATION_STR="release"
   GOTO :INSTALL
   )

IF /I "%1"=="installrelwithdebinfo" (
   SET BUILD_FOLDER=%BUILD_FOLDER%_%ARCH%
   SET CONFIGURATION_STR="relwithdebinfo"
   GOTO :INSTALL
   )

IF /I "%1"=="revision" (
   echo revisionStep
   git rev-parse --short=7 HEAD > mscore/revision.h
   GOTO :END
   )

IF /I "%1"=="clean" (
   for /d %%G in ("msvc.build*") do rd /s /q "%%~G"
   for /d %%G in ("MuseScorePortable") do rd /s /q "%%~G"
   GOTO :END
   ) ELSE (
@echo on
   echo "No valid parameters are set"
@echo off
   GOTO :END
   )

:BUILD
   SET BUILD_FOLDER=%BUILD_FOLDER%_%ARCH%
@echo on
   echo "Build folder is: %BUILD_FOLDER%"
   echo "Install folder is: %INSTALL_FOLDER%"
@echo off
   if not exist %BUILD_FOLDER%\nul mkdir %BUILD_FOLDER%
   if not exist %INSTALL_FOLDER%\nul mkdir %INSTALL_FOLDER%
@echo on
   echo "Building CMake configuration..."
@echo off

IF NOT "%CRASH_LOG_SERVER_URL%" == "" (
    SET CRASH_REPORT_URL_OPT=-DCRASH_REPORT_URL=%CRASH_LOG_SERVER_URL%
    )

set "INSTALL_FOLDER=%INSTALL_FOLDER:\=/%"

REM -DCMAKE_BUILD_NUMBER=%BUILD_NUMBER% -DCMAKE_BUILD_AUTOUPDATE=%BUILD_AUTOUPDATE% %CRASH_REPORT_URL_OPT% are used for CI only
   cd %BUILD_FOLDER% & cmake -G %GENERATOR_NAME% -DCMAKE_INSTALL_PREFIX=../%INSTALL_FOLDER% -DCMAKE_BUILD_TYPE=%CONFIGURATION_STR% -DBUILD_FOR_WINSTORE=%BUILD_FOR_WINSTORE% -DBUILD_64=%BUILD_64% -DCMAKE_BUILD_NUMBER=%BUILD_NUMBER% -DBUILD_AUTOUPDATE=%BUILD_AUTOUPDATE% %CRASH_REPORT_URL_OPT% -DBUILD_PORTABLEAPPS=%BUILD_WIN_PORTABLE% ..

@echo on
   echo "Running lrelease..."
@echo off
   cmake --build . --target lrelease
@echo on
   echo "Building MuseScore..."
@echo off
   cd %BUILD_FOLDER% & cmake --build . --config %CONFIGURATION_STR% --target mscore
   GOTO :END
   )

:INSTALL
   cd %BUILD_FOLDER%
@echo on
   echo "Installing MuseScore files..."
@echo off
   cmake --build . --config %CONFIGURATION_STR% --target install
   GOTO :END

:END