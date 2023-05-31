/**
There will be notes about read/write

1. Public / internal

The classes that are located in the root of the `rw` folder
and in the `rw` namespace are intended for use outside the read-write subsystem.
Classes that are located in subfolders (readXXX, write) are internal
and not intended for use outside the read-write subsystem.

2. Versions

Reading implementations of different versions of the format are located in different corresponding subfolders.
BUT! Version source code 114-400 can be used not only to read its version, but also others in this range,
i.e. its use is mixed between these versions, so its change should take this into keep in mind.

The source code of version 410 is copied from version 400 and is used only to read the format of version 410,
i.e. it can be refactored and changed without keep in mind previous versions,
we can (or even need to) remove backward compatibility from it.

*/
