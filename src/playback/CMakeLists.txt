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

declare_module(playback)

set(MODULE_QRC
    ${CMAKE_CURRENT_LIST_DIR}/playback.qrc
    )

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    #public
    ${CMAKE_CURRENT_LIST_DIR}/playbackmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playbackmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iplaybackcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/iplaybackconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/isoundprofilesrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/playbacktypes.h

    #internals
    ${CMAKE_CURRENT_LIST_DIR}/internal/playbackcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/playbackcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/playbackuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/playbackuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/playbackconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/playbackconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/soundprofilesrepository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/soundprofilesrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/drumsetloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/drumsetloader.h

    #view
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/mixerchannelitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/mixerchannelitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/abstractaudioresourceitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/abstractaudioresourceitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/outputresourceitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/outputresourceitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/inputresourceitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/inputresourceitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/auxsenditem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/auxsenditem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/msbasicpresetscategories.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/soundflag/soundflagsettingsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/soundflag/soundflagsettingsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/onlinesoundsstatusmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/onlinesoundsstatusmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/mixerpanelmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/mixerpanelmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/mixerpanelcontextmenumodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/mixerpanelcontextmenumodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/playbacktoolbarmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/playbacktoolbarmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/playbackloadingmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/playbackloadingmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/soundprofilesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/soundprofilesmodel.h
    )

if (QT_SUPPORT)
    list(APPEND MODULE_LINK Qt::Quick)
endif()

setup_module()

if (MUE_BUILD_PLAYBACK_TESTS)
    add_subdirectory(tests)
endif()
