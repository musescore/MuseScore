/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "global/defer.h"

#include "internal/worker/audioengine.h"
#include "internal/encoders/mp3encoder.h"
#include "internal/encoders/oggencoder.h"
#include "internal/encoders/flacencoder.h"
#include "internal/encoders/wavencoder.h"

#include "audioerrors.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::soundtrack;

static constexpr int PREPARE_STEP = 0;
static constexpr int ENCODE_STEP = 1;

static encode::AbstractAudioEncoderPtr createEncoder(const SoundTrackType type)
{
    switch (type) {
    case SoundTrackType::MP3: return std::make_unique<encode::Mp3Encoder>();
    case SoundTrackType::OGG: return std::make_unique<encode::OggEncoder>();
    case SoundTrackType::FLAC: return std::make_unique<encode::FlacEncoder>();
    case SoundTrackType::WAV: return std::make_unique<encode::WavEncoder>();
    case SoundTrackType::Undefined: break;
    }

    UNREACHABLE;
    return nullptr;
}

SoundTrackWriter::SoundTrackWriter(const io::path_t& destination, const SoundTrackFormat& format,
                                   const msecs_t totalDuration, IAudioSourcePtr source,
                                   const modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx), m_source(std::move(source))
{
    if (!m_source) {
        return;
    }

    samples_t totalSamplesNumber = (totalDuration / 1000000.f) * sizeof(float) * format.sampleRate;
    m_inputBuffer.resize(totalSamplesNumber);
    m_intermBuffer.resize(format.samplesPerChannel * format.audioChannelsNumber);
    m_renderStep = format.samplesPerChannel;

    m_encoderPtr = createEncoder(format.type);

    if (!m_encoderPtr) {
        return;
    }

    m_encoderPtr->init(destination, format, totalSamplesNumber);
    m_encoderPtr->progress().progressChanged().onReceive(this, [this](int64_t current, int64_t total, std::string) {
        sendStepProgress(ENCODE_STEP, current, total);
    });
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

    m_source->setSampleRate(m_encoderPtr->format().sampleRate);
    m_source->setIsActive(true);

    DEFER {
        m_encoderPtr->flush();

        audioEngine()->setMode(RenderMode::IdleMode);

        m_source->setSampleRate(audioEngine()->sampleRate());
        m_source->setIsActive(false);

        m_isAborted = false;
    };

    Ret ret = generateAudioData();
    if (!ret) {
        return ret;
    }

    size_t bytes = m_encoderPtr->encode(m_inputBuffer.size() / sizeof(float), m_inputBuffer.data());

    if (m_isAborted) {
        return make_ret(Ret::Code::Cancel);
    }

    if (bytes == 0) {
        return make_ret(Err::ErrorEncode);
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

Ret SoundTrackWriter::generateAudioData()
{
    TRACEFUNC;

    const size_t inputBufferMaxOffset = m_inputBuffer.size();
    size_t inputBufferOffset = 0;

    sendStepProgress(PREPARE_STEP, inputBufferOffset, inputBufferMaxOffset);

    while (inputBufferOffset < inputBufferMaxOffset && !m_isAborted) {
        m_source->process(m_intermBuffer.data(), m_renderStep);

        size_t samplesToCopy = std::min(m_intermBuffer.size(), inputBufferMaxOffset - inputBufferOffset);

        std::copy(m_intermBuffer.begin(),
                  m_intermBuffer.begin() + samplesToCopy,
                  m_inputBuffer.begin() + inputBufferOffset);

        inputBufferOffset += samplesToCopy;
        sendStepProgress(PREPARE_STEP, inputBufferOffset, inputBufferMaxOffset);
    }

    if (m_isAborted) {
        return make_ret(Ret::Code::Cancel);
    }

    if (inputBufferOffset == 0) {
        LOGI() << "No audio to export";
        return make_ret(Err::NoAudioToExport);
    }

    return muse::make_ok();
}

void SoundTrackWriter::sendStepProgress(int step, int64_t current, int64_t total)
{
    int stepRange = step == PREPARE_STEP ? 80 : 20;
    int stepProgressStart = step == PREPARE_STEP ? 0 : 80;
    int stepCurrentProgress = stepProgressStart + ((current * 100 / total) * stepRange) / 100;
    m_progress.progress(stepCurrentProgress, 100);
}
