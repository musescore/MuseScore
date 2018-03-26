:: download dependencies if necessary
IF NOT EXIST dependencies.7z ( START " " /wait "C:\MinGW\msys\1.0\bin\wget" --no-check-certificate "http://utils.musescore.org.s3.amazonaws.com/musescore_dependencies_win32.7z" -O dependencies.7z )

:: keep full PATH for later
SET OLD_PATH=%PATH%

:: copy dependencies
START " " /wait "7z" x -y dependencies.7z > nul
CD dependencies
XCOPY i686-w64-mingw32 "C:\Qt\Tools\mingw530_32\i686-w64-mingw32" /E /Y
XCOPY lib "C:\Qt\Tools\mingw530_32\lib" /E /Y
XCOPY Jack "C:\Program Files (x86)\Jack" /E /I /Y
XCOPY ccache "C:\ccache" /E /I /Y
CD C:\MuseScore

:: is MuseScore stable? Check here, no grep in PATH later on
for /f "delims=" %%i in ('grep "^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)" C:\MuseScore\CMakeLists.txt') do set UNSTABLE=%%i

:: add stable keys for musescore.com
IF "%UNSTABLE%" == "" (
python build/add-mc-keys.py %MC_CONSUMER_KEY% %MC_CONSUMER_SECRET%
)

:: get revision number
SET PATH=C:\Qt\5.9\mingw53_32\bin;C:\Qt\Tools\mingw530_32\bin;%PATH%
mingw32-make -f Makefile.mingw revision
SET /p MSversion=<mscore\revision.h

:: CMake refuses to generate MinGW Makefiles if sh.exe is in the PATH (C:\Program Files\Git\usr\bin)
SET PATH=C:\Qt\5.9\mingw53_32\bin;C:\Qt\Tools\mingw530_32\bin;C:\Program Files (x86)\CMake\bin;C:\Program Files\7-Zip;C:\ccache\bin;C:\Tools\curl\bin;%WIX%\bin;C:\Windows\system32;C:\Windows

:: set ccache dir
SET CCACHE_DIR=C:\ccache\cache
