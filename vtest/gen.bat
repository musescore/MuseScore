

#
# "compare" - image magick compare program
#
set SRC=mmrest-1,mmrest-2,mmrest-4,mmrest-5,mmrest-6,mmrest-7,mmrest-8,mmrest-9, ^
 fmrest-1,fmrest-2,fmrest-3,fmrest-4,fmrest-5,measure-repeat-1, ^
 noteheadposition-1,valign-1,emmentaler-1,bravura-1,emmentaler-2,bravura-2, ^
 emmentaler-3,bravura-3,emmentaler-5,bravura-5

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
      compare -metric AE -fuzz 50% %%a-1.png %%a-ref.png %%a-diff.png
)

del /q %F%

echo ^<html^> >> %F%
echo   ^<body^> >> %F%
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

