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

#include "global/defer.h"
#include "global/async/processevents.h"

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

static constexpr int PREPARE_STEP = 0;
static constexpr int ENCODE_STEP = 1;

// MUST MATCH src/notation/internal/notationplayback.cpp
static constexpr double PLAYBACK_TAIL_SECS = 3;

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

    const OutputSpec& outputSpec = format.outputSpec;
    samples_t totalSamplesNumber = (totalDuration / 1000000.f) * outputSpec.audioChannelCount * outputSpec.sampleRate;
    samples_t tailSamplesNumber = outputSpec.separateFilesForLooping ? PLAYBACK_TAIL_SECS * outputSpec.audioChannelCount
                                  * outputSpec.sampleRate : 0;
    samples_t intermediateSamplesNumber = outputSpec.samplesPerChannel * outputSpec.audioChannelCount;

    m_inputBuffer.resize(totalSamplesNumber);
    m_intermBuffer.resize(intermediateSamplesNumber);
    m_renderStep = outputSpec.samplesPerChannel;

    m_encoderPtr = createEncoder(format.type);

    if (!m_encoderPtr) {
        return;
    }

    io::path_t startDestination = destination;
    io::path_t loopDestination;
    io::path_t tailDestination;

    // If looping, generate -start and -end path names, extra encoders
    if (outputSpec.separateFilesForLooping) {
        io::path_t dir = io::dirpath(destination);
        io::path_t base = io::completeBasename(destination);
        std::string ext = io::suffix(destination);

        startDestination = io::path_t(base.toStdString() + "-start");
        tailDestination = io::path_t(base.toStdString() + "-end");

        startDestination = dir.appendingComponent(startDestination.appendingSuffix(io::path_t(ext)));
        tailDestination = dir.appendingComponent(tailDestination.appendingSuffix(io::path_t(ext)));

        loopDestination = destination;

        m_loopEncoderPtr = createEncoder(format.type);
        if (!m_loopEncoderPtr) {
            return;
        }

        m_tailEncoderPtr = createEncoder(format.type);
        if (!m_tailEncoderPtr) {
            return;
        }
    }

    m_encoderPtr->init(startDestination, format, totalSamplesNumber - tailSamplesNumber);

    // If looping, initialize extra encoders, account for both loop and main making progress
    if (outputSpec.separateFilesForLooping) {
        m_loopEncoderPtr->init(loopDestination, format, totalSamplesNumber - tailSamplesNumber);
        m_tailEncoderPtr->init(tailDestination, format, tailSamplesNumber);

        m_encoderPtr->progress().progressChanged().onReceive(this, [this](int64_t current, int64_t total, std::string) {
            sendStepProgress(ENCODE_STEP, current, total * 2);
        });
        m_encoderPtr->progress().progressChanged().onReceive(this, [this](int64_t current, int64_t total, std::string) {
            sendStepProgress(ENCODE_STEP, current + total, total * 2);
        });
    }
    // If not looping, only the main encoder makes progress
    else {
        m_encoderPtr->progress().progressChanged().onReceive(this, [this](int64_t current, int64_t total, std::string) {
            sendStepProgress(ENCODE_STEP, current, total);
        });
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

        if (m_tailEncoderPtr) {
            m_tailEncoderPtr->flush();
        }

        if (m_loopEncoderPtr) {
            m_loopEncoderPtr->flush();
        }

        audioEngine()->setMode(RenderMode::IdleMode);

        m_source->setOutputSpec(audioEngine()->outputSpec());
        m_source->setIsActive(false);

        m_isAborted = false;
    };

    Ret ret = generateAudioData();
    if (!ret) {
        return ret;
    }

    const OutputSpec& outputSpec = m_encoderPtr->format().outputSpec;
    samples_t tailSamplesNumber = outputSpec.separateFilesForLooping ? PLAYBACK_TAIL_SECS * outputSpec.sampleRate : 0;
    samples_t mainSamplesNumber = m_inputBuffer.size() / outputSpec.audioChannelCount - tailSamplesNumber;

    size_t bytes = m_encoderPtr->encode(mainSamplesNumber, m_inputBuffer.data());
    if (outputSpec.separateFilesForLooping) {
        bytes += m_tailEncoderPtr->encode(tailSamplesNumber, m_inputBuffer.data() + mainSamplesNumber * outputSpec.audioChannelCount);
        loopAudio(m_inputBuffer.data(), m_inputBuffer.data() + mainSamplesNumber * outputSpec.audioChannelCount, tailSamplesNumber);
        bytes += m_loopEncoderPtr->encode(mainSamplesNumber, m_inputBuffer.data());
    }

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

    const std::thread::id thisThId = std::this_thread::get_id();
    while (inputBufferOffset < inputBufferMaxOffset && !m_isAborted) {
        m_source->process(m_intermBuffer.data(), m_renderStep);

        size_t samplesToCopy = std::min(m_intermBuffer.size(), inputBufferMaxOffset - inputBufferOffset);

        std::copy(m_intermBuffer.begin(),
                  m_intermBuffer.begin() + samplesToCopy,
                  m_inputBuffer.begin() + inputBufferOffset);

        inputBufferOffset += samplesToCopy;
        sendStepProgress(PREPARE_STEP, inputBufferOffset, inputBufferMaxOffset);

        //! NOTE It is necessary for cancellation to work
        //! and for information about the audio signal to be transmitted.
        async::processMessages(thisThId);
        rpcChannel()->process();
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

// based on Mixer::mixOutputFromChannel
void SoundTrackWriter::loopAudio(float* head, const float* tail, unsigned int tailSamples)
{
    IF_ASSERT_FAILED(head && tail) {
        return;
    }

    audioch_t audioChannelCount = m_encoderPtr->format().outputSpec.audioChannelCount;
    for (samples_t s = 0; s < tailSamples; ++s) {
        size_t samplePos = s * audioChannelCount;

        for (audioch_t audioChNum = 0; audioChNum < audioChannelCount; ++audioChNum) {
            size_t idx = samplePos + audioChNum;
            float sample = tail[idx];
            head[idx] += sample;
        }
    }
}

void SoundTrackWriter::sendStepProgress(int step, int64_t current, int64_t total)
{
    int stepRange = step == PREPARE_STEP ? 80 : 20;
    int stepProgressStart = step == PREPARE_STEP ? 0 : 80;
    int stepCurrentProgress = stepProgressStart + ((current * 100 / total) * stepRange) / 100;
    m_progress.progress(stepCurrentProgress, 100);
}
