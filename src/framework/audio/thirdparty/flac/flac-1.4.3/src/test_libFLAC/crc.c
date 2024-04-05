/* test_libFLAC - Unit tester for libFLAC
 * Copyright (C) 2014-2023  Xiph.Org Foundation
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

#include <stdio.h>

#include "FLAC/assert.h"
#include "share/compat.h"
#include "private/crc.h"
#include "crc.h"

static FLAC__uint8 crc8_update_ref(FLAC__byte byte, FLAC__uint8 crc);
static FLAC__uint16 crc16_update_ref(FLAC__byte byte, FLAC__uint16 crc);

static FLAC__bool test_crc8(const FLAC__byte *data, size_t size);
static FLAC__bool test_crc16(const FLAC__byte *data, size_t size);
static FLAC__bool test_crc16_update(const FLAC__byte *data, size_t size);
static FLAC__bool test_crc16_32bit_words(const FLAC__uint32 *words, size_t size);
static FLAC__bool test_crc16_64bit_words(const FLAC__uint64 *words, size_t size);

#define DATA_SIZE 32768

FLAC__bool test_crc(void)
{
	uint32_t i;
	FLAC__byte data[DATA_SIZE] = { 0 };

	/* Initialize data reproducibly with pseudo-random values. */
	for (i = 1; i < DATA_SIZE; i++)
		data[i] = crc8_update_ref(i % 256, data[i - 1]);

	printf("\n+++ libFLAC unit test: crc\n\n");

	if (! test_crc8(data, DATA_SIZE))
		return false;

	if (! test_crc16(data, DATA_SIZE))
		return false;

	if (! test_crc16_update(data, DATA_SIZE))
		return false;

	if (! test_crc16_32bit_words((FLAC__uint32 *)data, DATA_SIZE / 4))
		return false;

	if (! test_crc16_64bit_words((FLAC__uint64 *)data, DATA_SIZE / 8))
		return false;

	printf("\nPASSED!\n");
	return true;
}

/*----------------------------------------------------------------------------*/

/* Reference implementations of CRC-8 and CRC-16 to check against. */

#define CRC8_POLYNOMIAL 0x07

static FLAC__uint8 crc8_update_ref(FLAC__byte byte, FLAC__uint8 crc)
{
    uint32_t i;

    crc ^= byte;

    for (i = 0; i < 8; i++) {
        crc = (crc << 1) ^ ((crc >> 7) ? CRC8_POLYNOMIAL : 0);
    }

    return crc;
}

#define CRC16_POLYNOMIAL 0x8005

static FLAC__uint16 crc16_update_ref(FLAC__byte byte, FLAC__uint16 crc)
{
    uint32_t i;

    crc ^= byte << 8;

    for (i = 0; i < 8; i++) {
        crc = (crc << 1) ^ ((crc >> 15) ? CRC16_POLYNOMIAL : 0);
    }

    return crc;
}

/*----------------------------------------------------------------------------*/

static FLAC__bool test_crc8(const FLAC__byte *data, size_t size)
{
	uint32_t i;
	FLAC__uint8 crc0,crc1;

	printf("testing FLAC__crc8 ... ");

	crc0 = 0;
	crc1 = FLAC__crc8(data, 0);

	if (crc1 != crc0) {
		printf("FAILED, FLAC__crc8 returned non-zero CRC for zero bytes of data\n");
		return false;
	}

	for (i = 0; i < size; i++) {
		crc0 = crc8_update_ref(data[i], crc0);
		crc1 = FLAC__crc8(data, i + 1);

		if (crc1 != crc0) {
			printf("FAILED, FLAC__crc8 result did not match reference CRC for %u bytes of test data\n", i + 1);
			return false;
		}
	}

	printf("OK\n");

	return true;
}

static FLAC__bool test_crc16(const FLAC__byte *data, size_t size)
{
	uint32_t i;
	FLAC__uint16 crc0,crc1;

	printf("testing FLAC__crc16 ... ");

	crc0 = 0;
	crc1 = FLAC__crc16(data, 0);

	if (crc1 != crc0) {
		printf("FAILED, FLAC__crc16 returned non-zero CRC for zero bytes of data\n");
		return false;
	}

	for (i = 0; i < size; i++) {
		crc0 = crc16_update_ref(data[i], crc0);
		crc1 = FLAC__crc16(data, i + 1);

		if (crc1 != crc0) {
			printf("FAILED, FLAC__crc16 result did not match reference CRC for %u bytes of test data\n", i + 1);
			return false;
		}
	}

	printf("OK\n");

	return true;
}

