/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "soundtrackwriter.h"

#include <algorithm>
#include <cstdint>

#include "global/defer.h"

#include "audio/common/audioerrors.h"

#include "mp3encoder.h"
#include "oggencoder.h"
#include "flacencoder.h"
#include "wavencoder.h"
#include "aacencoder.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;
using namespace muse::audio::soundtrack;

static encode::AbstractAudioEncoderPtr createEncoder(const SoundTrackType type)
{
    switch (type) {
    case SoundTrackType::MP3: return std::make_unique<encode::Mp3Encoder>();
    case SoundTrackType::OGG: return std::make_unique<encode::OggEncoder>();
    case SoundTrackType::FLAC: return std::make_unique<encode::FlacEncoder>();
    case SoundTrackType::WAV: return std::make_unique<encode::WavEncoder>();
    case SoundTrackType::AAC: return std::make_unique<encode::AacEncoder>();
    case SoundTrackType::Undefined: break;
    }

    UNREACHABLE;
    return nullptr;
}

SoundTrackWriter::SoundTrackWriter(const io::path_t& destination, const SoundTrackFormat& format,
                                   const msecs_t totalDuration, IAudioSourcePtr source,
                                   const modularity::ContextPtr& iocCtx)
    : muse::Contextable(iocCtx), m_source(std::move(source))
{
    if (!m_source) {
        return;
    }
    IF_ASSERT_FAILED(format.isValid()) {
        return;
    }

    const OutputSpec& outputSpec = format.outputSpec;

    auto durationToSamples = [&](msecs_t duration) {
        const uint64_t _duration = static_cast<uint64_t>(std::max<int64_t>(0, duration));
        return static_cast<samples_t>((_duration * static_cast<uint64_t>(outputSpec.sampleRate)) / 1000000ULL);
    };

    m_dataSamples = durationToSamples(totalDuration);
    m_leadingSilenceSamples = durationToSamples(format.leadingSilenceDuration);
    samples_t trailingSilenceSamples = durationToSamples(format.trailingSilenceDuration);
    m_totalSamples = m_leadingSilenceSamples + m_dataSamples + trailingSilenceSamples;

    const samples_t intermediateSamplesNumber = outputSpec.samplesPerChannel * outputSpec.audioChannelCount;
    m_intermBuffer.resize(intermediateSamplesNumber);
    m_renderStep = outputSpec.samplesPerChannel;

    m_encoderPtr = createEncoder(format.type);
    if (!m_encoderPtr) {
        return;
    }

    if (!m_encoderPtr->init(destination, format, m_totalSamples)) {
        m_encoderPtr.reset();
    }
}

SoundTrackWriter::~SoundTrackWriter()
{
    if (m_encoderPtr) {
        m_encoderPtr->deinit();
    }
}

Ret SoundTrackWriter::write()
{
    TRACEFUNC;

    if (!m_source || !m_encoderPtr) {
        return false;
    }

    audioEngine()->setMode(RenderMode::OfflineMode);

    m_source->setOutputSpec(m_encoderPtr->format().outputSpec);
    m_source->setIsActive(true);

    DEFER {
        m_encoderPtr->flush();

        audioEngine()->setMode(RenderMode::IdleMode);

        m_source->setOutputSpec(audioEngine()->outputSpec());
        m_source->setIsActive(false);

        m_isAborted = false;
    };

    Ret ret = writeStreaming();
    if (!ret) {
        return ret;
    }

    return muse::make_ok();
}

void SoundTrackWriter::abort()
{
    m_isAborted = true;
}

Progress SoundTrackWriter::progress()
{
    return m_progress;
}

Ret SoundTrackWriter::writeStreaming()
{
    if (m_totalSamples == 0) {
        LOGI() << "No audio to export";
        return make_ret(Err::NoAudioToExport);
    }

    samples_t framesWritten = 0;

    sendStreamingProgress(0, m_totalSamples);

    // Phase 1: leading silence
    const samples_t leadingEnd = m_leadingSilenceSamples;
    while (framesWritten < leadingEnd && !m_isAborted) {
        const samples_t chunk = static_cast<samples_t>(
            std::min<uint64_t>(m_renderStep, leadingEnd - framesWritten));

        std::fill(m_intermBuffer.begin(), m_intermBuffer.end(), 0.f);

        const size_t encoded = m_encoderPtr->encode(chunk, m_intermBuffer.data());
        if (encoded == 0) {
            return make_ret(Err::ErrorEncode);
        }

        framesWritten += chunk;
        sendStreamingProgress(framesWritten, m_totalSamples);
        rpcChannel()->process();
    }

    // Phase 2: actual audio data
    const samples_t audioEnd = m_leadingSilenceSamples + m_dataSamples;
    while (framesWritten < audioEnd && !m_isAborted) {
        const samples_t chunk = static_cast<samples_t>(
            std::min<uint64_t>(m_renderStep, audioEnd - framesWritten));

        m_source->process(m_intermBuffer.data(), chunk);

        const size_t encoded = m_encoderPtr->encode(chunk, m_intermBuffer.data());
        if (encoded == 0) {
            return make_ret(Err::ErrorEncode);
        }

        framesWritten += chunk;
        sendStreamingProgress(framesWritten, m_totalSamples);
        rpcChannel()->process();
    }

    // Phase 3: trailing silence
    while (framesWritten < m_totalSamples && !m_isAborted) {
        const samples_t chunk = static_cast<samples_t>(
            std::min<uint64_t>(m_renderStep, m_totalSamples - framesWritten));

        std::fill(m_intermBuffer.begin(), m_intermBuffer.end(), 0.f);

        const size_t encoded = m_encoderPtr->encode(chunk, m_intermBuffer.data());
        if (encoded == 0) {
            return make_ret(Err::ErrorEncode);
        }

        framesWritten += chunk;
        sendStreamingProgress(framesWritten, m_totalSamples);
        rpcChannel()->process();
    }

    if (m_isAborted) {
        return make_ret(Ret::Code::Cancel);
    }

    return muse::make_ok();
}

void SoundTrackWriter::sendStreamingProgress(uint64_t framesWritten, uint64_t totalFrames)
{
    const int current = totalFrames > 0 ? static_cast<int>((framesWritten * 100) / totalFrames) : 0;
    m_progress.progress(current, 100);
}
