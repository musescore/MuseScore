# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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

declare_module(iex_lyricsexport)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/lyricsexportmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/lyricsexportmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/ilyricsexportconfiguration.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/lyricsexportconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/lyricsexportconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/iex_lyricsexport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/iex_lyricsexport.h
    )

set(MODULE_LINK
    engraving
)

if (QT_SUPPORT)
    list(APPEND MODULE_LINK Qt::Core5Compat)
endif()

setup_module()

