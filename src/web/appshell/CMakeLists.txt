# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited and others
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

muse_create_module(appshell)

target_sources(appshell PRIVATE
    appshellmodule.cpp
    appshellmodule.h
    internal/startupscenario.cpp
    internal/startupscenario.h
    view/appmenumodel.cpp
    view/appmenumodel.h
    view/navigableappmenumodel.cpp
    view/navigableappmenumodel.h
)

# Shared with the desktop appshell
set(APPSHELL_SRC_DIR ${PROJECT_SOURCE_DIR}/src/appshell)
target_sources(appshell PRIVATE
    ${APPSHELL_SRC_DIR}/iappshellconfiguration.h
    ${APPSHELL_SRC_DIR}/appshelltypes.h
    ${APPSHELL_SRC_DIR}/internal/appshellconfiguration.cpp
    ${APPSHELL_SRC_DIR}/internal/appshellconfiguration.h
    ${APPSHELL_SRC_DIR}/internal/istartupscenario.h
)
target_include_directories(appshell PRIVATE ${APPSHELL_SRC_DIR})

qt_add_resources(appshell_rcc appshell.qrc)
target_sources(appshell PRIVATE ${appshell_rcc})

target_link_libraries(appshell PRIVATE
    muse::ui
    muse::uicomponents
    Qt::Quick
)

# we use QKeyMapper to fix https://github.com/musescore/MuseScore/issues/10181
find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
target_link_libraries(appshell PRIVATE Qt6::GuiPrivate)
