/* fuzzer_encoder_v2
 * Copyright (C) 2022-2023  Xiph.Org Foundation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>
#include <cstring> /* for memcpy */
#include "FLAC/stream_encoder.h"
#include "FLAC/metadata.h"
extern "C" {
#include "share/private.h"
}
#include "common.h"

/* This C++ fuzzer uses the FLAC and not FLAC++ because the latter lacks a few
 * hidden functions like FLAC__stream_encoder_disable_constant_subframes. It
 * is still processed by a C++ compiler because that's what oss-fuzz expects */


static FLAC__StreamEncoderWriteStatus write_callback(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame, void *client_data)
{
	(void)encoder, (void)buffer, (void)bytes, (void)samples, (void)current_frame, (void)client_data;
	return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	FLAC__bool encoder_valid = true;
	FLAC__StreamEncoder *encoder = 0;
	FLAC__StreamEncoderState state;
	FLAC__StreamMetadata *metadata[16] = {NULL};
	unsigned num_metadata = 0;
	FLAC__StreamMetadata_VorbisComment_Entry VorbisCommentField;

	unsigned sample_rate, channels, bps;
	uint64_t samples_estimate, samples_in_input;
	unsigned compression_level, input_data_width, blocksize, max_lpc_order, qlp_coeff_precision, min_residual_partition_order, max_residual_partition_order, metadata_mask, instruction_set_disable_mask;
	FLAC__bool ogg, write_to_file, interleaved;

	FLAC__bool data_bools[24];

	/* Set alloc threshold. This check was added later and no spare config
	 * bytes were left, so we're reusing the sample rate as that of little
	 * consequence to the encoder and decoder except reading the frame header */

	if(size < 3)
		return 0;
	alloc_check_threshold = data[2];
	alloc_check_counter = 0;

	/* allocate the encoder */
	if((encoder = FLAC__stream_encoder_new()) == NULL) {
		fprintf(stderr, "ERROR: allocating encoder\n");
		return 1;
	}

	/* Use first 20 byte for configuration */
	if(size < 20){
		FLAC__stream_encoder_delete(encoder);
		return 0;
	}

	/* First 3 byte for sample rate, 4th byte for channels, 5th byte for bps */
	sample_rate = ((unsigned)data[0] << 16) + ((unsigned)data[1] << 8) + data[2];
	channels = data[3];
	bps = data[4];

	/* Number of samples estimate, format accepts 36-bit max */
	samples_estimate = ((uint64_t)data[5] << 32) + ((unsigned)data[6] << 24) + ((unsigned)data[7] << 16) + ((unsigned)data[8] << 8) + data[9];

	compression_level = data[10]&0b1111;
	input_data_width = 1 + (data[10]>>4)%4;
	samples_in_input = (size-20)/input_data_width;
	blocksize = ((unsigned)data[11] << 8) + (unsigned)data[12];
	max_lpc_order = data[13];
	qlp_coeff_precision = data[14];
	min_residual_partition_order = data[15] & 0b1111;
	max_residual_partition_order = data[15] & 0b11110000;
	metadata_mask = data[16];
	instruction_set_disable_mask = data[17];

	/* Get array of bools from configuration */
	for(int i = 0; i < 16; i++)
		data_bools[i] = data[18+i/8] & (1 << (i % 8));

	ogg = data_bools[0];
	interleaved = data_bools[1];
	write_to_file = data_bools[13];

	/* Set input and process parameters */
	encoder_valid &= FLAC__stream_encoder_set_verify(encoder, data_bools[2]);
	encoder_valid &= FLAC__stream_encoder_set_channels(encoder, channels);
	encoder_valid &= FLAC__stream_encoder_set_bits_per_sample(encoder, bps);
	encoder_valid &= FLAC__stream_encoder_set_sample_rate(encoder, sample_rate);
	encoder_valid &= FLAC__stream_encoder_set_total_samples_estimate(encoder, samples_estimate);
	encoder_valid &= FLAC__stream_encoder_disable_instruction_set(encoder, instruction_set_disable_mask);
	encoder_valid &= FLAC__stream_encoder_set_limit_min_bitrate(encoder, data_bools[15]);

	/* Set compression related parameters */
	encoder_valid &= FLAC__stream_encoder_set_compression_level(encoder, compression_level);
	if(data_bools[3]){
		/* Bias towards regular compression levels */
		encoder_valid &= FLAC__stream_encoder_set_blocksize(encoder, blocksize);
		encoder_valid &= FLAC__stream_encoder_set_max_lpc_order(encoder, max_lpc_order);
		encoder_valid &= FLAC__stream_encoder_set_qlp_coeff_precision(encoder, qlp_coeff_precision);
		encoder_valid &= FLAC__stream_encoder_set_min_residual_partition_order(encoder, min_residual_partition_order);

		/* With large inputs and expensive options enabled, the fuzzer can get *really* slow.
		 * Some combinations can make the fuzzer timeout (>60 seconds). However, while combining
		 * options makes the fuzzer slower, most options do not expose new code when combined.
		 * Therefore, combining slow options is disabled for large inputs. Any input containing
		 * more than 65536 * 2 samples (max blocksize, stereo) is considered large
		 */
		if(samples_in_input < (2*65536)) {
			encoder_valid &= FLAC__stream_encoder_set_streamable_subset(encoder, data_bools[4]);
			encoder_valid &= FLAC__stream_encoder_set_do_qlp_coeff_prec_search(encoder, data_bools[5]);
			encoder_valid &= FLAC__stream_encoder_set_do_escape_coding(encoder, data_bools[6]);
			encoder_valid &= FLAC__stream_encoder_set_do_exhaustive_model_search(encoder, data_bools[7]);
			/* Combining model search, precision search and a high residual partition order is especially
			 * expensive, so limit that even further. This high partition order can only be set on
			 * large blocksize and with streamable subset disabled */
			if(samples_in_input < (2 * 4609) || data_bools[4] || !data_bools[7] || !data_bools[5] || max_residual_partition_order < 9 || blocksize < 4609)
				encoder_valid &= FLAC__stream_encoder_set_max_residual_partition_order(encoder, max_residual_partition_order);
		}
		else {
			if(!data_bools[4])
				encoder_valid &= FLAC__stream_encoder_set_streamable_subset(encoder, false);
			else if(data_bools[6])
				encoder_valid &= FLAC__stream_encoder_set_do_escape_coding(encoder, true);
			else if(data_bools[7])
				encoder_valid &= FLAC__stream_encoder_set_do_exhaustive_model_search(encoder, true);
			else if(data_bools[5])
				encoder_valid &= FLAC__stream_encoder_set_do_qlp_coeff_prec_search(encoder, true);
		}
		encoder_valid &= FLAC__stream_encoder_set_do_mid_side_stereo(encoder, data_bools[8]);
		encoder_valid &= FLAC__stream_encoder_set_loose_mid_side_stereo(encoder, data_bools[9]);

		encoder_valid &= FLAC__stream_encoder_disable_constant_subframes(encoder, data_bools[10]);
		encoder_valid &= FLAC__stream_encoder_disable_fixed_subframes(encoder, data_bools[11]);
		encoder_valid &= FLAC__stream_encoder_disable_verbatim_subframes(encoder, data_bools[12]);
	}

	/* Disable alloc check if requested */
	if(encoder_valid && data_bools[14])
		alloc_check_threshold = INT32_MAX;

	/* add metadata */
	if(encoder_valid && (metadata_mask & 1)) {
		if((metadata[num_metadata] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO)) == NULL)
			encoder_valid = false;
		else
			num_metadata++;
	}
	if(encoder_valid && (metadata_mask & 2) && size > 21){
		if((metadata[num_metadata] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL)
			encoder_valid = false;
		else {
			metadata[num_metadata++]->length = (((unsigned)data[20]) << 8) + (unsigned)(data[21]);
		}
	}
	if(encoder_valid && (metadata_mask & 4) && size > 20){
		FLAC__byte * application_data = (FLAC__byte *)malloc(size-20);
		if(0 != application_data && ((metadata[num_metadata] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_APPLICATION)) == NULL))
			encoder_valid = false;
		else {
			memcpy(application_data,data+20,size-20);
			FLAC__metadata_object_application_set_data(metadata[num_metadata++], application_data, size-20, 0);
		}
	}
	if(encoder_valid && (metadata_mask & 8) && size > 25){
		if((metadata[num_metadata] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_SEEKTABLE)) == NULL)
			encoder_valid = false;
		else {
			unsigned seekpoint_spacing = ((unsigned)data[22] << 8) + data[23];
			unsigned total_samples_for_seekpoints = ((unsigned)data[24] << 8) + data[25];
			FLAC__metadata_object_seektable_template_append_spaced_points_by_samples(metadata[num_metadata++], seekpoint_spacing, total_samples_for_seekpoints);
		}
	}
	if(encoder_valid && (metadata_mask & 16)){
		if((metadata[num_metadata] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) != NULL) {
			bool vorbiscomment_valid = true;
			/* Append a vorbis comment */
			if(!FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&VorbisCommentField, "COMMENTARY", "Nothing to 🤔 report"))
				vorbiscomment_valid = false;
			else {
				if(FLAC__metadata_object_vorbiscomment_append_comment(metadata[num_metadata], VorbisCommentField, false)) {

					/* Insert a vorbis comment at the first index */
					if(!FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&VorbisCommentField, "COMMENTARY", "Still nothing to report 🤔🤣"))
						vorbiscomment_valid = false;
					else
						if(!FLAC__metadata_object_vorbiscomment_insert_comment(metadata[num_metadata], 0, VorbisCommentField, false)) {
							free(VorbisCommentField.entry);
							vorbiscomment_valid = false;
						}
				}
				else {
					free(VorbisCommentField.entry);
					vorbiscomment_valid = false;
				}
			}
			if(!vorbiscomment_valid) {
				FLAC__metadata_object_delete(metadata[num_metadata]);
				metadata[num_metadata] = 0;
			}
			else
				num_metadata++;
		}
	}
	if(encoder_valid && (metadata_mask & 32)){
		if((metadata[num_metadata] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_CUESHEET)) != NULL) {
			if(!FLAC__metadata_object_cuesheet_insert_blank_track(metadata[num_metadata],0)) {
				FLAC__metadata_object_delete(metadata[num_metadata]);
				metadata[num_metadata] = 0;
			}
			else {
				if(!FLAC__metadata_object_cuesheet_track_insert_blank_index(metadata[num_metadata],0,0)) {
					FLAC__metadata_object_delete(metadata[num_metadata]);
					metadata[num_metadata] = 0;
				}
				else {
					metadata[num_metadata]->data.cue_sheet.tracks[0].number = 1;
					num_metadata++;
				}
			}
		}
	}
	if(encoder_valid && (metadata_mask & 64)){
		if((metadata[num_metadata] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE)) != NULL) {
			num_metadata++;
		}
	}
	if(encoder_valid && (metadata_mask & 128)){
		if((metadata[num_metadata] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_UNDEFINED)) != NULL) {
			metadata[num_metadata]->length = 24;
			metadata[num_metadata]->data.unknown.data = (FLAC__byte *)calloc(24, 1);
			num_metadata++;
		}
	}

	if(num_metadata && encoder_valid)
			encoder_valid = FLAC__stream_encoder_set_metadata(encoder, metadata, num_metadata);

	/* initialize encoder */
	if(encoder_valid) {
		FLAC__StreamEncoderInitStatus init_status;
		if(ogg)
			if(write_to_file)
				init_status = FLAC__stream_encoder_init_ogg_file(encoder, "/tmp/tmp.flac", NULL, NULL);
			else
				init_status = FLAC__stream_encoder_init_ogg_stream(encoder, NULL, write_callback, NULL, NULL, NULL, NULL);
		else
			if(write_to_file)
				init_status = FLAC__stream_encoder_init_file(encoder, "/tmp/tmp.flac", NULL, NULL);
			else
				init_status = FLAC__stream_encoder_init_stream(encoder, write_callback, NULL, NULL, NULL, NULL);
		if(init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
			encoder_valid = false;
		}
	}


	/* send samples to encoder */
	if(encoder_valid && size > (input_data_width*channels+26)) {
		unsigned samples = (size - 26)/input_data_width/channels;
		const uint8_t * pcm_data = data + 26;
		int32_t * data_as_int32 = (int32_t *)malloc(4*samples*channels);
		if(0 != data_as_int32){
			for(unsigned i = 0; i < samples*channels; i++)
				if(input_data_width == 1)
					data_as_int32[i] = (int32_t)pcm_data[i] - 0x80;
				else if(input_data_width == 2)
					data_as_int32[i] = (((int32_t)pcm_data[i*2] << 8) + pcm_data[i*2+1]) - 0x8000;
				else if(input_data_width == 3)
					data_as_int32[i] = (((int32_t)pcm_data[i*3] << 16) + ((int32_t)pcm_data[i*3+1] << 8) + pcm_data[i*3+2]) - 0x800000;
				else if(input_data_width == 4)
					data_as_int32[i] = (((int64_t)pcm_data[i*4] << 24) + ((int32_t)pcm_data[i*4+1] << 16) + ((int32_t)pcm_data[i*4+2] << 8) + pcm_data[i*4+3]) - 0x80000000;

			/* feed samples to encoder */
			if(interleaved)
				encoder_valid = FLAC__stream_encoder_process_interleaved(encoder, data_as_int32, samples);
			else {
				encoder_valid = FLAC__stream_encoder_process(encoder, (const int32_t*[]){data_as_int32,
				                                                       data_as_int32+samples,
				                                                       data_as_int32+samples*2,
				                                                       data_as_int32+samples*3,
				                                                       data_as_int32+samples*4, data_as_int32+samples*5, data_as_int32+samples*6, data_as_int32+samples*7}, samples);
			}
			free(data_as_int32);
		}
		else {
			encoder_valid = false;
		}
	}

	state = FLAC__stream_encoder_get_state(encoder);
	if(!(state == FLAC__STREAM_ENCODER_OK ||
	     state == FLAC__STREAM_ENCODER_UNINITIALIZED ||
	     state == FLAC__STREAM_ENCODER_CLIENT_ERROR ||
	     ((state == FLAC__STREAM_ENCODER_MEMORY_ALLOCATION_ERROR ||
               state == FLAC__STREAM_ENCODER_FRAMING_ERROR ||
	       (state == FLAC__STREAM_ENCODER_VERIFY_DECODER_ERROR &&
	        FLAC__stream_encoder_get_verify_decoder_state(encoder) == FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR)) &&
	      alloc_check_threshold < INT32_MAX))) {
		fprintf(stderr,"-----\nERROR: stream encoder returned %s\n-----\n",FLAC__stream_encoder_get_resolved_state_string(encoder));
		if(state == FLAC__STREAM_ENCODER_VERIFY_MISMATCH_IN_AUDIO_DATA) {
			uint32_t frame_number, channel, sample_number;
			FLAC__int32 expected, got;
			FLAC__stream_encoder_get_verify_decoder_error_stats(encoder, NULL, &frame_number, &channel, &sample_number, &expected, &got);
			fprintf(stderr,"Frame number %d\nChannel %d\n Sample number %d\nExpected value %d\nGot %d\n", frame_number, channel, sample_number, expected, got);
		}
		abort();
	}

	FLAC__stream_encoder_finish(encoder);

	/* now that encoding is finished, the metadata can be freed */
	for(unsigned i = 0; i < 16; i++)
		if(0 != metadata[i])
			FLAC__metadata_object_delete(metadata[i]);

	FLAC__stream_encoder_delete(encoder);

	return 0;
}

