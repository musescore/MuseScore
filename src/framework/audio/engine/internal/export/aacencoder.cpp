/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "aacencoder.h"

#include "aacenc_lib.h"
#include "FDK_audio.h"

#include "../dsp/audiomathutils.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::encode;

struct AacEncoderHandler
{
    HANDLE_AACENCODER handle = nullptr;
    AACENC_InfoStruct info = {};
    std::vector<INT_PCM> inputBuffer;
    std::vector<unsigned char> outputBuffer;
    size_t frameLength = 0;
    int channels = 0;

    ~AacEncoderHandler()
    {
        if (handle) {
            aacEncClose(&handle);
        }
    }

    bool init(const SoundTrackFormat& format)
    {
        channels = format.outputSpec.audioChannelCount;
        if (aacEncOpen(&handle, 0, channels) != AACENC_OK) {
            LOGE() << "Failed to open AAC encoder";
            return false;
        }

        if (aacEncoder_SetParam(handle, AACENC_AOT, AOT_AAC_LC) != AACENC_OK) {
            LOGE() << "Failed to set AAC AOT";
            return false;
        }

        if (aacEncoder_SetParam(handle, AACENC_SAMPLERATE, format.outputSpec.sampleRate) != AACENC_OK) {
            LOGE() << "Failed to set sample rate";
            return false;
        }

        CHANNEL_MODE mode = MODE_1;
        switch (channels) {
        case 1: mode = MODE_1;
            break;
        case 2: mode = MODE_2;
            break;
        case 3: mode = MODE_1_2;
            break;
        case 4: mode = MODE_1_2_1;
            break;
        case 5: mode = MODE_1_2_2;
            break;
        case 6: mode = MODE_1_2_2_1;
            break;
        default:
            LOGE() << "Unsupported channel count:" << channels;
            return false;
        }

        if (aacEncoder_SetParam(handle, AACENC_CHANNELMODE, mode) != AACENC_OK) {
            LOGE() << "Failed to set channel mode";
            return false;
        }

        if (aacEncoder_SetParam(handle, AACENC_CHANNELORDER, 1) != AACENC_OK) {
            LOGE() << "Failed to set channel order (WAV)";
            return false;
        }

        if (aacEncoder_SetParam(handle, AACENC_BITRATE, format.bitRate * 1000) != AACENC_OK) {
            LOGE() << "Failed to set bitrate";
            return false;
        }

        if (aacEncoder_SetParam(handle, AACENC_TRANSMUX, TT_MP4_ADTS) != AACENC_OK) {
            LOGE() << "Failed to set ADTS transport";
            return false;
        }

        if (aacEncoder_SetParam(handle, AACENC_AFTERBURNER, 1) != AACENC_OK) {
            LOGE() << "Failed to set afterburner";
            return false;
        }

        if (aacEncEncode(handle, nullptr, nullptr, nullptr, nullptr) != AACENC_OK) {
            LOGE() << "Failed to initialize AAC encoder";
            return false;
        }

        if (aacEncInfo(handle, &info) != AACENC_OK) {
            LOGE() << "Failed to get AAC encoder info";
            return false;
        }

        frameLength = info.frameLength;
        outputBuffer.resize(info.maxOutBufBytes);
        inputBuffer.resize(channels * frameLength);

        return true;
    }
};

AacEncoder::AacEncoder(const SoundTrackFormat& format, io::IODevice& dstDevice)
    : AbstractAudioEncoder(format), m_handler{std::make_unique<AacEncoderHandler>()}, m_dstDevice{&dstDevice}
{
    DO_ASSERT(m_dstDevice);
}

AacEncoder::~AacEncoder() noexcept = default;

bool AacEncoder::begin(const samples_t totalSamplesNumber)
{
    m_outputBuffer.resize(totalSamplesNumber);

    if (!m_handler->init(m_format)) {
        m_handler.reset();
        return false;
    }

    return true;
}

