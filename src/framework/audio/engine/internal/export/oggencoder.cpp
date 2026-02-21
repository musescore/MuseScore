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

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::encode;

bool OggEncoder::init(io::IODevice& dstDevice, const SoundTrackFormat& format, const samples_t totalSamplesNumber)
{
    m_dstDevice = &dstDevice;

    return AbstractAudioEncoder::init(dstDevice, format, totalSamplesNumber);
}

size_t OggEncoder::encode(samples_t samplesPerChannel, const float* input)
{
    IF_ASSERT_FAILED(m_dstDevice && m_opusEncoder) {
        return 0;
    }

    m_progress.progress(0, 100);
    int code = ope_encoder_write_float(m_opusEncoder, input, samplesPerChannel);
    m_progress.progress(100, 100);

    return code == OPE_OK ? samplesPerChannel : 0;
}

size_t OggEncoder::flush()
{
    IF_ASSERT_FAILED(m_dstDevice && m_opusEncoder) {
        return 0;
    }

    return ope_encoder_drain(m_opusEncoder);
}

size_t OggEncoder::requiredOutputBufferSize(samples_t /*totalSamplesNumber*/) const
{
    return 0;
}

bool OggEncoder::openDestination(const io::path_t&)
{
    IF_ASSERT_FAILED(m_dstDevice) {
        return false;
    }

    OggOpusComments* comments = ope_comments_create();
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

    if (error != OPE_OK && m_opusEncoder) {
        ope_encoder_destroy(m_opusEncoder);
        m_opusEncoder = nullptr;
        return false;
    }

    ope_encoder_ctl(m_opusEncoder, OPUS_SET_BITRATE(m_format.bitRate * 1000));

    return true;
}

void OggEncoder::closeDestination()
{
    ope_encoder_destroy(m_opusEncoder);
    m_opusEncoder = nullptr;
    m_dstDevice = nullptr;
}
