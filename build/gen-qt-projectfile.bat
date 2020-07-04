@echo off

set OLD_DIR=%CD%

echo TRANSLATIONS = \
for /r %1/share/locale/ %%a in (mscore_*.ts) do echo     %%a \
echo.

cd /d %1

echo FORMS = \
for /r %1 %%a in (*.ui) do echo     %%a \
echo.

echo SOURCES = \
for /r %1 %%a in (*.cpp) do echo     %%a \
echo.
echo.

echo RESOURCES = \
echo     ./mu4/scenes/palette/palette.qrc \
echo     ./telemetry/telemetry_resources.qrc \
echo.

cd /d %OLD_DIR%
