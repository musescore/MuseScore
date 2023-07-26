/* Copyright 2019 Guido Vranken
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cstddef>
#include <cstdint>
#include <limits>

#include <fuzzing/datasource/datasource.hpp>
#include <fuzzing/memory.hpp>

#include "FLAC++/encoder.h"
#include "FLAC++/decoder.h"
#include "FLAC++/metadata.h"
#include "common.h"

#define MAX_NUM_METADATA_BLOCKS 2048

namespace FLAC {
     namespace Encoder {
         class FuzzerStream : public Stream {
            private:
                // fuzzing::datasource::Datasource& ds;
            public:
                FuzzerStream(fuzzing::datasource::Datasource&) :
                    Stream() { }

                ::FLAC__StreamEncoderWriteStatus write_callback(const FLAC__byte buffer[], size_t bytes, uint32_t /* samples */, uint32_t /* current_frame */) override {
                    fuzzing::memory::memory_test(buffer, bytes);
                    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
                }
         };
    }
    namespace Decoder {
        class FuzzerDecoder : public Stream {
        private:
            fuzzing::datasource::Datasource& ds;
            FLAC::Encoder::FuzzerStream& encoder;
        public:
            FuzzerDecoder(fuzzing::datasource::Datasource& dsrc, FLAC::Encoder::FuzzerStream& encoder_arg) :
                Stream(), ds(dsrc), encoder(encoder_arg) { }

            ::FLAC__StreamMetadata * metadata_blocks[MAX_NUM_METADATA_BLOCKS] = {0};
            int num_metadata_blocks = 0;

            void metadata_callback(const ::FLAC__StreamMetadata *metadata) override {
		if(num_metadata_blocks < MAX_NUM_METADATA_BLOCKS)
	                if((metadata_blocks[num_metadata_blocks] = FLAC__metadata_object_clone(metadata)) != NULL)
				num_metadata_blocks++;
            }

            ::FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[], size_t *bytes)  override {
                try {
                    const size_t maxCopySize = *bytes;

                    if ( maxCopySize > 0 ) {
                        /* memset just to test if this overwrites anything, and triggers ASAN */
                        memset(buffer, 0, maxCopySize);
                    }

                    const auto data = ds.GetData(0);
                    const auto dataSize = data.size();
                    const auto copySize = std::min(maxCopySize, dataSize);

                    if ( copySize > 0 ) {
                        memcpy(buffer, data.data(), copySize);
                    }

                    *bytes = copySize;

                    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
                } catch ( ... ) {
                        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
                }
            }

            ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[])  override {
                {
                    fuzzing::memory::memory_test(&(frame->header), sizeof(frame->header));
                    fuzzing::memory::memory_test(&(frame->footer), sizeof(frame->footer));
                }

                {
                    const auto numChannels = get_channels();
                    const size_t bytesPerChannel = frame->header.blocksize * sizeof(FLAC__int32);
                    for (size_t i = 0; i < numChannels; i++) {
                        fuzzing::memory::memory_test(buffer[i], bytesPerChannel);
                    }
                }

		/* Data is checked, now pass it towards encoder */
                if(encoder.get_state() == FLAC__STREAM_ENCODER_OK) {
                    if(encoder.get_channels() != get_channels())
                         return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
                    if(encoder.get_bits_per_sample() != get_bits_per_sample())
                         return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
                    encoder.process(buffer, frame->header.blocksize);
                    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
                }
                else
                    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
            }
            void error_callback(::FLAC__StreamDecoderErrorStatus status)  override {
                fuzzing::memory::memory_test(status);
            }
        };
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    fuzzing::datasource::Datasource ds(data, size);
    FLAC::Encoder::FuzzerStream encoder(ds);
    FLAC::Decoder::FuzzerDecoder decoder(ds, encoder);

    try {
            const int channels = ds.Get<uint8_t>();
            const int bps = ds.Get<uint8_t>();
            encoder.set_channels(channels);
            encoder.set_bits_per_sample(bps);

        {
            const bool res = encoder.set_streamable_subset(ds.Get<bool>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_ogg_serial_number(ds.Get<long>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_verify(ds.Get<bool>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_compression_level(ds.Get<uint8_t>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_do_mid_side_stereo(ds.Get<bool>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_loose_mid_side_stereo(ds.Get<bool>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_max_lpc_order(ds.Get<uint8_t>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_qlp_coeff_precision(ds.Get<uint32_t>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_do_escape_coding(ds.Get<bool>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_min_residual_partition_order(ds.Get<uint32_t>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_max_residual_partition_order(ds.Get<uint32_t>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_total_samples_estimate(ds.Get<uint64_t>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_blocksize(ds.Get<uint16_t>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_limit_min_bitrate(ds.Get<bool>());
            fuzzing::memory::memory_test(res);
        }
        {
            const bool res = encoder.set_sample_rate(ds.Get<uint32_t>());
            fuzzing::memory::memory_test(res);
        }

        decoder.set_metadata_respond_all();

        {
            ::FLAC__StreamDecoderInitStatus ret;
            if ( ds.Get<bool>() ) {
                ret = decoder.init();
            } else {
                ret = decoder.init_ogg();
            }

            if ( ret != FLAC__STREAM_DECODER_INIT_STATUS_OK ) {
                goto end;
            }

            decoder.process_until_end_of_metadata();
            if(decoder.num_metadata_blocks > 0)
                encoder.set_metadata(decoder.metadata_blocks, decoder.num_metadata_blocks);
        }

        {
            ::FLAC__StreamEncoderInitStatus ret;
            if ( ds.Get<bool>() ) {
                ret = encoder.init();
            } else {
                ret = encoder.init_ogg();
            }

            if ( ret != FLAC__STREAM_ENCODER_INIT_STATUS_OK ) {
                goto end;
            }
        }

	/* These sets must fail, because encoder is already initialized */
        {
            bool res = false;
            res = res || encoder.set_streamable_subset(true);
            res = res || encoder.set_ogg_serial_number(0);
            res = res || encoder.set_verify(true);
            res = res || encoder.set_compression_level(0);
            res = res || encoder.set_do_exhaustive_model_search(true);
            res = res || encoder.set_do_mid_side_stereo(true);
            res = res || encoder.set_loose_mid_side_stereo(true);
            res = res || encoder.set_apodization("test");
            res = res || encoder.set_max_lpc_order(0);
            res = res || encoder.set_qlp_coeff_precision(0);
            res = res || encoder.set_do_qlp_coeff_prec_search(true);
            res = res || encoder.set_do_escape_coding(true);
            res = res || encoder.set_min_residual_partition_order(0);
            res = res || encoder.set_max_residual_partition_order(0);
            res = res || encoder.set_rice_parameter_search_dist(0);
            res = res || encoder.set_total_samples_estimate(0);
            res = res || encoder.set_channels(channels);
            res = res || encoder.set_bits_per_sample(16);
            res = res || encoder.set_limit_min_bitrate(true);
            res = res || encoder.set_blocksize(3021);
            res = res || encoder.set_sample_rate(44100);
            fuzzing::memory::memory_test(res);
            if(res)
                abort();
        }


        {
            /* XORing values as otherwise compiler will optimize, apparently */
            bool res = false;
            res = res != encoder.get_streamable_subset();
            res = res != encoder.get_verify();
            res = res != encoder.get_do_exhaustive_model_search();
            res = res != encoder.get_do_mid_side_stereo();
            res = res != encoder.get_loose_mid_side_stereo();
            res = res != encoder.get_max_lpc_order();
            res = res != encoder.get_qlp_coeff_precision();
            res = res != encoder.get_do_qlp_coeff_prec_search();
            res = res != encoder.get_do_escape_coding();
            res = res != encoder.get_min_residual_partition_order();
            res = res != encoder.get_max_residual_partition_order();
            res = res != encoder.get_rice_parameter_search_dist();
            res = res != encoder.get_total_samples_estimate();
            res = res != encoder.get_channels();
            res = res != encoder.get_bits_per_sample();
            res = res != encoder.get_limit_min_bitrate();
            res = res != encoder.get_blocksize();
            res = res != encoder.get_sample_rate();
            fuzzing::memory::memory_test(res);
        }

        decoder.process_until_end_of_stream();

    } catch ( ... ) { }

end:
    {
        const bool res = encoder.finish();
        fuzzing::memory::memory_test(res);
    }
    {
        const bool res = decoder.finish();
        fuzzing::memory::memory_test(res);
    }
    for(int i = 0; i < decoder.num_metadata_blocks; i++)
        FLAC__metadata_object_delete(decoder.metadata_blocks[i]);

    return 0;
}
