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

using namespace mu;
using namespace mu::audio;
using namespace mu::audio::soundtrack;

static constexpr audioch_t SUPPORTED_AUDIO_CHANNELS_COUNT = 2;
static constexpr samples_t SAMPLES_PER_CHANNEL = 2048;
static constexpr size_t INTERNAL_BUFFER_SIZE = SUPPORTED_AUDIO_CHANNELS_COUNT * SAMPLES_PER_CHANNEL;

SoundTrackWriter::SoundTrackWriter(const io::path& destination, const SoundTrackFormat& format, const msecs_t totalDuration, IAudioSourcePtr source)
    : m_format(format),
    m_source(std::move(source))
{
    m_fileStream = std::fopen(destination.c_str(), "a+");

    if (!m_fileStream || !m_source) {
        return;
    }

    samples_t totalSamplesNumber = (totalDuration / 1000.f) * source->audioChannelsCount() * format.sampleRate;
    m_inputBuffer.resize(totalSamplesNumber);
    m_intermBuffer.resize(INTERNAL_BUFFER_SIZE);
    m_outputBuffer.resize(requiredOutputBufferSize(format.type, totalSamplesNumber));
}

SoundTrackWriter::~SoundTrackWriter()
{
    std::fclose(m_fileStream);
}

bool SoundTrackWriter::write()
{
    TRACEFUNC;

    if (m_format.type == SoundTrackType::Undefined || !m_source) {
        return false;
    }

    if (!m_fileStream) {
        return false;
    }

    m_source->setSampleRate(m_format.sampleRate);
    m_source->setIsActive(true);

    if (!prepareInputBuffer()) {
        return false;
    }

    if (!writeEncodedOutput()) {
        return false;
    }

    completeOutput();

    m_source->setSampleRate(AudioEngine::instance()->sampleRate());
    m_source->setIsActive(false);

    return true;
}

SoundTrackWriter::EncodeFunc SoundTrackWriter::encodeHandler() const
{
    switch (m_format.type) {
    case SoundTrackType::MP3: return encode::Mp3Encoder::encode;
    case SoundTrackType::OGG: return nullptr;
    case SoundTrackType::FLAC: return nullptr;
    default: return nullptr;
    }
}

size_t SoundTrackWriter::requiredOutputBufferSize(const SoundTrackType type, const samples_t samplesPerChannel) const
{
    switch (type) {
    case SoundTrackType::MP3: return encode::Mp3Encoder::requiredOutputBufferSize(samplesPerChannel);
    case SoundTrackType::OGG: return 0;
    case SoundTrackType::FLAC: return 0;
    default: return 0;
    }
}

bool SoundTrackWriter::prepareInputBuffer()
{
    size_t inputBufferOffset = 0;
    size_t inputBufferMaxOffset = m_inputBuffer.size();

    while (m_source->process(m_intermBuffer.data(), SAMPLES_PER_CHANNEL) != 0) {
        if (inputBufferOffset >= inputBufferMaxOffset) {
            break;
        }

        size_t samplesToCopy = std::min(INTERNAL_BUFFER_SIZE, inputBufferMaxOffset - inputBufferOffset);

        std::copy(m_intermBuffer.begin(),
                  m_intermBuffer.begin() + samplesToCopy,
                  m_inputBuffer.begin() + inputBufferOffset);

        inputBufferOffset += samplesToCopy;
    }

    if (inputBufferOffset == 0) {
        LOGI() << "No audio to export";
        return false;
    }

    return true;
}

bool SoundTrackWriter::writeEncodedOutput()
{
    EncodeFunc encodeFunc = encodeHandler();

    if (!encodeFunc) {
        return false;
    }

    samples_t encodedBytes = encodeFunc(m_format, m_inputBuffer.size() / SUPPORTED_AUDIO_CHANNELS_COUNT,
                                        m_inputBuffer.data(), m_outputBuffer.data());
    std::fwrite(m_outputBuffer.data(), sizeof(char), encodedBytes, m_fileStream);

    return true;
}

void SoundTrackWriter::completeOutput()
{
    samples_t encodedBytes = encode::Mp3Encoder::flush(m_outputBuffer.data(), m_outputBuffer.size());
    std::fwrite(m_outputBuffer.data(), sizeof(char), encodedBytes, m_fileStream);
}
