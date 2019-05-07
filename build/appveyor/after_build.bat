:: Print ccache statistics
ccache.exe -s

CD C:\MuseScore

REM the code is used to generate MS version for both nightly and stable releases
SET input=C:\MuseScore\CMakeLists.txt
FOR /f tokens^=2^ delims^=^" %%A IN ('findstr /C:"SET(MUSESCORE_VERSION_MAJOR" %input%') DO set VERSION_MAJOR=%%A
FOR /f tokens^=2^ delims^=^" %%A IN ('findstr /C:"SET(MUSESCORE_VERSION_MINOR" %input%') DO set VERSION_MINOR=%%A
FOR /f tokens^=2^ delims^=^" %%A IN ('findstr /C:"SET(MUSESCORE_VERSION_PATCH" %input%') DO set VERSION_PATCH=%%A
SET MUSESCORE_VERSION=%VERSION_MAJOR%.%VERSION_MINOR%.%VERSION_PATCH%.%APPVEYOR_BUILD_NUMBER%

SET DEBUG_SYMS_FILE=musescore_win%TARGET_PROCESSOR_BITS%.sym
@echo on
C:\MuseScore\breakpad_tools\dump_syms.exe %APPVEYOR_BUILD_FOLDER%\%BUILD_FOLDER%\mscore\RelWithDebInfo\MuseScore3.pdb > %DEBUG_SYMS_FILE%
@echo off

:: Test MuseScore stability
IF "%NIGHTLY_BUILD%" == "" (
  goto :STABLE_LABEL
) ELSE (
  goto :UNSTABLE_LABEL
)

:STABLE_LABEL
echo "Stable: Build MSI package"
:: sign dlls and exe files
CD C:\MuseScore
SET dSource=msvc.install_%PLATFORM%
for /f "delims=" %%f in ('dir /a-d /b /s "%dSource%\*.dll" "%dSource%\*.exe"') do (
    echo "Signing %%f"
    "C:\Program Files (x86)\Windows Kits\8.1\bin\x64\signtool.exe" sign /f "C:\MuseScore\build\appveyor\resources\musescore.p12" /t http://timestamp.verisign.com/scripts/timstamp.dll /p "%CERTIFICATE_PASSWORD%" "%%f"
    )

CD C:\MuseScore

:: generate unique GUID
"C:\cygwin64\bin\uuidgen.exe" > uuid.txt
SET /p PACKAGE_UUID=<uuid.txt
echo on
echo %PACKAGE_UUID%
echo off
bash -c "sed -i 's/00000000-0000-0000-0000-000000000000/%PACKAGE_UUID%/' C:/MuseScore/build/Packaging.cmake"

call C:\MuseScore\msvc_build.bat package %TARGET_PROCESSOR_BITS% %APPVEYOR_BUILD_NUMBER%

CD C:\MuseScore

:: find the MSI file without the hardcoded version
for /r %%i in (msvc.build_%PLATFORM%\*.msi) do (
  SET "FILEPATH=%%i"
  SET "FILEBASE=%%~ni"
  SET "FILEEXT=%%~xi"
  SET "FILEDIR=%%~dpi"
  )
echo "Package: %FILEPATH%"
SET "FILEBASE=%FILEBASE%-%TARGET_PROCESSOR_ARCH%"
SET "FILENAME=%FILEBASE%%FILEEXT%"
RENAME "%FILEPATH%" "%FILENAME%"
SET "FILEPATH=%FILEDIR%%FILENAME%"
echo "Renamed: %FILENAME%"
echo "Location: %FILEPATH%"
@echo off
"C:\Program Files (x86)\Windows Kits\8.1\bin\x64\signtool.exe" sign /debug /f "C:\MuseScore\build\appveyor\resources\musescore.p12" /t http://timestamp.verisign.com/scripts/timstamp.dll /p "%CERTIFICATE_PASSWORD%" /d %FILENAME% %FILEPATH%
:: verify signature
"C:\Program Files (x86)\Windows Kits\8.1\bin\x64\signtool.exe" verify /pa %FILEPATH%

:: prepare upload
XCOPY %FILEPATH% C:\MuseScore /Y /Q
SET ARTIFACT_NAME=%FILENAME%

@echo off
REM WinSparkle staff. Generate appcast.xml
REM ------------------------------------------
bash C:\MuseScore\build\appveyor\winsparkle_appcast_generator.sh "C:\MuseScore\%ARTIFACT_NAME%" "https://ftp.osuosl.org/pub/musescore-nightlies/windows/%ARTIFACT_NAME%" "%MUSESCORE_VERSION%" "%MSREVISION%"
REM ------------------------------------------
@echo on
type C:\MuseScore\appcast.xml

SET /p MSCORE_RELEASE_CHANNEL=<MSCORE_RELEASE_CHANNEL.xml

goto :UPLOAD

:UNSTABLE_LABEL
echo "Unstable: build 7z package"
CD C:\MuseScore
RENAME C:\MuseScore\msvc.install_%PLATFORM%\bin\MuseScore3.exe nightly.exe
RENAME C:\MuseScore\msvc.install_%PLATFORM% MuseScoreNightly
XCOPY C:\MuseScore\build\appveyor\special C:\MuseScore\MuseScoreNightly\special /I /E /Y /Q
COPY C:\MuseScore\build\appveyor\support\README.txt C:\MuseScore\MuseScoreNightly\README.txt /Y
COPY C:\MuseScore\build\appveyor\support\nightly.bat C:\MuseScore\MuseScoreNightly\nightly.bat /Y
COPY C:\MuseScore\mscore\revision.h C:\MuseScore\MuseScoreNightly\revision.h
:: get hour with a trailing 0 if necessary (add 100)
SET hh0=%time:~0,2%
SET /a hh1=%hh0%+100
SET hh=%hh1:~1,2%
SET BUILD_DATE=%Date:~10,4%-%Date:~4,2%-%Date:~7,2%-%hh%%time:~3,2%
SET ARTIFACT_NAME=MuseScoreNightly-%BUILD_DATE%-%APPVEYOR_REPO_BRANCH%-%MSREVISION%-%TARGET_PROCESSOR_ARCH%.7z
7z a C:\MuseScore\%ARTIFACT_NAME% C:\MuseScore\MuseScoreNightly

:: create update file for S3
SET SHORT_DATE=%Date:~10,4%-%Date:~4,2%-%Date:~7,2%

@echo off

(
echo ^<update^>
echo ^<version^>%MUSESCORE_VERSION%^</version^>
echo ^<revision^>%MSREVISION%^</revision^>
echo ^<releaseType^>nightly^</releaseType^>
echo ^<date^>%SHORT_DATE%^</date^>
echo ^<description^>MuseScore %MUSESCORE_VERSION% %MSREVISION%^</description^>
echo ^<downloadUrl^>https://ftp.osuosl.org/pub/musescore-nightlies/windows/%ARTIFACT_NAME%^</downloadUrl^>
echo ^<infoUrl^>https://ftp.osuosl.org/pub/musescore-nightlies/windows/^</infoUrl^>
echo ^</update^>
)>"C:\MuseScore\update_win_nightly.xml"

@echo on
type C:\MuseScore\update_win_nightly.xml

:UPLOAD
SET SSH_IDENTITY=C:\MuseScore\build\appveyor\resources\osuosl_nighlies_rsa_nopp
SET PATH=%OLD_PATH%
IF DEFINED ENCRYPT_SECRET_SSH (
  scp -oStrictHostKeyChecking=no -C -i %SSH_IDENTITY% %ARTIFACT_NAME% musescore-nightlies@ftp-osl.osuosl.org:~/ftp/windows/
  ssh -oStrictHostKeyChecking=no -i %SSH_IDENTITY% musescore-nightlies@ftp-osl.osuosl.org "cd ~/ftp/windows; ls MuseScoreNightly* -t | tail -n +41 | xargs rm -f"
  rem create and upload index.html and RSS
  python build/appveyor/updateHTML.py %SSH_IDENTITY%
  scp -oStrictHostKeyChecking=no -C -i %SSH_IDENTITY% build/appveyor/web/index.html musescore-nightlies@ftp-osl.osuosl.org:ftp/windows
  scp -oStrictHostKeyChecking=no -C -i %SSH_IDENTITY% build/appveyor/web/nightly.xml musescore-nightlies@ftp-osl.osuosl.org:ftp/windows
  rem trigger distribution
  ssh -oStrictHostKeyChecking=no -i %SSH_IDENTITY% musescore-nightlies@ftp-osl.osuosl.org "~/trigger-musescore-nightlies"
  rem notify IRC channel
  pip install irc
  python build/appveyor/irccat.py "%APPVEYOR_REPO_BRANCH%-%MSREVISION% (Win) compiled successfully https://ftp.osuosl.org/pub/musescore-nightlies/windows/%ARTIFACT_NAME%"
  )


:: back to root
CD C:\MuseScore
