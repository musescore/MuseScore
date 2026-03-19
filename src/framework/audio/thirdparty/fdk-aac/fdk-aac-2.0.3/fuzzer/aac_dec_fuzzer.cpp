/******************************************************************************
 *
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *****************************************************************************
 * Originally developed and contributed by Ittiam Systems Pvt. Ltd, Bangalore
 */

#include <stdint.h>
#include <string.h>
#include <algorithm>
#include "aacdecoder_lib.h"

constexpr uint8_t kNumberOfLayers = 1;
constexpr uint8_t kMaxChannelCount = 8;
constexpr uint32_t kMaxConfigurationSize = 1024;
constexpr uint32_t kMaxOutBufferSize = 2048 * kMaxChannelCount;

// Value indicating the start of AAC Header Segment
constexpr const char *kAacSegStartSeq = "AAC_STRT";
constexpr uint8_t kAacSegStartSeqLen = sizeof(kAacSegStartSeq);
// Value indicating the end of AAC Header Segment
constexpr const char *kAacSegEndSeq = "AAC_ENDS";
constexpr uint8_t kAacSegEndSeqLen = sizeof(kAacSegEndSeq);

// Number of bytes used to signal the length of the header
constexpr uint8_t kHeaderLengthBytes = 2;
// Minimum size of an AAC header is 2
// Minimum data required is
// strlen(AAC_STRT) + strlen(AAC_ENDS) + kHeaderLengthBytes + 2;
constexpr UINT kMinDataSize = kAacSegStartSeqLen + kAacSegEndSeqLen + kHeaderLengthBytes + 2;

UINT getHeaderSize(UCHAR *data, UINT size) {
  if (size < kMinDataSize) {
    return 0;
  }

  int32_t result = memcmp(data, kAacSegStartSeq, kAacSegStartSeqLen);
  if (result) {
    return 0;
  }
  data += kAacSegStartSeqLen;
  size -= kAacSegStartSeqLen;

  uint32_t headerLengthInBytes = (data[0] << 8 | data[1]) & 0xFFFF;
  data += kHeaderLengthBytes;
  size -= kHeaderLengthBytes;

  if (headerLengthInBytes + kAacSegEndSeqLen > size) {
    return 0;
  }

  data += headerLengthInBytes;
  size -= headerLengthInBytes;
  result = memcmp(data, kAacSegEndSeq, kAacSegEndSeqLen);
  if (result) {
    return 0;
  }

  return std::min(headerLengthInBytes, kMaxConfigurationSize);
}

class Codec {
 public:
  Codec() = default;
  ~Codec() { deInitDecoder(); }
  bool initDecoder();
  void decodeFrames(UCHAR *data, UINT size);
  void deInitDecoder();

 private:
  HANDLE_AACDECODER mAacDecoderHandle = nullptr;
  AAC_DECODER_ERROR mErrorCode = AAC_DEC_OK;
};

bool Codec::initDecoder() {
  mAacDecoderHandle = aacDecoder_Open(TT_MP4_ADIF, kNumberOfLayers);
  if (!mAacDecoderHandle) {
    return false;
  }
  return true;
}

void Codec::deInitDecoder() {
  aacDecoder_Close(mAacDecoderHandle);
  mAacDecoderHandle = nullptr;
}

void Codec::decodeFrames(UCHAR *data, UINT size) {
  UINT headerSize = getHeaderSize(data, size);
  if (headerSize != 0) {
    data += kAacSegStartSeqLen + kHeaderLengthBytes;
    size -= kAacSegStartSeqLen + kHeaderLengthBytes;
    aacDecoder_ConfigRaw(mAacDecoderHandle, &data, &headerSize);
    data += headerSize + kAacSegEndSeqLen;
    size -= headerSize + kAacSegEndSeqLen;
  }
  while (size > 0) {
    UINT inputSize = size;
    UINT valid = size;
    mErrorCode = aacDecoder_Fill(mAacDecoderHandle, &data, &inputSize, &valid);
    if (mErrorCode != AAC_DEC_OK) {
      ++data;
      --size;
    } else {
      INT_PCM outputBuf[kMaxOutBufferSize];
      do {
        mErrorCode =
            aacDecoder_DecodeFrame(mAacDecoderHandle, outputBuf,
                    kMaxOutBufferSize /*size in number of INT_PCM, not bytes*/, 0);
      } while (mErrorCode == AAC_DEC_OK);
      UINT offset = inputSize - valid;
      data += offset;
      size = valid;
    }
  }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  Codec *codec = new Codec();
  if (!codec) {
    return 0;
  }
  if (codec->initDecoder()) {
    codec->decodeFrames((UCHAR *)(data), static_cast<UINT>(size));
  }
  delete codec;
  return 0;
}
