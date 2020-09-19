
SET MU_DIR=%CD%

IF [%1]==[] (
    SET BUILD_MODE=mu3
) ELSE (
    SET BUILD_MODE=%1
)

SET BUILD_UI_MU4=OFF
if %BUILD_MODE% == mu4 (
    SET BUILD_UI_MU4=ON
) 

ECHO "BUILD_MODE: %BUILD_MODE%"
ECHO "BUILD_UI_MU4: %BUILD_UI_MU4%"

XCOPY "C:\musescore_dependencies" %MU_DIR% /E /I /Y

SET GENERATOR_NAME=Visual Studio 16 2019
SET MSCORE_STABLE_BUILD="TRUE"

:: TODO We need define paths during image creation
SET "QT_DIR=C:\Qt\5.15.1"
SET "JACK_DIR=C:\Program Files (x86)\Jack"
SET PATH=%QT_DIR%\msvc2019_64\bin;%JACK_DIR%;%PATH% 


CD %MU_DIR% 
CALL msvc_build.bat revision 
CALL msvc_build.bat relwithdebinfo 
CALL msvc_build.bat installrelwithdebinfo

