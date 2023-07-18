@echo off

REM Default build is 64-bit
REM 32-bit compilation is available using "32" as a second parameter when you run msvc_build.bat
REM How to use:
REM BUILD 64-bit:
REM    "msvc_build.bat debug" builds 64-bit Debug version of MuseScore without optimizations
REM    "msvc_build.bat relwithdebinfo" builds optimized 64-bit version of MuseScore with almost all debug symbols
REM    "msvc_build.bat release" builds fully optimized 64-bit version of MuseScore without command line output
REM
REM BUILD 32-bit:
REM    "msvc_build.bat debug 32" builds 32-bit Debug version of MuseScore
REM    "msvc_build.bat relwithdebinfo 32" builds 32-bit RelWithDebInfo version of MuseScore
REM    "msvc_build.bat release 32" builds 32-bit Release version of MuseScore
REM
REM INSTALL 64-bit:
REM    "msvc_build.bat install" put all required files of 64-bit Release build to install folder (msvc.install_x64)
REM    "msvc_build.bat installdebug" put all required files of 64-bit Debug build to install folder (msvc.install_x64)
REM    "msvc_build.bat installrelwithdebinfo" put all required files of 64-bit RelWithDebInfo build to install folder (msvc.install_x64)
REM
REM INSTALL 32-bit:
REM    "msvc_build.bat install 32" put all required files of 32-bit Release build to install folder (msvc.install_x86)
REM    "msvc_build.bat installdebug 32" put all required files of 32-bit Debug build to install folder (msvc.install_x86)
REM    "msvc_build.bat installrelwithdebinfo 32" put all required files of 32-bit RelWithDebInfo build to install folder (msvc.install_x86)
REM
REM PACKAGE:
REM    "msvc_build.bat package" pack the installer for already built and installed 64-bit Release build (msvc.build_x64/MuseScore-*.msi)
REM    "msvc_build.bat package 32" pack the installer for already built and installed 32-bit Release build (msvc.build_x86/MuseScore-*.msi)
REM
REM CLEAN:
REM    "msvc_build.bat clean" remove all files in msvc.* folders and the folders itself
REM
REM Windows Portable build is triggered by defining BUILD_WIN_PORTABLE environment variable to "ON" before launching this script, e.g.
REM SET BUILD_WIN_PORTABLE=ON

SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

IF "%GENERATOR_NAME%"=="" (
   CALL :FIND_GENERATOR
)

IF "%GENERATOR_NAME%"=="" (
   ECHO "No supported version of Microsoft Visual Studio (2017, 2019 or 2022) found."
   GOTO :END
)

REM BUILD_64 used in CMakeLists.txt
SET "BUILD_FOLDER=msvc.build"
SET "INSTALL_FOLDER=msvc.install"

IF "%2"=="32" (
    SET PLATFORM_NAME=Win32
    SET "ARCH=x86"
    SET BUILD_64=OFF
) ELSE (
    IF NOT "%2"=="" (
        IF NOT "%2"=="64" (
            echo Invalid second argument
            GOTO :END
        ) ELSE (
            SET PLATFORM_NAME=x64
            SET "ARCH=x64"
            SET BUILD_64=ON
        )
    ) ELSE (
        SET PLATFORM_NAME=x64
        SET "ARCH=x64"
        SET BUILD_64=ON
    )
)

IF NOT "%3"=="" (
   SET BUILD_NUMBER="%3"
   SET BUILD_AUTOUPDATE="ON"
)

ECHO "BUILD_WIN_PORTABLE: %BUILD_WIN_PORTABLE%"
IF "%BUILD_WIN_PORTABLE%"=="ON" (
    SET "INSTALL_FOLDER=MuseScorePortable\App\MuseScore"
    SET "BUILD_AUTOUPDATE=OFF"
    SET "WIN_PORTABLE_OPT=-DMUSESCORE_BUILD_CONFIGURATION="app-portable"
)

ECHO "INSTALL_FOLDER: %INSTALL_FOLDER%"
ECHO "WIN_PORTABLE_OPT: %WIN_PORTABLE_OPT%"

IF /I "%1"=="release" (
   SET CONFIGURATION_STR="release"
   GOTO :BUILD
)

IF /I "%1"=="debug" (
   SET CONFIGURATION_STR="debug"
   GOTO :BUILD
)

IF /I "%1"=="relwithdebinfo" (
   SET CONFIGURATION_STR="relwithdebinfo"
   GOTO :BUILD
)

IF /I "%1"=="install" (
   SET "BUILD_FOLDER=%BUILD_FOLDER%_%ARCH%"
   SET CONFIGURATION_STR="release"
   GOTO :INSTALL
)

IF /I "%1"=="installdebug" (
   SET "BUILD_FOLDER=%BUILD_FOLDER%_%ARCH%"
   SET CONFIGURATION_STR="debug"
   GOTO :INSTALL
)

IF /I "%1"=="installrelwithdebinfo" (
   SET "BUILD_FOLDER=%BUILD_FOLDER%_%ARCH%"
   SET CONFIGURATION_STR="relwithdebinfo"
   GOTO :INSTALL
)

IF /I "%1"=="package" (
   cd "%BUILD_FOLDER%_%ARCH%"
   cmake --build . --config RelWithDebInfo --target package
   GOTO :END
)

IF /I "%1"=="revision" (
   echo revisionStep
   git rev-parse --short=7 HEAD > local_build_revision.env
   GOTO :END
)

