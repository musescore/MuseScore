
set(MODULE freetype)

set(MODULE_SRC
    src/autofit/autofit.c
    src/base/ftbase.c
    src/base/ftbbox.c
    src/base/ftbdf.c
    src/base/ftbitmap.c
    src/base/ftcid.c
    src/base/ftfntfmt.c
    src/base/ftfstype.c
    src/base/ftgasp.c
    src/base/ftglyph.c
    src/base/ftgxval.c
    src/base/ftinit.c
    src/base/ftlcdfil.c
    src/base/ftmm.c
    src/base/ftotval.c
    src/base/ftpatent.c
    src/base/ftpfr.c
    src/base/ftstroke.c
    src/base/ftsynth.c
    src/base/ftsystem.c
    src/base/fttype1.c
    src/base/ftwinfnt.c
    src/bdf/bdf.c
    src/bzip2/ftbzip2.c
    src/cache/ftcache.c
    src/cff/cff.c
    src/cid/type1cid.c
    src/gzip/ftgzip.c
    src/lzw/ftlzw.c
    src/pcf/pcf.c
    src/pfr/pfr.c
    src/psaux/psaux.c
    src/pshinter/pshinter.c
    src/psnames/psnames.c
    src/raster/raster.c
    src/sfnt/sfnt.c
    src/smooth/smooth.c
    src/truetype/truetype.c
    src/type1/type1.c
    src/type42/type42.c
    src/winfonts/winfnt.c
    )

set(MODULE_DEF
    -DFT2_BUILD_LIBRARY
    )

set(MODULE_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}/include
    )

set(MODULE_USE_PCH_NONE ON)
set(MODULE_USE_UNITY_NONE ON)
include(${PROJECT_SOURCE_DIR}/build/module.cmake)

target_no_warning(${MODULE} -Wimplicit-fallthrough=0)
target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -Wno-cast-function-type)

#include(CheckIncludeFile)

#set(BUILD_SHARED_LIBS OFF)

## CMAKE_TOOLCHAIN_FILE must be set before `project' is called, which
## configures the base build environment and references the toolchain file
#if (OS_IS_MAC)
#  if (DEFINED IOS_PLATFORM)
#    if (NOT "${IOS_PLATFORM}" STREQUAL "OS"
#        AND NOT "${IOS_PLATFORM}" STREQUAL "SIMULATOR")
#      message(FATAL_ERROR
#        "IOS_PLATFORM must be set to either OS or SIMULATOR")
#    endif ()
#    if (NOT "${CMAKE_GENERATOR}" STREQUAL "Xcode")
#      message(AUTHOR_WARNING
#        "You should use Xcode generator with IOS_PLATFORM enabled to get Universal builds.")
#    endif ()
#    if (BUILD_SHARED_LIBS)
#      message(FATAL_ERROR
#        "BUILD_SHARED_LIBS can not be on with IOS_PLATFORM enabled")
#    endif ()
#    if (BUILD_FRAMEWORK)
#      message(FATAL_ERROR
#        "BUILD_FRAMEWORK can not be on with IOS_PLATFORM enabled")
#    endif ()

#    # iOS only uses static libraries
#    set(BUILD_SHARED_LIBS OFF)

#    set(CMAKE_TOOLCHAIN_FILE
#      ${PROJECT_SOURCE_DIR}/thirdparty/freetype/builds/cmake/iOS.cmake)
#  endif ()
#else ()
#  if (DEFINED IOS_PLATFORM)
#    message(FATAL_ERROR "IOS_PLATFORM is not supported on this platform")
#  endif ()
#endif ()


## project(freetype)


## Add local cmake modules
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/thirdparty/freetype/builds)

## Compiler definitions for building the library
#add_definitions(-DFT2_BUILD_LIBRARY)

#message(STATUS
#  "Creating directory ${PROJECT_BINARY_DIR}/include/freetype/config")
#file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/freetype/config")


## Create the configuration file
#message(STATUS
#  "Creating file ${PROJECT_BINARY_DIR}/include/freetype/config/ftconfig.h")

#if (UNIX)
#  check_include_file("unistd.h" HAVE_UNISTD_H)
#  check_include_file("fcntl.h" HAVE_FCNTL_H)
#  check_include_file("stdint.h" HAVE_STDINT_H)

#  file(READ "${CMAKE_CURRENT_LIST_DIR}/builds/unix/ftconfig.in"
#    FTCONFIG_H)
#  if (HAVE_UNISTD_H)
#    string(REGEX REPLACE
#      "#undef +(HAVE_UNISTD_H)" "#define \\1"
#      FTCONFIG_H "${FTCONFIG_H}")
#  endif ()
#  if (HAVE_FCNTL_H)
#    string(REGEX REPLACE
#      "#undef +(HAVE_FCNTL_H)" "#define \\1"
#      FTCONFIG_H "${FTCONFIG_H}")
#  endif ()
#  if (HAVE_STDINT_H)
#    string(REGEX REPLACE
#      "#undef +(HAVE_STDINT_H)" "#define \\1"
#      FTCONFIG_H "${FTCONFIG_H}")
#  endif ()
#  string(REPLACE "/undef " "#undef "
#    FTCONFIG_H "${FTCONFIG_H}")
#  file(WRITE "${PROJECT_BINARY_DIR}/include/freetype/config/ftconfig.h"
#    "${FTCONFIG_H}")
#else ()
#  file(READ "${PROJECT_SOURCE_DIR}/thirdparty/freetype/include/freetype/config/ftconfig.h"
#    FTCONFIG_H)
#  file(WRITE "${PROJECT_BINARY_DIR}/include/freetype/config/ftconfig.h"
#    "${FTCONFIG_H}")
#endif ()


