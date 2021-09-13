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

set(MODULE pianoroll)

set(MODULE_QRC
    ${CMAKE_CURRENT_LIST_DIR}/pianoroll.qrc
    )

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/pianorollmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pianorollmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/ipianorollconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/ipianorollcontroller.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/pianorollconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/pianorollconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/pianorollcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/pianorollcontroller.h

    ${CMAKE_CURRENT_LIST_DIR}/view/pianorollview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/pianorollview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pianorollscrollbar.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pianorollscrollbar.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/pianorollruler.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pianorollruler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/pianorollkeyboard.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pianorollkeyboard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/pianorollautomationnote.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pianorollautomationnote.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/pianorollautomationcurves.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pianorollautomationcurves.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/ipianorollautomationmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/automation/automationvelocity.h
    ${CMAKE_CURRENT_LIST_DIR}/view/automation/automationvelocity.cpp
    )

set(MODULE_LINK
    engraving
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
