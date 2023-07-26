#include <cstddef>
#include <cstdint>
#include <limits>

#include <fuzzing/datasource/datasource.hpp>
#include <fuzzing/memory.hpp>

#include "FLAC++/encoder.h"

#define SAMPLE_VALUE_LIMIT (1024*1024*10)

static_assert(SAMPLE_VALUE_LIMIT <= std::numeric_limits<FLAC__int32>::max(), "Invalid SAMPLE_VALUE_LIMIT");
static_assert(-SAMPLE_VALUE_LIMIT >= std::numeric_limits<FLAC__int32>::min(), "Invalid SAMPLE_VALUE_LIMIT");

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

    const int channels = 2;
	encoder.set_channels(channels);
	encoder.set_bits_per_sample(16);

    try {
        ::FLAC__StreamEncoderInitStatus ret;

        if ( ds.Get<bool>() ) {
            ret = encoder.init();
        } else {
            ret = encoder.init_ogg();
        }

        if ( ret != FLAC__STREAM_ENCODER_INIT_STATUS_OK ) {
            goto end;
        }

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

        while ( ds.Get<bool>() ) {
            {
                auto dat = ds.GetVector<FLAC__int32>();
                for (size_t i = 0; i < dat.size(); i++) {
                    if ( SAMPLE_VALUE_LIMIT != 0 ) {
                        if ( dat[i] < -SAMPLE_VALUE_LIMIT ) {
                            dat[i] = -SAMPLE_VALUE_LIMIT;
                        } else if ( dat[i] > SAMPLE_VALUE_LIMIT ) {
                            dat[i] = SAMPLE_VALUE_LIMIT;
                        }
                    }
                }
                const uint32_t samples = dat.size() / 2;
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
