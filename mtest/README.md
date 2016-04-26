Building & running the tests
==================

To build all tests:

| Linux | OSX | Windows |
| ----- | --- | ------- |
| make debug<br>sudo make installdebug<br>cd build.debug/mtest<br>make</pre> | make -f Makefile.osx debug<br>make -f Makefile.osx installdebug<br>cd build.debug/mtest<br>make -f Makefile.osx | mingw32-make -f Makefile.mingw debug<br>mingw32-make -f Makefile.mingw installdebug<br>cd build.debug\mtest<br>mingw32-make -f Makefile.mingw |

To run all tests:

    ctest

To run only one test (for debugging purposes):

    cd libmscore/join/
    ./tst_join

To see how the CI environment is doing it check `.travis.yml` and `build/run_tests.sh`

**Note: You need to have `diff` in your path. For Windows, get a copy of [diffutils for Windows](http://gnuwin32.sourceforge.net/packages/diffutils.htm "diffutils for Windows").**

Test case conventions
====================

Tests are grouped in directories by feature (like libmscore or mxl). 
In these directories, each subdirectory represents a test suite for a particular sub feature.

The name of a test suite directory should be descriptive. The CPP file for the tests should use the same name as the directory, for example `tst_foo.cpp` in directory `foo`. It's good practice to include a README file in a test suite directory.

Test suite CPP files contain one slot per test case. Each file should be called foo-XX with XX being an incrementing count. If a test case uses a file and a ref file, they should be called `foo-XX` and `foo-XX-ref`, with the extension .mscx. A test case should not reuse a file from another test case.

To create reference or original files, MuseScore can be run with the `-t` command line argument and it will save all the files in the session in test mode. Such files do not contain platform or version information and do contain extra data for tracing (for example, they contains pixel level position for beams).

How to write a test case
===============

Import test
----------------

* Open a short file containing an individual case in one of the formats supported by MuseScore
* Save in MuseScore format
* Compare with reference file

At first the test will fail because there is no reference file. Open the file created by the test case in MuseScore and try to edit it to be sure it's valid. If the file is valid, save it (without version number) as a reference file.

Object read write
----------------

Create a test case for all elements and all properties in each element. See `libmscore/note`

* Create an object
* Set a property
* Write and read the object
* Check if the property has the right value

Action tests
----------------
See `libmscore/join` or `libmscore/split` for example 

* Read a score file
* Apply an action
* Write the file
* Compare with a reference
* (Undo the action)
* (Compare with original file)

Compatibility tests
----------------

Most of them are in `mtest/libmscore/compat`

* Read a score file from an older version of MuseScore
* Write the file
* Compare with a reference file
