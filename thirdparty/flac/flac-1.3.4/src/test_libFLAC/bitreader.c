/* test_libFLAC - Unit tester for libFLAC
 * Copyright (C) 2000-2009  Josh Coalson
 * Copyright (C) 2011-2018  Xiph.Org Foundation
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
#include "private/bitreader.h" /* from the libFLAC private include area */
#include "bitreader.h"
#include <stdio.h>
#include <string.h> /* for memcpy() */

/*
 * WATCHOUT!  Since FLAC__BitReader is a private structure, we use a copy of
 * the definition here to get at the internals.  Make sure this is kept up
 * to date with what is in ../libFLAC/bitreader.c
 */
#if (ENABLE_64_BIT_WORDS == 0)

typedef FLAC__uint32 brword;
#define FLAC__BYTES_PER_WORD 4
#define FLAC__BITS_PER_WORD 32

#else

typedef FLAC__uint64 brword;
#define FLAC__BYTES_PER_WORD 8
#define FLAC__BITS_PER_WORD 64

#endif

struct FLAC__BitReader {
	/* any partially-consumed word at the head will stay right-justified as bits are consumed from the left */
	/* any incomplete word at the tail will be left-justified, and bytes from the read callback are added on the right */
	brword *buffer;
	uint32_t capacity; /* in words */
	uint32_t words; /* # of completed words in buffer */
	uint32_t bytes; /* # of bytes in incomplete word at buffer[words] */
	uint32_t consumed_words; /* #words ... */
	uint32_t consumed_bits; /* ... + (#bits of head word) already consumed from the front of buffer */
	uint32_t read_crc16; /* the running frame CRC */
	uint32_t crc16_offset; /* the number of words in the current buffer that should not be CRC'd */
	uint32_t crc16_align; /* the number of bits in the current consumed word that should not be CRC'd */
	FLAC__BitReaderReadCallback read_callback;
	void *client_data;
};

static FLAC__bool read_callback(FLAC__byte buffer[], size_t *bytes, void *data);

