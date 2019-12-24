:: set platform-dependent variables
IF "%PLATFORM%" == "x64" (
  SET "QTURL=https://utils.musescore.org.s3.amazonaws.com/qt598_msvc2017_64.7z"
  SET "QTDIR=%cd%\qt\msvc2017_64" & :: uncomment to use our Qt
  SET "QTCACHE=qt598_msvc2017_64.7z" & :: bump version here and .appveyor.yml to trigger cache rebuild when upgrading Qt
  :: SET "QTDIR=C:\Qt\5.12.4\msvc2017_64" & :: uncomment to use AppVeyor's Qt
  SET "TARGET_PROCESSOR_BITS=64"
  SET "TARGET_PROCESSOR_ARCH=x86_64"
) ELSE (
  SET "QTURL=https://utils.musescore.org.s3.amazonaws.com/qt598_msvc2015.7z"
  SET "QTDIR=%cd%\qt\msvc2015" & :: uncomment to use our Qt
  SET "QTCACHE=qt598_msvc2015.7z" & :: bump version here and .appveyor.yml to trigger cache rebuild when upgrading Qt
  :: SET "QTDIR=C:\Qt\5.12.4\msvc2017" & :: uncomment to use AppVeyor's Qt
  SET "TARGET_PROCESSOR_BITS=32"
  SET "TARGET_PROCESSOR_ARCH=x86"
)

:: Download Qt if necessary
IF NOT EXIST "%QTCACHE%" ( START " " /wait "C:\cygwin64\bin\wget.exe" --no-check-certificate "%QTURL%" -O "%QTCACHE%" )
START " " /wait "7z" x -y "%QTCACHE%" "-oqt" & :: extract into `qt` directory

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
IF NOT EXIST dependencies.zip ( START " " /wait "C:\cygwin64\bin\wget.exe" --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/dependencies.7z" -O dependencies.zip )
:: assumung dependencies.zip to contain the dependencies directory (with is subdirs)
START " " /wait "7z" x -y dependencies.zip > nul
:: test
CD dependencies\include
CD C:\MuseScore

MKDIR breakpad_tools
CD breakpad_tools
SET TOOLS_ARCHIVE=dump_syms.7z
IF NOT EXIST %TOOLS_ARCHIVE% ( START " " /wait "C:\cygwin64\bin\wget.exe" --no-check-certificate "https://s3.amazonaws.com/utils.musescore.org/dump_syms.7z" -O %TOOLS_ARCHIVE% )
START " " /wait "7z" x -y %TOOLS_ARCHIVE% > nul
CD C:\MuseScore

:: is MuseScore stable? Check here, no grep in PATH later on
for /f "delims=" %%i in ('grep "^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)" C:\MuseScore\CMakeLists.txt') do set NIGHTLY_BUILD=%%i

:: get revision number
SET "PATH=%QTDIR%\bin;%PATH%"
qmake --version & :: check qt is in %PATH%
call C:\MuseScore\msvc_build.bat revision
git rev-parse --short=7 HEAD > mscore/revision.h
SET /p MSREVISION=<mscore\revision.h

:: set ccache dir
SET CCACHE_DIR=C:\ccache\cache
