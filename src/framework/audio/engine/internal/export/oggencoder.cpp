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

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::encode;

size_t OggEncoder::encode(samples_t samplesPerChannel, const float* input)
{
    if (samplesPerChannel == 0 || !m_opusEncoder) {
        return 0;
    }

    const int n = static_cast<int>(std::min(samplesPerChannel, static_cast<samples_t>(INT_MAX)));
    const int code = ope_encoder_write_float(m_opusEncoder, input, n);

    return code == OPE_OK ? static_cast<size_t>(n) : 0;
}

size_t OggEncoder::flush()
{
    if (!m_opusEncoder) {
        return 0;
    }

    return ope_encoder_drain(m_opusEncoder) == OPE_OK ? 1 : 0;
}

size_t OggEncoder::requiredOutputBufferSize(samples_t /*totalSamplesNumber*/) const
{
    return 0;
}

bool OggEncoder::openDestination(const io::path_t& path)
{
    OggOpusComments* comments = ope_comments_create();
    if (!comments) {
        return false;
    }

    int error = 0;

    m_opusEncoder = ope_encoder_create_file(path.c_str(), comments, m_format.outputSpec.sampleRate,
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

void OggEncoder::closeDestination()
{
    if (m_opusEncoder) {
        ope_encoder_destroy(m_opusEncoder);
        m_opusEncoder = nullptr;
    }
}
