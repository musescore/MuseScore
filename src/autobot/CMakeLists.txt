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

set(MODULE autobot)

set(MODULE_QRC autobot.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/autobotmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/autobotmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/abtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/iautobot.h
    ${CMAKE_CURRENT_LIST_DIR}/itestcase.h
    ${CMAKE_CURRENT_LIST_DIR}/iteststep.h
    ${CMAKE_CURRENT_LIST_DIR}/iabcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/iautobotconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iautobotscriptsrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobot.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abcontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abrunner.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abrunner.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/typicaltc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/typicaltc.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abbasestep.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abbasestep.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abinvoker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abinvoker.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abreport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abreport.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotactionscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotactionscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotscriptsrepository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobotscriptsrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/draw/abpaintprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/draw/abpaintprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abscoreloadstep.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abscoreloadstep.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abscoreclosestep.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abscoreclosestep.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abscorezoom.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abscorezoom.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abdrawcurrentstep.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abdrawcurrentstep.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abdrawrefstep.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abdrawrefstep.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abdrawcompstep.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abdrawcompstep.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abdiffdrawstep.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abdiffdrawstep.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/scripts/scriptengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/scripts/scriptengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/scripts/jsmoduleloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/scripts/jsmoduleloader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/iapiengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/iapiregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/apiregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/apiregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/scriptapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/scriptapi.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/apiobject.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/apiobject.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/logapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/logapi.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/autobotapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/autobotapi.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/dispatcherapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/dispatcherapi.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/navigationapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/navigationapi.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/contextapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/contextapi.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/shortcutsapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/api/shortcutsapi.h
    ${CMAKE_CURRENT_LIST_DIR}/view/autobotmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/autobotmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/abfilesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/abfilesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/autobotscriptsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/autobotscriptsmodel.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

