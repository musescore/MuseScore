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

set(MODULE audio)

include(${PROJECT_SOURCE_DIR}/thirdparty/soloud/soloud.cmake)

include(GetPlatformInfo)
if (OS_IS_WIN)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmmdriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmmdriver.h
    )
elseif(OS_IS_LIN)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxaudiodriver.h
    )
elseif(OS_IS_MAC)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/osxaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/osxaudiodriver.h
    )
endif()

set(MODULE_SRC
    ${SOLOUD_SRC}
    ${DRIVER_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/audiomodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audiomodule.h
    ${CMAKE_CURRENT_LIST_DIR}/audioerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/audiotypes.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioengine.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiosource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/iaudiodriver.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/sinesource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/sinesource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/imidisource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/midisource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/midisource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpcmidisource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpcmidisource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/loopsource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/loopsource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiointernaltypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioplayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/workertypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/rpcsourcebase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/rpcsourcebase.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/irpcaudiostreamchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audiothreadstreamworker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audiothreadstreamworker.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/queuedrpcstreamchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/queuedrpcstreamchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/rpcstreamchannelbase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/rpcstreamchannelbase.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/rpcstreamcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/rpcstreamcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/midistreamcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/midistreamcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/wavstreamcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/wavstreamcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/streamcontrollerbase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/streamcontrollerbase.h
    ${CMAKE_CURRENT_LIST_DIR}/devtools/audioenginedevtools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/devtools/audioenginedevtools.h
    )

set(MODULE_DEF
    ${SOLOUD_DEF}
    )

set(MODULE_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}
    ${SOLOUD_INC}
    )

if(OS_IS_WIN)
    set(MODULE_LINK winmm )
endif()

if(OS_IS_LIN)
    find_package(ALSA REQUIRED)
    set(MODULE_LINK ${ALSA_LIBRARIES} pthread )
    set(MODULE_INCLUDE ${MODULE_INCLUDE} ${ALSA_INCLUDE_DIRS} )
endif()

set(MODULE_USE_PCH_NONE ON) # strange error
set(MODULE_USE_UNITY_NONE ON) # soloud not compile
include(${PROJECT_SOURCE_DIR}/build/module.cmake)

# for soloud
target_no_warning(${MODULE} -Wno-unused-parameter)
target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -Wno-uninitialized)
target_no_warning(${MODULE} -Wno-array-bounds)
target_no_warning(${MODULE} -WMSVC-no-hides-previous)
