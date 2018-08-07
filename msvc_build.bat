SET BUILD_FOR_WINSTORE="OFF"

IF NOT "%2"=="" (
   SET BUILD_NUMBER=%2
   )

IF "%1"=="release" (
   echo releaseStep
   cd C:\MuseScore
   if not exist msvc.build.release\nul mkdir msvc.build.release
   if not exist msvc.install\nul mkdir msvc.install
   cd msvc.build.release & cmake -G "Visual Studio 15 2017" -DCMAKE_INSTALL_PREFIX=../msvc.install -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_FOR_WINSTORE=%BUILD_FOR_WINSTORE ..
   cmake --build . --target lrelease
   cd msvc.build.release & cmake --build . --config release --target mscore
   )

IF "%1"=="debug" (
   cd C:\MuseScore
   if not exist msvc.build.debug\nul mkdir msvc.build.debug
   if not exist msvc.install\nul mkdir msvc.install
   cd msvc.build.debug & cmake -G "Visual Studio 15 2017" -DCMAKE_INSTALL_PREFIX=../msvc.install -DCMAKE_BUILD_TYPE=DEBUG -DBUILD_FOR_WINSTORE=%BUILD_FOR_WINSTORE ..
   cmake --build . --target lrelease
   cd msvc.build.debug & cmake --build . --config debug --target mscore
   )

IF "%1"=="install" (
   echo InstallStep
   cd C:\MuseScore
   cd msvc.build.release
   cmake --build . --config release --target install
   )

IF "%1"=="package" (
   echo packageStep
   cd C:\MuseScore
   cd msvc.build.release
   cmake --build . --target package
   )

IF "%1"=="revision" (
   echo revisionStep
   git rev-parse --short=7 HEAD > mscore/revision.h
   )

IF "%1"=="clean" (
   -rmdir /S/Q msvc.build.debug msvc.build.release msvc.install
   )
   