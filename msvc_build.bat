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

REM BUILD_64 and BUILD_FOR_WINSTORE are used in CMakeLists.txt
SET BUILD_FOR_WINSTORE="OFF"
SET BUILD_64="ON"

SET "BUILD_FOLDER=msvc.build"
SET "INSTALL_FOLDER=msvc.install"
SET "ARCH=x64"
SET GENERATOR_NAME="Visual Studio 15 2017 Win64"

IF NOT "%2"=="" (
   IF "%2"=="32" (
       SET "ARCH=x86"
       SET GENERATOR_NAME="Visual Studio 15 2017"
       SET BUILD_64="OFF"
       )
   )

IF NOT "%3"=="" (
   SET BUILD_NUMBER="%3"
   SET BUILD_AUTOUPDATE="ON"
   )

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
   cmake --build . --config release --target package
   GOTO :END
   )

IF /I "%1"=="revision" (
   echo revisionStep
   git rev-parse --short=7 HEAD > mscore/revision.h
   GOTO :END
   )

IF /I "%1"=="clean" (
   for /d %%G in ("msvc.*") do rd /s /q "%%~G"
   GOTO :END
   ) ELSE (
   echo "No valid parameters are set"
   GOTO :END
   )

:BUILD
@echo off
   SET "BUILD_FOLDER=%BUILD_FOLDER%_%ARCH%"
   echo Build folder is: %BUILD_FOLDER%
   SET "INSTALL_FOLDER=%INSTALL_FOLDER%_%ARCH%"
   echo Install folder is: %INSTALL_FOLDER%
   if not exist "%BUILD_FOLDER%\nul" mkdir "%BUILD_FOLDER%"
   if not exist "%INSTALL_FOLDER%\nul" mkdir "%INSTALL_FOLDER%"
   echo Building CMake configuration...
REM -DCMAKE_BUILD_NUMBER=%BUILD_NUMBER% -DCMAKE_BUILD_AUTOUPDATE=%BUILD_AUTOUPDATE% are used for CI only
   cd "%BUILD_FOLDER%" & cmake -G %GENERATOR_NAME% -DCMAKE_INSTALL_PREFIX="../%INSTALL_FOLDER%" -DCMAKE_BUILD_TYPE=%CONFIGURATION_STR% -DBUILD_FOR_WINSTORE=%BUILD_FOR_WINSTORE% -DBUILD_64=%BUILD_64% -DCMAKE_BUILD_NUMBER=%BUILD_NUMBER% -DBUILD_AUTOUPDATE=%BUILD_AUTOUPDATE% ..
   echo Running lrelease...
   cmake --build . --target lrelease
   echo Building MuseScore...
   cd "%BUILD_FOLDER%" & cmake --build . --config %CONFIGURATION_STR% --target mscore
   GOTO :END
   )

:INSTALL
@echo off
   cd "%BUILD_FOLDER%"
   echo Installing MuseScore files...
   cmake --build . --config %CONFIGURATION_STR% --target install
   GOTO :END

:END