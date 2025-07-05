:: Install tools
where /q wget
IF ERRORLEVEL 1 ( choco install -y wget )

where /q 7z
IF ERRORLEVEL 1 ( choco install -y 7zip.install )

:: Install Qt
SET "Qt_ARCHIVE=Qt-6.2.11-Windows-amd64.zip"
SET "QT_DIR=C:\Qt\6.2.11"
SET "QT_URL=https://github.com/cbjeukendrup/musescore_build_qt/releases/download/v12861016856/%Qt_ARCHIVE%"

CALL "wget.exe" -q --show-progress --no-check-certificate "%QT_URL%" -O "%RUNNER_TEMP%\%Qt_ARCHIVE%" || exit /b 1
CALL "7z" x -y "%RUNNER_TEMP%\%Qt_ARCHIVE%" "-o%QT_DIR%" || exit /b 1

ECHO %QT_DIR%\bin>>%GITHUB_PATH%

ECHO QT_ROOT_DIR=%QT_DIR%>>%GITHUB_ENV%
ECHO QT_PLUGIN_PATH=%QT_DIR%\plugins>>%GITHUB_ENV%
ECHO QML2_IMPORT_PATH=%QT_DIR%\qml>>%GITHUB_ENV%
