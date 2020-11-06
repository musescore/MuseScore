REM This batch file is meant to facilitate regenerating prebuilt binaries for 
REM the Windows tools.
REM You MUST run it from a Visual Studio xxxx Command Prompt.  To do this,
REM navigate to:
REM 
REM    Start->Programs->Microsoft Visual Studio XXXX->Tools->
REM                        Visual Studio Command Prompt
REM
REM Then run this batch file.  It performs an SVN update, edits the
REM README.binaries file to contain
REM the revision number, and builds the tools.  You must run 'svn commit' to
REM commit the pending edits to the repository.

pushd %~dp0
if %VisualStudioVersion% == 14.0 set GYP_MSVS_VERSION=2015
gyp tools_windows.gyp
msbuild tools_windows.sln /p:Configuration=Release /t:Clean,Build
copy Release\symupload.exe binaries\
copy Release\dump_syms.exe binaries\
git add binaries
git commit -m "Built Windows binaries"
echo Done!
popd
