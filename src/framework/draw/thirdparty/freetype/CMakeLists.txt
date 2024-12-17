# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2023 MuseScore BVBA and others
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

declare_module(freetype)

set(FREETYPE_DIR ${CMAKE_CURRENT_LIST_DIR}/freetype-2.13.1)
set(FREETYPE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/freetype-2.13.1)

# Based on the info from ${FREETYPE_DIR}/docs/INSTALL.ANY and ${FREETYPE_DIR}/CMakeLists.txt

# Create the configuration file
if(UNIX)
    include(CheckIncludeFile)

    check_include_file("unistd.h" HAVE_UNISTD_H)
    check_include_file("fcntl.h" HAVE_FCNTL_H)

    file(READ "${FREETYPE_DIR}/builds/unix/ftconfig.h.in"
        FTCONFIG_H)

    if(HAVE_UNISTD_H)
        string(REGEX REPLACE
            "#undef +(HAVE_UNISTD_H)" "#define \\1 1"
            FTCONFIG_H "${FTCONFIG_H}")
    endif()

    if(HAVE_FCNTL_H)
        string(REGEX REPLACE
            "#undef +(HAVE_FCNTL_H)" "#define \\1 1"
            FTCONFIG_H "${FTCONFIG_H}")
    endif()
else()
    file(READ "${FREETYPE_DIR}/include/freetype/config/ftconfig.h"
        FTCONFIG_H)
endif()

set(FTCONFIG_H_NAME "${FREETYPE_BINARY_DIR}/include/freetype/config/ftconfig.h")

if(EXISTS "${FTCONFIG_H_NAME}")
    file(READ "${FTCONFIG_H_NAME}" ORIGINAL_FTCONFIG_H)
else()
    set(ORIGINAL_FTCONFIG_H "")
endif()

if(NOT(ORIGINAL_FTCONFIG_H STREQUAL FTCONFIG_H))
    file(WRITE "${FTCONFIG_H_NAME}" "${FTCONFIG_H}")
endif()

# Collect header files
file(GLOB FREETYPE_PUBLIC_HEADERS "${FREETYPE_DIR}/include/ft2build.h" "${FREETYPE_DIR}/include/freetype/*.h")
file(GLOB FREETYPE_PUBLIC_CONFIG_HEADERS "${FREETYPE_DIR}/include/freetype/config/*.h")
file(GLOB FREETYPE_PRIVATE_HEADERS "${FREETYPE_DIR}/include/freetype/internal/*.h")

# Collect source files
set(FREETYPE_FTSYSTEM_SRC ${FREETYPE_DIR}/src/base/ftsystem.c)

if(UNIX)
    set(FREETYPE_FTSYSTEM_SRC ${FREETYPE_DIR}/builds/unix/ftsystem.c)
elseif(WIN32)
    set(FREETYPE_FTSYSTEM_SRC ${FREETYPE_DIR}/builds/windows/ftsystem.c)
endif()

set(FREETYPE_FTDEBUG_SRC ${FREETYPE_DIR}/src/base/ftdebug.c)

if(WIN32)
    set(FREETYPE_FTDEBUG_SRC ${FREETYPE_DIR}/builds/windows/ftdebug.c)
elseif(WINCE)
    set(FREETYPE_FTDEBUG_SRC ${FREETYPE_DIR}/builds/wince/ftdebug.c)
endif()

set(FREETYPE_BASE_SRCS
    ${FREETYPE_FTSYSTEM_SRC}
    ${FREETYPE_DIR}/src/base/ftinit.c
    ${FREETYPE_FTDEBUG_SRC}

    ${FREETYPE_DIR}/src/base/ftbase.c

    ${FREETYPE_DIR}/src/base/ftbbox.c
    ${FREETYPE_DIR}/src/base/ftglyph.c

    ${FREETYPE_DIR}/src/base/ftbdf.c
    ${FREETYPE_DIR}/src/base/ftbitmap.c
    ${FREETYPE_DIR}/src/base/ftcid.c
    ${FREETYPE_DIR}/src/base/ftfstype.c
    ${FREETYPE_DIR}/src/base/ftgasp.c
    ${FREETYPE_DIR}/src/base/ftgxval.c
    ${FREETYPE_DIR}/src/base/ftmm.c
    ${FREETYPE_DIR}/src/base/ftotval.c
    ${FREETYPE_DIR}/src/base/ftpatent.c
    ${FREETYPE_DIR}/src/base/ftpfr.c
    ${FREETYPE_DIR}/src/base/ftstroke.c
    ${FREETYPE_DIR}/src/base/ftsynth.c
    ${FREETYPE_DIR}/src/base/fttype1.c
    ${FREETYPE_DIR}/src/base/ftwinfnt.c

    ${FREETYPE_DIR}/src/base/ftmac.c

    ${FREETYPE_DIR}/src/bdf/bdf.c
    ${FREETYPE_DIR}/src/cff/cff.c
    ${FREETYPE_DIR}/src/cid/type1cid.c
    ${FREETYPE_DIR}/src/pcf/pcf.c
    ${FREETYPE_DIR}/src/pfr/pfr.c
    ${FREETYPE_DIR}/src/sfnt/sfnt.c

    ${FREETYPE_DIR}/src/truetype/truetype.c
    ${FREETYPE_DIR}/src/type1/type1.c
    ${FREETYPE_DIR}/src/type42/type42.c
    ${FREETYPE_DIR}/src/winfonts/winfnt.c

    ${FREETYPE_DIR}/src/raster/raster.c
    ${FREETYPE_DIR}/src/sdf/sdf.c
    ${FREETYPE_DIR}/src/smooth/smooth.c
    ${FREETYPE_DIR}/src/svg/svg.c

    ${FREETYPE_DIR}/src/autofit/autofit.c
    ${FREETYPE_DIR}/src/cache/ftcache.c
    ${FREETYPE_DIR}/src/gzip/ftgzip.c
    ${FREETYPE_DIR}/src/lzw/ftlzw.c
    ${FREETYPE_DIR}/src/bzip2/ftbzip2.c
    ${FREETYPE_DIR}/src/gxvalid/gxvalid.c
    ${FREETYPE_DIR}/src/otvalid/otvalid.c
    ${FREETYPE_DIR}/src/psaux/psaux.c
    ${FREETYPE_DIR}/src/pshinter/pshinter.c
    ${FREETYPE_DIR}/src/psnames/psnames.c
)

set(MODULE_SRC
    ${FREETYPE_PUBLIC_HEADERS}
    ${FREETYPE_PUBLIC_CONFIG_HEADERS}
    ${FREETYPE_PRIVATE_HEADERS}
    ${FREETYPE_BASE_SRCS}
)

set(MODULE_INCLUDE
    ${FREETYPE_BINARY_DIR}/include
    ${FREETYPE_DIR}/include

    # Make <ftconfig.h> available for builds/unix/ftsystem.c.
    ${FREETYPE_BINARY_DIR}/include/freetype/config
)

set(MODULE_DEF
    -DFT2_BUILD_LIBRARY
)

set(MODULE_USE_PCH OFF)
set(MODULE_USE_UNITY OFF)
set(MODULE_USE_COVERAGE OFF)

setup_module()

# Export variables
set(FREETYPE_LIBRARIES freetype PARENT_SCOPE)
set(FREETYPE_INCLUDE_DIRS ${FREETYPE_DIR}/include PARENT_SCOPE)

target_no_warning(${MODULE} -WMSVC-no-signed-unsigned-mismatch)
target_no_warning(${MODULE} -Wno-float-conversion)
target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -WMSVC-no-assignment-within-conditional-expression)
