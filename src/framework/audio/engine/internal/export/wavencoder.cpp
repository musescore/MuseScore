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

#include "global/io/iodevice.h"
#include "global/types/bytearray.h"

#include "log.h"

namespace muse::audio::encode {
namespace {
template<typename T>
void writeTagData(io::IODevice& outDev, const T value, size_t offset = 0)
{
    if (offset != 0) {
        outDev.seek(offset);
    }

    // little endian
    outDev.write(reinterpret_cast<const std::uint8_t*>(&value), sizeof(T));
}

struct WavHeader {
    uint32_t chunkSize = 0;
    uint16_t audioChannelsNumber = 0;
    uint16_t bitsPerSample = 0;
    uint32_t samplesPerChannel = 0;
    uint16_t code = 0;
    uint32_t sampleRate = 0;

    void write(io::IODevice& outDev)
    {
        const uint32_t bytesPerSample = bitsPerSample / 8;
        const uint32_t sampleDataLength = audioChannelsNumber * samplesPerChannel * bytesPerSample;
        const uint32_t headerLength = 20 + chunkSize + 8;
        const uint32_t file_length = headerLength + sampleDataLength;
        const uint32_t overallSize = file_length - 8;
        const uint32_t bytesPerFrame = audioChannelsNumber * bytesPerSample;
        const uint32_t bytesPerSec = audioChannelsNumber * sampleRate * bytesPerSample;

        outDev.write(ByteArray::fromRawData("RIFF", 4)); // chunk ID

        writeTagData<uint32_t>(outDev, overallSize);
        outDev.write(ByteArray::fromRawData("WAVE", 4)); // WAVEID
        outDev.write(ByteArray::fromRawData("fmt ", 4)); // chunk ID

        writeTagData<uint32_t>(outDev, chunkSize);
        writeTagData<uint16_t>(outDev, code);
        writeTagData<uint16_t>(outDev, audioChannelsNumber);
        writeTagData<uint32_t>(outDev, sampleRate);
        writeTagData<uint32_t>(outDev, bytesPerSec);
        writeTagData<uint16_t>(outDev, bytesPerFrame);
        writeTagData<uint16_t>(outDev, bitsPerSample);

        uint16_t ext = 0;
        if (chunkSize > 16) {
            writeTagData<uint16_t>(outDev, ext); // cbSize (extension size)
        }
        outDev.write(ByteArray::fromRawData("data", 4)); // chunk ID

        writeTagData<uint32_t>(outDev, sampleDataLength);
    }
};
}

WavEncoder::WavEncoder(const SoundTrackFormat& format, io::IODevice& dstDevice)
    : AbstractAudioEncoder(format), m_dstDevice{&dstDevice}
{
    DO_ASSERT(m_dstDevice);
}

bool WavEncoder::begin(const samples_t)
{
    m_headerWritten = false;
    m_pcmStartPos = 0;
    return true;
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

    header.write(*m_dstDevice);
    return true;
}

void WavEncoder::patchHeaderSizes()
{
    const size_t endPos = m_dstDevice->pos();
    if (endPos < m_pcmStartPos || endPos < 8) {
        return;
    }

    const uint64_t dataBytes = endPos - m_pcmStartPos;
    if (dataBytes > std::numeric_limits<uint32_t>::max()) {
        LOGE() << "WAV export: file size exceeds 32-bit chunk limits";
        return;
    }

    const uint32_t dataSize = static_cast<uint32_t>(dataBytes);
    const uint32_t riffChunkSize = static_cast<uint32_t>(endPos - 8);

    writeTagData<uint32_t>(*m_dstDevice, riffChunkSize, 4); // RIFF chunk size at offset 4
    writeTagData<uint32_t>(*m_dstDevice, dataSize, 42); // data chunk size at offset 42 (with 18-byte fmt + cbSize)

    m_dstDevice->seek(endPos);
}

size_t WavEncoder::encode(const samples_t samplesPerChannel, const float* input)
{
    if (!m_headerWritten) {
        if (!writePlaceholderHeader()) {
            return 0;
        }
        m_pcmStartPos = m_dstDevice->pos();
        m_headerWritten = true;
    }

    const int channels = m_format.outputSpec.audioChannelCount;

    if (m_format.sampleFormat == AudioSampleFormat::Float32) {
        for (samples_t sampleIdx = 0; sampleIdx < samplesPerChannel; ++sampleIdx) {
            for (audioch_t audioChNum = 0; audioChNum < channels; ++audioChNum) {
                const int idx = static_cast<int>(sampleIdx) * channels + audioChNum;
                if (m_dstDevice->write(reinterpret_cast<const std::uint8_t*>(input + idx), 4) != 4) {
                    return 0;
                }
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

                m_dstDevice->write(reinterpret_cast<const std::uint8_t*>(&sampleInt), bytesToWrite);
            }
        }
    }

    return static_cast<size_t>(samplesPerChannel) * static_cast<size_t>(channels);
}

size_t WavEncoder::end()
{
    if (m_headerWritten) {
        patchHeaderSizes();
    }

    return 0;
}
}
