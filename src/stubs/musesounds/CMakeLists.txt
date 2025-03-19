# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore Limited
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

declare_module(musesounds)
set(MODULE_QRC musesounds.qrc)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/musesoundsstubmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/musesoundsstubmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/musesoundscheckupdatescenariostub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/musesoundscheckupdatescenariostub.h
    ${CMAKE_CURRENT_LIST_DIR}/musesoundscheckupdateservicestub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/musesoundscheckupdateservicestub.h
    )

set(MODULE_IS_STUB ON)
setup_module()
