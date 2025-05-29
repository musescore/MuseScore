# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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

declare_module(project)

set(MODULE_QRC project.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

include(GetPlatformInfo)

if (OS_IS_MAC)
    set(PLATFORM_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/macos/macosrecentfilescontroller.mm
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/macos/macosrecentfilescontroller.h
        )
    # Don't mix C++ and Objective-C++ in Unity Build
    set_source_files_properties(
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/macos/macosrecentfilescontroller.mm
        PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION ON
        SKIP_PRECOMPILE_HEADERS ON
        )
elseif(OS_IS_WIN)
    set(PLATFORM_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/windows/windowsrecentfilescontroller.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/windows/windowsrecentfilescontroller.h
    )
endif()

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/projectmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/projectmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/projecterrors.h
    ${CMAKE_CURRENT_LIST_DIR}/projectextensionpoints.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationproject.h
    ${CMAKE_CURRENT_LIST_DIR}/iprojectcreator.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationreader.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationreadersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationwritersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/iprojectaudiosettings.h
    ${CMAKE_CURRENT_LIST_DIR}/iprojectfilescontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/iprojectconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/irecentfilescontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/imscmetareader.h
    ${CMAKE_CURRENT_LIST_DIR}/iprojectautosaver.h
    ${CMAKE_CURRENT_LIST_DIR}/iprojectrwregister.h
    ${CMAKE_CURRENT_LIST_DIR}/iprojectwriter.h

    ${CMAKE_CURRENT_LIST_DIR}/types/projecttypes.h
    ${CMAKE_CURRENT_LIST_DIR}/types/projectmeta.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/iprojectmigrator.h
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
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectautosaver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectautosaver.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectactionscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectactionscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/exporttype.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/exporttype.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/iexportprojectscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/exportprojectscenario.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/exportprojectscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/iopensaveprojectscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/opensaveprojectscenario.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/opensaveprojectscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/recentfilescontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/recentfilescontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/mscmetareader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/mscmetareader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/itemplatesrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/templatesrepository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/templatesrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/mdlmigrator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/mdlmigrator.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectmigrator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectmigrator.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectfileinfoprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectfileinfoprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectrwregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/projectrwregister.h

    ${PLATFORM_SRC}

    ${CMAKE_CURRENT_LIST_DIR}/view/exportdialogmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/exportdialogmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/scorespagemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/scorespagemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractscoresmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractscoresmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/recentscoresmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/recentscoresmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/cloudscoresmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/cloudscoresmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/cloudscorestatuswatcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/cloudscorestatuswatcher.h
    ${CMAKE_CURRENT_LIST_DIR}/view/scorethumbnailloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/scorethumbnailloader.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pixmapscorethumbnailview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/pixmapscorethumbnailview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/newscoremodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/newscoremodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/additionalinfomodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/additionalinfomodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/templatesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/templatesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/templatepaintview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/templatepaintview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/projectpropertiesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/projectpropertiesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/audiogenerationsettingsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/audiogenerationsettingsmodel.h
    )

set(MODULE_LINK
    muse::uicomponents
    notation
    Qt::Quick
)

if (QT_ADD_CONCURRENT)
    list(APPEND MODULE_LINK Qt::Concurrent)
endif()

setup_module()

if (MUE_BUILD_PROJECT_TESTS)
    add_subdirectory(tests)
endif()
