IF "%BUILD_WIN_PORTABLE%" == "ON" (
call C:\MuseScore\build\msvc_build_portable.bat relwithdebinfo %TARGET_PROCESSOR_BITS% %APPVEYOR_BUILD_NUMBER%
call C:\MuseScore\build\msvc_build_portable.bat installrelwithdebinfo %TARGET_PROCESSOR_BITS% %APPVEYOR_BUILD_NUMBER%
) ELSE (
call C:\MuseScore\msvc_build.bat relwithdebinfo %TARGET_PROCESSOR_BITS% %APPVEYOR_BUILD_NUMBER%
call C:\MuseScore\msvc_build.bat installrelwithdebinfo %TARGET_PROCESSOR_BITS% %APPVEYOR_BUILD_NUMBER%
)