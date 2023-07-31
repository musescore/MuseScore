/* test_libFLAC - Unit tester for libFLAC
 * Copyright (C) 2000-2009  Josh Coalson
 * Copyright (C) 2011-2023  Xiph.Org Foundation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "FLAC/assert.h"
#include "share/compat.h"
#include "private/bitwriter.h" /* from the libFLAC private include area */
#include "bitwriter.h"
#include <stdio.h>
#include <string.h> /* for memcmp() */

/*
 * WATCHOUT!  Since FLAC__BitWriter is a private structure, we use a copy of
 * the definition here to get at the internals.  Make sure this is kept up
 * to date with what is in ../libFLAC/bitwriter.c
 */
#if (ENABLE_64_BIT_WORDS == 0)

typedef FLAC__uint32 bwword;
#define FLAC__BYTES_PER_WORD 4
#define FLAC__BITS_PER_WORD 32
#define PRI_BWWORD "08x"

#else

typedef FLAC__uint64 bwword;
#define FLAC__BYTES_PER_WORD 8
#define FLAC__BITS_PER_WORD 64
#define PRI_BWWORD "016" PRIx64

#endif

struct FLAC__BitWriter {
	bwword *buffer;
	bwword accum; /* accumulator; bits are right-justified; when full, accum is appended to buffer */
	uint32_t capacity; /* capacity of buffer in words */
	uint32_t words; /* # of complete words in buffer */
	uint32_t bits; /* # of used bits in accum */
};

#define WORDS_TO_BITS(words) ((words) * FLAC__BITS_PER_WORD)
#define TOTAL_BITS(bw) (WORDS_TO_BITS((bw)->words) + (bw)->bits)

static void FLAC__bitwriter_dump(const FLAC__BitWriter *bw, FILE *out)
{
	uint32_t i, j;
	if(bw == 0) {
		fprintf(out, "bitwriter is NULL\n");
	}
	else {
		fprintf(out, "bitwriter: capacity=%u words=%u bits=%u total_bits=%u\n", bw->capacity, bw->words, bw->bits, TOTAL_BITS(bw));

		for(i = 0; i < bw->words; i++) {
			fprintf(out, "%08X: ", i);
			for(j = 0; j < FLAC__BITS_PER_WORD; j++)
				fprintf(out, "%01d", bw->buffer[i] & ((bwword)1 << (FLAC__BITS_PER_WORD-j-1)) ? 1:0);
			fprintf(out, "\n");
		}
		if(bw->bits > 0) {
			fprintf(out, "%08X: ", i);
			for(j = 0; j < bw->bits; j++)
				fprintf(out, "%01d", bw->accum & ((bwword)1 << (bw->bits-j-1)) ? 1:0);
			fprintf(out, "\n");
		}
	}
}

