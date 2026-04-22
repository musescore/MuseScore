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

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;
using namespace muse::audio::soundtrack;

static encode::AbstractAudioEncoderPtr createEncoder(const SoundTrackFormat& format, io::IODevice& dstDevice)
{
    switch (format.type) {
    case SoundTrackType::MP3: return std::make_unique<encode::Mp3Encoder>(format, dstDevice);
    case SoundTrackType::OGG: return std::make_unique<encode::OggEncoder>(format, dstDevice);
    case SoundTrackType::FLAC: return std::make_unique<encode::FlacEncoder>(format, dstDevice);
    case SoundTrackType::WAV: return std::make_unique<encode::WavEncoder>(format, dstDevice);
    case SoundTrackType::Undefined: break;
    }

    UNREACHABLE;
    return nullptr;
}

SoundTrackWriter::SoundTrackWriter(io::IODevice& dstDevice, const SoundTrackFormat& format,
                                   const secs_t totalDuration, IAudioSourcePtr source)
    : m_source(std::move(source))
{
    if (!m_source) {
        return;
    }
    IF_ASSERT_FAILED(format.isValid()) {
        return;
    }

    const OutputSpec& outputSpec = format.outputSpec;
    const double totalSec = std::max(0.0, totalDuration.raw());
    m_totalSamplesPerChannel = static_cast<samples_t>(std::llround(totalSec * static_cast<double>(outputSpec.sampleRate)));

    const samples_t intermediateSamplesNumber = outputSpec.samplesPerChannel * outputSpec.audioChannelCount;
    m_intermBuffer.resize(intermediateSamplesNumber);
    m_renderStep = outputSpec.samplesPerChannel;

    m_encoderPtr = createEncoder(format, dstDevice);
    if (!m_encoderPtr) {
        return;
    }

    if (!m_encoderPtr->begin(m_totalSamplesPerChannel)) {
        m_encoderPtr.reset();
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
        if (!m_isAborted) {
            m_encoderPtr->end();
        }

        //! NOTE Changes to the source and audio engine state
        // must be performed via execOperation - so that synchronization with the audio driver process works
        IAudioEngine::Operation func = [this]() {
            audioEngine()->setMode(RenderMode::IdleMode);

            m_source->setOutputSpec(audioEngine()->outputSpec());
            m_source->setIsActive(false);
        };
        audioEngine()->execOperation(OperationType::LongOperation, func);

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
    TRACEFUNC;
    if (m_totalSamplesPerChannel == 0) {
        LOGI() << "No audio to export";
        return make_ret(Err::NoAudioToExport);
    }

    samples_t framesWritten = 0;

    sendProgress(0, m_totalSamplesPerChannel);

    while (framesWritten < m_totalSamplesPerChannel && !m_isAborted) {
        const samples_t chunk = static_cast<samples_t>(
            std::min<uint64_t>(m_renderStep, m_totalSamplesPerChannel - framesWritten));

        m_source->process(m_intermBuffer.data(), chunk);

        const size_t encoded = m_encoderPtr->encode(chunk, m_intermBuffer.data());
        if (encoded == 0) {
            return make_ret(Err::ErrorEncode);
        }

        framesWritten += chunk;
        sendProgress(framesWritten, m_totalSamplesPerChannel);

        //! NOTE It is necessary for cancellation to work
        //! and for information about the audio signal to be transmitted.
        rpcChannel()->process();
    }

    if (m_isAborted) {
        return make_ret(Ret::Code::Cancel);
    }

    return muse::make_ok();
}

void SoundTrackWriter::sendProgress(uint64_t framesWritten, uint64_t totalFrames)
{
    const int current = totalFrames > 0 ? static_cast<int>((framesWritten * 100) / totalFrames) : 0;
    m_progress.progress(current, 100);
}
