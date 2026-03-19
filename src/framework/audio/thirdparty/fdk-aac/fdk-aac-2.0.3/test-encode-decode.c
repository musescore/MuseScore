/* ------------------------------------------------------------------
 * Copyright (C) 2017 Martin Storsjo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "aacenc_lib.h"
#include "aacdecoder_lib.h"
#include "wavreader.h"
#include "sha1.h"


static int encoder_input_samples, encoder_input_size;
static int decoder_output_skip;
static int16_t *encoder_input;
static int max_diff;
static uint64_t diff_sum, diff_samples;
static SHA1Context encode_hash, decode_hash;

static void init_encoder_input(void) {
	encoder_input_samples = 0;
	max_diff = 0;
	diff_sum = diff_samples = 0;
}

static void free_encoder_input(void) {
	free(encoder_input);
	encoder_input = NULL;
	encoder_input_size = 0;
}

static void append_encoder_input(const int16_t *input, int samples) {
	if (encoder_input_samples + samples > encoder_input_size) {
		int size = 2*(encoder_input_samples + samples);
		int16_t *ptr = realloc(encoder_input, size * sizeof(*encoder_input));
		if (!ptr)
			abort();
		encoder_input = ptr;
		encoder_input_size = size;
	}
	memcpy(encoder_input + encoder_input_samples, input, samples * sizeof(*input));
	encoder_input_samples += samples;
}

static void compare_decoder_output(const int16_t *output, int samples) {
	int i;
	// TODO: Stereo upconvert?
	SHA1Input(&decode_hash, (const unsigned char*) output, samples * sizeof(*output));
	if (decoder_output_skip > 0) {
		int n = samples;
		if (n > decoder_output_skip)
			n = decoder_output_skip;
		output += n;
		samples -= n;
		decoder_output_skip -= n;
		if (samples <= 0)
			return;
	}
	if (samples > encoder_input_samples)
		samples = encoder_input_samples;
	for (i = 0; i < samples; i++) {
		int diff = abs(encoder_input[i] - output[i]);
		if (diff > max_diff)
			max_diff = diff;
		diff_sum += diff;
		diff_samples++;
	}
	memmove(encoder_input, encoder_input + samples, (encoder_input_samples - samples) * sizeof(*encoder_input));
	encoder_input_samples -= samples;
}

static int decode(HANDLE_AACDECODER decoder, const uint8_t *ptr, int size, uint8_t *decoder_buffer, int decoder_buffer_size, int channels) {
	AAC_DECODER_ERROR err;
	CStreamInfo *info;
	UINT valid, buffer_size;
	SHA1Input(&encode_hash, ptr, size);
	do {
		valid = buffer_size = size;
		err = aacDecoder_Fill(decoder, (UCHAR**) &ptr, &buffer_size, &valid);
		ptr += buffer_size - valid;
		size -= buffer_size - valid;
		if (err == AAC_DEC_NOT_ENOUGH_BITS)
			continue;
		if (err != AAC_DEC_OK)
			break;
		err = aacDecoder_DecodeFrame(decoder, (INT_PCM *) decoder_buffer, decoder_buffer_size / sizeof(INT_PCM), 0);
		if (!ptr && err != AAC_DEC_OK)
			break;
		if (err == AAC_DEC_NOT_ENOUGH_BITS)
			continue;
		if (err != AAC_DEC_OK) {
			fprintf(stderr, "Decoding failed\n");
			return 1;
		}
		info = aacDecoder_GetStreamInfo(decoder);
		if (info->numChannels != channels) {
			fprintf(stderr, "Mismatched number of channels, input %d, output %d\n", channels, info->numChannels);
			return 1;
		}
		compare_decoder_output((int16_t*) decoder_buffer, info->numChannels * info->frameSize);
	} while (size > 0);
	return 0;
}

static int test_encode_decode(const char *infile, int aot, int afterburner, int eld_sbr, int vbr, int bitrate, int adts) {
	void *wav;
	int format, sample_rate, channels, bits_per_sample;
	int input_size;
	uint8_t* input_buf;
	int16_t* convert_buf;
	HANDLE_AACENCODER encoder;
	CHANNEL_MODE mode;
	AACENC_InfoStruct info = { 0 };
	HANDLE_AACDECODER decoder;
	int ret = 0;
	int decoder_buffer_size = 2048 * 2 * 8;
	uint8_t* decoder_buffer = malloc(decoder_buffer_size);
	int avg_diff;

	fprintf(stderr, "Testing encoding with aot %d afterburner %d eld_sbr %d vbr %d bitrate %d adts %d\n", aot, afterburner, eld_sbr, vbr, bitrate, adts);
	init_encoder_input();

	wav = wav_read_open(infile);
	if (!wav) {
		fprintf(stderr, "Unable to open wav file %s\n", infile);
		return 1;
	}
	if (!wav_get_header(wav, &format, &channels, &sample_rate, &bits_per_sample, NULL)) {
		fprintf(stderr, "Bad wav file %s\n", infile);
		return 1;
	}
	if (format != 1) {
		fprintf(stderr, "Unsupported WAV format %d\n", format);
		return 1;
	}
	if (bits_per_sample != 16) {
		fprintf(stderr, "Unsupported WAV sample depth %d\n", bits_per_sample);
		return 1;
	}
	switch (channels) {
	case 1: mode = MODE_1;       break;
	case 2: mode = MODE_2;       break;
	case 3: mode = MODE_1_2;     break;
	case 4: mode = MODE_1_2_1;   break;
	case 5: mode = MODE_1_2_2;   break;
	case 6: mode = MODE_1_2_2_1; break;
	default:
		fprintf(stderr, "Unsupported WAV channels %d\n", channels);
		return 1;
	}
	if (aacEncOpen(&encoder, 0, channels) != AACENC_OK) {
		fprintf(stderr, "Unable to open encoder\n");
		return 1;
	}
	if (aacEncoder_SetParam(encoder, AACENC_AOT, aot) != AACENC_OK) {
		fprintf(stderr, "Unable to set the AOT\n");
		return 1;
	}
	if (aot == 39 && eld_sbr) {
		if (aacEncoder_SetParam(encoder, AACENC_SBR_MODE, 1) != AACENC_OK) {
			fprintf(stderr, "Unable to set SBR mode for ELD\n");
			return 1;
		}
	}
	if (aacEncoder_SetParam(encoder, AACENC_SAMPLERATE, sample_rate) != AACENC_OK) {
		fprintf(stderr, "Unable to set the AOT\n");
		return 1;
	}
	if (aacEncoder_SetParam(encoder, AACENC_CHANNELMODE, mode) != AACENC_OK) {
		fprintf(stderr, "Unable to set the channel mode\n");
		return 1;
	}
	if (aacEncoder_SetParam(encoder, AACENC_CHANNELORDER, 1) != AACENC_OK) {
		fprintf(stderr, "Unable to set the wav channel order\n");
		return 1;
	}
	if (vbr) {
		if (aacEncoder_SetParam(encoder, AACENC_BITRATEMODE, vbr) != AACENC_OK) {
			fprintf(stderr, "Unable to set the VBR bitrate mode\n");
			return 1;
		}
	} else {
		if (aacEncoder_SetParam(encoder, AACENC_BITRATE, bitrate) != AACENC_OK) {
			fprintf(stderr, "Unable to set the bitrate\n");
			return 1;
		}
	}
	if (aacEncoder_SetParam(encoder, AACENC_TRANSMUX, adts ? 2 : 0) != AACENC_OK) {
		fprintf(stderr, "Unable to set the ADTS transmux\n");
		return 1;
	}
	if (aacEncoder_SetParam(encoder, AACENC_AFTERBURNER, afterburner) != AACENC_OK) {
		fprintf(stderr, "Unable to set the afterburner mode\n");
		return 1;
	}
	if (aacEncEncode(encoder, NULL, NULL, NULL, NULL) != AACENC_OK) {
		fprintf(stderr, "Unable to initialize the encoder\n");
		return 1;
	}
	if (aacEncInfo(encoder, &info) != AACENC_OK) {
		fprintf(stderr, "Unable to get the encoder info\n");
		return 1;
	}

	input_size = channels*2*info.frameLength;
	input_buf = (uint8_t*) malloc(input_size);
	convert_buf = (int16_t*) malloc(input_size);

	decoder_output_skip = channels * info.nDelay;

	decoder = aacDecoder_Open(adts ? TT_MP4_ADTS : TT_MP4_RAW, 1);
	if (!adts) {
		UCHAR *bufArray[] = { info.confBuf };
		if (aacDecoder_ConfigRaw(decoder, (UCHAR**) bufArray, &info.confSize) != AAC_DEC_OK) {
			fprintf(stderr, "Unable to set ASC\n");
			ret = 1;
			goto end;
		}
	}
	aacDecoder_SetParam(decoder, AAC_CONCEAL_METHOD, 1);
	aacDecoder_SetParam(decoder, AAC_PCM_LIMITER_ENABLE, 0);
	while (1) {
		AACENC_BufDesc in_buf = { 0 }, out_buf = { 0 };
		AACENC_InArgs in_args = { 0 };
		AACENC_OutArgs out_args = { 0 };
		int in_identifier = IN_AUDIO_DATA;
		int in_size, in_elem_size;
		int out_identifier = OUT_BITSTREAM_DATA;
		int out_size, out_elem_size;
		int read, i;
		void *in_ptr, *out_ptr;
		uint8_t outbuf[20480];
		AACENC_ERROR err;

		read = wav_read_data(wav, input_buf, input_size);
		for (i = 0; i < read/2; i++) {
			const uint8_t* in = &input_buf[2*i];
			convert_buf[i] = in[0] | (in[1] << 8);
		}
		in_ptr = convert_buf;
		in_size = read;
		in_elem_size = 2;

		in_buf.numBufs = 1;
		in_buf.bufs = &in_ptr;
		in_buf.bufferIdentifiers = &in_identifier;
		in_buf.bufSizes = &in_size;
		in_buf.bufElSizes = &in_elem_size;

		if (read <= 0) {
			in_args.numInSamples = -1;
		} else {
			in_args.numInSamples = read/2;
			append_encoder_input(convert_buf, in_args.numInSamples);
		}
		out_ptr = outbuf;
		out_size = sizeof(outbuf);
		out_elem_size = 1;
		out_buf.numBufs = 1;
		out_buf.bufs = &out_ptr;
		out_buf.bufferIdentifiers = &out_identifier;
		out_buf.bufSizes = &out_size;
		out_buf.bufElSizes = &out_elem_size;

		if ((err = aacEncEncode(encoder, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK) {
			if (err == AACENC_ENCODE_EOF)
				break;
			fprintf(stderr, "Encoding failed\n");
			ret = 1;
			goto end;
		}
		if (out_args.numOutBytes == 0)
			continue;

		if (decode(decoder, outbuf, out_args.numOutBytes, decoder_buffer, decoder_buffer_size, channels)) {
			ret = 1;
			goto end;
		}
	}

	if (encoder_input_samples > 0) {
		fprintf(stderr, "%d unmatched samples left at the end\n", encoder_input_samples);
		ret = 1;
		goto end;
	}
	avg_diff = 0;
	if (diff_samples > 0)
		avg_diff = diff_sum / diff_samples;
	if (/*max_diff > 10000 ||*/ avg_diff > ((aot == 23) ? 2500 : (aot == 29) ? 1500 : 300)) {
		fprintf(stderr, "max_diff %d, avg_diff %d\n", max_diff, avg_diff);
		ret = 1;
		goto end;
	}

