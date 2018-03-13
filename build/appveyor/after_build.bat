:: Print ccache statistics
ccache.exe -s

:: Test MueScore stability
IF ["%UNSTABLE%"] == [] (
  :: sign dlls and exe files
  SET dSource=C:\MuseScore\win32install
  for /f "delims=" %%f in ('dir /a-d /b /s "%dSource%\*.dll" "%dSource%\*.exe"') do (
      echo "Signing %%f"
      "SignTool" sign /f "C:\MuseScore\build\build\appveyor\resources\musescore.p12" /t http://timestamp.verisign.com/scripts/timstamp.dll /p "%CERTIFICATE_PASSWORD%" "%%f"
      )

  :: Create msi package
  mingw32-make -f Makefile.mingw package

  :: ind the MSI file without the hardcoded version
  for /r %%i in (C:\MuseScore\build.release\*.msi) do ( SET FILEPATH=%i )
  echo %FILEPATH%
  for /F %%f in ("%FILEPATH%") do (
      SET FILENAME=%%~nxf
      echo %FILENAME%
      "SignTool" sign /debug /f "C:\MuseScore\build\build\appveyor\resources\musescore.p12" /t http://timestamp.verisign.com/scripts/timstamp.dll /p "%CERTIFICATE_PASSWORD%" /d %FILENAME% %FILEPATH%
      :: verify signature
      "SignTool" verify %FILEPATH%
      )
  :: prepare upload
  XCOPY %FILEPATH% C:\MuseScore /Y /Q
  SET ARTIFACT_NAME=%FILENAME%
) ELSE (
  echo "UNSTABLE"
  CD C:\MuseScore
  RENAME C:\MuseScore\win32install\bin\musescore.exe nightly.exe
  RENAME C:\MuseScore\win32install MuseScoreNightly
  XCOPY C:\MuseScore\build\appveyor\special C:\MuseScore\MuseScoreNightly\special /I /E /Y /Q
  COPY C:\MuseScore\build\appveyor\support\README.txt C:\MuseScore\MuseScoreNightly\README.txt /Y
  COPY C:\MuseScore\build\appveyor\support\nightly.bat C:\MuseScore\MuseScoreNightly\nightly.bat /Y
  COPY C:\MuseScore\mscore\revision.h C:\MuseScore\MuseScoreNightly\revision.h
  :: get hour with a trailing 0 if necessary (add 100)
  SET hh0=%time:~0,2%
  SET /a hh1=%hh0%+100
  SET hh=%hh1:~1,2%
  SET BUILD_DATE=%Date:~10,4%-%Date:~4,2%-%Date:~7,2%-%hh%%time:~3,2%
  SET ARTIFACT_NAME=MuseScoreNightly-%BUILD_DATE%-%APPVEYOR_REPO_BRANCH%-%MSversion%.7z
  7z a C:\MuseScore\%ARTIFACT_NAME% C:\MuseScore\MuseScoreNightly
)


:: SET SSH_IDENTITY=C:\MuseScore\build\appveyor\resources\osuosl_nighlies_rsa_nopp
:: SET PATH=%OLD_PATH%
::IF DEFINED ENCRYPT_SECRET_SSH (
  :: scp -oStrictHostKeyChecking=no -C -i %SSH_IDENTITY% %ARTIFACT_NAME% musescore-nightlies@ftp-osl.osuosl.org:~/ftp/windows/
  :: ssh -oStrictHostKeyChecking=no -i %SSH_IDENTITY% musescore-nightlies@ftp-osl.osuosl.org "cd ~/ftp/windows; ls MuseScoreNightly* -t | tail -n +41 | xargs rm -f"
  :: create and upload index.html and RSS
  :: python build/appveyor/updateHTML.py %SSH_IDENTITY%
  :: scp -oStrictHostKeyChecking=no -C -i %SSH_IDENTITY% build/appveyor/web/index.html musescore-nightlies@ftp-osl.osuosl.org:ftp/windows
  :: scp -oStrictHostKeyChecking=no -C -i %SSH_IDENTITY% build/appveyor/web/nightly.xml musescore-nightlies@ftp-osl.osuosl.org:ftp/windows
  :: trigger distribution
  :: ssh -oStrictHostKeyChecking=no -i %SSH_IDENTITY% musescore-nightlies@ftp-osl.osuosl.org "~/trigger-musescore-nightlies"
  :: notify IRC channel
  :: pip install irc
  :: python build/appveyor/irccat.py "%APPVEYOR_REPO_BRANCH%-%MSversion% (Win) compiled successfully https://ftp.osuosl.org/pub/musescore-nightlies/windows/%ARTIFACT_NAME%"
::  )


:: back to root
CD C:\MuseScore
