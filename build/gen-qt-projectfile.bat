@echo off

set OLD_DIR=%CD%

echo TRANSLATIONS = \
echo       %1/share/locale/mscore_en.ts \
for /f %%a in ('type ..\build\locales') do echo       %1/share/locale/mscore_%%a.ts \
echo.

cd /d %1

echo FORMS = \
for /r %1 %%a in (*.ui) do echo     %%a \
echo.

echo SOURCES = \
for /r %1 %%a in (*.cpp) do echo     %%a \
echo.

echo.

cd /d %OLD_DIR%
