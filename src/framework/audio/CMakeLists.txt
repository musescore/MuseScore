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
        #${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmmdriver.cpp
        #${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmmdriver.h
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

    # Conflicted with AudioBuffer
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/osxaudiodriver.cpp
                                PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
elseif(OS_IS_WASM)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/web/webaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/web/webaudiodriver.h
    )
endif()

include(${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/zerberus/internal/zerberus.cmake)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/fluidsynth fluidsynth)

set(MODULE_SRC

    # Common public
    ${CMAKE_CURRENT_LIST_DIR}/audiomodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audiomodule.h
    ${CMAKE_CURRENT_LIST_DIR}/audioerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/isequencer.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiostream.h
    ${CMAKE_CURRENT_LIST_DIR}/isynthesizer.h
    ${CMAKE_CURRENT_LIST_DIR}/isynthesizersregister.h
    ${CMAKE_CURRENT_LIST_DIR}/isoundfontsprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiodriver.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiosource.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioprocessor.h
    ${CMAKE_CURRENT_LIST_DIR}/synthtypes.h

    # Common internal
    ${CMAKE_CURRENT_LIST_DIR}/internal/iaudiobuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiobuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiobuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiothread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiothread.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiosanitizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiosanitizer.h

    # Driver
    ${DRIVER_SRC}

    # Worker
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/iaudioengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/imixer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/imixerchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/iplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/iaudioplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/imidiplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/abstractaudiosource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/abstractaudiosource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/samplerateconvertor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/samplerateconvertor.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audiostream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audiostream.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audioplayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audioplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/midiplayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/midiplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/sinesource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/sinesource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/noisesource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/noisesource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audioengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audioengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/sequencer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/sequencer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/mixer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/mixer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/mixerchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/mixerchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/clock.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/clock.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/equaliser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/equaliser.h

    # Synthesizers
    ${ZERBERUS_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/sanitysynthesizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/sanitysynthesizer.h
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

    # Rpc
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpctypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/irpcchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/irpccontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/queuedrpcchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/queuedrpcchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpccontrollers.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpccontrollers.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpccontrollerbase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpccontrollerbase.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpcdevtoolscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpcdevtoolscontroller.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpcaudioenginecontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpcaudioenginecontroller.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpcsequencer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpcsequencer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpcsequencercontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpc/rpcsequencercontroller.h

    # DevTools
    ${CMAKE_CURRENT_LIST_DIR}/devtools/audioenginedevtools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/devtools/audioenginedevtools.h
    ${CMAKE_CURRENT_LIST_DIR}/devtools/audiodevtoolscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/devtools/audiodevtoolscontroller.h
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

set(MODULE_QRC audio.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
