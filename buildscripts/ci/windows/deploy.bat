REM @echo off
ECHO "MuseScore deploy"

SET ARTIFACTS_DIR=build.artifacts
SET INSTALL_DIR=build.install

:: Remove unnecessary files installed by KDDockWidgets and Opus
RMDIR /Q /S %INSTALL_DIR%\include || EXIT /b 1
RMDIR /Q /S %INSTALL_DIR%\lib || EXIT /b 1

:: Add Qt DLLs, plugins, translations, and QML files
windeployqt --verbose 2 ^
    --qmldir . ^
    --dir %INSTALL_DIR% ^
    --release ^
    %INSTALL_DIR%/bin/MuseScore4.exe ^
    || EXIT /b 1
