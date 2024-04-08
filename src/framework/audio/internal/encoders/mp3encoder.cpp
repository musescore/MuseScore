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

#include "mp3encoder.h"

#include "lame.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::encode;

struct LameHandler
{
    LameHandler() = default;

    ~LameHandler()
    {
        lame_close(flags);
    }

    bool init(const SoundTrackFormat& format)
    {
        flags = lame_init();
        if (!flags) {
            return false;
        }

        lame_set_errorf(flags, [](const char* msg, va_list /*ap*/) {
            LOGE() << msg;
        });
        lame_set_debugf(flags, [](const char* msg, va_list /*ap*/) {
            LOGD() << msg;
        });
        lame_set_msgf(flags, [](const char* msg, va_list /*ap*/) {
            LOGI() << msg;
        });

        lame_set_num_channels(flags, format.audioChannelsNumber);
        lame_set_in_samplerate(flags, format.sampleRate);
        lame_set_brate(flags, format.bitRate);

        return lame_init_params(flags) >= 0;
    }

    lame_global_flags* flags = nullptr;
};

bool Mp3Encoder::init(const io::path_t& path, const SoundTrackFormat& format, const samples_t totalSamplesNumber)
{
    m_handler = new LameHandler();

    if (!AbstractAudioEncoder::init(path, format, totalSamplesNumber)) {
        return false;
    }

    if (!m_handler->init(format)) {
        return false;
    }

    return true;
}

size_t Mp3Encoder::requiredOutputBufferSize(samples_t totalSamplesNumber) const
{
    //!Note See thirdparty/lame/API

    return totalSamplesNumber;
}

size_t Mp3Encoder::encode(samples_t samplesPerChannel, const float* input)
{
    m_progress.progressChanged.send(0, 100, "");

    int encodedBytes = lame_encode_buffer_interleaved_ieee_float(m_handler->flags, input, samplesPerChannel,
                                                                 m_outputBuffer.data(),
                                                                 static_cast<int>(m_outputBuffer.size()));

    m_progress.progressChanged.send(50, 100, "");
    size_t result = std::fwrite(m_outputBuffer.data(), sizeof(unsigned char), encodedBytes, m_fileStream);
    m_progress.progressChanged.send(100, 100, "");

    return result;
}

size_t Mp3Encoder::flush()
{
    int encodedBytes = lame_encode_flush(m_handler->flags,
                                         m_outputBuffer.data(),
                                         static_cast<int>(m_outputBuffer.size()));

    return std::fwrite(m_outputBuffer.data(), sizeof(unsigned char), encodedBytes, m_fileStream);
}

void Mp3Encoder::closeDestination()
{
    AbstractAudioEncoder::closeDestination();

    delete m_handler;
    m_handler = nullptr;
}