FLAC__bool test_bitwriter(void)
{
	FLAC__BitWriter *bw;
	FLAC__bool ok;
	uint32_t i, j;
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	static bwword test_pattern1[5] = { 0xaaf0aabe, 0xaaaaaaa8, 0x300aaaaa, 0xaaadeadb, 0x00eeface };
#else
	static bwword test_pattern1[5] = { 0xbeaaf0aa, 0xa8aaaaaa, 0xaaaa0a30, 0xdbeaadaa, 0x00eeface };
#endif
#elif FLAC__BYTES_PER_WORD == 8
#if WORDS_BIGENDIAN
	static bwword test_pattern1[3] = { FLAC__U64L(0xaaf0aabeaaaaaaa8), FLAC__U64L(0x300aaaaaaaadeadb), FLAC__U64L(0x0000000000eeface) };
#else
	static bwword test_pattern1[3] = { FLAC__U64L(0xa8aaaaaabeaaf0aa), FLAC__U64L(0xdbeaadaaaaaa0a30), FLAC__U64L(0x0000000000eeface) };
#endif
#else
#error FLAC__BYTES_PER_WORD is neither 4 nor 8 -- not implemented
#endif
	uint32_t words, bits; /* what we think bw->words and bw->bits should be */

	printf("\n+++ libFLAC unit test: bitwriter\n\n");

	/*
	 * test new -> delete
	 */
	printf("testing new... ");
	bw = FLAC__bitwriter_new();
	if(0 == bw) {
		printf("FAILED, returned NULL\n");
		return false;
	}
	printf("OK\n");

	printf("testing delete... ");
	FLAC__bitwriter_delete(bw);
	printf("OK\n");

	/*
	 * test new -> init -> delete
	 */
	printf("testing new... ");
	bw = FLAC__bitwriter_new();
	if(0 == bw) {
		printf("FAILED, returned NULL\n");
		return false;
	}
	printf("OK\n");

	printf("testing init... ");
	if(!FLAC__bitwriter_init(bw)) {
		printf("FAILED, returned false\n");
		return false;
	}
	printf("OK\n");

	printf("testing delete... ");
	FLAC__bitwriter_delete(bw);
	printf("OK\n");

	/*
	 * test new -> init -> clear -> delete
	 */
	printf("testing new... ");
	bw = FLAC__bitwriter_new();
	if(0 == bw) {
		printf("FAILED, returned NULL\n");
		return false;
	}
	printf("OK\n");

	printf("testing init... ");
	if(!FLAC__bitwriter_init(bw)) {
		printf("FAILED, returned false\n");
		return false;
	}
	printf("OK\n");

	printf("testing clear... ");
	FLAC__bitwriter_clear(bw);
	printf("OK\n");

	printf("testing delete... ");
	FLAC__bitwriter_delete(bw);
	printf("OK\n");

	/*
	 * test normal usage
	 */
	printf("testing new... ");
	bw = FLAC__bitwriter_new();
	if(0 == bw) {
		printf("FAILED, returned NULL\n");
		return false;
	}
	printf("OK\n");

	printf("testing init... ");
	if(!FLAC__bitwriter_init(bw)) {
		printf("FAILED, returned false\n");
		return false;
	}
	printf("OK\n");

	printf("testing clear... ");
	FLAC__bitwriter_clear(bw);
	printf("OK\n");

	words = bits = 0;

	printf("capacity = %u\n", bw->capacity);

	printf("testing zeroes, raw_uint32*... ");
	ok =
		FLAC__bitwriter_write_raw_uint32(bw, 0x1, 1) &&
		FLAC__bitwriter_write_raw_uint32(bw, 0x1, 2) &&
		FLAC__bitwriter_write_raw_uint32(bw, 0xa, 5) &&
		FLAC__bitwriter_write_raw_uint32(bw, 0xf0, 8) &&
		FLAC__bitwriter_write_raw_uint32(bw, 0x2aa, 10) &&
		FLAC__bitwriter_write_raw_uint32(bw, 0xf, 4) &&
		FLAC__bitwriter_write_raw_uint32(bw, 0xaaaaaaaa, 32) &&
		FLAC__bitwriter_write_zeroes(bw, 4) &&
		FLAC__bitwriter_write_raw_uint32(bw, 0x3, 2) &&
		FLAC__bitwriter_write_zeroes(bw, 8) &&
		FLAC__bitwriter_write_raw_uint64(bw, FLAC__U64L(0xaaaaaaaadeadbeef), 64) &&
		FLAC__bitwriter_write_raw_uint32(bw, 0xace, 12)
	;
	if(!ok) {
		printf("FAILED\n");
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}
	/* we wrote 152 bits (=19 bytes) to the bitwriter */
	words = 152 / FLAC__BITS_PER_WORD;
	bits = 152 - words*FLAC__BITS_PER_WORD;

	if(bw->words != words) {
		printf("FAILED word count %u != %u\n", bw->words, words);
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}
	if(bw->bits != bits) {
		printf("FAILED bit count %u != %u\n", bw->bits, bits);
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}
	if(memcmp(bw->buffer, test_pattern1, sizeof(bwword)*words) != 0) {
		printf("FAILED pattern match (buffer)\n");
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}
	if((bw->accum & 0x00ffffff) != test_pattern1[words]) {
		printf("FAILED pattern match (bw->accum=%" PRI_BWWORD " != %" PRI_BWWORD ")\n", bw->accum&0x00ffffff, test_pattern1[words]);
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}
	printf("OK\n");
	FLAC__bitwriter_dump(bw, stdout);

	printf("testing raw_uint32 some more... ");
	ok = FLAC__bitwriter_write_raw_uint32(bw, 0x3d, 6);
	if(!ok) {
		printf("FAILED\n");
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}
	bits += 6;
	test_pattern1[words] <<= 6;
	test_pattern1[words] |= 0x3d;
	if(bw->words != words) {
		printf("FAILED word count %u != %u\n", bw->words, words);
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}
	if(bw->bits != bits) {
		printf("FAILED bit count %u != %u\n", bw->bits, bits);
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}
	if(memcmp(bw->buffer, test_pattern1, sizeof(bwword)*words) != 0) {
		printf("FAILED pattern match (buffer)\n");
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}
	if((bw->accum & 0x3fffffff) != test_pattern1[words]) {
		printf("FAILED pattern match (bw->accum=%" PRI_BWWORD " != %" PRI_BWWORD ")\n", bw->accum&0x3fffffff, test_pattern1[words]);
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}
	printf("OK\n");
	FLAC__bitwriter_dump(bw, stdout);

	printf("testing utf8_uint32(0x00000000)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint32(bw, 0x00000000);
	ok = TOTAL_BITS(bw) == 8 && (bw->accum & 0xff) == 0;
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint32(0x0000007F)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint32(bw, 0x0000007F);
	ok = TOTAL_BITS(bw) == 8 && (bw->accum & 0xff) == 0x7F;
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint32(0x00000080)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint32(bw, 0x00000080);
	ok = TOTAL_BITS(bw) == 16 && (bw->accum & 0xffff) == 0xC280;
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint32(0x000007FF)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint32(bw, 0x000007FF);
	ok = TOTAL_BITS(bw) == 16 && (bw->accum & 0xffff) == 0xDFBF;
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint32(0x00000800)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint32(bw, 0x00000800);
	ok = TOTAL_BITS(bw) == 24 && (bw->accum & 0xffffff) == 0xE0A080;
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint32(0x0000FFFF)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint32(bw, 0x0000FFFF);
	ok = TOTAL_BITS(bw) == 24 && (bw->accum & 0xffffff) == 0xEFBFBF;
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint32(0x00010000)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint32(bw, 0x00010000);
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 32 && bw->buffer[0] == 0xF0908080;
#else
	ok = TOTAL_BITS(bw) == 32 && bw->buffer[0] == 0x808090F0;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 32 && (bw->accum & 0xffffffff) == 0xF0908080;
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint32(0x001FFFFF)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint32(bw, 0x001FFFFF);
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 32 && bw->buffer[0] == 0xF7BFBFBF;
#else
	ok = TOTAL_BITS(bw) == 32 && bw->buffer[0] == 0xBFBFBFF7;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 32 && (bw->accum & 0xffffffff) == 0xF7BFBFBF;
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint32(0x00200000)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint32(bw, 0x00200000);
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 40 && bw->buffer[0] == 0xF8888080 && (bw->accum & 0xff) == 0x80;
#else
	ok = TOTAL_BITS(bw) == 40 && bw->buffer[0] == 0x808088F8 && (bw->accum & 0xff) == 0x80;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 40 && (bw->accum & FLAC__U64L(0xffffffffff)) == FLAC__U64L(0xF888808080);
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint32(0x03FFFFFF)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint32(bw, 0x03FFFFFF);
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 40 && bw->buffer[0] == 0xFBBFBFBF && (bw->accum & 0xff) == 0xBF;
#else
	ok = TOTAL_BITS(bw) == 40 && bw->buffer[0] == 0xBFBFBFFB && (bw->accum & 0xff) == 0xBF;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 40 && (bw->accum & FLAC__U64L(0xffffffffff)) == FLAC__U64L(0xFBBFBFBFBF);
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint32(0x04000000)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint32(bw, 0x04000000);
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 48 && bw->buffer[0] == 0xFC848080 && (bw->accum & 0xffff) == 0x8080;
#else
	ok = TOTAL_BITS(bw) == 48 && bw->buffer[0] == 0x808084FC && (bw->accum & 0xffff) == 0x8080;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 48 && (bw->accum & FLAC__U64L(0xffffffffffff)) == FLAC__U64L(0xFC8480808080);
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint32(0x7FFFFFFF)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint32(bw, 0x7FFFFFFF);
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 48 && bw->buffer[0] == 0xFDBFBFBF && (bw->accum & 0xffff) == 0xBFBF;
#else
	ok = TOTAL_BITS(bw) == 48 && bw->buffer[0] == 0xBFBFBFFD && (bw->accum & 0xffff) == 0xBFBF;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 48 && (bw->accum & FLAC__U64L(0xffffffffffff)) == FLAC__U64L(0xFDBFBFBFBFBF);
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x0000000000000000)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x0000000000000000));
	ok = TOTAL_BITS(bw) == 8 && (bw->accum & 0xff) == 0;
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x000000000000007F)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x000000000000007F));
	ok = TOTAL_BITS(bw) == 8 && (bw->accum & 0xff) == 0x7F;
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x0000000000000080)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x0000000000000080));
	ok = TOTAL_BITS(bw) == 16 && (bw->accum & 0xffff) == 0xC280;
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x00000000000007FF)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x00000000000007FF));
	ok = TOTAL_BITS(bw) == 16 && (bw->accum & 0xffff) == 0xDFBF;
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x0000000000000800)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x0000000000000800));
	ok = TOTAL_BITS(bw) == 24 && (bw->accum & 0xffffff) == 0xE0A080;
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x000000000000FFFF)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x000000000000FFFF));
	ok = TOTAL_BITS(bw) == 24 && (bw->accum & 0xffffff) == 0xEFBFBF;
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x0000000000010000)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x0000000000010000));
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 32 && bw->buffer[0] == 0xF0908080;
#else
	ok = TOTAL_BITS(bw) == 32 && bw->buffer[0] == 0x808090F0;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 32 && (bw->accum & 0xffffffff) == 0xF0908080;
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x00000000001FFFFF)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x00000000001FFFFF));
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 32 && bw->buffer[0] == 0xF7BFBFBF;
#else
	ok = TOTAL_BITS(bw) == 32 && bw->buffer[0] == 0xBFBFBFF7;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 32 && (bw->accum & 0xffffffff) == 0xF7BFBFBF;
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x0000000000200000)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x0000000000200000));
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 40 && bw->buffer[0] == 0xF8888080 && (bw->accum & 0xff) == 0x80;
#else
	ok = TOTAL_BITS(bw) == 40 && bw->buffer[0] == 0x808088F8 && (bw->accum & 0xff) == 0x80;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 40 && (bw->accum & FLAC__U64L(0xffffffffff)) == FLAC__U64L(0xF888808080);
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x0000000003FFFFFF)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x0000000003FFFFFF));
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 40 && bw->buffer[0] == 0xFBBFBFBF && (bw->accum & 0xff) == 0xBF;
#else
	ok = TOTAL_BITS(bw) == 40 && bw->buffer[0] == 0xBFBFBFFB && (bw->accum & 0xff) == 0xBF;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 40 && (bw->accum & FLAC__U64L(0xffffffffff)) == FLAC__U64L(0xFBBFBFBFBF);
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x0000000004000000)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x0000000004000000));
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 48 && bw->buffer[0] == 0xFC848080 && (bw->accum & 0xffff) == 0x8080;
#else
	ok = TOTAL_BITS(bw) == 48 && bw->buffer[0] == 0x808084FC && (bw->accum & 0xffff) == 0x8080;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 48 && (bw->accum & FLAC__U64L(0xffffffffffff)) == FLAC__U64L(0xFC8480808080);
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x000000007FFFFFFF)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x000000007FFFFFFF));
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 48 && bw->buffer[0] == 0xFDBFBFBF && (bw->accum & 0xffff) == 0xBFBF;
#else
	ok = TOTAL_BITS(bw) == 48 && bw->buffer[0] == 0xBFBFBFFD && (bw->accum & 0xffff) == 0xBFBF;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 48 && (bw->accum & FLAC__U64L(0xffffffffffff)) == FLAC__U64L(0xFDBFBFBFBFBF);
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x0000000080000000)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x0000000080000000));
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 56 && bw->buffer[0] == 0xFE828080 && (bw->accum & 0xffffff) == 0x808080;
#else
	ok = TOTAL_BITS(bw) == 56 && bw->buffer[0] == 0x808082FE && (bw->accum & 0xffffff) == 0x808080;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 56 && (bw->accum & FLAC__U64L(0xffffffffffffff)) == FLAC__U64L(0xFE828080808080);
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing utf8_uint64(0x0000000FFFFFFFFF)... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_utf8_uint64(bw, FLAC__U64L(0x0000000FFFFFFFFF));
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == 56 && bw->buffer[0] == 0xFEBFBFBF && (bw->accum & 0xffffff) == 0xBFBFBF;
#else
	ok = TOTAL_BITS(bw) == 56 && bw->buffer[0] == 0xBFBFBFFE && (bw->accum & 0xffffff) == 0xBFBFBF;
