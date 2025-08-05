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

#include "flacencoder.h"

#include "FLAC++/encoder.h"

#include "audio/worker/internal/dsp/audiomathutils.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::encode;

struct FlacHandler : public FLAC::Encoder::File
{
    using ProgressCallBack = std::function<void (int64_t /*current*/, int64_t /*total*/)>;

    FlacHandler(const ProgressCallBack& progressCallBack)
        : FLAC::Encoder::File(), m_callBack(progressCallBack) {}

    void progress_callback(FLAC__uint64 bytes_written,
                           FLAC__uint64 samples_written,
                           uint32_t frames_written,
                           uint32_t total_frames_estimate) override
    {
        LOGI() << "wrote " << bytes_written << " bytes, "
               << samples_written << " samples, "
               << frames_written << " frames\n"
               << "TOTAL FRAMES: " << total_frames_estimate;

        m_callBack(frames_written * 4, total_frames_estimate);
    }

    ProgressCallBack m_callBack;
};

bool FlacEncoder::init(const io::path_t& path, const SoundTrackFormat& format, const samples_t totalSamplesNumber)
{
    if (!format.isValid()) {
        return false;
    }

    m_format = format;

    m_flac = new FlacHandler([this](int64_t current, int64_t total){
        m_progress.progress(current, total);
    });

    if (!m_flac->set_verify(true)
        || !m_flac->set_compression_level(0)
        || !m_flac->set_channels(m_format.audioChannelsNumber)
        || !m_flac->set_sample_rate(m_format.sampleRate)
        || !m_flac->set_bits_per_sample(16)
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

size_t FlacEncoder::encode(samples_t samplesPerChannel, const float* input)
{
    IF_ASSERT_FAILED(m_flac) {
        return 0;
    }

    size_t result = 0;
    size_t totalSamplesNumber = samplesPerChannel * m_format.audioChannelsNumber;
    uint32_t frameSize = 1024;
    size_t stepSize = frameSize * m_format.audioChannelsNumber;

    std::vector<FLAC__int32> buff(samplesPerChannel * sizeof(float));

    for (size_t i = 0; i < buff.size(); ++i) {
        buff[i] = static_cast<FLAC__int32>(dsp::convertFloatSamples<FLAC__int16>(input[i]));
    }

    std::vector<FLAC__int32> intermBuff(stepSize);

    for (size_t i = 0; i < totalSamplesNumber; i += stepSize) {
        std::copy(buff.data() + i, buff.data() + i + stepSize, intermBuff.data());

        if (m_flac->process_interleaved(intermBuff.data(), frameSize)) {
            result += stepSize;
        } else {
            break;
        }
    }

    return result;
}

size_t FlacEncoder::flush()
{
    m_flac->finish();
    return 0;
}

size_t FlacEncoder::requiredOutputBufferSize(samples_t totalSamplesNumber) const
{
    return totalSamplesNumber;
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
}
