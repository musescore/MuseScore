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

declare_module(muse_mpe)
set(MODULE_ALIAS muse::mpe)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/mpemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mpemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/soundid.h
    ${CMAKE_CURRENT_LIST_DIR}/mpetypes.h
    ${CMAKE_CURRENT_LIST_DIR}/events.h
    ${CMAKE_CURRENT_LIST_DIR}/playbacksetupdata.h
    ${CMAKE_CURRENT_LIST_DIR}/iarticulationprofilesrepository.h

    ${CMAKE_CURRENT_LIST_DIR}/view/articulationpatternsegmentitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/articulationpatternsegmentitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/articulationpatternitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/articulationpatternitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/articulationsprofileeditormodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/articulationsprofileeditormodel.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/articulationstringutils.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/articulationprofilesrepository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/articulationprofilesrepository.h
    )

set(MODULE_QRC mpe.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

if (QT_SUPPORT)
    list(APPEND MODULE_LINK Qt::Qml)
endif()

setup_module()

if (MUSE_MODULE_MPE_TESTS)
    add_subdirectory(tests)
endif()
