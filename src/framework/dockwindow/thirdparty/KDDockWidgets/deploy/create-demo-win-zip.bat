@echo off
rem
rem This file is part of KDDockWidgets.
rem
rem SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
rem
rem SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
rem
rem Contact KDAB at <info@kdab.com> for commercial licensing options.
rem

rem assumes the builddir is the workdir
rem assumes Qt is in PATH
rem %1 is the name of the deployment

if [%1] == [] (
  echo "Usage: %~nx0 <deploymentName>"
  goto theEnd
)

rem Prepare the deployDir
set "deploy=%1"
if exist %deploy% (
  rmdir /Q /S %deploy%
)
mkdir %deploy%

rem Deploy programs
for %%s in (bin\*.dll bin\*_example.exe) do (
    copy %%s %deploy% >nul
)

rem Deploy Qt
windeployqt --compiler-runtime %deploy%

rem Zip it
set zipFile=%cd%\%deploy%.7z
if exist %zipFile% (
  del /F /Q %zipFile%
)
7z a %zipFile% %deploy%

rem Must echo this line exactly for the CI
echo Created zip file "%zipFile%" successfully

:theEnd
exit /b %ERRORLEVEL%
