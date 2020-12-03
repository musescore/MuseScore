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

set(MODULE_QRC vst.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

if (NOT VST3_SDK_PATH)
    message(FATAL_ERROR "VST3_SDK path is not setted")
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

    #VST SDK
    ${VST3_SDK_PATH}/base/source/fobject.cpp
    ${VST3_SDK_PATH}/base/source/baseiids.cpp
    ${VST3_SDK_PATH}/base/source/fbuffer.cpp
    ${VST3_SDK_PATH}/base/source/fdebug.cpp
    ${VST3_SDK_PATH}/base/source/fdynlib.cpp
    ${VST3_SDK_PATH}/base/source/fobject.cpp
    ${VST3_SDK_PATH}/base/source/fstreamer.cpp
    ${VST3_SDK_PATH}/base/source/fstring.cpp
    ${VST3_SDK_PATH}/base/source/timer.cpp
    ${VST3_SDK_PATH}/base/source/updatehandler.cpp
    #${VST3_SDK_PATH}/base/thread/source/fcondition.cpp
    ${VST3_SDK_PATH}/base/thread/source/flock.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/funknown.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/conststringtable.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/coreiids.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/funknown.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/ustring.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/ipluginbase.h
    ${VST3_SDK_PATH}/pluginterfaces/vst/ivstcomponent.h
    )

set(MODULE_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}
    ${VST3_SDK_PATH}
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
