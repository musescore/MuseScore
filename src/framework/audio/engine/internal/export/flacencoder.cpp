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

#include "FLAC++/encoder.h"

#include "../dsp/audiomathutils.h"

#include "global/io/iodevice.h"

#include "log.h"

namespace muse::audio::encode {
class FlacHandler final : public FLAC::Encoder::Stream
{
public:
    explicit FlacHandler(io::IODevice& outDev)
        : m_outDev{&outDev}
    {
    }

protected:
    FLAC__StreamEncoderWriteStatus write_callback(const FLAC__byte buffer[], const size_t bytes, uint32_t, uint32_t) override
    {
        if (m_outDev->write(buffer, bytes) != bytes) {
            return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
        }

        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    }

    FLAC__StreamEncoderSeekStatus seek_callback(const FLAC__uint64 absolute_byte_offset) override
    {
        if (!m_outDev->seek(absolute_byte_offset)) {
            return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;
        }

        return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
    }

    FLAC__StreamEncoderTellStatus tell_callback(FLAC__uint64* absolute_byte_offset) override
    {
        *absolute_byte_offset = m_outDev->pos();
        return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
    }

private:
    io::IODevice* m_outDev;
};

FlacEncoder::FlacEncoder() = default;

FlacEncoder::~FlacEncoder() noexcept = default;

bool FlacEncoder::init(io::IODevice& dstDevice, const SoundTrackFormat& format, const samples_t totalSamplesNumber)
{
    if (!format.isValid()) {
        return false;
    }

    m_format = format;

    m_flac = std::make_unique<FlacHandler>(dstDevice);

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

    //!Note Temporary workaround, since QIODevice is the alias for QIODevice, which falls with SIGSEGV
    //!     on any call from background thread. Once we have our own implementation of QIODevice
    //!     we can pass QIODevice directly into IPlayback::IAudioOutput::saveSoundTrack

    const std::string& path = dstDevice.meta("file_path");
    IF_ASSERT_FAILED(!path.empty()) {
        return false;
    }

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
    size_t totalSamplesNumber = samplesPerChannel * m_format.outputSpec.audioChannelCount;
    uint32_t frameSize = 1024;
    size_t stepSize = frameSize * m_format.outputSpec.audioChannelCount;

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

    std::vector<FLAC__int32> buff(totalSamplesNumber);

    for (size_t i = 0; i < totalSamplesNumber; ++i) {
        buff[i] = dsp::convertFloatSamples<FLAC__int32>(input[i], bitsPerSample);
    }

    std::vector<FLAC__int32> intermBuff(stepSize);

    for (size_t i = 0; i < totalSamplesNumber; i += stepSize) {
        size_t remainingSamples = totalSamplesNumber - i;
        size_t samplesToCopy = std::min(stepSize, remainingSamples);
        uint32_t samplesPerChannelToProcess = static_cast<uint32_t>(samplesToCopy) / m_format.outputSpec.audioChannelCount;

        std::copy(buff.data() + i, buff.data() + i + samplesToCopy, intermBuff.data());

        if (m_flac->process_interleaved(intermBuff.data(), samplesPerChannelToProcess)) {
            result += stepSize;
            m_progress.progress(i, totalSamplesNumber);
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

bool FlacEncoder::openDestination(const io::path_t&)
{
    IF_ASSERT_FAILED(m_flac) {
        return false;
    }

    if (m_flac->init() != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
        return false;
    }

    return true;
}

void FlacEncoder::closeDestination()
{
    m_flac.reset();
}
}
