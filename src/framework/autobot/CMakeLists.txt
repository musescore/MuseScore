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

declare_module(muse_autobot)
set(MODULE_ALIAS muse::autobot)

set(MODULE_QRC autobot.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/autobotmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/autobotmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/autobottypes.h
    ${CMAKE_CURRENT_LIST_DIR}/iautobot.h
    ${CMAKE_CURRENT_LIST_DIR}/itestcasecontext.h
    ${CMAKE_CURRENT_LIST_DIR}/iautobotconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iautobotscriptsrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/autobotutils.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/autobot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobot.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/testcasecontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/testcasecontext.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/testcaserunner.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/testcaserunner.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/testcasereport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/testcasereport.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotactionscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotactionscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotscriptsrepository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotscriptsrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/draw/abpaintprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/draw/abpaintprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/scriptengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/scriptengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/jsmoduleloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/jsmoduleloader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotinteractive.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotinteractive.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/api/scriptapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/scriptapi.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/autobotapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/autobotapi.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/contextapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/contextapi.h

    ${CMAKE_CURRENT_LIST_DIR}/view/abfilesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/abfilesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/autobotscriptsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/autobotscriptsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/testcaserunmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/testcaserunmodel.h
    )

list(APPEND MODULE_LINK Qt::Quick)

setup_module()

