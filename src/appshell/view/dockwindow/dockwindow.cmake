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

set (DOCK_LIBS
    kddockwidgets
)

set (DOCKWINDOW_SRC
    ${CMAKE_CURRENT_LIST_DIR}/docksetup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/docksetup.h
    ${CMAKE_CURRENT_LIST_DIR}/docktypes.h
    ${CMAKE_CURRENT_LIST_DIR}/dockwindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dockwindow.h
    ${CMAKE_CURRENT_LIST_DIR}/dockpage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dockpage.h
    ${CMAKE_CURRENT_LIST_DIR}/dockpanel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dockpanel.h
    ${CMAKE_CURRENT_LIST_DIR}/dockpanelholder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dockpanelholder.h
    ${CMAKE_CURRENT_LIST_DIR}/dockstatusbar.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dockstatusbar.h
    ${CMAKE_CURRENT_LIST_DIR}/docktoolbar.cpp
    ${CMAKE_CURRENT_LIST_DIR}/docktoolbar.h
    ${CMAKE_CURRENT_LIST_DIR}/docktoolbarholder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/docktoolbarholder.h
    ${CMAKE_CURRENT_LIST_DIR}/dockcentral.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dockcentral.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/idockwindow.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dockbase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dockbase.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dropindicators.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dropindicators.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dropindicatorswindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dropindicatorswindow.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dockseparator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dockseparator.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dockframemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dockframemodel.h
    ${DOCKWINDOW_PLATFORM_SRC}
)

