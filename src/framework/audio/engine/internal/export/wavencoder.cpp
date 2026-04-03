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

#include "wavencoder.h"

#include <limits>

#include "../dsp/audiomathutils.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::encode;

namespace {
template<typename T>
void writeTagData(std::ofstream& stream, const T value, size_t offset = 0)
{
    if (offset != 0) {
        stream.seekp(offset);
    }

    // little endian
    stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

struct WavHeader {
    uint32_t chunkSize = 0;
    uint16_t audioChannelsNumber = 0;
    uint16_t bitsPerSample = 0;
    uint32_t samplesPerChannel = 0;
    uint16_t code = 0;
    uint32_t sampleRate = 0;

    void write(std::ofstream& stream)
    {
        const uint32_t bytesPerSample = bitsPerSample / 8;
        const uint32_t sampleDataLength = audioChannelsNumber * samplesPerChannel * bytesPerSample;
        const uint32_t headerLength = 20 + chunkSize + 8;
        const uint32_t file_length = headerLength + sampleDataLength;
        const uint32_t overallSize = file_length - 8;
        const uint32_t bytesPerFrame = audioChannelsNumber * bytesPerSample;
        const uint32_t bytesPerSec = audioChannelsNumber * sampleRate * bytesPerSample;

        stream.write("RIFF", 4);

        writeTagData<uint32_t>(stream, overallSize);
        stream.write("WAVE", 4);
        stream.write("fmt ", 4);

        writeTagData<uint32_t>(stream, chunkSize);
        writeTagData<uint16_t>(stream, code);
        writeTagData<uint16_t>(stream, audioChannelsNumber);
        writeTagData<uint32_t>(stream, sampleRate);
        writeTagData<uint32_t>(stream, bytesPerSec);
        writeTagData<uint16_t>(stream, bytesPerFrame);
        writeTagData<uint16_t>(stream, bitsPerSample);

        uint16_t ext = 0;
        if (chunkSize > 16) {
            writeTagData<uint16_t>(stream, ext);
        }
        stream.write("data", 4);

        writeTagData<uint32_t>(stream, sampleDataLength);
    }
};
}

bool WavEncoder::init(const io::path_t& path, const SoundTrackFormat& format, const samples_t totalSamplesPerChannel)
{
    m_headerWritten = false;
    m_pcmStartPos = 0;
    return AbstractAudioEncoder::init(path, format, totalSamplesPerChannel);
}

bool WavEncoder::writePlaceholderHeader()
{
    WavHeader header;
    header.chunkSize = 18; // 18 is 2 bytes more to include cbsize field / extension size

    switch (m_format.sampleFormat) {
    case AudioSampleFormat::Int16:
        header.bitsPerSample = 16;
        header.code = 1; // PCM
        break;
    case AudioSampleFormat::Int24:
        header.bitsPerSample = 24;
        header.code = 1; // PCM
        break;
    case AudioSampleFormat::Float32:
        header.bitsPerSample = 32;
        header.code = 3; // IEEE_FLOAT
        break;
    case AudioSampleFormat::Undefined:
    default:
        return false;
    }

    header.audioChannelsNumber = m_format.outputSpec.audioChannelCount;
    header.sampleRate = m_format.outputSpec.sampleRate;
    header.samplesPerChannel = 0;

    header.write(m_fileStream);
    return true;
}

void WavEncoder::patchHeaderSizes()
{
    m_fileStream.flush();
    const auto endPos = m_fileStream.tellp();
    if (endPos == std::streampos(-1)) {
        return;
    }

    const uint64_t endOff = static_cast<uint64_t>(static_cast<std::streamoff>(endPos));
    if (endOff < m_pcmStartPos || endOff < 8) {
        return;
    }

    const uint64_t dataBytes = endOff - m_pcmStartPos;
    if (dataBytes > std::numeric_limits<uint32_t>::max()) {
        LOGE() << "WAV export: file size exceeds 32-bit chunk limits";
        return;
    }

    const uint32_t dataSize = static_cast<uint32_t>(dataBytes);
    const uint32_t riffChunkSize = static_cast<uint32_t>(endOff - 8);

    writeTagData<uint32_t>(m_fileStream, riffChunkSize, 4); // RIFF chunk size at offset 4
    writeTagData<uint32_t>(m_fileStream, dataSize, 42); // data chunk size at offset 42 (with 18-byte fmt + cbSize)
    m_fileStream.seekp(endPos);
}

size_t WavEncoder::encode(samples_t samplesPerChannel, const float* input)
{
    if (!m_fileStream.is_open()) {
        return 0;
    }

    if (!m_headerWritten) {
        if (!writePlaceholderHeader()) {
            return 0;
        }
        m_pcmStartPos = static_cast<uint64_t>(static_cast<std::streamoff>(m_fileStream.tellp()));
        m_headerWritten = true;
    }

    const int channels = m_format.outputSpec.audioChannelCount;

    if (m_format.sampleFormat == AudioSampleFormat::Float32) {
        for (samples_t sampleIdx = 0; sampleIdx < samplesPerChannel; ++sampleIdx) {
            for (audioch_t audioChNum = 0; audioChNum < channels; ++audioChNum) {
                const int idx = static_cast<int>(sampleIdx) * channels + audioChNum;
                m_fileStream.write(reinterpret_cast<const char*>(input + idx), 4);
            }
        }
    } else {
        int bitsPerSample = 0;
        switch (m_format.sampleFormat) {
        case AudioSampleFormat::Int16:
            bitsPerSample = 16;
            break;
        case AudioSampleFormat::Int24:
            bitsPerSample = 24;
            break;
        default:
            return 0;
        }

        const int bytesToWrite = bitsPerSample / 8;

        for (samples_t sampleIdx = 0; sampleIdx < samplesPerChannel; ++sampleIdx) {
            for (audioch_t audioChNum = 0; audioChNum < channels; ++audioChNum) {
                const int idx = static_cast<int>(sampleIdx) * channels + audioChNum;

                const int32_t sampleInt = dsp::convertFloatSamples<int32_t>(input[idx], bitsPerSample);

                m_fileStream.write(reinterpret_cast<const char*>(&sampleInt), bytesToWrite);
            }
        }
    }

    return static_cast<size_t>(samplesPerChannel) * static_cast<size_t>(channels);
}

size_t WavEncoder::flush()
{
    if (m_fileStream.is_open() && m_headerWritten) {
        patchHeaderSizes();
    }

    return 0;
}

size_t WavEncoder::requiredOutputBufferSize(samples_t) const
{
    return 0;
}

bool WavEncoder::openDestination(const io::path_t& path)
{
    prepareWriting();
    m_fileStream.open(path.toStdString(), std::ios_base::binary);

    return m_fileStream.is_open();
}

void WavEncoder::closeDestination()
{
    m_fileStream.close();

    completeWriting();
}
