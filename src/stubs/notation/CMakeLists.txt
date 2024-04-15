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
declare_module(notation)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/notationstubmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/notationstubmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsrepositorystub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsrepositorystub.h
    ${CMAKE_CURRENT_LIST_DIR}/notationconfigurationstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/notationconfigurationstub.h
    )

set(MODULE_IS_STUB ON)
setup_module()
