# Reading and writing MuseScore files

## Public / internal

The classes that are located in the root of the `rw` folder and in the `rw` namespace are intended for use outside the read-write subsystem. Classes that are located in subfolders (readXXX, write) are internal and not intended for use outside the read-write subsystem.

## Versions

Reading implementations of different versions of the format are located in different corresponding subfolders. An important exception is formed by versions 114-400, which share code, so this must be kept in mind when changing the code of these versions.

The source code of version 410 is copied from version 400 and is used only to read the format of version 410, i.e. it can be refactored and changed without worrying about previous versions. We can, and should, remove code that's only relevant for older versions from the 410 implementation.

The code for 460 is in turn a copy of the code for 410.

## TODO

We should find a better balance between isolation and code duplication between different versions.