static FLAC__bool test_crc16_update(const FLAC__byte *data, size_t size)
{
	uint32_t i;
	FLAC__uint16 crc0,crc1;

	printf("testing FLAC__CRC16_UPDATE macro ... ");

	crc0 = 0;
	crc1 = 0;

	for (i = 0; i < size; i++) {
		crc0 = crc16_update_ref(data[i], crc0);
		crc1 = FLAC__CRC16_UPDATE(data[i], crc1);

		if (crc1 != crc0) {
			printf("FAILED, FLAC__CRC16_UPDATE result did not match reference CRC after %u bytes of test data\n", i + 1);
			return false;
		}
	}

	printf("OK\n");

	return true;
}

static FLAC__bool test_crc16_32bit_words(const FLAC__uint32 *words, size_t size)
{
	uint32_t n,i,k;
	FLAC__uint16 crc0,crc1;

	for (n = 1; n <= 16; n++) {
		printf("testing FLAC__crc16_update_words32 (length=%i) ... ", n);

		crc0 = 0;
		crc1 = 0;

		for (i = 0; i <= size - n; i += n) {
			for (k = 0; k < n; k++) {
				crc0 = crc16_update_ref( words[i + k] >> 24,         crc0);
				crc0 = crc16_update_ref((words[i + k] >> 16) & 0xFF, crc0);
				crc0 = crc16_update_ref((words[i + k] >>  8) & 0xFF, crc0);
				crc0 = crc16_update_ref( words[i + k]        & 0xFF, crc0);
			}

			crc1 = FLAC__crc16_update_words32(words + i, n, crc1);

			if (crc1 != crc0) {
				printf("FAILED, FLAC__crc16_update_words32 result did not match reference CRC after %u words of test data\n", i + n);
				return false;
			}
		}

		crc1 = FLAC__crc16_update_words32(words, 0, crc1);

		if (crc1 != crc0) {
			printf("FAILED, FLAC__crc16_update_words32 called with zero bytes changed CRC value\n");
			return false;
		}

		printf("OK\n");
	}

	return true;
}

static FLAC__bool test_crc16_64bit_words(const FLAC__uint64 *words, size_t size)
{
	uint32_t n,i,k;
	FLAC__uint16 crc0,crc1;

	for (n = 1; n <= 16; n++) {
		printf("testing FLAC__crc16_update_words64 (length=%i) ... ", n);

		crc0 = 0;
		crc1 = 0;

		for (i = 0; i <= size - n; i += n) {
			for (k = 0; k < n; k++) {
				crc0 = crc16_update_ref( words[i + k] >> 56,         crc0);
				crc0 = crc16_update_ref((words[i + k] >> 48) & 0xFF, crc0);
				crc0 = crc16_update_ref((words[i + k] >> 40) & 0xFF, crc0);
				crc0 = crc16_update_ref((words[i + k] >> 32) & 0xFF, crc0);
				crc0 = crc16_update_ref((words[i + k] >> 24) & 0xFF, crc0);
				crc0 = crc16_update_ref((words[i + k] >> 16) & 0xFF, crc0);
				crc0 = crc16_update_ref((words[i + k] >>  8) & 0xFF, crc0);
				crc0 = crc16_update_ref( words[i + k]        & 0xFF, crc0);
			}

			crc1 = FLAC__crc16_update_words64(words + i, n, crc1);

			if (crc1 != crc0) {
				printf("FAILED, FLAC__crc16_update_words64 result did not match reference CRC after %u words of test data\n", i + n);
				return false;
			}
		}

		crc1 = FLAC__crc16_update_words64(words, 0, crc1);

		if (crc1 != crc0) {
			printf("FAILED, FLAC__crc16_update_words64 called with zero bytes changed CRC value\n");
			return false;
		}

		printf("OK\n");
	}

	return true;
}
