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

declare_module(playback)

set(MODULE_QRC ${CMAKE_CURRENT_LIST_DIR}/playback.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/playbackstubmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playbackstubmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/playbackcontrollerstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playbackcontrollerstub.h
    ${CMAKE_CURRENT_LIST_DIR}/playbackconfigurationstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playbackconfigurationstub.h
    )

set(MODULE_IS_STUB ON)
setup_module()
