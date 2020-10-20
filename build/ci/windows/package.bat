@echo off
ECHO "MuseScore package"

SET ARTIFACTS_DIR=build.artifacts

SET BUILD_MODE=""
SET TARGET_PROCESSOR_BITS=64
SET TARGET_PROCESSOR_ARCH=x86_64
SET BUILD_DIR=msvc.build_x64
SET INSTALL_DIR=msvc.install_x64
SET SIGN_CERTIFICATE_ENCRYPT_SECRET=""
SET SIGN_CERTIFICATE_PASSWORD=""
SET BUILD_WIN_PORTABLE=OFF
SET UPGRADE_UUID="11111111-1111-1111-1111-111111111111"

:GETOPTS
IF /I "%1" == "-m" SET BUILD_MODE=%2& SHIFT
IF /I "%1" == "-b" SET TARGET_PROCESSOR_BITS=%2& SHIFT
IF /I "%1" == "--signsecret" SET SIGN_CERTIFICATE_ENCRYPT_SECRET=%2& SHIFT
IF /I "%1" == "--signpass" SET SIGN_CERTIFICATE_PASSWORD=%2& SHIFT
IF /I "%1" == "--portable" SET BUILD_WIN_PORTABLE=%2& SHIFT
IF /I "%1" == "--guid" SET UPGRADE_UUID=%2& SHIFT
SHIFT
IF NOT "%1" == "" GOTO GETOPTS

: Try get from env
IF %BUILD_MODE% == "" ( SET /p BUILD_MODE=<%ARTIFACTS_DIR%\env\build_mode.env)

: Check args
IF %BUILD_MODE% == "" ( ECHO "error: not set BUILD_MODE" & GOTO END_ERROR)
IF NOT %TARGET_PROCESSOR_BITS% == 64 (
    IF NOT %TARGET_PROCESSOR_BITS% == 32 (
        ECHO "error: not set TARGET_PROCESSOR_BITS, must be 32 or 64, current TARGET_PROCESSOR_BITS: %TARGET_PROCESSOR_BITS%"
        GOTO END_ERROR
    )
)

IF %TARGET_PROCESSOR_BITS% == 32 ( 
    SET TARGET_PROCESSOR_ARCH=x86
    SET BUILD_DIR=msvc.build_x86
    SET INSTALL_DIR=msvc.install_x86
)  

IF %BUILD_WIN_PORTABLE% == ON (
    SET INSTALL_DIR=MuseScorePortable
)

:: Setup package type
IF %BUILD_WIN_PORTABLE% == ON    ( SET PACKAGE_TYPE="portable") ELSE (
IF %BUILD_MODE% == devel_build   ( SET PACKAGE_TYPE="7z") ELSE (
IF %BUILD_MODE% == nightly_build ( SET PACKAGE_TYPE="msi") ELSE (
IF %BUILD_MODE% == testing_build ( SET PACKAGE_TYPE="msi") ELSE (    
IF %BUILD_MODE% == stable_build  ( SET PACKAGE_TYPE="msi") ELSE ( 
    ECHO "Unknown BUILD_MODE: %BUILD_MODE%"
    GOTO END_ERROR
)))))

SET DO_SIGN=OFF
IF %PACKAGE_TYPE% == "msi" ( 
    SET DO_SIGN=ON
    IF %SIGN_CERTIFICATE_ENCRYPT_SECRET% == "" ( 
        SET DO_SIGN=OFF
        ECHO "warning: not set SIGN_CERTIFICATE_ENCRYPT_SECRET"
    )
    IF %SIGN_CERTIFICATE_PASSWORD% == "" ( 
        SET DO_SIGN=OFF
        ECHO "warning: not set SIGN_CERTIFICATE_PASSWORD"
    )
)

SET /p BUILD_VERSION=<%ARTIFACTS_DIR%\env\build_version.env
SET /p BUILD_DATETIME=<%ARTIFACTS_DIR%\env\build_datetime.env
SET /p BUILD_BRANCH=<%ARTIFACTS_DIR%\env\build_branch.env
SET /p BUILD_REVISION=<%ARTIFACTS_DIR%\env\build_revision.env

