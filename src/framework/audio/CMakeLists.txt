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

set(MODULE audio)

include(GetPlatformInfo)
if (OS_IS_WIN)
    set(DRIVER_SRC
        #${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmmdriver.cpp
        #${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmmdriver.h
        #${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/wincoreaudiodriver.cpp
        #${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/wincoreaudiodriver.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/wasapiaudioclient.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/wasapiaudioclient.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/wasapitypes.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/wasapiaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/wasapiaudiodriver.h
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

add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/fluidsynth fluidsynth)

set(MODULE_SRC

    # Common public
    ${CMAKE_CURRENT_LIST_DIR}/audiomodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audiomodule.h
    ${CMAKE_CURRENT_LIST_DIR}/audioerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiothreadsecurer.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiostream.h
    ${CMAKE_CURRENT_LIST_DIR}/abstractsynthesizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/abstractsynthesizer.h
    ${CMAKE_CURRENT_LIST_DIR}/abstracteventsequencer.h
    ${CMAKE_CURRENT_LIST_DIR}/ifxprocessor.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiodriver.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiosource.h
    ${CMAKE_CURRENT_LIST_DIR}/synthtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/audiotypes.h
    ${CMAKE_CURRENT_LIST_DIR}/iplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/itracks.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiooutput.h
    ${CMAKE_CURRENT_LIST_DIR}/isynthresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/ifxresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/iplayback.h
    ${CMAKE_CURRENT_LIST_DIR}/isoundfontrepository.h

    # Common internal
    ${CMAKE_CURRENT_LIST_DIR}/internal/iaudiobuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiothreadsecurer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiothreadsecurer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiobuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiobuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiothread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiothread.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiosanitizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiosanitizer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/soundfontrepository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/soundfontrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiooutputdevicecontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiooutputdevicecontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiodeviceslistener.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiodeviceslistener.h

    # Driver
    ${DRIVER_SRC}

    # Worker
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/playback.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/playback.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/abstractaudiosource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/abstractaudiosource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/samplerateconvertor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/samplerateconvertor.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audiostream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audiostream.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/eventaudiosource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/eventaudiosource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/sinesource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/sinesource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/noisesource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/noisesource.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audioengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audioengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/tracksequence.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/tracksequence.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/mixer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/mixer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/mixerchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/mixerchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/iclock.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/clock.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/clock.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/equaliser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/equaliser.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/trackshandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/trackshandler.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/playerhandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/playerhandler.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audiooutputhandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audiooutputhandler.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/itracksequence.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/igettracksequence.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/igettracks.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/isequenceplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/isequenceio.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/sequenceplayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/sequenceplayer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/sequenceio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/sequenceio.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/track.h

    # DSP
    ${CMAKE_CURRENT_LIST_DIR}/internal/dsp/envelopefilterconfig.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dsp/compressor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dsp/compressor.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dsp/limiter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dsp/limiter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dsp/audiomathutils.h

    # fx
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/fxresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/fxresolver.h

    # Synthesizers
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/soundmapping.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/sfcachedloader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidsynth.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidsynth.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidsequencer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidsequencer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/synthresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/synthresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/view/synthssettingsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/synthssettingsmodel.h

    # DevTools
    ${CMAKE_CURRENT_LIST_DIR}/devtools/waveformmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/devtools/waveformmodel.h
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

set(MODULE_LINK
    fluidsynth
    )

if (ENABLE_AUDIO_EXPORT)
    set(MODULE_SRC
        ${MODULE_SRC}

        # Encoders
        ${CMAKE_CURRENT_LIST_DIR}/internal/encoders/mp3encoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/encoders/mp3encoder.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/encoders/oggencoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/encoders/oggencoder.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/encoders/flacencoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/encoders/flacencoder.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/encoders/wavencoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/encoders/wavencoder.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/encoders/abstractaudioencoder.h

        # SoundTracks
        ${CMAKE_CURRENT_LIST_DIR}/internal/soundtracks/soundtrackwriter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/soundtracks/soundtrackwriter.h
        )

    add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/lame lame)
    add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/opusenc opusenc)
    add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/flac flac)

    set(MODULE_LINK ${MODULE_LINK} lame opusenc flac)
endif()

if (OS_IS_MAC)
    find_library(AudioToolbox NAMES AudioToolbox)
    set(MODULE_LINK ${MODULE_LINK} ${AudioToolbox})
elseif (OS_IS_WIN)
    set(MODULE_LINK ${MODULE_LINK} winmm mmdevapi mfplat)
elseif (OS_IS_LIN)
    find_package(ALSA REQUIRED)
    set(MODULE_INCLUDE ${MODULE_INCLUDE} ${ALSA_INCLUDE_DIRS} )
    set(MODULE_LINK ${MODULE_LINK} ${ALSA_LIBRARIES} pthread )
endif()

set(MODULE_QRC audio.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
