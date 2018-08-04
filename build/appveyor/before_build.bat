:: keep full PATH for later
SET OLD_PATH=%PATH%
mkdir archive
cd archive
:: download dependencies if necessary
IF NOT EXIST dependencies.7z ( START " " /wait "C:\cygwin64\bin\wget.exe" --no-check-certificate "http://utils.musescore.org.s3.amazonaws.com/musescore_dependencies_win32.7z" -O dependencies.7z )
:: copy dependencies
START " " /wait "7z" x -y dependencies.7z > nul
CD dependencies
XCOPY Jack "C:\Program Files (x86)\Jack" /E /I /Y
XCOPY ccache "C:\ccache" /E /I /Y

CD C:\MuseScore
mkdir dependencies
cd dependencies
IF NOT EXIST dependencies.zip ( START " " /wait "C:\cygwin64\bin\wget.exe" --no-check-certificate "https://musescore.org/sites/musescore.org/files/2018-05/dependencies.zip" -O dependencies.zip )
START " " /wait "7z" x -y dependencies.zip > nul
CD include
CD C:\MuseScore

:: is MuseScore stable? Check here, no grep in PATH later on
for /f "delims=" %%i in ('grep "^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)" C:\MuseScore\CMakeLists.txt') do set UNSTABLE=%%i

:: add stable keys for musescore.com
IF "%UNSTABLE%" == "" (
python build/add-mc-keys.py %MC_CONSUMER_KEY% %MC_CONSUMER_SECRET%
)

:: get revision number
SET PATH=C:\Qt\5.9\msvc2015\bin;%PATH%
call C:\MuseScore\msvc_build.bat revision
::git rev-parse --short=7 HEAD > mscore/revision.h
SET /p MSversion=<mscore\revision.h

echo on
echo MSVersion:
echo %MSversion%
echo off

@echo on
echo MSVersion:
echo %MSversion%
@echo off

:: CMake refuses to generate MinGW Makefiles if sh.exe is in the PATH (C:\Program Files\Git\usr\bin)
SET PATH=C:\Qt\5.9\msvc2015\bin;C:\Program Files (x86)\CMake\bin;C:\Program Files\7-Zip;C:\ccache\bin;C:\Tools\curl\bin;%WIX%\bin;C:\Windows\system32;C:\Windows

:: set ccache dir
SET CCACHE_DIR=C:\ccache\cache