ECHO "BUILD_MODE: %BUILD_MODE%"
ECHO "BUILD_DATETIME: %BUILD_DATETIME%"
ECHO "BUILD_BRANCH: %BUILD_BRANCH%"
ECHO "BUILD_REVISION: %BUILD_REVISION%"
ECHO "BUILD_VERSION: %BUILD_VERSION%"
ECHO "TARGET_PROCESSOR_BITS: %TARGET_PROCESSOR_BITS%"
ECHO "TARGET_PROCESSOR_ARCH: %TARGET_PROCESSOR_ARCH%"
ECHO "BUILD_DIR: %BUILD_DIR%"
ECHO "INSTALL_DIR: %INSTALL_DIR%"
ECHO "PACKAGE_TYPE: %PACKAGE_TYPE%"

:: For MSI
SET SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe"
SET UUIDGEN="C:\Program Files (x86)\Windows Kits\10\bin\x64\uuidgen.exe"
SET WIX_DIR=%WIX%

IF %PACKAGE_TYPE% == "portable" ( GOTO PACK_PORTABLE) ELSE (
IF %PACKAGE_TYPE% == "7z" ( GOTO PACK_7z ) ELSE (
IF %PACKAGE_TYPE% == "msi" (  GOTO PACK_MSI ) ELSE (
    ECHO "Unknown package type: %PACKAGE_TYPE%"
    GOTO END_ERROR
)))

:: ============================
:: PACK_7z
:: ============================
:PACK_7z
ECHO "Start 7z packing..."
7z a MuseScore.7z %INSTALL_DIR%

SET ARTIFACT_NAME=MuseScore-%BUILD_VERSION%-%TARGET_PROCESSOR_ARCH%.7z
COPY MuseScore.7z %ARTIFACTS_DIR%\%ARTIFACT_NAME% /Y 
bash ./build/ci/tools/make_artifact_name_env.sh %ARTIFACT_NAME%
ECHO "Finished 7z packing"
GOTO END_SUCCESS

:: ============================
:: PACK_MSI
:: ============================
:PACK_MSI
ECHO "Start msi packing..."
:: sign dlls and exe files
IF %DO_SIGN% == ON (
    where /q secure-file
    IF ERRORLEVEL 1 ( choco install -y choco install -y --ignore-checksums secure-file )
    secure-file -decrypt build\ci\windows\resources\musescore.p12.enc -secret %SIGN_CERTIFICATE_ENCRYPT_SECRET%

    for /f "delims=" %%f in ('dir /a-d /b /s "%INSTALL_DIR%\*.dll" "%INSTALL_DIR%\*.exe"') do (
        ECHO "Signing %%f"
        %SIGNTOOL% sign /f "build\ci\windows\resources\musescore.p12" /t http://timestamp.verisign.com/scripts/timstamp.dll /p %SIGN_CERTIFICATE_PASSWORD% "%%f"
    )

) ELSE (
    ECHO "Sign disabled"
)

:: generate unique GUID
%UUIDGEN% > uuid.txt
SET /p PACKAGE_UUID=<uuid.txt
ECHO on
ECHO "PACKAGE_UUID: %PACKAGE_UUID%"
ECHO off
sed -i 's/00000000-0000-0000-0000-000000000000/%PACKAGE_UUID%/' build/Packaging.cmake
sed -i 's/11111111-1111-1111-1111-111111111111/%UPGRADE_UUID%/' build/Packaging.cmake

SET PACKAGE_FILE_ASSOCIATION=OFF
IF %BUILD_MODE% == stable_build ( 
    SET PACKAGE_FILE_ASSOCIATION=ON
)
cd "%BUILD_DIR%" 
cmake -DPACKAGE_FILE_ASSOCIATION=%PACKAGE_FILE_ASSOCIATION% ..
cd ..

SET PATH=%WIX_DIR%;%PATH% 
CALL msvc_build.bat package %TARGET_PROCESSOR_BITS%

