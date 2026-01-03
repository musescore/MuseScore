# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(muse_audio_engine)

target_include_directories(muse_audio_engine PRIVATE ..)

if (WEBENGINE_FACADE_MODE)
    # Web Facade
    target_sources(muse_audio_engine PRIVATE
        platform/web/networksfloader.cpp
        platform/web/networksfloader.h
    )

else()
    # Real engine
    target_sources(muse_audio_engine PRIVATE
        iaudioengineconfiguration.h
        ienginecontroller.h
        iengineplayback.h
        iaudioengine.h
        iaudiosource.h
        iclock.h
        isequenceio.h
        itracksequence.h
        isequenceplayer.h
        ifxprocessor.h
        ifxresolver.h
        isynthesizer.h
        isynthresolver.h
        isoundfontrepository.h

        # internal
        internal/audioengineconfiguration.cpp
        internal/audioengineconfiguration.h
        internal/enginecontroller.cpp
        internal/enginecontroller.h
        internal/engineplayback.cpp
        internal/engineplayback.h
        internal/enginerpccontroller.cpp
        internal/enginerpccontroller.h
        internal/audioengine.cpp
        internal/audioengine.h
        internal/audiobuffer.cpp
        internal/audiobuffer.h
        internal/mixer.cpp
        internal/mixer.h
        internal/mixerchannel.cpp
        internal/mixerchannel.h
        internal/clock.cpp
        internal/clock.h
        internal/sequenceio.cpp
        internal/sequenceio.h
        internal/tracksequence.cpp
        internal/tracksequence.h
        internal/igettracks.h
        internal/igetplaybackposition.h
        internal/sequenceplayer.cpp
        internal/sequenceplayer.h
        internal/samplerateconvertor.cpp
        internal/samplerateconvertor.h
        internal/track.h
        internal/abstractaudiosource.cpp
        internal/abstractaudiosource.h
        internal/eventaudiosource.cpp
        internal/eventaudiosource.h
        internal/sinesource.cpp
        internal/sinesource.h
        # internal/noisesource.cpp
        # internal/noisesource.h
        internal/abstracteventsequencer.h

        # DSP
        internal/dsp/envelopefilterconfig.h
        internal/dsp/compressor.cpp
        internal/dsp/compressor.h
        internal/dsp/limiter.cpp
        internal/dsp/limiter.h
        internal/dsp/audiomathutils.h

        # FX
        internal/fx/abstractfxresolver.cpp
        internal/fx/abstractfxresolver.h
        internal/fx/fxresolver.cpp
        internal/fx/fxresolver.h
        internal/fx/musefxresolver.cpp
        internal/fx/musefxresolver.h
        internal/fx/equaliser.cpp
        internal/fx/equaliser.h
        internal/fx/reverb/allpassdispersion.h
        internal/fx/reverb/allpassmodulateddelay.h
        internal/fx/reverb/circularsamplebuffer.h
        internal/fx/reverb/iirbiquadfilter.h
        internal/fx/reverb/ivndecorrelation.cpp
        internal/fx/reverb/ivndecorrelation.h
        internal/fx/reverb/reverbfilters.h
        internal/fx/reverb/reverbmatrices.h
        internal/fx/reverb/sampledelay.h
        internal/fx/reverb/simdtypes.h
        internal/fx/reverb/smoothlinearvalue.h
        internal/fx/reverb/sparsefirfilter.h
        internal/fx/reverb/vectorops.h
        internal/fx/reverb/reverbprocessor.cpp
        internal/fx/reverb/reverbprocessor.h

        # Synthesizers
        internal/synthesizers/synthresolver.cpp
        internal/synthesizers/synthresolver.h
        internal/synthesizers/abstractsynthesizer.cpp
        internal/synthesizers/abstractsynthesizer.h
        internal/synthesizers/soundfontrepository.cpp
        internal/synthesizers/soundfontrepository.h
        internal/synthesizers/fluidsynth/soundmapping.h
        internal/synthesizers/fluidsynth/sfcachedloader.h
        internal/synthesizers/fluidsynth/fluidsynth.cpp
        internal/synthesizers/fluidsynth/fluidsynth.h
        internal/synthesizers/fluidsynth/fluidsequencer.cpp
        internal/synthesizers/fluidsynth/fluidsequencer.h
        internal/synthesizers/fluidsynth/fluidresolver.cpp
        internal/synthesizers/fluidsynth/fluidresolver.h
        internal/synthesizers/fluidsynth/fluidsoundfontparser.h
        internal/synthesizers/fluidsynth/fluidsoundfontparser.cpp

        # codecs
        internal/codecs/vorbisdecoder.cpp
        internal/codecs/vorbisdecoder.h
    )

    if (OS_IS_WASM)
        target_sources(muse_audio_engine PRIVATE
            platform/web/networksfloader.cpp
            platform/web/networksfloader.h
        )
    else()
        target_sources(muse_audio_engine PRIVATE
            platform/general/generalaudioworker.cpp
            platform/general/generalaudioworker.h
        )
    endif()

    if (ARCH_IS_X86_64)
        target_sources(muse_audio_engine PRIVATE
            internal/fx/reverb/simdtypes_sse2.h
        )
    elseif (ARCH_IS_AARCH64)
        target_sources(muse_audio_engine PRIVATE
            internal/fx/reverb/simdtypes_neon.h
        )
    else ()
        target_sources(muse_audio_engine PRIVATE
            internal/fx/reverb/simdtypes_scalar.h
        )
    endif()

    add_subdirectory(../thirdparty/fluidsynth fluidsynth)
    target_link_libraries(muse_audio_engine PRIVATE fluidsynth)
endif() # End of real engine

if (MUSE_MODULE_AUDIO_EXPORT)
    target_sources(muse_audio_engine PRIVATE
        internal/export/soundtrackwriter.cpp
        internal/export/soundtrackwriter.h

        # Encoders
        internal/export/abstractaudioencoder.h
        internal/export/mp3encoder.cpp
        internal/export/mp3encoder.h
        internal/export/oggencoder.cpp
        internal/export/oggencoder.h
        internal/export/flacencoder.cpp
        internal/export/flacencoder.h
        internal/export/wavencoder.cpp
        internal/export/wavencoder.h
    )

    add_subdirectory(../thirdparty/lame lame EXCLUDE_FROM_ALL)
    target_link_libraries(muse_audio_engine PRIVATE lame)

    include(../cmake/SetupOpusEnc.cmake)
    target_link_libraries(muse_audio_engine PRIVATE ${LIBOPUSENC_TARGETS})

    include(../cmake/SetupFlac.cmake)
    target_link_libraries(muse_audio_engine PRIVATE ${FLAC_TARGETS})
endif() # MUSE_MODULE_AUDIO_EXPORT

target_link_libraries(muse_audio_engine PRIVATE muse_audio_common)
