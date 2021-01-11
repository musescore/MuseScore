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

set(MODULE vst3_sdk)

if (NOT VST3_SDK_PATH)
    message(FATAL_ERROR "VST3_SDK path is not setted")
endif()

set(MODULE_SRC
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
    ${VST3_SDK_PATH}/base/thread/source/flock.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/funknown.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/coreiids.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/funknown.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/ustring.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/ipluginbase.h
    ${VST3_SDK_PATH}/pluginterfaces/vst/ivstcomponent.h
    )

set(MODULE_INCLUDE
    ${VST3_SDK_PATH}
    )

string(TOLOWER ${CMAKE_BUILD_TYPE} build_mode )
if (build_mode STREQUAL "debug")
    set(MODULE_DEF -D_DEBUG)
endif()

set(MODULE_USE_UNITY_NONE ON) # not linking
include(${PROJECT_SOURCE_DIR}/build/module.cmake)

target_no_warning(${MODULE} -w)
