


rem "compare" - image magick compare program

set SRC=mmrest-1,bravura-mmrest,mmrest-2,mmrest-4,mmrest-5,mmrest-6,mmrest-7,mmrest-8,mmrest-9, ^
 fmrest-1,fmrest-2,fmrest-3,fmrest-4,fmrest-5,fmrest-6,measure-repeat-1, ^
 noteheadposition-1,valign-1,emmentaler-1,bravura-1,emmentaler-2,bravura-2, ^
 emmentaler-3,bravura-3,emmentaler-4,bravura-4,emmentaler-5,bravura-5, ^
 emmentaler-6,bravura-6,emmentaler-7,bravura-7, ^
 emmentaler-8,bravura-8,emmentaler-9,bravura-9,emmentaler-10,bravura-10, ^
 chord-layout-1,chord-layout-2,chord-layout-3,chord-layout-4,chord-layout-5, ^
 chord-layout-6,chord-layout-7,chord-layout-8,chord-layout-9,chord-layout-10, chord-layout-11, ^
 accidental-1,accidental-2,accidental-3,accidental-4, ^
 accidental-5,accidental-6,accidental-7,accidental-8, ^
 tie-1,grace-1,harmony-1,beams-1,beams-2,user-offset-1,user-offset-2

set MSCORE=..\win32install\bin\mscore.exe
set DPI=130
set F=vtest.html

rd /s/q html
md html
cd html

FOR /D %%a IN (%SRC%) DO (
      echo process %%a
      xcopy ..\%%a-ref.png .
      ..\%MSCORE% ..\%%a.mscz -r %DPI% -o %%a.png
      compare -metric AE -fuzz 50%% %%a-1.png %%a-ref.png %%a-diff.png
)

xcopy ..\style.css .

del /q %F%

echo ^<html^> >> %F%
echo   ^<head^> >> %F%
echo     ^<link rel="stylesheet" type="text/css" href="style.css"^> >> %F%
echo   ^<head^> >> %F%
echo   ^<body^> >> %F%
echo     ^<div id="topbar"^> >> %F%
echo       ^<span^>Current^</span^> >> %F%
echo       ^<span^>Reference^</span^> >> %F%
echo       ^<span^>Comparison^</span^> >> %F%
echo     ^</div^> >> %F%
echo     ^<div id="topmargin"^>^</div^> >> %F%
FOR /D %%a IN (%SRC%) DO (
      echo     ^<h2 id="%%a"^>%%a ^<a class="toc-anchor" href="#%%a"^>#^</a^>^</h2^> >> %F%
      echo     ^<div^> >> %F%
      echo       ^<img src="%%a-1.png"^> >> %F%
      echo       ^<img src="%%a-ref.png"^> >> %F%
      echo       ^<img src="%%a-diff.png"^> >> %F%
      echo     ^</div^> >> %F%
)
echo   ^</body^> >> %F%
echo ^</html^> >> %F%

%F%
cd ..

