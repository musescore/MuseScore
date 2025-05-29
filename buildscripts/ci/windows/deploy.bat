SET INSTALL_DIR=build.install
SET BUILD_WIN_PORTABLE=OFF

:GETOPTS
IF /I "%1" == "--portable" SET BUILD_WIN_PORTABLE=%2& SHIFT
SHIFT
IF NOT "%1" == "" GOTO GETOPTS

IF %BUILD_WIN_PORTABLE% == ON ( 
    SET INSTALL_DIR=build.install/App/MuseScore
)

:: Add Qt DLLs, plugins, translations, and QML files
windeployqt --verbose 2 ^
    --qmldir . ^
    --release ^
    %INSTALL_DIR%/bin/MuseScore4.exe ^
    || EXIT /b 1
