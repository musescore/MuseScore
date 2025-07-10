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

#include "wavencoder.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::encode;

struct WavHeader {
    enum class WavFileType {
        int16,                                             // 16 bit signed integer
        float32
    };

    uint32_t chunkSize = 0;
    uint16_t audioChannelsNumber = 0;
    uint16_t bitsPerSample = 0;
    uint32_t samplesPerChannel = 0;
    uint16_t code = 0;
    uint32_t sampleRate = 0;

    void write(std::ofstream& stream)
    {
        const uint32_t bytesPerSample = 4;
        const uint32_t sampleDataLength = audioChannelsNumber * samplesPerChannel * bytesPerSample;
        const uint32_t headerLength = 20 + chunkSize + 8;
        const uint32_t file_length = headerLength + sampleDataLength;
        const uint32_t overallSize = file_length - 8;
        const uint32_t bytesPerFrame = audioChannelsNumber * bytesPerSample;
        const uint32_t bytesPerSec = audioChannelsNumber * sampleRate * bytesPerSample;

        stream.write("RIFF", 4); // chunk ID

        writeTagData<uint32_t>(stream, overallSize);
        stream.write("WAVE", 4); // WAVEID
        stream.write("fmt ", 4); // chunk ID

        writeTagData<uint32_t>(stream, chunkSize);
        writeTagData<uint16_t>(stream, code);
        writeTagData<uint16_t>(stream, 2);
        writeTagData<uint32_t>(stream, sampleRate);
        writeTagData<uint32_t>(stream, bytesPerSec);
        writeTagData<uint16_t>(stream, bytesPerFrame);
        writeTagData<uint16_t>(stream, bitsPerSample);

        uint16_t ext = 0;
        if (chunkSize > 16) {
            writeTagData<uint16_t>(stream, ext); // cbSize (extension size)
        }
        stream.write("data", 4); // chunk ID

        writeTagData<uint32_t>(stream, sampleDataLength);
    }

private:
    template<typename T>
    void writeTagData(std::ofstream& stream, const T value)
    {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(T));  // little endian
    }
};

size_t WavEncoder::encode(samples_t samplesPerChannel, const float* input)
{
    if (!m_fileStream.is_open()) {
        return 0;
    }

    WavHeader header;
    header.chunkSize = 18; // 18 is 2 bytes more to include cbsize field / extension size
    header.bitsPerSample = 32;
    header.code = 3; // IEEE_FLOAT = 3, PCM = 1
    header.audioChannelsNumber = m_format.audioChannelsNumber;
    header.sampleRate = m_format.sampleRate;
    header.samplesPerChannel = samplesPerChannel;

    header.write(m_fileStream);

    int total = header.samplesPerChannel;
    int progressStep = (total * 5) / 100; // every 5%
    QVector<samples_t> progressValues;
    for (samples_t sampleIdx = 0; sampleIdx < header.samplesPerChannel;) {
        progressValues << sampleIdx;
        sampleIdx += progressStep;
    }

    for (samples_t sampleIdx = 0; sampleIdx < header.samplesPerChannel; ++sampleIdx) {
        for (audioch_t audioChNum = 0; audioChNum < m_format.audioChannelsNumber; ++audioChNum) {
            int idx = sampleIdx * m_format.audioChannelsNumber + audioChNum;
            m_fileStream.write(reinterpret_cast<const char*>(input + idx), 4);
        }

        if (progressValues.contains(sampleIdx)) {
            m_progress.progress(sampleIdx, total);
        }
    }

    return samplesPerChannel * m_format.audioChannelsNumber;
}

size_t WavEncoder::flush()
{
    NOT_SUPPORTED;

    return 0;
}

size_t WavEncoder::requiredOutputBufferSize(samples_t totalSamplesNumber) const
{
    return totalSamplesNumber;
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
