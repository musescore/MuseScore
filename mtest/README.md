Building the tests
==================

Adapt for your own platform
    
    make debug
    make debug install
    cd build.debug/mtest
    make

running them all

    ctest

running only one for debug purpose

    cd libmscore/join/
    ./tst_join

To see how the CI environment is doing it check `.travis.yml` and `build/run_tests.sh`

Test cases convention
====================

Tests are grouped in directories by feature (like libmscore or mxl). 
In these directories, each subdirectory represent a test suite for a particular sub feature.

A test suite directory for a test suite should be descriptive. The CPP file for the tests should use the same name than the directory `tst_foo.cpp`. It's good practise to include a README file in a test suite directory.

A test suite CPP files contains a signal per test case. Each signal should be called fooXX with XX being incremental. If a test case refers to a file and ref file, they should be call `fooXX.mscx` and `foo-ref.mscx`. A test case should not reuse a file from another test case.

To create reference or original files, MuseScore can be run with the `-t` command line argument and it will save all the files in the session in test mode. Such files do not contain platform specific information, version information, and can be instrumented (for example, they contains pixel level position for beams)

How to write a test case
===============

Import test
----------------

* Open a short file in one of the format supported by MuseScore and containing a special case
* Save in MuseSCore format
* Compare with reference file

First the test will fail because there is no reference file. Open the file created by the test case in MuseScore and try to edit it to be sure it's sane. If the file is sane, save it (without your edition) as a reference file.

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

* Read a score file from an older version of MuseScore (currently only 1.2)
* Write the file
* Compare with a reference file