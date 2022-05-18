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

using namespace mu;
using namespace mu::audio;
using namespace mu::audio::encode;

struct LameHandler
{
    static LameHandler* instance()
    {
        static LameHandler handler;
        return &handler;
    }

    bool updateSpec(const SoundTrackFormat& format)
    {
        if (!isValid()) {
            return false;
        }

        if (m_format == format) {
            return true;
        }

        m_format = format;

        lame_set_num_channels(flags, m_format.audioChannelsNumber);
        lame_set_in_samplerate(flags, m_format.sampleRate);
        lame_set_brate(flags, m_format.bitRate);

        return lame_init_params(flags) != 0;
    }

    const SoundTrackFormat& format() const
    {
        return m_format;
    }

    bool isValid() const
    {
        return flags;
    }

    lame_global_flags* flags = nullptr;

private:
    LameHandler()
    {
        flags = lame_init();

        lame_set_errorf(flags, [](const char* msg, va_list /*ap*/) {
            LOGE() << msg;
        });
        lame_set_debugf(flags, [](const char* msg, va_list /*ap*/) {
            LOGD() << msg;
        });
        lame_set_msgf(flags, [](const char* msg, va_list /*ap*/) {
            LOGI() << msg;
        });
    }

    ~LameHandler()
    {
        lame_close(flags);
    }

    SoundTrackFormat m_format;
};

size_t Mp3Encoder::requiredOutputBufferSize(samples_t totalSamplesNumber) const
{
    //!Note See thirdparty/lame/API

    return totalSamplesNumber;
}

size_t Mp3Encoder::encode(samples_t samplesPerChannel, const float* input)
{
    LameHandler::instance()->updateSpec(m_format);

    int encodedBytes = lame_encode_buffer_interleaved_ieee_float(LameHandler::instance()->flags, input, samplesPerChannel,
                                                                 m_outputBuffer.data(),
                                                                 static_cast<int>(m_outputBuffer.size()));

    return std::fwrite(m_outputBuffer.data(), sizeof(unsigned char), encodedBytes, m_fileStream);
}

size_t Mp3Encoder::flush()
{
    int encodedBytes = lame_encode_flush(LameHandler::instance()->flags,
                                         m_outputBuffer.data(),
                                         static_cast<int>(m_outputBuffer.size()));

    return std::fwrite(m_outputBuffer.data(), sizeof(unsigned char), encodedBytes, m_fileStream);
}
