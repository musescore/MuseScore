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

set(HUNSPELL_FOUND OFF)

if (MUE_COMPILE_USE_SYSTEM_HUNSPELL)
    # Try find_library first for reliable full path resolution
    find_path(HUNSPELL_INCLUDE_DIR hunspell/hunspell.hxx
        PATHS /usr/include /usr/local/include /opt/homebrew/include
    )
    find_library(HUNSPELL_LIBRARY NAMES hunspell hunspell-1.7 hunspell-1.6
        PATHS /usr/lib /usr/local/lib /opt/homebrew/lib
    )
    if (HUNSPELL_INCLUDE_DIR AND HUNSPELL_LIBRARY)
        set(HUNSPELL_FOUND ON)
        set(HUNSPELL_INCLUDE_DIRS ${HUNSPELL_INCLUDE_DIR})
        set(HUNSPELL_LIBRARIES ${HUNSPELL_LIBRARY})
        message(STATUS "Found hunspell: ${HUNSPELL_LIBRARY}")
    endif()

    # Fallback to pkg-config if find_library didn't work
    if (NOT HUNSPELL_FOUND)
        find_package(PkgConfig QUIET)
        if (PkgConfig_FOUND)
            pkg_check_modules(HUNSPELL_PKG hunspell)
            if (HUNSPELL_PKG_FOUND)
                # Use LINK_LIBRARIES which contains full paths
                set(HUNSPELL_FOUND ON)
                set(HUNSPELL_INCLUDE_DIRS ${HUNSPELL_PKG_INCLUDE_DIRS})
                set(HUNSPELL_LIBRARIES ${HUNSPELL_PKG_LINK_LIBRARIES})
                message(STATUS "Found hunspell via pkg-config: ${HUNSPELL_PKG_VERSION}")
            endif()
        endif()
    endif()

    if (NOT HUNSPELL_FOUND)
        message(STATUS "Hunspell not found - lyrics spellcheck will be disabled")
    endif()
endif()
