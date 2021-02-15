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

set(MODULE_QRC vst.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

if (NOT VST3_SDK_PATH)
    message(FATAL_ERROR "VST3_SDK path is not defined")
endif()

#need to build vst in debug mode
if (${APPLE})
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        add_definitions(-D_DEBUG)
    endif()
endif()

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/vstmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vstmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/ivstinstanceregister.h
    ${CMAKE_CURRENT_LIST_DIR}/vsttypes.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pluginlistmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pluginlistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/vstinstanceeditormodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/vstinstanceeditormodel.cpp

    ${CMAKE_CURRENT_LIST_DIR}/internal/vstsynthesizer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstsynthesizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstscanner.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstscanner.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/pluginloader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/pluginloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/plugin.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/plugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/eventlist.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/eventlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/plugininstance.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/plugininstance.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/pluginparameter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/pluginparameter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/message.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/message.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/attributevalue.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/attributevalue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/attributelist.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/attributelist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/hostapplication.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/hostapplication.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/connectionproxy.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/connectionproxy.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/plugineditorview.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/plugineditorview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstinstanceregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/vstinstanceregister.cpp

    ${CMAKE_CURRENT_LIST_DIR}/devtools/vstdevtools.h
    ${CMAKE_CURRENT_LIST_DIR}/devtools/vstdevtools.cpp
    )

set(MODULE_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}
    ${VST3_SDK_PATH}
    )

set(MODULE_LINK
    vst3_sdk
    )

string(TOLOWER ${CMAKE_BUILD_TYPE} build_mode )
if (build_mode STREQUAL "debug")
    set(MODULE_DEF -D_DEBUG)
endif()

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
