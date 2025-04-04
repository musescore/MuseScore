# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore BVBA and others
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
set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/musesoundsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/musesoundsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/imusesoundsrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/imusesoundsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/musesoundstypes.h
    ${CMAKE_CURRENT_LIST_DIR}/musesoundserrors.h
    ${CMAKE_CURRENT_LIST_DIR}/imusesoundscheckupdatescenario.h
    ${CMAKE_CURRENT_LIST_DIR}/imusesoundscheckupdateservice.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/musesoundsrepository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/musesoundsrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/musesoundsconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/musesoundsconfiguration.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/musesoundscheckupdatescenario.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/musesoundscheckupdatescenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/musesoundscheckupdateservice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/musesoundscheckupdateservice.h

    ${CMAKE_CURRENT_LIST_DIR}/view/musesoundslistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/musesoundslistmodel.h
    )

setup_module()
