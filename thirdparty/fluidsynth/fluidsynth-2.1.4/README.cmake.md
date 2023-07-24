## For users - how to compile FluidSynth

The latest information on how to compile FluidSynth using the cmake build system can be found in our wiki:

https://github.com/FluidSynth/fluidsynth/wiki/BuildingWithCMake


## For developers - how to add a new feature to the CMake build system

Let's explain this issue with an example. We are adding D-Bus support to 
FluidSynth as an optional feature, conditionally adding source files that 
require this feature. The first step is to add a macro "option()" to the main 
CMakeLists.txt file, the one that is located at the fluidsynth root directory.

file CMakeLists.txt, line 64:

```cmake
option ( enable-dbus "compile DBUS support (if it is available)" on )
```

Now, let's check if the dbus-1 library and headers are installed, using 
pkg-config:

file CMakeLists.txt, lines 371-377:

```cmake
unset ( DBUS_SUPPORT CACHE )
if ( enable-dbus )
    pkg_check_modules ( DBUS dbus-1>=1.0.0 )
    set ( DBUS_SUPPORT ${DBUS_FOUND} )
else ( enable-dbus )
    unset_pkg_config ( DBUS )
endif ( enable-dbus )
```

The first line clears the value of the CMake variable DBUS_SUPPORT. If the 
value of the option "enable-dbus" is true, then the macro  pkg_check_modules() 
is used to test a package named "dbus-1" with version 1.0.0 or later. This macro 
automatically defines the variables DBUS_LIBRARIES, DBUS_INCLUDEDIR, DBUS_FOUND 
and others. The value of the last one is assigned to our variable DBUS_SUPPORT 
for later use.

There is a report to summarize the performed checks and the enabled features 
after the configuration steps, so let's add a line in this report regarding 
the D-Bus support.

file cmake_admin/report.cmake, lines 14-18:

```cmake
if ( DBUS_SUPPORT )
    message ( "D-Bus:                 yes" )
else ( DBUS_SUPPORT ) 
    message ( "D-Bus:                 no" )
endif ( DBUS_SUPPORT )
```

The variable DBUS_SUPPORT is available for the CMake files, but we want to make 
it available to the compilers as well, to conditionally build code using 
"#ifdef DBUS_SUPPORT". This can be done adding a line to the config.cmake file:

file src/config.cmake, lines 22-23:

```c
/* Define if D-Bus support is enabled */
#cmakedefine DBUS_SUPPORT @DBUS_SUPPORT@
```

The file config.cmake will be processed at configure time, producing a header 
file "config.h" in the build directory with this content, if the dbus support 
has been enabled and found:

```c
/* Define if D-Bus support is enabled */
#define DBUS_SUPPORT  1
```

Finally, we can add the new source files to the build system for the compiler 
target with the macro add_library(), and the libraries for the linker target 
with the macros link_directories() and target_link_libraries().

file src/CMakeLists.txt, lines 57-60

```cmake
if ( DBUS_SUPPORT )
    set ( fluid_dbus_SOURCES fluid_rtkit.c fluid_rtkit.h )
    include_directories ( ${DBUS_INCLUDEDIR} ${DBUS_INCLUDE_DIRS} )
endif ( DBUS_SUPPORT )
```

file src/CMakeLists.txt, lines 163-197

```cmake
link_directories (
    ...
    ${DBUS_LIBDIR} 
    ${DBUS_LIBRARY_DIRS} 
)

add_library ( libfluidsynth  
    ...
    ${fluid_dbus_SOURCES}
    ...
)
```

file src/CMakeLists.txt, lines 163-197

```cmake
target_link_libraries ( libfluidsynth
    ...
    ${DBUS_LIBRARIES}
    ...
)
```