#endif
#elif FLAC__BYTES_PER_WORD == 8
	ok = TOTAL_BITS(bw) == 56 && (bw->accum & FLAC__U64L(0xffffffffffffff)) == FLAC__U64L(0xFEBFBFBFBFBFBF);
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}

	printf("testing grow... ");
	FLAC__bitwriter_clear(bw);
	FLAC__bitwriter_write_raw_uint32(bw, 0x5, 4);
	j = bw->capacity;
	for(i = 0; i < j; i++)
		FLAC__bitwriter_write_raw_uint32(bw, 0xaaaaaaaa, 32);
#if FLAC__BYTES_PER_WORD == 4
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == i*32+4 && bw->buffer[0] == 0x5aaaaaaa && (bw->accum & 0xf) == 0xa;
#else
	ok = TOTAL_BITS(bw) == i*32+4 && bw->buffer[0] == 0xaaaaaa5a && (bw->accum & 0xf) == 0xa;
#endif
#elif FLAC__BYTES_PER_WORD == 8
#if WORDS_BIGENDIAN
	ok = TOTAL_BITS(bw) == i*32+4 && bw->buffer[0] == FLAC__U64L(0x5aaaaaaaaaaaaaaa) && (bw->accum & 0xf) == 0xa;
#else
	ok = TOTAL_BITS(bw) == i*32+4 && bw->buffer[0] == FLAC__U64L(0xaaaaaaaaaaaaaa5a) && (bw->accum & 0xf) == 0xa;
#endif
#endif
	printf("%s\n", ok?"OK":"FAILED");
	if(!ok) {
		FLAC__bitwriter_dump(bw, stdout);
		return false;
	}
	printf("capacity = %u\n", bw->capacity);

	printf("testing free... ");
	FLAC__bitwriter_free(bw);
	printf("OK\n");

	printf("testing delete... ");
	FLAC__bitwriter_delete(bw);
	printf("OK\n");

	printf("\nPASSED!\n");
	return true;
}
