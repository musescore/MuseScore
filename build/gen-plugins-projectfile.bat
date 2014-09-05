@echo off

set OLD_DIR=%CD%

echo TRANSLATIONS = \
for /f %%a in ('type ..\build\locales') do echo       %1/translations/locale_%%a.ts \
echo.

cd /d %1

echo SOURCES = \
for /r %1 %%a in (*.qml *.js) do echo     %%a \
echo.

echo.

cd /d %OLD_DIR%
