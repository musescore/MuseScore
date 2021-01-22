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

include(GetPlatformInfo)
if (OS_IS_WIN)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/wincoreaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/wincoreaudiodriver.h
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
elseif(OS_IS_WASM)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/web/webaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/web/webaudiodriver.h
    )
endif()

include(${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/zerberus/internal/zerberus.cmake)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/fluidsynth fluidsynth)

set(MODULE_SRC
    ${DRIVER_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/audiomodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audiomodule.h
    ${CMAKE_CURRENT_LIST_DIR}/audioerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioengine.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioinsert.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiostream.h
    ${CMAKE_CURRENT_LIST_DIR}/isequencer.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiosource.h
    ${CMAKE_CURRENT_LIST_DIR}/iplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/imidiplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiodriver.h
    ${CMAKE_CURRENT_LIST_DIR}/imixer.h
    ${CMAKE_CURRENT_LIST_DIR}/imixerchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiobuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractaudiosource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractaudiosource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractaudioinsert.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractaudioinsert.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/samplerateconvertor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/samplerateconvertor.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiostream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiostream.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioplayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiplayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/sinesource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/sinesource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/noisesource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/noisesource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiobuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiobuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/sequencer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/sequencer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/mixer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/mixer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/clock.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/clock.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/mixerchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/mixerchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/equaliser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/equaliser.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiothread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiothread.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiosanitizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiosanitizer.h
    ${CMAKE_CURRENT_LIST_DIR}/devtools/audioenginedevtools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/devtools/audioenginedevtools.h

    # rpc
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpctypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/irpcchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/queuedrpcchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/queuedrpcchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpccontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpccontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpcsequencer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpcsequencer.h

    # synthesizers
    ${ZERBERUS_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/synthtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/isynthesizer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/isynthesizersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/isoundfontsprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidsynth.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidsynth.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/zerberus/zerberussynth.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/zerberus/zerberussynth.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/synthesizercontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/synthesizercontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/synthesizersregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/synthesizersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/soundfontsprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/soundfontsprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/view/synthssettingsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/synthssettingsmodel.h
    )

set(FLUIDSYNTH_DIR ${PROJECT_SOURCE_DIR}/thirdparty/fluidsynth/fluidsynth-2.1.4)
set (FLUIDSYNTH_INC
    ${FLUIDSYNTH_DIR}/include
    ${FLUIDSYNTH_DIR}/src
    ${FLUIDSYNTH_DIR}/src/external
    ${FLUIDSYNTH_DIR}/src/utils
    ${FLUIDSYNTH_DIR}/src/midi
    ${FLUIDSYNTH_DIR}/src/rvoice
    ${FLUIDSYNTH_DIR}/src/sfloader
    ${FLUIDSYNTH_DIR}/src/bindings
    ${FLUIDSYNTH_DIR}/src/synth
    ${FLUIDSYNTH_DIR}/src/drivers
    )

set (MODULE_INCLUDE
    ${FLUIDSYNTH_INC}
    )

if(OS_IS_WIN)
    set(MODULE_LINK winmm )
endif()

if(OS_IS_LIN)
    find_package(ALSA REQUIRED)
    set(MODULE_LINK ${ALSA_LIBRARIES} pthread )
    set(MODULE_INCLUDE ${MODULE_INCLUDE} ${ALSA_INCLUDE_DIRS} )
endif()

set(MODULE_LINK ${MODULE_LINK} fluidsynth )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
