
SET NUMBER_OF_PROCESSORS=2

REM Create build directory
rem IF not exist "build.debug\" MKDIR "build.debug"
rem CD build.debug

REM Configure, build and install MuseScore
cmake .. -DCMAKE_BUILD_TYPE=RELWITHDEBINFO -DCMAKE_INSTALL_PREFIX=install
cmake --build . -j %NUMBER_OF_PROCESSORS%
cmake --build . --target install
