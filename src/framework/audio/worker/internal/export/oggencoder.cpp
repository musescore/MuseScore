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

#include "oggencoder.h"

#ifdef SYSTEM_OPUSENC
#include <opus/opusenc.h>
#else
#include "opusenc.h"
#endif

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::encode;

size_t OggEncoder::encode(samples_t samplesPerChannel, const float* input)
{
    m_progress.progress(0, 100);
    int code = ope_encoder_write_float(m_opusEncoder, input, samplesPerChannel + m_format.sampleRate * 2);
    m_progress.progress(100, 100);

    return code == OPE_OK ? samplesPerChannel : 0;
}

size_t OggEncoder::flush()
{
    return ope_encoder_flush_header(m_opusEncoder);
}

size_t OggEncoder::requiredOutputBufferSize(samples_t /*totalSamplesNumber*/) const
{
    return 0;
}

bool OggEncoder::openDestination(const io::path_t& path)
{
    OggOpusComments* comments = ope_comments_create();
    int error = 0;

    m_opusEncoder = ope_encoder_create_file(path.c_str(), comments, m_format.sampleRate,
                                            m_format.audioChannelsNumber, 0, &error);

    if (error != OPE_OK && m_opusEncoder) {
        ope_encoder_destroy(m_opusEncoder);
        return false;
    }

    ope_encoder_ctl(m_opusEncoder, OPUS_SET_BITRATE(m_format.bitRate * 1000));

    return true;
}

void OggEncoder::closeDestination()
{
    ope_encoder_destroy(m_opusEncoder);
}
