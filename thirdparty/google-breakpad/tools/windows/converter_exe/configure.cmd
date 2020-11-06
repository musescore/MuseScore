@if "%ECHOON%"=="" @echo off
SETLOCAL

REM ******************************************************************
REM Please, make sure to run this in an Elevated Command Prompt.
REM Usage:
REM        configure.cmd
REM ******************************************************************

REM ******************************************************************
REM Initialize
REM ******************************************************************
SET SCRIPT_LOCATION=%~dp0

REM ******************************************************************
REM Go to script location
REM ******************************************************************
pushd %SCRIPT_LOCATION%

REM ******************************************************************
REM Register msdia140.dll.
REM ******************************************************************
SET MSG=Failed to register msdia140.dll.  Make sure to run this in elevated command prompt.
%systemroot%\SysWoW64\regsvr32.exe /s msdia140.dll & if errorlevel 1 echo %MSG% & goto :fail

:success
echo Configuration was successful.
ENDLOCAL
exit /b 0

:fail
ENDLOCAL
exit /b 1
