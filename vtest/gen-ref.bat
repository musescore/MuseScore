
rem create reference

set MSCORE=..\msvc.install_x64\bin\musescore.exe
set DPI=130

%MSCORE% %1.mscz -r %DPI% -o %1.png
del %1-ref.png 2>nul
rename %1-1.png %1-ref.png
