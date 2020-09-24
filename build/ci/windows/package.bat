ECHO "MuseScore package"

SET BUILD_DIR=msvc.build_x64
SET INSTALL_DIR=msvc.install_x64
SET ARTIFACTS_DIR=build.artifacts
SET TARGET_PROCESSOR_BITS=64
SET TARGET_PROCESSOR_ARCH=x86_64

SET RELEASE_CHANNEL=""
SET SIGN_CERTIFICATE_ENCRYPT_SECRET=""
SET SIGN_CERTIFICATE_PASSWORD=""

:GETOPTS
IF /I "%1" == "-c" SET RELEASE_CHANNEL=%2 & SHIFT
IF /I "%1" == "--signsecret" SET SIGN_CERTIFICATE_ENCRYPT_SECRET=%2 & SHIFT
IF /I "%1" == "--signpass" SET SIGN_CERTIFICATE_PASSWORD=%2 & SHIFT
SHIFT
IF NOT "%1" == "" GOTO GETOPTS

: Try get from env
IF %RELEASE_CHANNEL% == "" ( SET /p RELEASE_CHANNEL=<%ARTIFACTS_DIR%\env\release_channel.env)

: Check args
IF %RELEASE_CHANNEL% == "" ( ECHO "error: not set RELEASE_CHANNEL" & GOTO END_ERROR)

:: Setup package type
IF %RELEASE_CHANNEL% == stable  ( SET PACKAGE_TYPE="msi" ) ELSE (
IF %RELEASE_CHANNEL% == testing ( SET PACKAGE_TYPE="msi" ) ELSE (
IF %RELEASE_CHANNEL% == devel   ( SET PACKAGE_TYPE="msi" ) ELSE ( 
    ECHO "Unknown RELEASE_CHANNEL: %RELEASE_CHANNEL%"
    GOTO END_ERROR
)))

IF %PACKAGE_TYPE% == "msi" ( 
    IF %SIGN_CERTIFICATE_ENCRYPT_SECRET% == "" ( ECHO "error: not set SIGN_CERTIFICATE_ENCRYPT_SECRET" & GOTO END_ERROR)
    IF %SIGN_CERTIFICATE_PASSWORD% == "" ( ECHO "error: not set SIGN_CERTIFICATE_PASSWORD" & GOTO END_ERROR)
)


ECHO "RELEASE_CHANNEL: %RELEASE_CHANNEL%"
ECHO "PACKAGE_TYPE: %PACKAGE_TYPE%"


SET PUBLISH_SERVER_URL=https://ftp.osuosl.org/pub/musescore-nightlies/windows/

:: For MSI
SET SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe"
SET UUIDGEN="C:\Program Files (x86)\Windows Kits\10\bin\x64\uuidgen.exe"
SET WIX_DIR=%WIX%

IF %PACKAGE_TYPE% == "7z" ( GOTO PACK_7z ) ELSE (
IF %PACKAGE_TYPE% == "msi" (  GOTO PACK_MSI ) ELSE (
    ECHO "Unknown package type: %PACKAGE_TYPE%"
    GOTO END_ERROR
)
)


:PACK_7z
ECHO "Start 7z packing..."
COPY mscore\revision.h %INSTALL_DIR%\revision.h
7z a MuseScore_x86-64.7z %INSTALL_DIR%
ECHO "Finished 7z packing"
goto END_SUCCESS

:PACK_MSI

:: sign dlls and exe files
where /q secure-file
IF ERRORLEVEL 1 ( choco install -y choco install -y --ignore-checksums secure-file )
secure-file -decrypt build\ci\windows\resources\musescore.p12.enc -secret %SIGN_CERTIFICATE_ENCRYPT_SECRET%

for /f "delims=" %%f in ('dir /a-d /b /s "%INSTALL_DIR%\*.dll" "%INSTALL_DIR%\*.exe"') do (
    ECHO "Signing %%f"
    %SIGNTOOL% sign /f "build\ci\windows\resources\musescore.p12" /t http://timestamp.verisign.com/scripts/timstamp.dll /p %SIGN_CERTIFICATE_PASSWORD% "%%f"
)

:: generate unique GUID
%UUIDGEN% > uuid.txt
SET /p PACKAGE_UUID=<uuid.txt
ECHO on
ECHO "PACKAGE_UUID: %PACKAGE_UUID%"
ECHO off
sed -i 's/00000000-0000-0000-0000-000000000000/%PACKAGE_UUID%/' build/Packaging.cmake

SET PATH=%WIX_DIR%;%PATH% 
CALL msvc_build.bat package %TARGET_PROCESSOR_BITS%

:: find the MSI file without the hardcoded version
for /r %%i in (%BUILD_DIR%\*.msi) do (
  SET "FILEPATH=%%i"d
  )


SET /p BUILD_VERSION=<%ARTIFACTS_DIR%\env\build_version.env
SET ARTIFACT_NAME=MuseScore-%BUILD_VERSION%-%TARGET_PROCESSOR_ARCH%.msi

COPY %FILEPATH% %ARTIFACTS_DIR%\%ARTIFACT_NAME% /Y 
SET ARTIFACT_PATH=%ARTIFACTS_DIR%\%ARTIFACT_NAME%

%SIGNTOOL% sign /debug /f "build\ci\windows\resources\musescore.p12" /t http://timestamp.verisign.com/scripts/timstamp.dll /p %SIGN_CERTIFICATE_PASSWORD% /d %ARTIFACT_NAME% %ARTIFACT_PATH%
:: verify signature
%SIGNTOOL% verify /pa %ARTIFACT_PATH%

bash ./build/ci/tools/make_artifact_name_env.sh %ARTIFACT_NAME%
bash ./build/ci/tools/make_publish_url_env.sh -p windows -a %ARTIFACT_NAME%

SET /p PUBLISH_URL=<%ARTIFACTS_DIR%\env\publish_url.env

bash ./build/ci/tools/sparkle_appcast_gen.sh -p windows -u %PUBLISH_URL%

:: DEBUG SYM

SET DEBUG_SYMS_FILE=musescore_win%TARGET_PROCESSOR_BITS%.sym
REM Add one of the directories containing msdia140.dll (x86 version), for dump_syms.exe
::SET PATH="C:\Program Files (x86)\Microsoft Visual Studio 14.0\DIA SDK\bin";%PATH%
C:\breakpad_tools\dump_syms.exe %BUILD_DIR%\main\RelWithDebInfo\MuseScore3.pdb > %DEBUG_SYMS_FILE%
COPY %DEBUG_SYMS_FILE% %ARTIFACTS_DIR%\%DEBUG_SYMS_FILE% /Y 

GOTO END_SUCCESS

:: ============================
:: END
:: ============================

:END_SUCCESS
exit /b 0

:END_ERROR
exit /b 1