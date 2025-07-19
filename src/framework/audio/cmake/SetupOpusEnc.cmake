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

if (MUE_COMPILE_USE_SYSTEM_OPUSENC)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(libopusenc REQUIRED IMPORTED_TARGET libopusenc)

    # Transitive dependency
    pkg_check_modules(opus REQUIRED IMPORTED_TARGET opus)

    set(LIBOPUSENC_TARGETS PkgConfig::libopusenc PkgConfig::opus)
else()
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../thirdparty/opusenc opusenc EXCLUDE_FROM_ALL)
    set(LIBOPUSENC_TARGETS opusenc)
endif()
