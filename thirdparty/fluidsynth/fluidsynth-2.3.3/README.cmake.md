## For users - how to compile FluidSynth

The latest information on how to compile FluidSynth using the cmake build system can be found in our wiki:

https://github.com/FluidSynth/fluidsynth/wiki/BuildingWithCMake

## For developers - how to add a new feature to the CMake build system

Let's explain this issue with an example. We are adding InstPatch support to
FluidSynth as an optional feature, conditionally adding source files that
require this feature. The first step is to add a macro `option()` to the main
CMakeLists.txt file, the one that is located at the fluidsynth root directory.

file [CMakeLists.txt](./CMakeLists.txt#L79), line 79:

```cmake
option ( enable-libinstpatch "use libinstpatch (if available) to load DLS and GIG files" on )
```

If you require a minimum version of the library, set it as such:

file [CMakeLists.txt](./CMakeLists.txt#L505), line 505:

```cmake
set ( LIBINSTPATCH_MINIMUM_VERSION 1.1.0 )
```

Now, let's check if the InstPatch library and headers are installed, using
find_package:

file [CMakeLists.txt](./CMakeLists.txt#L634), lines 634-641:

```cmake
unset ( LIBINSTPATCH_SUPPORT CACHE )
if ( enable-libinstpatch )
    find_package ( InstPatch ${LIBINSTPATCH_MINIMUM_VERSION} )
    set ( LIBINSTPATCH_SUPPORT ${InstPatch_FOUND} )
    if ( LIBINSTPATCH_SUPPORT )
        list( APPEND PC_REQUIRES_PRIV "libinstpatch-1.0")
    endif (LIBINSTPATCH_SUPPORT )
endif ( enable-libinstpatch )
```

The first line clears the value of the CMake variable LIBINSTPATCH_SUPPORT. If
the value of the option `enable-libinstpatch` is true, then the function
`find_package()` is used to test a package named "InstPatch" with version 1.1.0
or later. Unfortunately, libinstpatch does not provide an official CMake
configuration, so we will need to write it a Find module. Let's start by
creating a new file in the cmake_admin directory called
[FindInstPatch.cmake](./cmake_admin/FindInstPatch.cmake).

The first thing we should do is to document what this module provides:

file [cmake_admin/FindInstPatch.cmake](./cmake_admin/FindInstPatch.cmake), lines 1-25:

```cmake
#[=======================================================================[.rst:
FindInstPatch
-------

Finds the InstPatch library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``InstPatch::libinstpatch``
  The InstPatch library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``InstPatch_FOUND``
  True if the system has the InstPatch library.
``InstPatch_VERSION``
  The version of the InstPatch library which was found.

#]=======================================================================]
```

In this case, we provide a CMake target `InstPatch::libinstpatch`, and the two
variables `InstPatch_FOUND` and `InstPatch_VERSION`. If upstream provides an
official CMake config, the find module should provide the exact same variables
and targets, even if they are not used. Moreover, the namespace of the target
(`InstPatch::`) should match the file name (`FindInstPatch.cmake`).

If the library provides a pkg-config file, we should use it to get information
about location, version, and usage requirements. We can achieve this with the
`pkg_check_modules` macro:

file [cmake_admin/FindInstPatch.cmake](./cmake_admin/FindInstPatch.cmake#L27), lines 27-29:

```cmake
# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_INSTPATCH QUIET libinstpatch-1.0)
```

We specifically want both calls to have the `QUIET` specifier as the system may
not have pkg-config or the pc file for the library, but this should not stop
searching for the library.

Next, we need to search for the headers and the library. For the headers,
libinstpatch installs them in `libinstpatch-X/libinstpatch/*.h`, with `X` being
`1` for versions 1.1.0 and below, and `2` for 1.1.1 and above. In our code, we
use the headers as such: `#include <libinstpatch/libinstpatch.h>`. With this
in mind, we use `find_path` to find the include directory:

file [cmake_admin/FindInstPatch.cmake](./cmake_admin/FindInstPatch.cmake#L32), lines 32-36:

```cmake
find_path(
  InstPatch_INCLUDE_DIR
  NAMES "libinstpatch/libinstpatch.h"
  PATHS "${PC_INSTPATCH_INCLUDEDIR}"
  PATH_SUFFIXES "libinstpatch-1" "libinstpatch-2")
```

Here is a breakdown of the arguments:

- `InstPatch_INCLUDE_DIR` is the variable where the result will be stored;
- `NAMES` should match what is used in code to include the header;
- `PATHS` should be set to what pkg-config provided us, if pkg-config is
  unavailable it will be ignored;
- `PATH_SUFFIXES` should have the potential intermediate directories between
  CMake's search paths and what was specified in `NAMES`, it may omitted when
  not applicable.

To search for the library, we call `find_library`:

file [cmake_admin/FindInstPatch.cmake](./cmake_admin/FindInstPatch.cmake#L38), lines 38-41:

```cmake
find_library(
  InstPatch_LIBRARY
  NAMES "instpatch-1.0"
  PATHS "${PC_INSTPATCH_LIBDIR}")
```

Here is a breakdown of the arguments:

- `InstPatch_LIBRARY` is the variable where the result will be stored;
- `NAMES` should list every name the library may have, some libraries may have
  a different prefix or suffix on different configurations (`-static` suffix,
  `d` suffix for debug builds);
- `PATHS` should be set to what pkg-config provided us, if pkg-config is
  unavailable it will be ignored.

Getting the version of the library is trivial with pkg-config, but may not be
possible to get without. Fortunately, the file `libinstpatch/version.h`
provides the version:

```c
#define IPATCH_VERSION       "1.2.0"
```

We can extract the version using regex:

file [cmake_admin/FindInstPatch.cmake](./cmake_admin/FindInstPatch.cmake#L44), lines 44-52:

```cmake
if(PC_INSTPATCH_VERSION)
  set(InstPatch_VERSION "${PC_INSTPATCH_VERSION}")
elseif(InstPatch_INCLUDE_DIR)
  file(READ "${InstPatch_INCLUDE_DIR}/libinstpatch/version.h" _version_h)
  string(REGEX MATCH
               "#define[ \t]+IPATCH_VERSION[ \t]+\"([0-9]+.[0-9]+.[0-9]+)\""
               _instpatch_version_re "${_version_h}")
  set(InstPatch_VERSION "${CMAKE_MATCH_1}")
endif()
```

The last thing we need to search for is the usage requirements, providing them
makes static linking much easier. If pkg-config is available, we use a helper
function to get the correct set of properties, otherwise you need to refer to 
the upstream documentation. In the case of libinstpatch, we need `glib-2` and
`libsndfile`, fortunately, we already have Find modules for both libraries:

file [cmake_admin/FindInstPatch.cmake](./cmake_admin/FindInstPatch.cmake#L55), lines 55-69:

```cmake
if(PC_INSTPATCH_FOUND)
  get_target_properties_from_pkg_config("${InstPatch_LIBRARY}" "PC_INSTPATCH"
                                        "_instpatch")
else()
  if(NOT TARGET GLib2::gobject-2
     OR NOT TARGET GLib2::gthread-2
     OR NOT TARGET GLib2::glib-2)
    find_package(GLib2 QUIET)
  endif()
  if(NOT TARGET SndFile::sndfile)
    find_package(SndFile QUIET)
  endif()
  set(_instpatch_link_libraries "GLib2::gobject-2" "GLib2::gthread-2"
                                "GLib2::glib-2" "SndFile::sndfile")
endif()
```

We then use `find_package_handle_standard_args` to check that all the
variables are set and the version matches what was requested:

file [cmake_admin/FindInstPatch.cmake](./cmake_admin/FindInstPatch.cmake#L72), lines 72-76:

```cmake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  InstPatch
  REQUIRED_VARS "InstPatch_LIBRARY" "InstPatch_INCLUDE_DIR"
  VERSION_VAR "InstPatch_VERSION")
```

This will set `InstPatch_FOUND` to true if the checks succeed and false
otherwise.

If the library is found, we then create the target and set its properties:

file [cmake_admin/FindInstPatch.cmake](./cmake_admin/FindInstPatch.cmake#L78), lines 78-87:

```cmake
if(InstPatch_FOUND AND NOT TARGET InstPatch::libinstpatch)
  add_library(InstPatch::libinstpatch UNKNOWN IMPORTED)
  set_target_properties(
    InstPatch::libinstpatch
    PROPERTIES IMPORTED_LOCATION "${InstPatch_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_instpatch_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${InstPatch_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_instpatch_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_instpatch_link_directories}")
endif()
```

Here is a breakdown of the properties:

- `IMPORTED_LOCATION` should be the result of `find_library`;
- `INTERFACE_INCLUDE_DIRECTORES` should be the result of `find_path`;
- `INTERFACE_LINK_LIBRARIES` should be the transitive usage requirements, it
  may be omitted if there are none;
- `INTERFACE_COMPILE_OPTIONS` and `INTERFACE_LINK_DIRECTORIES` should be set to
  what our `get_flags_from_pkg_config` helper provided us, if pkg-config is
  unavailable they will be ignored.

Lastly, we call mark a few cache variables as advanced:

file [cmake_admin/FindInstPatch.cmake](./cmake_admin/FindInstPatch.cmake#L88), line 88:

```cmake
mark_as_advanced(InstPatch_INCLUDE_DIR InstPatch_LIBRARY)
```

Next, we need to inform downstream projects of this so if fluidsynth was built
as a static library, all the transitive dependencies are properly forwarded. To
achieve this, we need to edit FluidSynthConfig.cmake.in:

file [FluidSynthConfig.cmake](./FluidSynthConfig.cmake.in#L22), line 22:

```cmake
set(FLUIDSYNTH_SUPPORT_LIBINSTPATCH @LIBINSTPATCH_SUPPORT@)
```

file [FluidSynthConfig.cmake](./FluidSynthConfig.cmake.in#L91), lines 91-93:

```cmake
  if(FLUIDSYNTH_SUPPORT_LIBINSTPATCH AND NOT TARGET InstPatch::libinstpatch)
    find_dependency(InstPatch @LIBINSTPATCH_MINIMUM_VERSION@)
  endif()
```

There is a report to summarize the performed checks and the enabled features
after the configuration steps, so let's add a line in this report regarding
the InstPatch support.

file [cmake_admin/report.cmake](./cmake_admin/report.cmake#L119), lines 119-124:

```cmake
set ( INPUTS_REPORT "${INPUTS_REPORT}Support for DLS files:   " )
if ( LIBINSTPATCH_SUPPORT )
    set ( INPUTS_REPORT "${INPUTS_REPORT}yes\n" )
else ( LIBINSTPATCH_SUPPORT )
    set ( INPUTS_REPORT "${INPUTS_REPORT}no (libinstpatch not found)\n" )
endif ( LIBINSTPATCH_SUPPORT )
```

The variable `LIBINSTPATCH_SUPPORT` is available for the CMake files, but we
want to make it available to the compilers as well, to conditionally build code
using `#ifdef LIBINSTPATCH_SUPPORT`. This can be done adding a line to the
config.cmake file:

file [src/config.cmake](./src/config.cmake#L148), lines 148-149:

```c
/* libinstpatch for DLS and GIG */
#cmakedefine LIBINSTPATCH_SUPPORT @LIBINSTPATCH_SUPPORT@
```

The file config.cmake will be processed at configure time, producing a header
file "config.h" in the build directory with this content, if the InstPatch
support has been enabled and found:

```c
/* libinstpatch for DLS and GIG */
#define LIBINSTPATCH_SUPPORT 1
```

Finally, we add the new sources files to the library during the `add_library`
call, and link the library using the `target_link_libraries` function. Note
that linking to a target will automatically set the necessary include
directories.

file [src/CMakeLists.txt](./src/CMakeLists.txt#L96), lines 96-98:

```cmake
if ( LIBINSTPATCH_SUPPORT )
  set ( fluid_libinstpatch_SOURCES sfloader/fluid_instpatch.c sfloader/fluid_instpatch.h )
endif ( LIBINSTPATCH_SUPPORT )
```

file [src/CMakeLists.txt](./src/CMakeLists.txt#L230), lines 230-257:

```cmake
add_library ( libfluidsynth-OBJ OBJECT
    ...
    ${fluid_libinstpatch_SOURCES}
    ...
)
```

file [src/CMakeLists.txt](./src/CMakeLists.txt#L386), lines 386-388:

```cmake
if ( TARGET InstPatch::libinstpatch AND LIBINSTPATCH_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC InstPatch::libinstpatch )
endif()
```

If the library is used in the fluidsynth executable, you also need to link it to its target:

file [src/CMakeLists.txt](./src/CMakeLists.txt#L484), lines 484-486:

```cmake
if ( TARGET InstPatch::libinstpatch AND LIBINSTPATCH_SUPPORT )
    target_link_libraries ( fluidsynth PRIVATE InstPatch::libinstpatch )
endif()
```

### Find modules guideline

It is not always necessary to provide find modules if:

- The upstream library provides its own config
- Most distribution ship this config

For example:

- libsndfile provides a CMake config file only when built with CMake but not
  with autotools. In that case, it may be preferable to provide a Find module
  that matches what the config would provide. See [cmake_admin/FindSndFile](./cmake_admin/FindSndFile.cmake).
- SDL2 provides a config file that defines targets starting 2.0.12. Since even
  Debian stable provides this config, it is safe to assume that a find module
  will not be needed.

If upstream has only begun shipping CMake config recently, it is preferable to
add a Find module.

If upstream does not provide a CMake config altogether, you will need to make
your own from scratch. There is no defined rule for naming, but the common
practice is to have the namespace use PascalCase.

You can have CMake prefer the upstream config file over the find modules by
setting `CMAKE_FIND_PACKAGE_PREFER_CONFIG` to `ON`.
