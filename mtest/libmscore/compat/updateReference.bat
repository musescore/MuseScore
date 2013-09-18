

set P=..\..\..\build.debug\mtest\libmscore\compat

xcopy /y %P%\articulations-test.mscx articulations-ref.mscx
xcopy /y %P%\keysig-test.mscx keysig-ref.mscx
xcopy /y %P%\hairpin-test.mscx hairpin-ref.mscx
xcopy /y %P%\notes-test.mscx notes-ref.mscx
xcopy /y %P%\textstyles-test.mscx textstyles-ref.mscx
xcopy /y %P%\title-test.mscx title-ref.mscx
xcopy /y %P%\notes_useroffset-test.mscx notes_useroffset-ref.mscx
xcopy /y %P%\tremolo2notes-test.mscx tremolo2notes-ref.mscx
xcopy /y %P%\accidentals-test.mscx accidentals-ref.mscx
xcopy /y %P%\slurs-test.mscx slurs-ref.mscx


