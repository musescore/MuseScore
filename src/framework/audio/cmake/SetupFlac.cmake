# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore Limited
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

if (MUE_COMPILE_USE_SYSTEM_FLAC)
    find_package(FLAC)

    if (FLAC_FOUND)
        message(STATUS "Found flac: ${FLAC_VERSION}")
        set(FLAC_TARGETS FLAC::FLAC FLAC::FLAC++)
        return()
    endif()

    find_package(PkgConfig REQUIRED)

    pkg_check_modules(flac REQUIRED IMPORTED_TARGET flac)
    pkg_check_modules(flacpp REQUIRED IMPORTED_TARGET flac++)

    if (flac_FOUND)
        message(STATUS "Found flac: ${flac_VERSION}")
        set(FLAC_TARGETS PkgConfig::flac PkgConfig::flacpp)
        return()
    else()
        message(WARNING "Set MUE_COMPILE_USE_SYSTEM_FLAC=ON, but system flac not found, built-in will be used")
    endif()
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../thirdparty/flac flac EXCLUDE_FROM_ALL)
set(FLAC_TARGETS flac)
