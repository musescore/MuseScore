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

# This is module for convert mscz files to various formats (image, pdf, audio and etc).
# Main goal - determine what need to do and call necceasers functions.
# Usually use from command line.

set(MODULE diagnostics)

set(MODULE_QRC diagnostics.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/diagnosticsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/diagnosticsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/idiagnosticspathsregister.h
    ${CMAKE_CURRENT_LIST_DIR}/diagnosticutils.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsactionscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticsactionscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticspathsregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/diagnosticspathsregister.h

    ${CMAKE_CURRENT_LIST_DIR}/view/diagnosticspathsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/diagnosticspathsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/diagnosticnavigationmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/diagnosticnavigationmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/abstractkeynavdevitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/abstractkeynavdevitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/keynavdevcontrol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/keynavdevcontrol.h
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/keynavdevsubsection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/keynavdevsubsection.h
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/keynavdevsection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynav/keynavdevsection.h
    ${CMAKE_CURRENT_LIST_DIR}/view/diagnosticaccessiblemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/diagnosticaccessiblemodel.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

