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

#include "mp3encoder.h"

#ifdef SYSTEM_LAME
#  include <lame/lame.h>
#else
#  include "lame.h"
#endif

#include "global/io/iodevice.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::encode;

struct LameHandler
{
    explicit LameHandler(const SoundTrackFormat& format)
    {
        DO_ASSERT(flags);

        lame_set_errorf(flags, [](const char* msg, va_list /*ap*/) {
            LOGE() << msg;
        });
        lame_set_debugf(flags, [](const char* msg, va_list /*ap*/) {
            LOGD() << msg;
        });
        lame_set_msgf(flags, [](const char* msg, va_list /*ap*/) {
            LOGI() << msg;
        });

        lame_set_num_channels(flags, format.outputSpec.audioChannelCount);
        lame_set_in_samplerate(flags, format.outputSpec.sampleRate);
        lame_set_brate(flags, format.bitRate);
    }

    ~LameHandler() noexcept
    {
        lame_close(flags);
    }

    bool init()
    {
        return lame_init_params(flags) >= 0;
    }

    lame_global_flags* flags = lame_init();
};

Mp3Encoder::Mp3Encoder(const SoundTrackFormat& format, io::IODevice& dstDevice)
    : AbstractAudioEncoder(format), m_handler{std::make_unique<LameHandler>(format)}, m_dstDevice{&dstDevice}
{
    DO_ASSERT(m_dstDevice);
}

Mp3Encoder::~Mp3Encoder() noexcept = default;

bool Mp3Encoder::begin(const samples_t totalSamplesNumber)
{
    m_progress.progress(0, 100);

    //! Note See thirdparty/lame/API
    m_outputBuffer.resize(totalSamplesNumber);

    if (!m_handler->init()) {
        return false;
    }

    return true;
}

size_t Mp3Encoder::encode(const samples_t samplesPerChannel, const float* input)
{
    int encodedBytes = lame_encode_buffer_interleaved_ieee_float(m_handler->flags, input, samplesPerChannel,
                                                                 m_outputBuffer.data(),
                                                                 static_cast<int>(m_outputBuffer.size()));

    m_progress.progress(50, 100);
    const size_t result = m_dstDevice->write(m_outputBuffer.data(), static_cast<std::size_t>(encodedBytes));

    return result;
}

size_t Mp3Encoder::end()
{
    int encodedBytes = lame_encode_flush(m_handler->flags,
                                         m_outputBuffer.data(),
                                         static_cast<int>(m_outputBuffer.size()));
    const std::size_t numBytesWritten = m_dstDevice->write(m_outputBuffer.data(), static_cast<std::size_t>(encodedBytes));

    m_progress.progress(100, 100);

    return numBytesWritten;
}
