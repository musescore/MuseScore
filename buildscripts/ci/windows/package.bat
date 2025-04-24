@echo off
ECHO "MuseScore package"

SET ARTIFACTS_DIR=build.artifacts

SET BUILD_MODE=""
SET TARGET_PROCESSOR_BITS=64
SET TARGET_PROCESSOR_ARCH=x86_64
SET BUILD_DIR=build.release
SET INSTALL_DIR=build.install
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

:: Setup package type
IF %BUILD_WIN_PORTABLE% == ON    ( SET PACKAGE_TYPE="portable") ELSE (
IF %BUILD_MODE% == devel   ( SET PACKAGE_TYPE="7z") ELSE (
IF %BUILD_MODE% == nightly ( SET PACKAGE_TYPE="7z") ELSE (
IF %BUILD_MODE% == testing ( SET PACKAGE_TYPE="msi") ELSE (    
IF %BUILD_MODE% == stable  ( SET PACKAGE_TYPE="msi") ELSE ( 
    ECHO "Unknown BUILD_MODE: %BUILD_MODE%"
    GOTO END_ERROR
)))))

SET DO_SIGN=OFF
IF %PACKAGE_TYPE% == "msi" ( 
    SET DO_SIGN=ON
)
IF %PACKAGE_TYPE% == "portable" ( 
    IF %BUILD_MODE% == testing (
        SET DO_SIGN=ON
    )
    IF %BUILD_MODE% == stable (
        SET DO_SIGN=ON
    )
)
IF %DO_SIGN% == ON (
    IF %SIGN_CERTIFICATE_ENCRYPT_SECRET% == "" ( 
        SET DO_SIGN=OFF
        ECHO "warning: not set SIGN_CERTIFICATE_ENCRYPT_SECRET"
    )
    IF %SIGN_CERTIFICATE_PASSWORD% == "" ( 
        SET DO_SIGN=OFF
        ECHO "warning: not set SIGN_CERTIFICATE_PASSWORD"
    )

    ECHO "warning: at the moment sign is disabled"
    SET DO_SIGN=OFF
)

SET /p BUILD_VERSION=<%ARTIFACTS_DIR%\env\build_version.env
SET /p BUILD_NUMBER=<%ARTIFACTS_DIR%\env\build_number.env
SET /p BUILD_BRANCH=<%ARTIFACTS_DIR%\env\build_branch.env
SET /p BUILD_REVISION=<%ARTIFACTS_DIR%\env\build_revision.env

ECHO "BUILD_MODE: %BUILD_MODE%"
ECHO "BUILD_NUMBER: %BUILD_NUMBER%"
ECHO "BUILD_BRANCH: %BUILD_BRANCH%"
ECHO "BUILD_REVISION: %BUILD_REVISION%"
ECHO "BUILD_VERSION: %BUILD_VERSION%"
ECHO "TARGET_PROCESSOR_BITS: %TARGET_PROCESSOR_BITS%"
ECHO "TARGET_PROCESSOR_ARCH: %TARGET_PROCESSOR_ARCH%"
ECHO "BUILD_DIR: %BUILD_DIR%"
ECHO "INSTALL_DIR: %INSTALL_DIR%"
ECHO "PACKAGE_TYPE: %PACKAGE_TYPE%"

:: For MSI
SET SIGN="buildscripts\ci\windows\sign.bat"
SET UUIDGEN="C:\Program Files (x86)\Windows Kits\10\bin\10.0.20348.0\x64\uuidgen.exe"
SET WIX_DIR=%WIX%

IF %PACKAGE_TYPE% == "portable" ( GOTO PACK_PORTABLE) ELSE (
IF %PACKAGE_TYPE% == "7z" ( GOTO PACK_7z ) ELSE (
IF %PACKAGE_TYPE% == "msi" (  GOTO PACK_MSI ) ELSE (
IF %PACKAGE_TYPE% == "dir" (  GOTO PACK_DIR ) ELSE (    
    ECHO "Unknown package type: %PACKAGE_TYPE%"
    GOTO END_ERROR
))))

:: ============================
:: PACK_7z
:: ============================
:PACK_7z
ECHO "Start 7z packing..."
IF %BUILD_MODE% == nightly ( 
    SET ARTIFACT_NAME=MuseScore-Studio-Nightly-%BUILD_NUMBER%-%BUILD_BRANCH%-%BUILD_REVISION%-%TARGET_PROCESSOR_ARCH%
) ELSE (
    SET ARTIFACT_NAME=MuseScore-Studio-%BUILD_VERSION%-%TARGET_PROCESSOR_ARCH%
)

RENAME %INSTALL_DIR% %ARTIFACT_NAME%
7z a %ARTIFACTS_DIR%\%ARTIFACT_NAME%.7z %ARTIFACT_NAME%

bash ./buildscripts/ci/tools/make_artifact_name_env.sh %ARTIFACT_NAME%.7z
ECHO "Finished 7z packing"
GOTO END_SUCCESS

:: ============================
:: PACK_DIR
:: ============================
:PACK_DIR
ECHO "Start dir packing..."
MKDIR %ARTIFACTS_DIR%\MuseScore
XCOPY %INSTALL_DIR% %ARTIFACTS_DIR%\MuseScore /E /S /Y
ECHO "Finished dir packing"
GOTO END_SUCCESS

