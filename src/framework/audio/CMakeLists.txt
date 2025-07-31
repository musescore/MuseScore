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

declare_module(muse_audio)
set(MODULE_ALIAS muse::audio)

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
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/audiodeviceslistener.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/audiodeviceslistener.h
    )
elseif(OS_IS_LIN OR OS_IS_FBSD)
    if (MUSE_MODULE_AUDIO_JACK)
        set(DRIVER_SRC
            ${CMAKE_CURRENT_LIST_DIR}/internal/platform/jack/jackaudiodriver.cpp
            ${CMAKE_CURRENT_LIST_DIR}/internal/platform/jack/jackaudiodriver.h
            ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxaudiodriver.cpp
            ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxaudiodriver.h
            ${CMAKE_CURRENT_LIST_DIR}/internal/platform/jack/audiodeviceslistener.cpp
            ${CMAKE_CURRENT_LIST_DIR}/internal/platform/jack/audiodeviceslistener.h
        )
    else()
        set(DRIVER_SRC
            ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxaudiodriver.cpp
            ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxaudiodriver.h
            ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/audiodeviceslistener.cpp
            ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/audiodeviceslistener.h
        )
    endif()
elseif(OS_IS_MAC)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/osxaudiodriver.mm
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/osxaudiodriver.h
    )

    set_source_files_properties(
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/osxaudiodriver.mm
        PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION ON
        SKIP_PRECOMPILE_HEADERS ON
    )
elseif(OS_IS_WASM)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/web/webaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/web/webaudiodriver.h
    )
endif()

include(common/audio_common.cmake)
include(main/audio_main.cmake)

set(MODULE_SRC

    ${AUDIO_COMMON_SRC}
    ${AUDIO_MAIN_SRC}

    # Temporary
    ${CMAKE_CURRENT_LIST_DIR}/audiotypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiosanitizer.h

    # Common public

    ${CMAKE_CURRENT_LIST_DIR}/audioerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudioconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiothreadsecurer.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiostream.h
    ${CMAKE_CURRENT_LIST_DIR}/ifxprocessor.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiodriver.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiosource.h
    ${CMAKE_CURRENT_LIST_DIR}/soundfonttypes.h
    ${CMAKE_CURRENT_LIST_DIR}/audioutils.h
    ${CMAKE_CURRENT_LIST_DIR}/isynthresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/ifxresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/abstractfxresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/abstractfxresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/isoundfontrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/isynthesizer.h
    ${CMAKE_CURRENT_LIST_DIR}/devtools/inputlag.h

    # Common internal
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audioconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiothreadsecurer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiothreadsecurer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiobuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiobuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiothread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiothread.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/soundfontrepository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/soundfontrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiooutputdevicecontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiooutputdevicecontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractsynthesizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractsynthesizer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstracteventsequencer.h

    # Driver
    ${DRIVER_SRC}

    # Worker
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/iworkerplayback.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/workerplayback.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/workerplayback.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/workerchannelcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/workerchannelcontroller.h
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
    ${CMAKE_CURRENT_LIST_DIR}/internal/worker/iaudioengine.h
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
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/musefxresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/musefxresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/allpassdispersion.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/allpassmodulateddelay.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/circularsamplebuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/iirbiquadfilter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/ivndecorrelation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/ivndecorrelation.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/reverbfilters.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/reverbmatrices.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/sampledelay.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/simdtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/smoothlinearvalue.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/sparsefirfilter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/vectorops.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/reverbprocessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/reverbprocessor.h

    # Synthesizers
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/soundmapping.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/sfcachedloader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidsynth.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidsynth.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidsequencer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidsequencer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidsoundfontparser.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/fluidsynth/fluidsoundfontparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/synthresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/synthesizers/synthresolver.h
    )

# fluidsynth dependency
include(SetupSndFile)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/fluidsynth fluidsynth)

set(MODULE_LINK
    fluidsynth
    )

if (MUSE_MODULE_AUDIO_EXPORT)
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

    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/lame lame EXCLUDE_FROM_ALL)
    set(MODULE_LINK
        ${MODULE_LINK}
        lame
    )

    include(cmake/SetupOpusEnc.cmake)
    set(MODULE_LINK
        ${MODULE_LINK}
        ${LIBOPUSENC_TARGETS}
    )

    include(cmake/SetupFlac.cmake)
    set(MODULE_LINK
        ${MODULE_LINK}
        ${FLAC_TARGETS}
    )
endif()

if (ARCH_IS_X86_64)
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/simdtypes_sse2.h
        )
elseif (ARCH_IS_AARCH64)
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/simdtypes_neon.h
        )
else ()
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/fx/reverb/simdtypes_scalar.h
        )
endif()

if (MUSE_MODULE_AUDIO_JACK)
    find_package(Jack REQUIRED)
    set(MODULE_INCLUDE_PRIVATE ${MODULE_INCLUDE_PRIVATE} ${JACK_INCLUDE_DIRS} )
    set(MODULE_LINK ${MODULE_LINK} ${JACK_LIBRARIES} pthread )
    find_package(ALSA REQUIRED)
    set(MODULE_INCLUDE_PRIVATE ${MODULE_INCLUDE_PRIVATE} ${ALSA_INCLUDE_DIRS} )
    set(MODULE_LINK ${MODULE_LINK} ${ALSA_LIBRARIES} pthread )
else ()
    if (OS_IS_MAC)
        find_library(AudioToolbox NAMES AudioToolbox)
        find_library(CoreAudio NAMES CoreAudio)
        set(MODULE_LINK ${MODULE_LINK} ${AudioToolbox} ${CoreAudio})
    elseif (OS_IS_WIN)
        set(MODULE_LINK ${MODULE_LINK} winmm mmdevapi mfplat
            WindowsApp) # needed for C++/WinRT
    elseif (OS_IS_LIN)
        find_package(ALSA REQUIRED)
        set(MODULE_INCLUDE_PRIVATE ${MODULE_INCLUDE_PRIVATE} ${ALSA_INCLUDE_DIRS} )
        set(MODULE_LINK ${MODULE_LINK} ${ALSA_LIBRARIES} pthread )
    endif()
endif()

set(MODULE_QRC audio.qrc)

set(MODULE_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

if (OS_IS_WIN)
    # Prevent mysterious compile errors in debug builds
    set_source_files_properties(
        ${CMAKE_CURRENT_LIST_DIR}/internal/worker/audiostream.cpp
        PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION ON
    )
endif()

setup_module()

if (MUSE_MODULE_AUDIO_TESTS)
    add_subdirectory(tests)
endif()
