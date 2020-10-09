@echo off
ECHO "Publish package"

SET ARTIFACTS_DIR="build.artifacts"

SET OSUOSL_SSH_ENCRYPT_SECRET=""
SET ARTIFACT_NAME=""

:GETOPTS
IF /I "%1" == "--secret" SET OSUOSL_SSH_ENCRYPT_SECRET=%2 & SHIFT
IF /I "%1" == "-a" SET ARTIFACT_NAME=%2 & SHIFT
SHIFT
IF NOT "%1" == "" GOTO GETOPTS


: Try get from env
IF %ARTIFACT_NAME% == "" ( SET /p ARTIFACT_NAME=<%ARTIFACTS_DIR%\env\artifact_name.env)

: Check args
IF %OSUOSL_SSH_ENCRYPT_SECRET% == "" ( ECHO "error: not set OSUOSL_SSH_ENCRYPT_SECRET" & EXIT /b 1 )
IF %ARTIFACT_NAME% == "" ( ECHO "error: not set ARTIFACT_NAME" & EXIT /b 1 )

ECHO "ARTIFACT_NAME: %ARTIFACT_NAME%"

7z x -y build\ci\tools\osuosl\osuosl_nighlies_rsa.enc -obuild\ci\tools\osuosl\ -p%OSUOSL_SSH_ENCRYPT_SECRET%

CD %ARTIFACTS_DIR%

IF NOT EXIST %ARTIFACT_NAME% (
    ECHO "Not exists artifact, name: %ARTIFACT_NAME%"
    EXIT /b 1
)

SET SSH_KEY=..\build\ci\tools\osuosl\osuosl_nighlies_rsa

Icacls %SSH_KEY%
Icacls %SSH_KEY% /Inheritance:r
Icacls %SSH_KEY% /Grant:r "%Username%":(R,W)
Icacls %SSH_KEY%

ECHO "Copy %ARTIFACT_NAME% to ftp/windows"
scp -oStrictHostKeyChecking=no -C -i %SSH_KEY% %ARTIFACT_NAME% musescore-nightlies@ftp-osl.osuosl.org:~/ftp/windows/

:: Delete old files
ECHO "Delete old MuseScoreNightly files"
ssh -oStrictHostKeyChecking=no -i %SSH_KEY% musescore-nightlies@ftp-osl.osuosl.org "cd ~/ftp/windows; ls MuseScoreNightly* -t | tail -n +41 | xargs rm -f"


CD ..