IF /I "%1"=="clean" (
   for /d %%G in ("msvc.*") do rd /s /q "%%~G"
   for /d %%G in ("MuseScorePortable") do rd /s /q "%%~G"
   GOTO :END
) ELSE (
   echo No valid parameters are set
   GOTO :END
)

:FIND_GENERATOR

   REM Usage: CALL :FIND_GENERATOR
   REM Detects the highest supported VS version installed and sets GENERATOR_NAME to the appropriate CMake generator name.

   REM vswhere.exe is a helper utility that is automatically installed with VS2017 and later (and always at a fixed location).
   SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
   IF NOT EXIST %VSWHERE% EXIT /B !ERRORLEVEL!

   REM Try Visual Studio 2022 first.
   CALL :FIND_GENERATOR_VERSION %VSWHERE% 17 2022

   REM Fall back to Visual Studio 2019.
   IF "%GENERATOR_NAME%"=="" CALL :FIND_GENERATOR_VERSION %VSWHERE% 16 2019

   EXIT /B !ERRORLEVEL!

:FIND_GENERATOR_VERSION

   REM Usage: CALL :FIND_GENERATOR_VERSION "[path\]vswhere.exe" major_version_number year
   REM Checks if the specified VS version is installed, and if so, sets GENERATOR_NAME to the appropriate CMake generator name.

   FOR /F "usebackq delims=. tokens=1" %%I IN (`%1 -version [%2^,^) -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion -format value`) DO (
      IF "%%I"=="%2" SET GENERATOR_NAME=Visual Studio %2 %3
   )

   EXIT /B !ERRORLEVEL!

:BUILD
   echo Generator is: %GENERATOR_NAME%
   echo Platform is: %PLATFORM_NAME%
   SET "BUILD_FOLDER=%BUILD_FOLDER%_%ARCH%"
   echo Build folder is: %BUILD_FOLDER%
   IF NOT "%BUILD_WIN_PORTABLE%"=="ON" (
      SET "INSTALL_FOLDER=%INSTALL_FOLDER%_%ARCH%"
   )
   echo Install folder is: %INSTALL_FOLDER%
   if not exist "%BUILD_FOLDER%\" mkdir "%BUILD_FOLDER%"
   if not exist "%INSTALL_FOLDER%\" mkdir "%INSTALL_FOLDER%"

IF NOT "%MSCORE_STABLE_BUILD%" == "" (
    IF NOT "%CRASH_LOG_SERVER_URL%" == "" (
        SET CRASH_REPORT_URL_OPT=-DMUE_CRASH_REPORT_URL=%CRASH_LOG_SERVER_URL% -DBUILD_CRASH_REPORTER=ON
    )

    IF NOT "%YOUTUBE_API_KEY%" == "" (
        SET YOUTUBE_API_KEY_OPT=-DMUE_LEARN_YOUTUBE_API_KEY=%YOUTUBE_API_KEY%
    )
)

IF "%MUSESCORE_BUILD_MODE%" == "" (
    SET MUSESCORE_BUILD_MODE="dev"
)

IF NOT "%BUILD_VST%" == "" (
    SET BUILD_VST_OPT=-DBUILD_VST_MODULE=%BUILD_VST%
    SET VST3_SDK_PATH_OPT=-DVST3_SDK_PATH=%VST3_SDK_PATH%
)

SET "INSTALL_FOLDER=%INSTALL_FOLDER:\=/%"
REM -DCMAKE_BUILD_NUMBER=%BUILD_NUMBER% -DCMAKE_BUILD_AUTOUPDATE=%BUILD_AUTOUPDATE% %CRASH_REPORT_URL_OPT% are used for CI only
   cd "%BUILD_FOLDER%"
   IF EXIST "CMakeCache.txt" (
      echo Using existing CMake configuration to save time.
      echo To force reconfiguration, delete CMakeCache.txt or run "msvc_build.bat clean".
      echo You only need to do this if you want to use different build options to before.
      REM Note: If a CMakeLists.txt file was edited then the build system will detect it
      REM and run CMake again automatically with the same options as before.
   ) ELSE (
      echo Building CMake configuration...
      cmake -G "%GENERATOR_NAME%" -A "%PLATFORM_NAME%" -DCMAKE_INSTALL_PREFIX=../%INSTALL_FOLDER% -DCMAKE_BUILD_TYPE=%CONFIGURATION_STR% -DMUSESCORE_BUILD_MODE=%MUSESCORE_BUILD_MODE% -DMUSESCORE_REVISION=%MUSESCORE_REVISION% -DMUE_COMPILE_BUILD_64=%BUILD_64% -DCMAKE_BUILD_NUMBER=%BUILD_NUMBER% -DBUILD_AUTOUPDATE=%BUILD_AUTOUPDATE% %BUILD_VST_OPT% %VST3_SDK_PATH_OPT% %CRASH_REPORT_URL_OPT% %YOUTUBE_API_KEY_OPT% %WIN_PORTABLE_OPT% ..
      IF !ERRORLEVEL! NEQ 0 (
         set OLD_ERRORLEVEL=!ERRORLEVEL!
         del /f "CMakeCache.txt"
         exit /b !OLD_ERRORLEVEL!
      )
   )
   echo Building MuseScore...
   cmake --build . --config %CONFIGURATION_STR% --target mscore
   GOTO :END

:INSTALL
   cd "%BUILD_FOLDER%"
   echo Installing MuseScore files...
   cmake --build . --config %CONFIGURATION_STR% --target install
   GOTO :END

:END
exit /b !ERRORLEVEL!
