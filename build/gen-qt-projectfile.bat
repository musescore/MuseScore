@echo off

set OLD_DIR=%CD%

echo TRANSLATIONS = \
for /r %1/share/locale/ %%a in (*.ts) do echo     %%a \
echo.

cd /d %1

echo FORMS = \
for /r %1 %%a in (*.ui) do echo     %%a \
echo.

echo SOURCES = \
for /r %1 %%a in (*.cpp) do echo     %%a \
for /r %1/share/instruments %%a in (*.h) do echo     %%a \
echo.
echo.

cd /d %OLD_DIR%
