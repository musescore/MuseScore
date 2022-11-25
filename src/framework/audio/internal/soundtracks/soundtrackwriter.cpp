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

#include "internal/worker/audioengine.h"
#include "internal/encoders/mp3encoder.h"
#include "internal/encoders/oggencoder.h"
#include "internal/encoders/flacencoder.h"
#include "internal/encoders/wavencoder.h"

#include "defer.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::audio::soundtrack;

static constexpr int WRITE_STEPS = 2;
static constexpr int PREPARE_STEP = 0;
static constexpr int ENCODE_STEP = 1;

SoundTrackWriter::SoundTrackWriter(const io::path_t& destination, const SoundTrackFormat& format, const msecs_t totalDuration,
                                   IAudioSourcePtr source)
    : m_source(std::move(source))
{
    if (!m_source) {
        return;
    }

    samples_t totalSamplesNumber = (totalDuration / 1000000.f) * sizeof(float) * format.sampleRate;
    m_inputBuffer.resize(totalSamplesNumber);
    m_intermBuffer.resize(config()->renderStep() * config()->audioChannelsCount());

    m_encoderPtr = createEncoder(format.type);

    if (!m_encoderPtr) {
        return;
    }

    m_encoderPtr->init(destination, format, totalSamplesNumber);
    m_encoderPtr->progress().progressChanged.onReceive(this, [this](int64_t current, int64_t total, std::string) {
        sendStepProgress(ENCODE_STEP, current, total);
    });
}

bool SoundTrackWriter::write()
{
    TRACEFUNC;

    if (!m_source || !m_encoderPtr) {
        return false;
    }

    AudioEngine::instance()->setMode(RenderMode::OfflineMode);

    m_source->setSampleRate(m_encoderPtr->format().sampleRate);
    m_source->setIsActive(true);

    DEFER {
        m_encoderPtr->flush();

        AudioEngine::instance()->setMode(RenderMode::RealTimeMode);

        m_source->setSampleRate(AudioEngine::instance()->sampleRate());
        m_source->setIsActive(false);
    };

    if (!prepareInputBuffer()) {
        return false;
    }

    if (m_encoderPtr->encode(m_inputBuffer.size() / sizeof(float), m_inputBuffer.data()) == 0) {
        return false;
    }

    return true;
}

framework::Progress SoundTrackWriter::progress()
{
    return m_progress;
}

encode::AbstractAudioEncoderPtr SoundTrackWriter::createEncoder(const SoundTrackType& type) const
{
    switch (type) {
    case SoundTrackType::MP3: return std::make_unique<encode::Mp3Encoder>();
    case SoundTrackType::OGG: return std::make_unique<encode::OggEncoder>();
    case SoundTrackType::FLAC: return std::make_unique<encode::FlacEncoder>();
    case SoundTrackType::WAV: return std::make_unique<encode::WavEncoder>();
    default: return nullptr;
    }
}

bool SoundTrackWriter::prepareInputBuffer()
{
    size_t inputBufferOffset = 0;
    size_t inputBufferMaxOffset = m_inputBuffer.size();

    sendStepProgress(PREPARE_STEP, inputBufferOffset, inputBufferMaxOffset);

    samples_t renderStep = config()->renderStep();

    while (inputBufferOffset < inputBufferMaxOffset) {
        m_source->process(m_intermBuffer.data(), renderStep);

        size_t samplesToCopy = std::min(m_intermBuffer.size(), inputBufferMaxOffset - inputBufferOffset);

        std::copy(m_intermBuffer.begin(),
                  m_intermBuffer.begin() + samplesToCopy,
                  m_inputBuffer.begin() + inputBufferOffset);

        inputBufferOffset += samplesToCopy;
        sendStepProgress(PREPARE_STEP, inputBufferOffset, inputBufferMaxOffset);
    }

    if (inputBufferOffset == 0) {
        LOGI() << "No audio to export";
        return false;
    }

    return true;
}

void SoundTrackWriter::sendStepProgress(int step, int64_t current, int64_t total)
{
    int stepRange = step == PREPARE_STEP ? 80 : 20;
    int stepProgressStart = step == PREPARE_STEP ? 0 : 80;
    int stepCurrentProgress = stepProgressStart + ((current * 100 / total) * stepRange) / 100;
    m_progress.progressChanged.send(stepCurrentProgress, 100, "");
}