size_t AacEncoder::encode(samples_t samplesPerChannel, const float* input)
{
    IF_ASSERT_FAILED(m_handler) {
        return 0;
    }

    const int channels = m_handler->channels;
    const size_t frameLength = m_handler->frameLength;
    const size_t samplesPerFrame = frameLength * channels;

    size_t totalBytesWritten = 0;
    size_t inputOffset = 0;
    const size_t totalSamples = samplesPerChannel * channels;

    while (inputOffset < totalSamples) {
        size_t samplesToProcess = std::min(samplesPerFrame, totalSamples - inputOffset);

        for (size_t i = 0; i < samplesToProcess; ++i) {
            m_handler->inputBuffer[i] = dsp::convertFloatSamples<INT_PCM>(input[inputOffset + i], 16);
        }

        AACENC_BufDesc inBufDesc = {};
        AACENC_BufDesc outBufDesc = {};
        AACENC_InArgs inArgs = {};
        AACENC_OutArgs outArgs = {};

        int inIdentifier = IN_AUDIO_DATA;
        int inSize = static_cast<int>(samplesToProcess * sizeof(INT_PCM));
        int inElemSize = sizeof(INT_PCM);
        void* inPtr = m_handler->inputBuffer.data();

        int outIdentifier = OUT_BITSTREAM_DATA;
        int outSize = static_cast<int>(m_handler->outputBuffer.size());
        int outElemSize = 1;
        void* outPtr = m_handler->outputBuffer.data();

        inBufDesc.numBufs = 1;
        inBufDesc.bufs = &inPtr;
        inBufDesc.bufferIdentifiers = &inIdentifier;
        inBufDesc.bufSizes = &inSize;
        inBufDesc.bufElSizes = &inElemSize;

        outBufDesc.numBufs = 1;
        outBufDesc.bufs = &outPtr;
        outBufDesc.bufferIdentifiers = &outIdentifier;
        outBufDesc.bufSizes = &outSize;
        outBufDesc.bufElSizes = &outElemSize;

        inArgs.numInSamples = static_cast<int>(samplesToProcess);

        AACENC_ERROR err = aacEncEncode(m_handler->handle, &inBufDesc, &outBufDesc, &inArgs, &outArgs);

        if (err == AACENC_ENCODE_EOF) {
            break;
        }
        if (err != AACENC_OK) {
            LOGE() << "AAC encoding failed:" << err;
            break;
        }

        if (outArgs.numOutBytes > 0) {
            const size_t written = m_dstDevice->write(m_outputBuffer.data(), static_cast<std::size_t>(outArgs.numOutBytes));
            totalBytesWritten += written;
        }

        inputOffset += samplesToProcess;

        if (samplesToProcess < samplesPerFrame) {
            break;
        }
    }

    return totalBytesWritten;
}

size_t AacEncoder::end()
{
    IF_ASSERT_FAILED(m_handler) {
        return 0;
    }

    size_t totalBytesWritten = 0;

    AACENC_BufDesc inBufDesc = {};
    AACENC_BufDesc outBufDesc = {};
    AACENC_InArgs inArgs = {};
    AACENC_OutArgs outArgs = {};

    int inIdentifier = IN_AUDIO_DATA;
    int inSize = 0;
    int inElemSize = sizeof(INT_PCM);
    void* inPtr = nullptr;

    int outIdentifier = OUT_BITSTREAM_DATA;
    int outSize = static_cast<int>(m_handler->outputBuffer.size());
    int outElemSize = 1;
    void* outPtr = m_handler->outputBuffer.data();

    inBufDesc.numBufs = 1;
    inBufDesc.bufs = &inPtr;
    inBufDesc.bufferIdentifiers = &inIdentifier;
    inBufDesc.bufSizes = &inSize;
    inBufDesc.bufElSizes = &inElemSize;

    outBufDesc.numBufs = 1;
    outBufDesc.bufs = &outPtr;
    outBufDesc.bufferIdentifiers = &outIdentifier;
    outBufDesc.bufSizes = &outSize;
    outBufDesc.bufElSizes = &outElemSize;

    inArgs.numInSamples = -1;

    while (true) {
        AACENC_ERROR err = aacEncEncode(m_handler->handle, &inBufDesc, &outBufDesc, &inArgs, &outArgs);

        if (err == AACENC_ENCODE_EOF) {
            break;
        }
        if (err != AACENC_OK) {
            break;
        }

        if (outArgs.numOutBytes > 0) {
            const size_t written = m_dstDevice->write(m_outputBuffer.data(), static_cast<std::size_t>(outArgs.numOutBytes));
            totalBytesWritten += written;
        }
    }

    return totalBytesWritten;
}
