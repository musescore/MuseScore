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

#include "../dsp/audiomathutils.h"

#include "global/types/bytearray.h"

#include "log.h"

namespace muse::audio::encode {
namespace {
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

private:
    template<typename T>
    void writeTagData(io::IODevice& outDev, const T value)
    {
        // little endian
        outDev.write(reinterpret_cast<const std::uint8_t*>(&value), sizeof(T));
    }
};
}

bool WavEncoder::init(io::IODevice& dstDevice, const SoundTrackFormat& format, const samples_t totalSamplesNumber)
{
    m_dstDevice = &dstDevice;

    return AbstractAudioEncoder::init(dstDevice, format, totalSamplesNumber);
}

size_t WavEncoder::encode(samples_t samplesPerChannel, const float* input)
{
    IF_ASSERT_FAILED(m_dstDevice) {
        return 0;
    }

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
        return 0;
    }

    header.audioChannelsNumber = m_format.outputSpec.audioChannelCount;
    header.sampleRate = m_format.outputSpec.sampleRate;
    header.samplesPerChannel = samplesPerChannel;

    header.write(*m_dstDevice);

    int total = header.samplesPerChannel;
    int progressStep = (total * 5) / 100; // every 5%

    const int channels = m_format.outputSpec.audioChannelCount;

    if (m_format.sampleFormat == AudioSampleFormat::Float32) {
        for (samples_t sampleIdx = 0; sampleIdx < header.samplesPerChannel; ++sampleIdx) {
            for (audioch_t audioChNum = 0; audioChNum < channels; ++audioChNum) {
                int idx = sampleIdx * channels + audioChNum;
                m_dstDevice->write(reinterpret_cast<const std::uint8_t*>(input + idx), 4);
            }
            if (sampleIdx % progressStep == 0) {
                m_progress.progress(sampleIdx, total);
            }
        }
    } else {
        const int bits = header.bitsPerSample;
        const int bytesToWrite = bits / 8;

        for (samples_t sampleIdx = 0; sampleIdx < header.samplesPerChannel; ++sampleIdx) {
            for (audioch_t audioChNum = 0; audioChNum < channels; ++audioChNum) {
                int idx = sampleIdx * channels + audioChNum;

                int32_t sampleInt = dsp::convertFloatSamples<int32_t>(input[idx], bits);

                m_dstDevice->write(reinterpret_cast<const std::uint8_t*>(&sampleInt), bytesToWrite);
            }
            if (sampleIdx % progressStep == 0) {
                m_progress.progress(sampleIdx, total);
            }
        }
    }

    return samplesPerChannel * m_format.outputSpec.audioChannelCount;
}

size_t WavEncoder::flush()
{
    NOT_SUPPORTED;

    return 0;
}

bool WavEncoder::openDestination(const io::path_t&)
{
    IF_ASSERT_FAILED(m_dstDevice) {
        return false;
    }

    prepareWriting();

    return true;
}

void WavEncoder::closeDestination()
{
    completeWriting();
}
}
