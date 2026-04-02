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

#include <cmath>

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

bool Mp3Encoder::begin(const samples_t /*totalSamplesNumber*/)
{
    // LAME (lame.h): mp3buf for one encode call — worst case ≈ 1.25 * num_samples_per_channel + 7200 bytes;
    // flush needs at least 7200. Same buffer is used for encode + lame_encode_flush, hence margin + ceil.
    const samples_t n = m_format.outputSpec.samplesPerChannel > 0 ? m_format.outputSpec.samplesPerChannel : 4096;
    const double sz = 7200.0 + 1.25 * static_cast<double>(n) + 7200.0;
    m_outputBuffer.resize(static_cast<size_t>(std::ceil(sz)) + 512);

    if (!m_handler->init()) {
        return false;
    }

    return true;
}

size_t Mp3Encoder::encode(const samples_t samplesPerChannel, const float* input)
{
    if (samplesPerChannel > m_format.outputSpec.samplesPerChannel) {
        LOGE() << "Chunk size exceeds buffer capacity";
        return 0;
    }

    const int encodedBytes = lame_encode_buffer_interleaved_ieee_float(m_handler->flags, input, static_cast<int>(samplesPerChannel),
                                                                       m_outputBuffer.data(),
                                                                       static_cast<int>(m_outputBuffer.size()));

    if (encodedBytes < 0) {
        LOGE() << "LAME encoder failed: " << encodedBytes;
        return 0;
    }

    if (encodedBytes > 0) {
        const size_t written = m_dstDevice->write(m_outputBuffer.data(), static_cast<size_t>(encodedBytes));
        if (written != static_cast<size_t>(encodedBytes)) {
            return 0;
        }
    }

    return samplesPerChannel;
}

size_t Mp3Encoder::end()
{
    const int encodedBytes = lame_encode_flush(m_handler->flags,
                                               m_outputBuffer.data(),
                                               static_cast<int>(m_outputBuffer.size()));
    if (encodedBytes > 0) {
        m_dstDevice->write(m_outputBuffer.data(), static_cast<size_t>(encodedBytes));
    }

    return 0;
}
