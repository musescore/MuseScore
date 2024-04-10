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

declare_module(muse_languages)
set(MODULE_ALIAS muse::languages)

set(MODULE_QRC languages.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/languagesstubmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/languagesstubmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/languagesconfigurationstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/languagesconfigurationstub.h
    ${CMAKE_CURRENT_LIST_DIR}/languagesservicestub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/languagesservicestub.h
    )

set(MODULE_IS_STUB ON)
setup_module()

