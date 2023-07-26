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

#include <fuzzing/datasource/datasource.hpp>
#include <fuzzing/memory.hpp>

#include "FLAC++/decoder.h"
#include "FLAC++/metadata.h"
#include "common.h"

template <> FLAC__MetadataType fuzzing::datasource::Base::Get<FLAC__MetadataType>(const uint64_t id) {
    (void)id;
    switch ( Get<uint8_t>() ) {
        case 0:
            return FLAC__METADATA_TYPE_STREAMINFO;
        case 1:
            return FLAC__METADATA_TYPE_PADDING;
        case 2:
            return FLAC__METADATA_TYPE_APPLICATION;
        case 3:
            return FLAC__METADATA_TYPE_SEEKTABLE;
        case 4:
            return FLAC__METADATA_TYPE_VORBIS_COMMENT;
        case 5:
            return FLAC__METADATA_TYPE_CUESHEET;
        case 6:
            return FLAC__METADATA_TYPE_PICTURE;
        case 7:
            return FLAC__METADATA_TYPE_UNDEFINED;
        case 8:
            return FLAC__MAX_METADATA_TYPE;
        default:
            return FLAC__METADATA_TYPE_STREAMINFO;
    }
}

namespace FLAC {
	namespace Decoder {
        class FuzzerStream : public Stream {
            private:
                fuzzing::datasource::Datasource& ds;
            public:
                FuzzerStream(fuzzing::datasource::Datasource& dsrc) :
                    Stream(), ds(dsrc) { }

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

                    try {
                        if ( ds.Get<bool>() == true ) {
	                        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
                        }
                    } catch ( ... ) { }
                    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
                }

                void error_callback(::FLAC__StreamDecoderErrorStatus status)  override {
                    fuzzing::memory::memory_test(status);
                }

                void metadata_callback(const ::FLAC__StreamMetadata *metadata) override {
                    Metadata::Prototype * cloned_object = nullptr;
                    fuzzing::memory::memory_test(metadata->type);
                    fuzzing::memory::memory_test(metadata->is_last);
                    fuzzing::memory::memory_test(metadata->length);
                    fuzzing::memory::memory_test(metadata->data);
                    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
                        cloned_object = new Metadata::StreamInfo(metadata);
                    else if (metadata->type == FLAC__METADATA_TYPE_PADDING)
                        cloned_object = new Metadata::Padding(metadata);
                    else if (metadata->type == FLAC__METADATA_TYPE_APPLICATION)
                        cloned_object = new Metadata::Application(metadata);
                    else if (metadata->type == FLAC__METADATA_TYPE_SEEKTABLE)
                        cloned_object = new Metadata::SeekTable(metadata);
                    else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT)
                        cloned_object = new Metadata::VorbisComment(metadata);
                    else if (metadata->type == FLAC__METADATA_TYPE_CUESHEET)
                        cloned_object = new Metadata::CueSheet(metadata);
                    else if (metadata->type == FLAC__METADATA_TYPE_PICTURE)
                        cloned_object = new Metadata::Picture(metadata);
                    else
                        return;
                    if (0 != cloned_object && *cloned_object == *metadata && cloned_object->is_valid()) {
                        if (cloned_object->get_type() == FLAC__METADATA_TYPE_SEEKTABLE)
                            dynamic_cast<Metadata::SeekTable *>(cloned_object)->is_legal();
                        if (cloned_object->get_type() == FLAC__METADATA_TYPE_PICTURE)
                            dynamic_cast<Metadata::Picture *>(cloned_object)->is_legal(NULL);
                        if (cloned_object->get_type() == FLAC__METADATA_TYPE_CUESHEET)
                            dynamic_cast<Metadata::CueSheet *>(cloned_object)->is_legal(true,NULL);
                    }
                    delete cloned_object;
                }

                ::FLAC__StreamDecoderSeekStatus seek_callback(FLAC__uint64 absolute_byte_offset) override {
                    fuzzing::memory::memory_test(absolute_byte_offset);

                    try {
                        if ( ds.Get<bool>() == true ) {
                            return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
                        } else {
                            return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
                        }
                    } catch ( ... ) {
                        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
                    }
                }
#if 0
                ::FLAC__StreamDecoderTellStatus tell_callback(FLAC__uint64 *absolute_byte_offset) override {
                    fuzzing::memory::memory_test(*absolute_byte_offset);

                    try {
                        if ( ds.Get<bool>() == true ) {
                            return FLAC__STREAM_DECODER_TELL_STATUS_OK;
                        } else {
                            return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
                        }
                    } catch ( ... ) {
                        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
                    }
                }