end:
	free(input_buf);
	free(convert_buf);
	wav_read_close(wav);
	aacEncClose(&encoder);
	free(decoder_buffer);
	aacDecoder_Close(decoder);

	return ret;
}

int main(int argc, char *argv[]) {
	const char* infile;
	void *wav;
	int sample_rate, channels;
	int failures = 0;
	int i;
	unsigned char encode_digest[SHA_DIGEST_LENGTH], decode_digest[SHA_DIGEST_LENGTH];
	if (argc < 2) {
		printf("%s input.wav\n", argv[0]);
		return 1;
	}
	infile = argv[1];

	wav = wav_read_open(infile);
	if (!wav) {
		fprintf(stderr, "Unable to open wav file %s\n", infile);
		return 1;
	}
	if (!wav_get_header(wav, NULL, &channels, &sample_rate, NULL, NULL)) {
		fprintf(stderr, "Bad wav file %s\n", infile);
		return 1;
	}
	wav_read_close(wav);

	SHA1Reset(&encode_hash);
	SHA1Reset(&decode_hash);

	failures += test_encode_decode(infile,  2, 0, 0, 0, 64000, 0); // AAC-LC, without afterburner
	for (i = 0; i < 2; i++)
		failures += test_encode_decode(infile, 2, 1, 0, 0, 64000, i); // AAC-LC
	for (i = 1; i <= 5; i++)
		failures += test_encode_decode(infile, 2, 1, 0, i, 0, 0); // AAC-LC VBR
	if (channels == 2) {
		// HE-AACv2 only works for stereo; HE-AACv1 gets upconverted to stereo (which we don't match properly)
		for (i = 0; i < 2; i++)
			failures += test_encode_decode(infile, 5, 1, 0, 0, 64000, i); // HE-AAC
		for (i = 1; i <= 5; i++)
			failures += test_encode_decode(infile, 5, 1, 0, i, 0, 0); // HE-AAC VBR
		for (i = 0; i < 2; i++)
			failures += test_encode_decode(infile, 29, 1, 0, 0, 64000, i); // HE-AACv2
		for (i = 1; i <= 5; i++)
			failures += test_encode_decode(infile, 29, 1, 0, i, 0, 0); // HE-AACv2 VBR
	}
	if (channels == 1)
		failures += test_encode_decode(infile, 23, 1, 0, 0, 64000, 0); // AAC-LD
	failures += test_encode_decode(infile, 39, 1, 0, 0, 64000, 0); // AAC-ELD
	failures += test_encode_decode(infile, 39, 1, 1, 0, 64000, 0); // AAC-ELD with SBR

	free_encoder_input();
	fprintf(stderr, "%d failures\n", failures);
	SHA1Result(&encode_hash, encode_digest);
	SHA1Result(&decode_hash, decode_digest);
	printf("encode hash: ");
	for (i = 0; i < SHA_DIGEST_LENGTH; i++)
		printf("%02x", encode_digest[i]);
	printf("\n");
	printf("decode hash: ");
	for (i = 0; i < SHA_DIGEST_LENGTH; i++)
		printf("%02x", decode_digest[i]);
	printf("\n");
	return failures;
}
