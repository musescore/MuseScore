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

set(MODULE project)

#set(MODULE_QRC project.qrc)

#set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/projectmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/projectmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/projecttypes.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationproject.h
    ${CMAKE_CURRENT_LIST_DIR}/iprojectcreator.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationreader.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationreadersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationwritersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/iprojectaudiosettings.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/notationproject.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationproject.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectcreator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectcreator.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectaudiosettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectaudiosettings.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationreadersregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationreadersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationwritersregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationwritersregister.h
    )

set(MODULE_LINK notation)

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

