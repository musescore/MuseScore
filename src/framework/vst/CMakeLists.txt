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

declare_module(muse_vst)
set(MODULE_ALIAS muse::vst)

add_subdirectory("sdk")

include(GetPlatformInfo)

set(MODULE_QRC vst.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/ivstmodulesrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/ivstconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/ivstinstancesregister.h
    ${CMAKE_CURRENT_LIST_DIR}/ivstplugininstance.h
    ${CMAKE_CURRENT_LIST_DIR}/vsterrors.h
    ${CMAKE_CURRENT_LIST_DIR}/vsttypes.h
    ${CMAKE_CURRENT_LIST_DIR}/vstmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vstmodule.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/vstactionscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstactionscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstmodulesrepository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstmodulesrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstinstancesregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstinstancesregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstplugininstance.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstplugininstance.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstcomponenthandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstcomponenthandler.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstaudioclient.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstaudioclient.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstpluginprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstpluginprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstpluginsscanner.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstpluginsscanner.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstpluginmetareader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstpluginmetareader.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/synth/vstsynthesiser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synth/vstsynthesiser.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synth/vstiresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synth/vstiresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synth/vstsequencer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synth/vstsequencer.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/vstfxprocessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/vstfxprocessor.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/vstfxresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/vstfxresolver.h

    ${CMAKE_CURRENT_LIST_DIR}/view/vstviewdialog_qwidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/vstviewdialog_qwidget.h
    ${CMAKE_CURRENT_LIST_DIR}/view/vstview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/vstview.h
    )

if(OS_IS_LIN OR OS_IS_FBSD)
    set(MODULE_SRC
        ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/linux/runloop.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/linux/runloop.h
    )
endif()

set(MODULE_INCLUDE
    ${MUSE_MODULE_VST_VST3_SDK_PATH}
    )

set(MODULE_LINK
    vst_sdk_3
    )

if (OS_IS_LIN)
    set(MODULE_LINK
        ${MODULE_LINK}
        stdc++fs
        )
endif()

if (QT_SUPPORT)
    list(APPEND MODULE_LINK Qt::Qml Qt::Widgets)
endif()

setup_module()

target_no_warning(${MODULE} -Wno-deprecated-declarations)
