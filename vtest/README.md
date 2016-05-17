MuseScore Visual Tests
======================

The shell script "gen" creates a subdirectory `html` with the
HTML file `vtest.html` for visual compare including all needed
image files.
Travis generates this too and then uploads to http://vtest.musescore.org/index.html

Requirements
---
In order to generate the diff between the reference 
file and the generated one, *Image Magick* needs to be
installed and "compare" should be in the `PATH`.

Add a new test
---
- clone a mscz test file into `xxx.mscz`
- create the test by editing the `xxx.mscz` file
- create reference PNG `xxx-ref.png`:
        mscore xxx.mscz -o -r 130 xxx-ref.png
- add the file to `gen` and `gen.bat`