## Create the options file
#message(STATUS
#  "Creating file ${PROJECT_BINARY_DIR}/include/freetype/config/ftoption.h")

#file(READ "${PROJECT_SOURCE_DIR}/thirdparty/freetype/include/freetype/config/ftoption.h"
#  FTOPTION_H)
#if (ZLIB_FOUND)
#  string(REGEX REPLACE
#    "/\\* +(#define +FT_CONFIG_OPTION_SYSTEM_ZLIB) +\\*/" "\\1"
#    FTOPTION_H "${FTOPTION_H}")
#endif ()
#if (BZIP2_FOUND)
#  string(REGEX REPLACE
#    "/\\* +(#define +FT_CONFIG_OPTION_USE_BZIP2) +\\*/" "\\1"
#    FTOPTION_H "${FTOPTION_H}")
#endif ()
#if (PNG_FOUND)
#  string(REGEX REPLACE
#    "/\\* +(#define +FT_CONFIG_OPTION_USE_PNG) +\\*/" "\\1"
#    FTOPTION_H "${FTOPTION_H}")
#endif ()
#if (HARFBUZZ_FOUND)
#  string(REGEX REPLACE
#    "/\\* +(#define +FT_CONFIG_OPTION_USE_HARFBUZZ) +\\*/" "\\1"
#    FTOPTION_H "${FTOPTION_H}")
#endif ()
#file(WRITE "${PROJECT_BINARY_DIR}/include/freetype/config/ftoption.h"
#  "${FTOPTION_H}")


## Specify library include directories
#include_directories("${PROJECT_SOURCE_DIR}/thirdparty/freetype/include")
#include_directories(BEFORE "${PROJECT_BINARY_DIR}/thirdparty/freetype/include")

#file(GLOB PUBLIC_HEADERS "include/ft2build.h" "include/freetype/*.h")
#file(GLOB PUBLIC_CONFIG_HEADERS "include/freetype/config/*.h")
#file(GLOB PRIVATE_HEADERS "include/freetype/internal/*.h")


#set(BASE_SRCS
#  src/autofit/autofit.c
#  src/base/ftbase.c
#  src/base/ftbbox.c
#  src/base/ftbdf.c
#  src/base/ftbitmap.c
#  src/base/ftcid.c
#  src/base/ftfntfmt.c
#  src/base/ftfstype.c
#  src/base/ftgasp.c
#  src/base/ftglyph.c
#  src/base/ftgxval.c
#  src/base/ftinit.c
#  src/base/ftlcdfil.c
#  src/base/ftmm.c
#  src/base/ftotval.c
#  src/base/ftpatent.c
#  src/base/ftpfr.c
#  src/base/ftstroke.c
#  src/base/ftsynth.c
#  src/base/ftsystem.c
#  src/base/fttype1.c
#  src/base/ftwinfnt.c
#  src/bdf/bdf.c
#  src/bzip2/ftbzip2.c
#  src/cache/ftcache.c
#  src/cff/cff.c
#  src/cid/type1cid.c
#  src/gzip/ftgzip.c
#  src/lzw/ftlzw.c
#  src/pcf/pcf.c
#  src/pfr/pfr.c
#  src/psaux/psaux.c
#  src/pshinter/pshinter.c
#  src/psnames/psnames.c
#  src/raster/raster.c
#  src/sfnt/sfnt.c
#  src/smooth/smooth.c
#  src/truetype/truetype.c
#  src/type1/type1.c
#  src/type42/type42.c
#  src/winfonts/winfnt.c
#)

#if (WIN32)
#  set(BASE_SRCS ${BASE_SRCS} builds/windows/ftdebug.c)
#elseif (WINCE)
#  set(BASE_SRCS ${BASE_SRCS} builds/wince/ftdebug.c)
#else ()
#  set(BASE_SRCS ${BASE_SRCS} src/base/ftdebug.c)
#endif ()

#add_library(mscore_freetype
#  ${PUBLIC_HEADERS}
#  ${PUBLIC_CONFIG_HEADERS}
#  ${PRIVATE_HEADERS}
#  ${BASE_SRCS}
#)

####-###if (MSVC)
####-###  set_target_properties(freetype PROPERTIES
####-###    COMPILE_FLAGS /Fd"$(IntDir)$(TargetName).pdb")
####-###endif ()
#if (MSVC)
#  set_target_properties( mscore_freetype PROPERTIES
#    COMPILE_FLAGS "/wd4244 /wd4267"
#  )
#endif (MSVC)