ECHO "Create logs dir"
MKDIR %ARTIFACTS_DIR%\logs
MKDIR %ARTIFACTS_DIR%\logs\WIX

SET WIX_LOG_DIR=win64
IF %TARGET_PROCESSOR_BITS% == 32 ( SET WIX_LOG_DIR=win32 ) 

SET WIX_LOGS_PATH="%BUILD_DIR%\_CPack_Packages\%WIX_LOG_DIR%\WIX"
ECHO "Copy from %WIX_LOGS_PATH% to %ARTIFACTS_DIR%\logs\WIX"

ECHO .msi > excludedmsi.txt
XCOPY /Y /EXCLUDE:excludedmsi.txt %WIX_LOGS_PATH% %ARTIFACTS_DIR%\logs\WIX

:: find the MSI file without the hardcoded version
for /r %%i in (%BUILD_DIR%\*.msi) do (
    SET "FILEPATH=%%i"d
)

IF %BUILD_MODE% == nightly_build ( 
    SET ARTIFACT_NAME=MuseScoreNightly-%BUILD_DATETIME%-%BUILD_BRANCH%-%BUILD_REVISION%-%TARGET_PROCESSOR_ARCH%.msi
) ELSE (
    SET ARTIFACT_NAME=MuseScore-%BUILD_VERSION%-%TARGET_PROCESSOR_ARCH%.msi
)

ECHO "Copy from %FILEPATH% to %ARTIFACT_NAME%"

COPY %FILEPATH% %ARTIFACTS_DIR%\%ARTIFACT_NAME% /Y 
SET ARTIFACT_PATH=%ARTIFACTS_DIR%\%ARTIFACT_NAME%

IF %DO_SIGN% == ON (
    %SIGNTOOL% sign /debug /f "build\ci\windows\resources\musescore.p12" /t http://timestamp.verisign.com/scripts/timstamp.dll /p %SIGN_CERTIFICATE_PASSWORD% /d %ARTIFACT_NAME% %ARTIFACT_PATH%
    :: verify signature
    %SIGNTOOL% verify /pa %ARTIFACT_PATH%
)

bash ./build/ci/tools/make_artifact_name_env.sh %ARTIFACT_NAME%

:: DEBUG SYM
ECHO "Debug symbols generating.."
SET DEBUG_SYMS_FILE=musescore_win%TARGET_PROCESSOR_BITS%.sym
C:\breakpad_tools\dump_syms.exe %BUILD_DIR%\main\RelWithDebInfo\MuseScore3.pdb > %DEBUG_SYMS_FILE%
COPY %DEBUG_SYMS_FILE% %ARTIFACTS_DIR%\%DEBUG_SYMS_FILE% /Y 
ECHO "Finished debug symbols generating"

GOTO END_SUCCESS

:: ============================
:: PACK_PORTABLE
:: ============================
:PACK_PORTABLE
ECHO "Start portable packing..."

:: Create launcher
CALL C:\portableappslauncher\Launcher\PortableApps.comLauncherGenerator.exe %CD%\%INSTALL_DIR%
ECHO "Finished comLauncherGenerator"

:: Create Installer
CALL C:\portableappsinstaller\Installer\PortableApps.comInstaller.exe %CD%\%INSTALL_DIR%
ECHO "Finished comInstaller"

:: find the paf.exe file
for /r %%i in (.\*.paf.exe) do (
  SET "FILEPATH=%%i"
)

SET ARTIFACT_NAME=MuseScore-%BUILD_VERSION%-%TARGET_PROCESSOR_ARCH%.paf.exe

ECHO "Copy from %FILEPATH% to %ARTIFACT_NAME%"
COPY %FILEPATH% %ARTIFACTS_DIR%\%ARTIFACT_NAME% /Y 

bash ./build/ci/tools/make_artifact_name_env.sh %ARTIFACT_NAME%

ECHO "Finished portable packing"

GOTO END_SUCCESS

:: ============================
:: END
:: ============================

:END_SUCCESS
exit /b 0

:END_ERROR
exit /b 1
