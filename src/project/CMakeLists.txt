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

muse_create_module(project)

include(GetPlatformInfo)

target_sources(project PRIVATE
    projectmodule.cpp
    projectmodule.h
    projecterrors.h
    projectextensionpoints.h
    iexportprojectscenario.h
    inotationproject.h
    iprojectcreator.h
    inotationreader.h
    inotationreadersregister.h
    inotationwriter.h
    inotationwritersregister.h
    iprojectaudiosettings.h
    iprojectfilescontroller.h
    iprojectconfiguration.h
    irecentfilescontroller.h
    imscmetareader.h
    iprojectautosaver.h
    iprojectrwregister.h
    iprojectwriter.h

    types/projecttypes.h
    types/projectmeta.h

    internal/iprojectmigrator.h
    internal/notationproject.cpp
    internal/notationproject.h
    internal/projectcreator.cpp
    internal/projectcreator.h
    internal/projectaudiosettings.cpp
    internal/projectaudiosettings.h
    internal/notationreadersregister.cpp
    internal/notationreadersregister.h
    internal/notationwritersregister.cpp
    internal/notationwritersregister.h
    internal/projectautosaver.cpp
    internal/projectautosaver.h
    internal/projectactionscontroller.cpp
    internal/projectactionscontroller.h
    internal/engravingpluginapihelper.cpp
    internal/engravingpluginapihelper.h
    internal/projectuiactions.cpp
    internal/projectuiactions.h
    internal/projectconfiguration.cpp
    internal/projectconfiguration.h
    internal/exporttype.cpp
    internal/exporttype.h
    internal/exportprojectscenario.cpp
    internal/exportprojectscenario.h
    internal/iopensaveprojectscenario.h
    internal/opensaveprojectscenario.cpp
    internal/opensaveprojectscenario.h
    internal/recentfilescontroller.cpp
    internal/recentfilescontroller.h
    internal/mscmetareader.cpp
    internal/mscmetareader.h
    internal/itemplatesrepository.h
    internal/templatesrepository.cpp
    internal/templatesrepository.h
    internal/mdlmigrator.cpp
    internal/mdlmigrator.h
    internal/projectmigrator.cpp
    internal/projectmigrator.h
    internal/projectfileinfoprovider.cpp
    internal/projectfileinfoprovider.h
    internal/projectrwregister.cpp
    internal/projectrwregister.h
)

if (OS_IS_MAC)
    target_sources(project PRIVATE
        internal/platform/macos/macosrecentfilescontroller.mm
        internal/platform/macos/macosrecentfilescontroller.h
    )
    # Don't mix C++ and Objective-C++ in Unity Build
    set_source_files_properties(
        internal/platform/macos/macosrecentfilescontroller.mm
        PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION ON
        SKIP_PRECOMPILE_HEADERS ON
    )
elseif (OS_IS_WIN)
    target_sources(project PRIVATE
        internal/platform/windows/windowsrecentfilescontroller.cpp
        internal/platform/windows/windowsrecentfilescontroller.h
    )
endif()

target_link_libraries(project PRIVATE notation)

if (QT_ADD_CONCURRENT)
    target_link_libraries(project PRIVATE Qt::Concurrent)
endif()

if (MUE_BUILD_PROJECT_TESTS)
    add_subdirectory(tests)
endif()

if (MUE_BUILD_PROJECT_QML)
    add_subdirectory(qml/MuseScore/Project)
endif()
