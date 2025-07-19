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

declare_module(muse_learn)
set(MODULE_ALIAS muse::learn)

set(MODULE_QRC learn.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/learnmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/learnmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/learnerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/ilearnconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/ilearnservice.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/learnconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/learnconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/learnservice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/learnservice.h
    ${CMAKE_CURRENT_LIST_DIR}/view/learnpagemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/learnpagemodel.h
    )

if (QT_SUPPORT)
    list(APPEND MODULE_LINK Qt::Concurrent Qt::Network Qt::Qml)
endif()

setup_module()
