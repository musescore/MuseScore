mingw32-make -f Makefile.mingw release BUILD_NUMBER=%APPVEYOR_BUILD_NUMBER%
mingw32-make -f Makefile.mingw install BUILD_NUMBER=%APPVEYOR_BUILD_NUMBER%
