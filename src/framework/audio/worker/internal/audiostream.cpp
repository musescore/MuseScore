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
#include "audiostream.h"
#include "log.h"

#define DR_WAV_IMPLEMENTATION
#define DR_MP3_IMPLEMENTATION
#define DR_MP3_FLOAT_OUTPUT
#include "thirdparty/dr_libs/dr_wav.h"
#include "thirdparty/dr_libs/dr_mp3.h"

/* open if you want to add flac
#define DR_FLAC_IMPLEMENTATION
#include "thirdparty/dr_libs/dr_flac.h"
*/

#if (defined (_MSCVER) || defined (_MSC_VER))
#pragma warning(push)
#pragma warning(disable: 4456) // declaration hides previous local declaration
#endif
#include "thirdparty/stb/stb_vorbis.c"
#if (defined (_MSCVER) || defined (_MSC_VER))
#pragma warning(pop)
#endif

using namespace muse::audio;
using namespace muse::audio::worker;

AudioStream::AudioStream()
    : m_src(m_data, 0, 1, 1)
{
}

bool AudioStream::loadFile(const io::path_t& path)
{
    bool loaded = loadWAV(path) || loadMP3(path) || loadOGG(path);
    if (loaded) {
        m_src.setChannelCount(m_channels);
        m_src.setSampleRateIn(m_sampleRate);
    }
    return loaded;
}

void AudioStream::convertSampleRate(unsigned int sampleRate)
{
    if (sampleRate != m_sampleRate) {
        auto src = new SampleRateConvertor(m_data, m_channels, m_sampleRate, sampleRate);
        m_data = src->convert();
        m_sampleRate = sampleRate;
    }
}

unsigned int AudioStream::channelsCount() const
{
    return m_channels;
}

unsigned int AudioStream::sampleRate() const
{
    return m_sampleRate;
}

unsigned int AudioStream::copySamplesToBuffer(float* buffer, unsigned int fromSample, unsigned int sampleCount, unsigned int sampleRate)
{
    if (m_sampleRate != sampleRate) {
        m_src.setSampleRateOut(sampleRate);

        return m_src.convert(buffer, fromSample, sampleCount);
    }

    auto from = fromSample * m_channels;
    auto to = from + sampleCount * m_channels;
    auto count = sampleCount * m_channels;

    IF_ASSERT_FAILED(from < m_data.size()) {
        return 0;
    }

    if (to >= m_data.size()) {
        to = static_cast<unsigned int>(m_data.size() - 1);
        count = to - from;
    }

    auto start = m_data.begin() + from;
    std::copy_n(start, count, buffer);

    return count / m_channels;
}

bool AudioStream::loadWAV(io::path_t path)
{
    drwav wav;
    if (!drwav_init_file(&wav, path.c_str(), NULL)) {
        return false;
    }

    m_channels = wav.channels;
    m_sampleRate = wav.sampleRate;

    m_data.resize(wav.totalPCMFrameCount * wav.channels);
    drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, m_data.data());

    drwav_uninit(&wav);

    return true;
}

bool AudioStream::loadMP3(io::path_t path)
{
    drmp3 mp3;

    if (!drmp3_init_file(&mp3, path.c_str(), NULL)) {
        return false;
    }

    m_channels = mp3.channels;
    m_sampleRate = mp3.sampleRate;

    auto frames = drmp3_get_pcm_frame_count(&mp3);
    m_data.resize(frames * m_channels);
    drmp3_read_pcm_frames_f32(&mp3, frames, m_data.data());
    drmp3_uninit(&mp3);

    return true;
}

bool AudioStream::loadMP3FromMemory(const void* pData, size_t dataSize)
{
    drmp3 mp3;

    if (!drmp3_init_memory(&mp3, pData, dataSize, NULL)) {
        return false;
    }

    m_channels = mp3.channels;
    m_sampleRate = mp3.sampleRate;

    auto frames = drmp3_get_pcm_frame_count(&mp3);
    m_data.resize(frames * m_channels);
    drmp3_read_pcm_frames_f32(&mp3, frames, m_data.data());
    drmp3_uninit(&mp3);

    return true;
}

bool AudioStream::loadOGG(io::path_t path)
{
    int vorbis_error;
    stb_vorbis* vorbisData = stb_vorbis_open_filename(path.c_str(), &vorbis_error, NULL);

    if (!vorbisData) {
        return false;
    }

    float predata[1000];
    int total = 0;
    m_channels = vorbisData->channels;
    m_sampleRate = vorbisData->sample_rate;

    while (auto readed = stb_vorbis_get_samples_float_interleaved(vorbisData, m_channels, predata, 1000)) {
        m_data.resize(total + readed, 0.f);
        auto start = m_data.begin() + total;
        std::copy_n(predata, readed, start);

        total += readed;
    }

    stb_vorbis_close(vorbisData);
    return true;
}
