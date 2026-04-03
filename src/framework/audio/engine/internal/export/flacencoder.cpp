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

#include "flacencoder.h"

#include <algorithm>
#include <vector>

#include "FLAC++/encoder.h"

#include "../dsp/audiomathutils.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::encode;

struct FlacHandler : public FLAC::Encoder::File
{
    void progress_callback(FLAC__uint64, FLAC__uint64, uint32_t, uint32_t) override {}
};

bool FlacEncoder::init(const io::path_t& path, const SoundTrackFormat& format, const samples_t totalSamplesNumber)
{
    if (!format.isValid()) {
        return false;
    }

    m_format = format;

    m_flac = new FlacHandler();

    int bitsPerSample = 0;
    switch (m_format.sampleFormat) {
    case AudioSampleFormat::Int16:
        bitsPerSample = 16;
        break;
    case AudioSampleFormat::Int24:
        bitsPerSample = 24;
        break;
    default:
        return false;
    }

    if (!m_flac->set_verify(true)
        || !m_flac->set_compression_level(0)
        || !m_flac->set_channels(m_format.outputSpec.audioChannelCount)
        || !m_flac->set_sample_rate(m_format.outputSpec.sampleRate)
        || !m_flac->set_bits_per_sample(bitsPerSample)
        || !m_flac->set_total_samples_estimate(totalSamplesNumber)) {
        return false;
    }

    FLAC__StreamMetadata_VorbisComment_Entry entry;
    FLAC__StreamMetadata* metadata[2];
    metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
    metadata[1] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING);
    FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", "Some Artist");
    FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/ false);
    FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "YEAR", "1984");
    FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/ false);

    metadata[1]->length = 1234; /* set the padding length */
    m_flac->set_metadata(metadata, 2);

    if (!openDestination(path)) {
        return false;
    }

    prepareOutputBuffer(totalSamplesNumber);

    return true;
}

void FlacEncoder::prepareOutputBuffer(const samples_t /*totalSamplesNumber*/)
{
    m_outputBuffer.clear();
}

size_t FlacEncoder::encode(samples_t samplesPerChannel, const float* input)
{
    IF_ASSERT_FAILED(m_flac) {
        return 0;
    }

    int bitsPerSample = 0;
    switch (m_format.sampleFormat) {
    case AudioSampleFormat::Int16:
        bitsPerSample = 16;
        break;
    case AudioSampleFormat::Int24:
        bitsPerSample = 24;
        break;
    default:
        return 0;
    }

    const auto channels = m_format.outputSpec.audioChannelCount;
    if (channels == 0 || samplesPerChannel == 0) {
        return 0;
    }

    constexpr uint32_t BLOCK_FRAMES = 1024;
    std::vector<FLAC__int32> blockBuffer(static_cast<size_t>(BLOCK_FRAMES) * channels);

    for (samples_t frameStart = 0; frameStart < samplesPerChannel;) {
        const uint32_t nFrames = static_cast<uint32_t>(std::min<samples_t>(BLOCK_FRAMES, samplesPerChannel - frameStart));

        for (uint32_t f = 0; f < nFrames; ++f) {
            for (audioch_t c = 0; c < channels; ++c) {
                const size_t i = (static_cast<size_t>(frameStart) + f) * channels + c;
                blockBuffer[static_cast<size_t>(f) * channels + c]
                    = dsp::convertFloatSamples<FLAC__int32>(input[i], bitsPerSample);
            }
        }

        if (!m_flac->process_interleaved(blockBuffer.data(), nFrames)) {
            return 0;
        }

        frameStart += nFrames;
    }

    return static_cast<size_t>(samplesPerChannel) * channels;
}

size_t FlacEncoder::flush()
{
    m_flac->finish();
    return 0;
}

size_t FlacEncoder::requiredOutputBufferSize(samples_t /*totalSamplesNumber*/) const
{
    return 0;
}

bool FlacEncoder::openDestination(const io::path_t& path)
{
    IF_ASSERT_FAILED(m_flac) {
        return false;
    }

    if (m_flac->init(path.c_str()) != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
        return false;
    }

    return true;
}

void FlacEncoder::closeDestination()
{
    delete m_flac;
    m_flac = nullptr;
}
