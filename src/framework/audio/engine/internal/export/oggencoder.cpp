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

#include "oggencoder.h"

#ifdef SYSTEM_OPUSENC
#include <opus/opusenc.h>
#else
#include "opusenc.h"
#endif

#include <algorithm>
#include <climits>

#include "global/io/iodevice.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::encode;

OggEncoder::OggEncoder(const SoundTrackFormat& format, io::IODevice& dstDevice)
    : AbstractAudioEncoder(format), m_dstDevice{&dstDevice}
{
    DO_ASSERT(m_dstDevice);
}

OggEncoder::~OggEncoder() noexcept
{
    if (m_opusEncoder) {
        ope_encoder_destroy(m_opusEncoder);
    }
}

bool OggEncoder::begin(const samples_t)
{
    OggOpusComments* comments = ope_comments_create();
    if (!comments) {
        return false;
    }

    int error = 0;

    const OpusEncCallbacks callbacks{
        [] (void* userData, const unsigned char* ptr, opus_int32 len) -> int {
            auto* dstDevice = static_cast<io::IODevice*>(userData);
            IF_ASSERT_FAILED(dstDevice) {
                return 1;
            }

            const auto numBytesToWrite = static_cast<std::size_t>(len);
            if (dstDevice->write(ptr, numBytesToWrite) != numBytesToWrite) {
                return 1;
            }

            return 0;
        },
        [] (void*) -> int { return 0; }
    };

    m_opusEncoder = ope_encoder_create_callbacks(&callbacks, m_dstDevice, comments, m_format.outputSpec.sampleRate,
                                                 m_format.outputSpec.audioChannelCount, 0, &error);
    ope_comments_destroy(comments);

    if (error != OPE_OK || !m_opusEncoder) {
        if (m_opusEncoder) {
            ope_encoder_destroy(m_opusEncoder);
            m_opusEncoder = nullptr;
        }
        return false;
    }

    ope_encoder_ctl(m_opusEncoder, OPUS_SET_BITRATE(m_format.bitRate * 1000));

    return true;
}

size_t OggEncoder::encode(const samples_t samplesPerChannel, const float* input)
{
    IF_ASSERT_FAILED(m_opusEncoder) {
        return 0;
    }

    if (samplesPerChannel == 0) {
        return 0;
    }

    const int n = static_cast<int>(std::min(samplesPerChannel, static_cast<samples_t>(INT_MAX)));
    const int code = ope_encoder_write_float(m_opusEncoder, input, n);

    return code == OPE_OK ? static_cast<size_t>(n) : 0;
}

size_t OggEncoder::end()
{
    IF_ASSERT_FAILED(m_opusEncoder) {
        return 0;
    }

    ope_encoder_drain(m_opusEncoder);

    return 0;
}
