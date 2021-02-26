#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2021 MuseScore BVBA and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

# This is module for convert mscz files to various formats (image, pdf, audio and etc).
# Main goal - determine what need to do and call necceasers functions.
# Usually use from command line.

set(MODULE autobot)

set(MODULE_QRC autobot.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_DEF
    SCORES_ROOT="${PROJECT_SOURCE_DIR}/vtest/scores"
)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/autobotmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/autobotmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iautobot.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/autobot.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abrunner.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abrunner.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abscorelist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abscorelist.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abbasestep.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abbasestep.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abinvoker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abinvoker.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abscoreloadstep.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abscoreloadstep.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abscoreclosestep.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abscoreclosestep.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abscorezoom.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/steps/abscorezoom.h
    ${CMAKE_CURRENT_LIST_DIR}/view/autobotmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/autobotmodel.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

