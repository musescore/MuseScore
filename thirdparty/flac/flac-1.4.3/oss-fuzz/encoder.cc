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
#include "common.h"

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
#if 0
                    try {
                        if ( ds.Get<bool>() == true ) {
	                        return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
                        }
                    } catch ( ... ) { }
#endif
                    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
                }
        };
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    fuzzing::datasource::Datasource ds(data, size);
    FLAC::Encoder::FuzzerStream encoder(ds);

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
            const bool res = encoder.set_do_exhaustive_model_search(ds.Get<bool>());
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
            const auto s = ds.Get<std::string>();
            const bool res = encoder.set_apodization(s.data());
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
            const bool res = encoder.set_do_qlp_coeff_prec_search(ds.Get<bool>());
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
            const bool res = encoder.set_rice_parameter_search_dist(ds.Get<uint32_t>());
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

        if ( size > 2 * 65535 * 4 ) {
            /* With large inputs and expensive options enabled, the fuzzer can get *really* slow.
            * Some combinations can make the fuzzer timeout (>60 seconds). However, while combining
            * options makes the fuzzer slower, most options do not expose new code when combined.
            * Therefore, combining slow options is disabled for large inputs. Any input containing
            * more than 65536 * 2 samples of 32 bits each (max blocksize, stereo) is considered large
            */
            encoder.set_do_qlp_coeff_prec_search(false);
            encoder.set_do_exhaustive_model_search(false);
        }
        if ( size > 2 * 4096 * 4 + 250 ) {
            /* With subdivide_tukey in the mix testing apodizations can get really expensive. Therefore
             * this is disabled for inputs of more than one whole stereo block of 32-bit inputs plus a
             * bit of overhead */
            encoder.set_apodization("");
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


        while ( ds.Get<bool>() ) {
            {
                auto dat = ds.GetVector<FLAC__int32>();

                if( ds.Get<bool>() )
                    /* Mask */
                    for (size_t i = 0; i < dat.size(); i++)
			/* If we get here, bps is 4 or larger, or init will have failed */
                        dat[i] = (int32_t)(((uint32_t)(dat[i]) << (32-bps)) >> (32-bps));

                const uint32_t samples = dat.size() / channels;
                if ( samples > 0 ) {
                    const int32_t* ptr = dat.data();
                    const bool res = encoder.process_interleaved(ptr, samples);
                    fuzzing::memory::memory_test(res);
                }
            }
        }
    } catch ( ... ) { }

end:
    {
        const bool res = encoder.finish();
        fuzzing::memory::memory_test(res);
    }
    return 0;
}