:: ============================
:: PACK_MSI
:: ============================
:PACK_MSI
ECHO "Start msi packing..."
:: sign dlls and exe files
IF %DO_SIGN% == ON (
    CALL %SIGN% --secret %SIGN_CERTIFICATE_ENCRYPT_SECRET% --pass %SIGN_CERTIFICATE_PASSWORD% --dir %INSTALL_DIR% || exit \b 1
) ELSE (
    ECHO "Sign disabled"
)

:: generate unique GUID
%UUIDGEN% > uuid.txt
SET /p PACKAGE_UUID=<uuid.txt
ECHO on
ECHO "PACKAGE_UUID: %PACKAGE_UUID%"
ECHO off

cd "%BUILD_DIR%" 
cmake -DCPACK_WIX_PRODUCT_GUID=%PACKAGE_UUID% ^
    -DCPACK_WIX_UPGRADE_GUID=%UPGRADE_UUID% ^
    ..

SET PATH=%WIX_DIR%;%PATH% 
cmake --build . --target package || SET WIX_ERROR=1
cd ..

ECHO "Create logs dir"
MKDIR %ARTIFACTS_DIR%\logs
MKDIR %ARTIFACTS_DIR%\logs\WIX

SET WIX_LOG_DIR=win64
IF %TARGET_PROCESSOR_BITS% == 32 ( SET WIX_LOG_DIR=win32 ) 

SET WIX_LOGS_PATH="%BUILD_DIR%\_CPack_Packages\%WIX_LOG_DIR%\WIX"
ECHO "Copy from %WIX_LOGS_PATH% to %ARTIFACTS_DIR%\logs\WIX"

ECHO .msi > excludedmsi.txt
XCOPY /Y /EXCLUDE:excludedmsi.txt %WIX_LOGS_PATH% %ARTIFACTS_DIR%\logs\WIX

IF DEFINED WIX_ERROR (
    GOTO END_ERROR
)

:: find the MSI file without the hardcoded version
for /r %%i in (%BUILD_DIR%\*.msi) do (
    SET "FILEPATH=%%i"d
)

IF %BUILD_MODE% == nightly ( 
    SET ARTIFACT_NAME=MuseScore-Studio-Nightly-%BUILD_NUMBER%-%BUILD_BRANCH%-%BUILD_REVISION%-%TARGET_PROCESSOR_ARCH%.msi
) ELSE (
    SET ARTIFACT_NAME=MuseScore-Studio-%BUILD_VERSION%-%TARGET_PROCESSOR_ARCH%.msi
)

ECHO "Copy from %FILEPATH% to %ARTIFACT_NAME%"

COPY %FILEPATH% %ARTIFACTS_DIR%\%ARTIFACT_NAME% /Y || GOTO END_ERROR
SET ARTIFACT_PATH=%ARTIFACTS_DIR%\%ARTIFACT_NAME%

IF %DO_SIGN% == ON (
    CALL %SIGN% --secret %SIGN_CERTIFICATE_ENCRYPT_SECRET% --pass %SIGN_CERTIFICATE_PASSWORD% --name %ARTIFACT_NAME% --file %ARTIFACT_PATH% || exit \b 1
)

bash ./buildscripts/ci/tools/make_artifact_name_env.sh %ARTIFACT_NAME%

GOTO END_SUCCESS

:: ============================
:: PACK_PORTABLE
:: ============================
:PACK_PORTABLE
ECHO "Start portable packing..."

:: sign dlls and exe files
IF %DO_SIGN% == ON (
    CALL %SIGN% --secret %SIGN_CERTIFICATE_ENCRYPT_SECRET% --pass %SIGN_CERTIFICATE_PASSWORD% --dir %INSTALL_DIR% || exit \b 1
) ELSE (
    ECHO "Sign disabled"
)

:: Create launcher
ECHO "Start comLauncherGenerator..."
CALL C:\portableappslauncher\Launcher\PortableApps.comLauncherGenerator.exe %CD%\%INSTALL_DIR%
ECHO "Finished comLauncherGenerator"

:: Create Installer
ECHO "Start comInstaller..."
CALL C:\portableappsinstaller\Installer\PortableApps.comInstaller.exe %CD%\%INSTALL_DIR%
ECHO "Finished comInstaller"

:: find the paf.exe file
for /r %%i in (.\*.paf.exe) do (
  SET "FILEPATH=%%i"
)

IF %BUILD_MODE% == nightly ( 
    SET ARTIFACT_NAME=MuseScore-Studio-Nightly-%BUILD_NUMBER%-%BUILD_BRANCH%-%BUILD_REVISION%-%TARGET_PROCESSOR_ARCH%.paf.exe
) ELSE (
    SET ARTIFACT_NAME=MuseScore-Studio-%BUILD_VERSION%-%TARGET_PROCESSOR_ARCH%.paf.exe
)

ECHO "Copy from %FILEPATH% to %ARTIFACT_NAME%"
COPY %FILEPATH% %ARTIFACTS_DIR%\%ARTIFACT_NAME% /Y 
SET ARTIFACT_PATH=%ARTIFACTS_DIR%\%ARTIFACT_NAME%

IF %DO_SIGN% == ON (
    CALL %SIGN% --secret %SIGN_CERTIFICATE_ENCRYPT_SECRET% --pass %SIGN_CERTIFICATE_PASSWORD% --name %ARTIFACT_NAME% --file %ARTIFACT_PATH% || exit \b 1
)

bash ./buildscripts/ci/tools/make_artifact_name_env.sh %ARTIFACT_NAME%

ECHO "Finished portable packing"

GOTO END_SUCCESS

:: ============================
:: END
:: ============================

:END_SUCCESS
exit /b 0

:END_ERROR
exit /b 1
