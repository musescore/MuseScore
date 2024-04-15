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

declare_module(iex_audioexport)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/audioexportmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audioexportmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioexportconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioexportconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioexportconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractaudiowriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractaudiowriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/mp3writer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/mp3writer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/wavewriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/wavewriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/oggwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/oggwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/flacwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/flacwriter.h
    )

set(MODULE_LINK
    engraving
    )

setup_module()
