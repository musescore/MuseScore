# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
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

declare_module(muse_audio)
set(MODULE_ALIAS muse::audio)

set(MODULE_QRC audio.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/audiostubmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audiostubmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/audioconfigurationstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audioconfigurationstub.h
    ${CMAKE_CURRENT_LIST_DIR}/synthresolverstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/synthresolverstub.h
    ${CMAKE_CURRENT_LIST_DIR}/synthesizerstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/synthesizerstub.h
    ${CMAKE_CURRENT_LIST_DIR}/fxresolverstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fxresolverstub.h
    ${CMAKE_CURRENT_LIST_DIR}/soundfontcontrollerstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/soundfontcontrollerstub.h
    ${CMAKE_CURRENT_LIST_DIR}/audiodriverstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audiodriverstub.h
    )                           

set(MODULE_IS_STUB ON)
setup_module()
