::mingw32-make -f Makefile.mingw release BUILD_NUMBER=%APPVEYOR_BUILD_NUMBER%
::mingw32-make -f Makefile.mingw install BUILD_NUMBER=%APPVEYOR_BUILD_NUMBER%

call C:\MuseScore\msvc_build.bat release %APPVEYOR_BUILD_NUMBER%
call C:\MuseScore\msvc_build.bat install %APPVEYOR_BUILD_NUMBER%
 