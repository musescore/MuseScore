#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2020 MuseScore BVBA and others
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

set(MODULE vst)

add_subdirectory("sdk")

include(GetPlatformInfo)

set(MODULE_QRC vst.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/ivstpluginrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/ivstconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/vsterrors.h
    ${CMAKE_CURRENT_LIST_DIR}/vsttypes.h
    ${CMAKE_CURRENT_LIST_DIR}/vstmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vstmodule.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/vstpluginrepository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstpluginrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstplugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstplugin.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synth/vstaudioclient.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synth/vstaudioclient.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synth/vstsynthesiser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synth/vstsynthesiser.h
    ${CMAKE_CURRENT_LIST_DIR}/devtools/vstpluginlistmodelexample.cpp
    ${CMAKE_CURRENT_LIST_DIR}/devtools/vstpluginlistmodelexample.h
    ${CMAKE_CURRENT_LIST_DIR}/view/vstplugineditorview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/vstplugineditorview.h
    )

set(MODULE_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}
    ${VST3_SDK_PATH}
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

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/VST3)
