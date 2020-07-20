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

set(MODULE audio_engine)

include(${CMAKE_CURRENT_LIST_DIR}/3rdparty/soloud.cmake)

include(${PROJECT_SOURCE_DIR}/build/platform.cmake)
if (PLATFORM_WINDOWS)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/platform/win/winaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/platform/win/winaudiodriver.h
    )
elseif(PLATFORM_LINUX)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/platform/lin/linuxaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/platform/lin/linuxaudiodriver.h
    )
elseif(PLATFORM_OSX)
    message(WARNING "Not implemented audio driver for OSX")
    set(DRIVER_SRC
    )
endif()

set(MODULE_SRC
    ${SOLOUD_SRC}
    ${DRIVER_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/audioenginemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audioenginemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiodriver.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioengine.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiosource.h
    ${CMAKE_CURRENT_LIST_DIR}/audioerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/audiotypes.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/sinesource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sinesource.h
    ${CMAKE_CURRENT_LIST_DIR}/midisource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/midisource.h
    ${CMAKE_CURRENT_LIST_DIR}/rpcmidistream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rpcmidistream.h
    ${CMAKE_CURRENT_LIST_DIR}/loopsource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loopsource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioplayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/workertypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/rpcstreambase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/rpcstreambase.h
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

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