                ::FLAC__StreamDecoderLengthStatus length_callback(FLAC__uint64 *stream_length) override {
                    fuzzing::memory::memory_test(*stream_length);

                    try {
                        if ( ds.Get<bool>() == true ) {
                            return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
                        } else {
                            return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
                        }
                    } catch ( ... ) {
                        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
                    }
                }
#endif
        };
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    fuzzing::datasource::Datasource ds(data, size);
    FLAC::Decoder::FuzzerStream decoder(ds);
    bool use_ogg = true;

    try {
        if ( ds.Get<bool>() ) {
            use_ogg = false;
        }
        if ( ds.Get<bool>() ) {
#ifdef FUZZER_DEBUG
            printf("set_ogg_serial_number\n");
#endif
            decoder.set_ogg_serial_number(ds.Get<long>());
        }
        if ( ds.Get<bool>() ) {
#ifdef FUZZER_DEBUG
            printf("set_md5_checking\n");
#endif
            decoder.set_md5_checking(ds.Get<bool>());
        }
        if ( ds.Get<bool>() ) {
#ifdef FUZZER_DEBUG
            printf("set_metadata_respond\n");
#endif
            decoder.set_metadata_respond(ds.Get<::FLAC__MetadataType>());
        }
        if ( ds.Get<bool>() ) {
            const auto idVector = ds.GetData(0);
            unsigned char id[4];
            if ( idVector.size() >= sizeof(id) ) {
                memcpy(id, idVector.data(), sizeof(id));
#ifdef FUZZER_DEBUG
                printf("set_metadata_respond_application\n");
#endif
                decoder.set_metadata_respond_application(id);
            }
        }
        if ( ds.Get<bool>() ) {
#ifdef FUZZER_DEBUG
            printf("set_metadata_respond_all\n");
#endif
            decoder.set_metadata_respond_all();
        }
        if ( ds.Get<bool>() ) {
#ifdef FUZZER_DEBUG
            printf("set_metadata_ignore\n");
#endif
            decoder.set_metadata_ignore(ds.Get<::FLAC__MetadataType>());
        }
        if ( ds.Get<bool>() ) {
            const auto idVector = ds.GetData(0);
            unsigned char id[4];
            if ( idVector.size() >= sizeof(id) ) {
                memcpy(id, idVector.data(), sizeof(id));
#ifdef FUZZER_DEBUG
                printf("set_metadata_ignore_application\n");
#endif
                decoder.set_metadata_ignore_application(id);
            }
        }
        if ( ds.Get<bool>() ) {
#ifdef FUZZER_DEBUG
            printf("set_metadata_ignore_all\n");
#endif
            decoder.set_metadata_ignore_all();
        }
        {
            ::FLAC__StreamDecoderInitStatus ret;
            if ( !use_ogg ) {
                ret = decoder.init();
            } else {
                ret = decoder.init_ogg();
            }

            if ( ret != FLAC__STREAM_DECODER_INIT_STATUS_OK ) {
                goto end;
            }
        }

        while ( ds.Get<bool>() ) {
            switch ( ds.Get<uint8_t>() ) {
                case    0:
                    {
#ifdef FUZZER_DEBUG
                        printf("flush\n");
#endif
                        const bool res = decoder.flush();
                        fuzzing::memory::memory_test(res);
                    }
                    break;
                case    1:
                    {
#ifdef FUZZER_DEBUG
                        printf("reset\n");
#endif
                        const bool res = decoder.reset();
                        fuzzing::memory::memory_test(res);
                    }
                    break;
                case    2:
                    {
#ifdef FUZZER_DEBUG
                        printf("process_single\n");
#endif
                        const bool res = decoder.process_single();
                        fuzzing::memory::memory_test(res);
                    }
                    break;
                case    3:
                    {
#ifdef FUZZER_DEBUG
                        printf("process_until_end_of_metadata\n");
#endif
                        const bool res = decoder.process_until_end_of_metadata();
                        fuzzing::memory::memory_test(res);
                    }
                    break;
                case    4:
                    {
#ifdef FUZZER_DEBUG
                        printf("process_until_end_of_stream\n");
#endif
                        const bool res = decoder.process_until_end_of_stream();
                        fuzzing::memory::memory_test(res);
                    }
                    break;
                case    5:
                    {
#ifdef FUZZER_DEBUG
                        printf("skip_single_frame\n");
#endif
                        const bool res = decoder.skip_single_frame();
                        fuzzing::memory::memory_test(res);
                    }
                    break;
                case    6:
                    {
#ifdef FUZZER_DEBUG
                        printf("seek_absolute\n");
#endif
                        const bool res = decoder.seek_absolute(ds.Get<uint64_t>());
                        fuzzing::memory::memory_test(res);
                    }
                    break;
                case    7:
                    {
#ifdef FUZZER_DEBUG
                        printf("get_md5_checking\n");
#endif
                        const bool res = decoder.get_md5_checking();
                        fuzzing::memory::memory_test(res);
                    }
                    break;
                case    8:
                    {
#ifdef FUZZER_DEBUG
                        printf("get_total_samples\n");
#endif
                        const bool res = decoder.get_total_samples();
                        fuzzing::memory::memory_test(res);
                    }
                    break;
                case    9:
                    {
#ifdef FUZZER_DEBUG
                        printf("get_channels\n");
#endif
                        const bool res = decoder.get_channels();
                        fuzzing::memory::memory_test(res);
                    }
                    break;
                case    10:
                    {
#ifdef FUZZER_DEBUG
                        printf("get_bits_per_sample\n");
#endif
                        const bool res = decoder.get_bits_per_sample();
                        fuzzing::memory::memory_test(res);
                    }
                    break;
                case    11:
                    {
#ifdef FUZZER_DEBUG
                        printf("get_sample_rate\n");
#endif
                        const bool res = decoder.get_sample_rate();
                        fuzzing::memory::memory_test(res);
                    }
                    break;
                case    12:
                    {
#ifdef FUZZER_DEBUG
                        printf("get_blocksize\n");
#endif
                        const bool res = decoder.get_blocksize();
                        fuzzing::memory::memory_test(res);
                    }
                    break;
            }
        }
    } catch ( ... ) { }

end:
    {
        const bool res = decoder.finish();
        fuzzing::memory::memory_test(res);
    }
    return 0;
}
