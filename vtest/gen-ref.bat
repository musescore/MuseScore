
rem create reference

set MSCORE=..\win64install\bin\musescore3.exe
set DPI=130

%MSCORE% %1.mscx -r %DPI% -o %1.png
del %1-ref.png 2>nul
rename %1-1.png %1-ref.png
