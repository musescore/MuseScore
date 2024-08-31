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

if (MUE_COMPILE_USE_SYSTEM_OPUS)
    find_package(Opus)

    if (OPUS_FOUND)
        message(STATUS "Found opus: ${OPUS_VERSION}")
        return()
    endif()

    find_package(PkgConfig REQUIRED)

    pkg_check_modules(OPUS opus)

    if (OPUS_FOUND)
        message(STATUS "Found opus: ${OPUS_VERSION}")
        return()
    endif ()

    message(WARNING "Set MUE_COMPILE_USE_SYSTEM_OPUS=ON, but system opus not found, built-in will be used")
endif ()

set(OPUS_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../opus/opus-1.4)
add_subdirectory(${OPUS_LIB_DIR} opus)

include(GetPlatformInfo)
if (ARCH_IS_ARMV7L)
    target_compile_options(opus PUBLIC -mfpu=neon)
endif()

set(OPUS_INCLUDE_DIRS ${OPUS_LIB_DIR}/include)
set(OPUS_LIBRARIES opus)

target_no_warning(opus -Wno-conversion)
target_no_warning(opus -Wno-truncate)
target_no_warning(opus -Wno-uninitialized)
