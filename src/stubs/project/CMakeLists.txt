# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2023 MuseScore Limited
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
declare_module(project)

set(MODULE_QRC project.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/projectstubmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/projectstubmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/projectconfigurationstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/projectconfigurationstub.h
    ${CMAKE_CURRENT_LIST_DIR}/recentfilescontrollerstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/recentfilescontrollerstub.h
    )

set(MODULE_IS_STUB ON)
setup_module()
