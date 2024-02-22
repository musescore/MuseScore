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

set(MODULE update)
set(MODULE_QRC update.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/updatemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/updatemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/updatetypes.h
    ${CMAKE_CURRENT_LIST_DIR}/updateerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/iupdateconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iupdatescenario.h
    ${CMAKE_CURRENT_LIST_DIR}/iupdateservice.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/updateconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/updateconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/updatescenario.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/updatescenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/updateservice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/updateservice.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/updateuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/updateuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/updateactioncontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/updateactioncontroller.h

    ${CMAKE_CURRENT_LIST_DIR}/view/updatemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/updatemodel.h
    )

include(SetupModule)

if (MUE_BUILD_UPDATE_TESTS)
    add_subdirectory(tests)
endif()