FLAC__bool test_bitreader(void)
{
	FLAC__BitReader *br;
	FLAC__bool ok;
	uint32_t i;
	uint32_t words, bits; /* what we think br->consumed_words and br->consumed_bits should be */

	FLAC__uint16	 crc,expected_crcs[4] = { 0x5e4c, 0x7f6b, 0x2272, 0x42bf };
	FLAC__byte	 data[32];

	FLAC__uint32	 val_uint32;
	FLAC__uint64	 val_uint64;

	for (i = 0; i < 32; i++)
		data[i] = i * 8 + 7;

	printf("\n+++ libFLAC unit test: bitreader\n\n");

	/*
	 * test new -> delete
	 */
	printf("testing new... ");
	br = FLAC__bitreader_new();
	if(0 == br) {
		printf("FAILED, returned NULL\n");
		return false;
	}
	printf("OK\n");

	printf("testing delete... ");
	FLAC__bitreader_delete(br);
	printf("OK\n");

	/*
	 * test new -> init -> delete
	 */
	printf("testing new... ");
	br = FLAC__bitreader_new();
	if(0 == br) {
		printf("FAILED, returned NULL\n");
		return false;
	}
	printf("OK\n");

	printf("testing init... ");
	if(!FLAC__bitreader_init(br, read_callback, data)) {
		printf("FAILED, returned false\n");
		return false;
	}
	printf("OK\n");

	printf("testing delete... ");
	FLAC__bitreader_delete(br);
	printf("OK\n");

	/*
	 * test new -> init -> clear -> delete
	 */
	printf("testing new... ");
	br = FLAC__bitreader_new();
	if(0 == br) {
		printf("FAILED, returned NULL\n");
		return false;
	}
	printf("OK\n");

	printf("testing init... ");
	if(!FLAC__bitreader_init(br, read_callback, data)) {
		printf("FAILED, returned false\n");
		return false;
	}
	printf("OK\n");

	printf("testing clear... ");
	if(!FLAC__bitreader_clear(br)) {
		printf("FAILED, returned false\n");
		return false;
	}
	printf("OK\n");

	printf("testing delete... ");
	FLAC__bitreader_delete(br);
	printf("OK\n");

	/*
	 * test normal usage
	 */
	printf("testing new... ");
	br = FLAC__bitreader_new();
	if(0 == br) {
		printf("FAILED, returned NULL\n");
		return false;
	}
	printf("OK\n");

	printf("testing init... ");
	if(!FLAC__bitreader_init(br, read_callback, data)) {
		printf("FAILED, returned false\n");
		return false;
	}
	printf("OK\n");

	printf("testing clear... ");
	if(!FLAC__bitreader_clear(br)) {
		printf("FAILED, returned false\n");
		return false;
	}
	printf("OK\n");

	words = bits = 0;

	printf("capacity = %u\n", br->capacity);

	printf("testing raw reads... ");
	ok =
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 1) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 2) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 5) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 8) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 10) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 4) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 32) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 4) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 2) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 8) &&
		FLAC__bitreader_read_raw_uint64(br, &val_uint64, 64) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 12)
	;
	if(!ok) {
		printf("FAILED\n");
		FLAC__bitreader_dump(br, stdout);
		return false;
	}
	/* we read 152 bits (=19 bytes) from the bitreader */
	words = 152 / FLAC__BITS_PER_WORD;
	bits = 152 - words*FLAC__BITS_PER_WORD;

	if(br->consumed_words != words) {
		printf("FAILED word count %u != %u\n", br->consumed_words, words);
		FLAC__bitreader_dump(br, stdout);
		return false;
	}
	if(br->consumed_bits != bits) {
		printf("FAILED bit count %u != %u\n", br->consumed_bits, bits);
		FLAC__bitreader_dump(br, stdout);
		return false;
	}
	crc = FLAC__bitreader_get_read_crc16(br);
	if(crc != expected_crcs[0]) {
		printf("FAILED reported CRC 0x%04x does not match expected 0x%04x\n", crc, expected_crcs[0]);
		FLAC__bitreader_dump(br, stdout);
		return false;
	}
	printf("OK\n");
	FLAC__bitreader_dump(br, stdout);

	printf("testing CRC reset... ");
	FLAC__bitreader_clear(br);
	FLAC__bitreader_reset_read_crc16(br, 0xFFFF);
	crc = FLAC__bitreader_get_read_crc16(br);
	if(crc != 0xFFFF) {
		printf("FAILED reported CRC 0x%04x does not match expected 0xFFFF\n", crc);
		FLAC__bitreader_dump(br, stdout);
		return false;
	}
	FLAC__bitreader_reset_read_crc16(br, 0);
	crc = FLAC__bitreader_get_read_crc16(br);
	if(crc != 0) {
		printf("FAILED reported CRC 0x%04x does not match expected 0x0000\n", crc);
		FLAC__bitreader_dump(br, stdout);
		return false;
	}
	FLAC__bitreader_read_raw_uint32(br, &val_uint32, 16);
	FLAC__bitreader_reset_read_crc16(br, 0);
	FLAC__bitreader_read_raw_uint32(br, &val_uint32, 32);
	crc = FLAC__bitreader_get_read_crc16(br);
	if(crc != expected_crcs[1]) {
		printf("FAILED reported CRC 0x%04x does not match expected 0x%04x\n", crc, expected_crcs[1]);
		FLAC__bitreader_dump(br, stdout);
		return false;
	}
	printf("OK\n");

	printf("testing unaligned < 32 bit reads... ");
	FLAC__bitreader_clear(br);
	FLAC__bitreader_skip_bits_no_crc(br, 8);
	FLAC__bitreader_reset_read_crc16(br, 0);
	ok =
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 1) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 2) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 5) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 8)
	;
	if(!ok) {
		printf("FAILED\n");
		FLAC__bitreader_dump(br, stdout);
		return false;
	}
	crc = FLAC__bitreader_get_read_crc16(br);
	if(crc != expected_crcs[2]) {
		printf("FAILED reported CRC 0x%04x does not match expected 0x%04x\n", crc, expected_crcs[2]);
		FLAC__bitreader_dump(br, stdout);
		return false;
	}
	printf("OK\n");
	FLAC__bitreader_dump(br, stdout);

	printf("testing unaligned < 64 bit reads... ");
	FLAC__bitreader_clear(br);
	FLAC__bitreader_skip_bits_no_crc(br, 8);
	FLAC__bitreader_reset_read_crc16(br, 0);
	ok =
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 1) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 2) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 5) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 8) &&
		FLAC__bitreader_read_raw_uint32(br, &val_uint32, 32)
	;
	if(!ok) {
		printf("FAILED\n");
		FLAC__bitreader_dump(br, stdout);
		return false;
	}
	crc = FLAC__bitreader_get_read_crc16(br);
	if(crc != expected_crcs[3]) {
		printf("FAILED reported CRC 0x%04x does not match expected 0x%04x\n", crc, expected_crcs[3]);
		FLAC__bitreader_dump(br, stdout);
		return false;
	}
	printf("OK\n");
	FLAC__bitreader_dump(br, stdout);

	printf("testing free... ");
	FLAC__bitreader_free(br);
	printf("OK\n");

	printf("testing delete... ");
	FLAC__bitreader_delete(br);
	printf("OK\n");

	printf("\nPASSED!\n");
	return true;
}

/*----------------------------------------------------------------------------*/

static FLAC__bool read_callback(FLAC__byte buffer[], size_t *bytes, void *data)
{
	if (*bytes > 32)
		*bytes = 32;

	memcpy(buffer, data, *bytes);

	return true;
}
