/* ------------------------------------------------------------------
 * Copyright (C) 2009 Martin Storsjo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#include "wavreader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define TAG(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

struct wav_reader {
	FILE *wav;
	uint32_t data_length;

	int format;
	int sample_rate;
	int bits_per_sample;
	int channels;
	int byte_rate;
	int block_align;

	int streamed;
};

static uint32_t read_tag(struct wav_reader* wr) {
	uint32_t tag = 0;
	tag = (tag << 8) | fgetc(wr->wav);
	tag = (tag << 8) | fgetc(wr->wav);
	tag = (tag << 8) | fgetc(wr->wav);
	tag = (tag << 8) | fgetc(wr->wav);
	return tag;
}

static uint32_t read_int32(struct wav_reader* wr) {
	uint32_t value = 0;
	value |= fgetc(wr->wav) <<  0;
	value |= fgetc(wr->wav) <<  8;
	value |= fgetc(wr->wav) << 16;
	value |= fgetc(wr->wav) << 24;
	return value;
}

static uint16_t read_int16(struct wav_reader* wr) {
	uint16_t value = 0;
	value |= fgetc(wr->wav) << 0;
	value |= fgetc(wr->wav) << 8;
	return value;
}

static void skip(FILE *f, int n) {
	int i;
	for (i = 0; i < n; i++)
		fgetc(f);
}

void* wav_read_open(const char *filename) {
	struct wav_reader* wr = (struct wav_reader*) malloc(sizeof(*wr));
	long data_pos = 0;
	memset(wr, 0, sizeof(*wr));

	if (!strcmp(filename, "-"))
		wr->wav = stdin;
	else
		wr->wav = fopen(filename, "rb");
	if (wr->wav == NULL) {
		free(wr);
		return NULL;
	}

	while (1) {
		uint32_t tag, tag2, length;
		tag = read_tag(wr);
		if (feof(wr->wav))
			break;
		length = read_int32(wr);
		if (!length || length >= 0x7fff0000) {
			wr->streamed = 1;
			length = ~0;
		}
		if (tag != TAG('R', 'I', 'F', 'F') || length < 4) {
			fseek(wr->wav, length, SEEK_CUR);
			continue;
		}
		tag2 = read_tag(wr);
		length -= 4;
		if (tag2 != TAG('W', 'A', 'V', 'E')) {
			fseek(wr->wav, length, SEEK_CUR);
			continue;
		}
		// RIFF chunk found, iterate through it
		while (length >= 8) {
			uint32_t subtag, sublength;
			subtag = read_tag(wr);
			if (feof(wr->wav))
				break;
			sublength = read_int32(wr);
			length -= 8;
			if (length < sublength)
				break;
			if (subtag == TAG('f', 'm', 't', ' ')) {
				if (sublength < 16) {
					// Insufficient data for 'fmt '
					break;
				}
				wr->format          = read_int16(wr);
				wr->channels        = read_int16(wr);
				wr->sample_rate     = read_int32(wr);
				wr->byte_rate       = read_int32(wr);
				wr->block_align     = read_int16(wr);
				wr->bits_per_sample = read_int16(wr);
				if (wr->format == 0xfffe) {
					if (sublength < 28) {
						// Insufficient data for waveformatex
						break;
					}
					skip(wr->wav, 8);
					wr->format = read_int32(wr);
					skip(wr->wav, sublength - 28);
				} else {
					skip(wr->wav, sublength - 16);
				}
			} else if (subtag == TAG('d', 'a', 't', 'a')) {
				data_pos = ftell(wr->wav);
				wr->data_length = sublength;
				if (!wr->data_length || wr->streamed) {
					wr->streamed = 1;
					return wr;
				}
				fseek(wr->wav, sublength, SEEK_CUR);
			} else {
				skip(wr->wav, sublength);
			}
			length -= sublength;
		}
		if (length > 0) {
			// Bad chunk?
			fseek(wr->wav, length, SEEK_CUR);
		}
	}
	fseek(wr->wav, data_pos, SEEK_SET);
	return wr;
}

void wav_read_close(void* obj) {
	struct wav_reader* wr = (struct wav_reader*) obj;
	if (wr->wav != stdin)
		fclose(wr->wav);
	free(wr);
}

int wav_get_header(void* obj, int* format, int* channels, int* sample_rate, int* bits_per_sample, unsigned int* data_length) {
	struct wav_reader* wr = (struct wav_reader*) obj;
	if (format)
		*format = wr->format;
	if (channels)
		*channels = wr->channels;
	if (sample_rate)
		*sample_rate = wr->sample_rate;
	if (bits_per_sample)
		*bits_per_sample = wr->bits_per_sample;
	if (data_length)
		*data_length = wr->data_length;
	return wr->format && wr->sample_rate;
}

int wav_read_data(void* obj, unsigned char* data, unsigned int length) {
	struct wav_reader* wr = (struct wav_reader*) obj;
	int n;
	if (wr->wav == NULL)
		return -1;
	if (length > wr->data_length && !wr->streamed)
		length = wr->data_length;
	n = fread(data, 1, length, wr->wav);
	wr->data_length -= length;
	return n;
}

