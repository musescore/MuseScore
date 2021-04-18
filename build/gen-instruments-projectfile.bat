@echo off

set OLD_DIR=%CD%

echo TRANSLATIONS = \
for /r %1/../../share/locale/ %%a in (instruments_*.ts) do echo     %%a \
echo.

cd /d %1

echo HEADERS= \
for /r %1 %%a in (*.h) do echo     %%a \
echo.
echo.

cd /d %OLD_DIR%
