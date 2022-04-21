#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>		/* for malloc() */
#include <string.h>		/* for memcpy() */

#include "private/md5.h"
#include "share/alloc.h"
#include "share/compat.h"
#include "share/endswap.h"

/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 *
 * Changed so as no longer to depend on Colin Plumb's `usual.h' header
 * definitions; now uses stuff from dpkg's config.h.
 *  - Ian Jackson <ijackson@nyx.cs.du.edu>.
 * Still in the public domain.
 *
 * Josh Coalson: made some changes to integrate with libFLAC.
 * Still in the public domain.
 */

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f,w,x,y,z,in,s) \
	 (w += f(x,y,z) + in, w = (w<<s | w>>(32-s)) + x)

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void FLAC__MD5Transform(FLAC__uint32 buf[4], FLAC__uint32 const in[16])
{
	register FLAC__uint32 a, b, c, d;

	a = buf[0];
	b = buf[1];
	c = buf[2];
	d = buf[3];

	MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
	MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
	MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
	MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
	MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
	MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
	MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
	MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
	MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
	MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
	MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
	MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
	MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
	MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
	MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
	MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

	MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
	MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
	MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
	MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
	MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
	MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
	MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
	MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
	MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
	MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
	MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
	MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
	MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
	MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
	MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
	MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

	MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
	MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
	MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
	MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
	MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
	MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
	MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
	MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
	MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
	MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
	MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
	MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
	MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
	MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
	MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
	MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

	MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
	MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
	MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
	MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
	MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
	MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
	MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
	MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
	MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
	MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
	MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
	MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
	MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
	MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
	MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
	MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}

#if WORDS_BIGENDIAN
//@@@@@@ OPT: use bswap/intrinsics
static void byteSwap(FLAC__uint32 *buf, uint32_t words)
{
	register FLAC__uint32 x;
	do {
		x = *buf;
		x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff);
		*buf++ = (x >> 16) | (x << 16);
	} while (--words);
}
static void byteSwapX16(FLAC__uint32 *buf)
{
	register FLAC__uint32 x;

	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf++ = (x >> 16) | (x << 16);
	x = *buf; x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); *buf   = (x >> 16) | (x << 16);
}
#else
#define byteSwap(buf, words)
#define byteSwapX16(buf)
#endif

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static void FLAC__MD5Update(FLAC__MD5Context *ctx, FLAC__byte const *buf, uint32_t len)
{
	FLAC__uint32 t;

	/* Update byte count */

	t = ctx->bytes[0];
	if ((ctx->bytes[0] = t + len) < t)
		ctx->bytes[1]++;	/* Carry from low to high */

	t = 64 - (t & 0x3f);	/* Space available in ctx->in (at least 1) */
	if (t > len) {
		memcpy((FLAC__byte *)ctx->in + 64 - t, buf, len);
		return;
	}
	/* First chunk is an odd size */
	memcpy((FLAC__byte *)ctx->in + 64 - t, buf, t);
	byteSwapX16(ctx->in);
	FLAC__MD5Transform(ctx->buf, ctx->in);
	buf += t;
	len -= t;

	/* Process data in 64-byte chunks */
	while (len >= 64) {
		memcpy(ctx->in, buf, 64);
		byteSwapX16(ctx->in);
		FLAC__MD5Transform(ctx->buf, ctx->in);
		buf += 64;
		len -= 64;
	}

	/* Handle any remaining bytes of data. */
	memcpy(ctx->in, buf, len);
}

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
void FLAC__MD5Init(FLAC__MD5Context *ctx)
{
	ctx->buf[0] = 0x67452301;
	ctx->buf[1] = 0xefcdab89;
	ctx->buf[2] = 0x98badcfe;
	ctx->buf[3] = 0x10325476;

	ctx->bytes[0] = 0;
	ctx->bytes[1] = 0;

	ctx->internal_buf.p8 = 0;
	ctx->capacity = 0;
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void FLAC__MD5Final(FLAC__byte digest[16], FLAC__MD5Context *ctx)
{
	int count = ctx->bytes[0] & 0x3f;	/* Number of bytes in ctx->in */
	FLAC__byte *p = (FLAC__byte *)ctx->in + count;

	/* Set the first char of padding to 0x80.  There is always room. */
	*p++ = 0x80;

	/* Bytes of padding needed to make 56 bytes (-8..55) */
	count = 56 - 1 - count;

	if (count < 0) {	/* Padding forces an extra block */
		memset(p, 0, count + 8);
		byteSwapX16(ctx->in);
		FLAC__MD5Transform(ctx->buf, ctx->in);
		p = (FLAC__byte *)ctx->in;
		count = 56;
	}
	memset(p, 0, count);
	byteSwap(ctx->in, 14);

	/* Append length in bits and transform */
	ctx->in[14] = ctx->bytes[0] << 3;
	ctx->in[15] = ctx->bytes[1] << 3 | ctx->bytes[0] >> 29;
	FLAC__MD5Transform(ctx->buf, ctx->in);

	byteSwap(ctx->buf, 4);
	memcpy(digest, ctx->buf, 16);
	if (0 != ctx->internal_buf.p8) {
		free(ctx->internal_buf.p8);
		ctx->internal_buf.p8 = 0;
		ctx->capacity = 0;
	}
	memset(ctx, 0, sizeof(*ctx));	/* In case it's sensitive */
}

/*
 * Convert the incoming audio signal to a byte stream
 */
static void format_input_(FLAC__multibyte *mbuf, const FLAC__int32 * const signal[], uint32_t channels, uint32_t samples, uint32_t bytes_per_sample)
{
	FLAC__byte *buf_ = mbuf->p8;
	FLAC__int16 *buf16 = mbuf->p16;
	FLAC__int32 *buf32 = mbuf->p32;
	FLAC__int32 a_word;
	uint32_t channel, sample;

	/* Storage in the output buffer, buf, is little endian. */

#define BYTES_CHANNEL_SELECTOR(bytes, channels)   (bytes * 100 + channels)

	/* First do the most commonly used combinations. */
	switch (BYTES_CHANNEL_SELECTOR (bytes_per_sample, channels)) {
		/* One byte per sample. */
		case (BYTES_CHANNEL_SELECTOR (1, 1)):
			for (sample = 0; sample < samples; sample++)
				*buf_++ = signal[0][sample];
			return;

		case (BYTES_CHANNEL_SELECTOR (1, 2)):
			for (sample = 0; sample < samples; sample++) {
				*buf_++ = signal[0][sample];
				*buf_++ = signal[1][sample];
			}
			return;

		case (BYTES_CHANNEL_SELECTOR (1, 4)):
			for (sample = 0; sample < samples; sample++) {
				*buf_++ = signal[0][sample];
				*buf_++ = signal[1][sample];
				*buf_++ = signal[2][sample];
				*buf_++ = signal[3][sample];
			}
			return;

		case (BYTES_CHANNEL_SELECTOR (1, 6)):
			for (sample = 0; sample < samples; sample++) {
				*buf_++ = signal[0][sample];
				*buf_++ = signal[1][sample];
				*buf_++ = signal[2][sample];
				*buf_++ = signal[3][sample];
				*buf_++ = signal[4][sample];
				*buf_++ = signal[5][sample];
			}
			return;

		case (BYTES_CHANNEL_SELECTOR (1, 8)):
			for (sample = 0; sample < samples; sample++) {
				*buf_++ = signal[0][sample];
				*buf_++ = signal[1][sample];
				*buf_++ = signal[2][sample];
				*buf_++ = signal[3][sample];
				*buf_++ = signal[4][sample];
				*buf_++ = signal[5][sample];
				*buf_++ = signal[6][sample];
				*buf_++ = signal[7][sample];
			}
			return;

		/* Two bytes per sample. */
		case (BYTES_CHANNEL_SELECTOR (2, 1)):
			for (sample = 0; sample < samples; sample++)
				*buf16++ = H2LE_16(signal[0][sample]);
			return;

		case (BYTES_CHANNEL_SELECTOR (2, 2)):
			for (sample = 0; sample < samples; sample++) {
				*buf16++ = H2LE_16(signal[0][sample]);
				*buf16++ = H2LE_16(signal[1][sample]);
			}
			return;

		case (BYTES_CHANNEL_SELECTOR (2, 4)):
			for (sample = 0; sample < samples; sample++) {
				*buf16++ = H2LE_16(signal[0][sample]);
				*buf16++ = H2LE_16(signal[1][sample]);
				*buf16++ = H2LE_16(signal[2][sample]);
				*buf16++ = H2LE_16(signal[3][sample]);
			}
			return;

		case (BYTES_CHANNEL_SELECTOR (2, 6)):
			for (sample = 0; sample < samples; sample++) {
				*buf16++ = H2LE_16(signal[0][sample]);
				*buf16++ = H2LE_16(signal[1][sample]);
				*buf16++ = H2LE_16(signal[2][sample]);
				*buf16++ = H2LE_16(signal[3][sample]);
				*buf16++ = H2LE_16(signal[4][sample]);
				*buf16++ = H2LE_16(signal[5][sample]);
			}
			return;

		case (BYTES_CHANNEL_SELECTOR (2, 8)):
			for (sample = 0; sample < samples; sample++) {
				*buf16++ = H2LE_16(signal[0][sample]);
				*buf16++ = H2LE_16(signal[1][sample]);
				*buf16++ = H2LE_16(signal[2][sample]);
				*buf16++ = H2LE_16(signal[3][sample]);
				*buf16++ = H2LE_16(signal[4][sample]);
				*buf16++ = H2LE_16(signal[5][sample]);
				*buf16++ = H2LE_16(signal[6][sample]);
				*buf16++ = H2LE_16(signal[7][sample]);
			}
			return;

		/* Three bytes per sample. */
		case (BYTES_CHANNEL_SELECTOR (3, 1)):
			for (sample = 0; sample < samples; sample++) {
				a_word = signal[0][sample];
				*buf_++ = (FLAC__byte)a_word; a_word >>= 8;
				*buf_++ = (FLAC__byte)a_word; a_word >>= 8;
				*buf_++ = (FLAC__byte)a_word;
			}
			return;

		case (BYTES_CHANNEL_SELECTOR (3, 2)):
			for (sample = 0; sample < samples; sample++) {
				a_word = signal[0][sample];
				*buf_++ = (FLAC__byte)a_word; a_word >>= 8;
				*buf_++ = (FLAC__byte)a_word; a_word >>= 8;
				*buf_++ = (FLAC__byte)a_word;
				a_word = signal[1][sample];
				*buf_++ = (FLAC__byte)a_word; a_word >>= 8;
				*buf_++ = (FLAC__byte)a_word; a_word >>= 8;
				*buf_++ = (FLAC__byte)a_word;
			}
			return;

		/* Four bytes per sample. */
		case (BYTES_CHANNEL_SELECTOR (4, 1)):
			for (sample = 0; sample < samples; sample++)
				*buf32++ = H2LE_32(signal[0][sample]);
			return;

		case (BYTES_CHANNEL_SELECTOR (4, 2)):
			for (sample = 0; sample < samples; sample++) {
				*buf32++ = H2LE_32(signal[0][sample]);
				*buf32++ = H2LE_32(signal[1][sample]);
			}
			return;

		case (BYTES_CHANNEL_SELECTOR (4, 4)):
			for (sample = 0; sample < samples; sample++) {
				*buf32++ = H2LE_32(signal[0][sample]);
				*buf32++ = H2LE_32(signal[1][sample]);
				*buf32++ = H2LE_32(signal[2][sample]);
				*buf32++ = H2LE_32(signal[3][sample]);
			}
			return;

		case (BYTES_CHANNEL_SELECTOR (4, 6)):
			for (sample = 0; sample < samples; sample++) {
				*buf32++ = H2LE_32(signal[0][sample]);
				*buf32++ = H2LE_32(signal[1][sample]);
				*buf32++ = H2LE_32(signal[2][sample]);
				*buf32++ = H2LE_32(signal[3][sample]);
				*buf32++ = H2LE_32(signal[4][sample]);
				*buf32++ = H2LE_32(signal[5][sample]);
			}
			return;

		case (BYTES_CHANNEL_SELECTOR (4, 8)):
			for (sample = 0; sample < samples; sample++) {
				*buf32++ = H2LE_32(signal[0][sample]);
				*buf32++ = H2LE_32(signal[1][sample]);
				*buf32++ = H2LE_32(signal[2][sample]);
				*buf32++ = H2LE_32(signal[3][sample]);
				*buf32++ = H2LE_32(signal[4][sample]);
				*buf32++ = H2LE_32(signal[5][sample]);
				*buf32++ = H2LE_32(signal[6][sample]);
				*buf32++ = H2LE_32(signal[7][sample]);
			}
			return;

		default:
			break;
	}

	/* General version. */
	switch (bytes_per_sample) {
		case 1:
			for (sample = 0; sample < samples; sample++)
				for (channel = 0; channel < channels; channel++)
					*buf_++ = signal[channel][sample];
			return;

		case 2:
			for (sample = 0; sample < samples; sample++)
				for (channel = 0; channel < channels; channel++)
					*buf16++ = H2LE_16(signal[channel][sample]);
			return;

		case 3:
			for (sample = 0; sample < samples; sample++)
				for (channel = 0; channel < channels; channel++) {
					a_word = signal[channel][sample];
					*buf_++ = (FLAC__byte)a_word; a_word >>= 8;
					*buf_++ = (FLAC__byte)a_word; a_word >>= 8;
					*buf_++ = (FLAC__byte)a_word;
				}
			return;

		case 4:
			for (sample = 0; sample < samples; sample++)
				for (channel = 0; channel < channels; channel++)
					*buf32++ = H2LE_32(signal[channel][sample]);
			return;

		default:
			break;
	}
}

/*
 * Convert the incoming audio signal to a byte stream and FLAC__MD5Update it.
 */
FLAC__bool FLAC__MD5Accumulate(FLAC__MD5Context *ctx, const FLAC__int32 * const signal[], uint32_t channels, uint32_t samples, uint32_t bytes_per_sample)
{
	const size_t bytes_needed = (size_t)channels * (size_t)samples * (size_t)bytes_per_sample;

	/* overflow check */
	if ((size_t)channels > SIZE_MAX / (size_t)bytes_per_sample)
		return false;
	if ((size_t)channels * (size_t)bytes_per_sample > SIZE_MAX / (size_t)samples)
		return false;

	if (ctx->capacity < bytes_needed) {
		if (0 == (ctx->internal_buf.p8 = safe_realloc_(ctx->internal_buf.p8, bytes_needed))) {
			if (0 == (ctx->internal_buf.p8 = safe_malloc_(bytes_needed))) {
				ctx->capacity = 0;
				return false;
			}
		}
		ctx->capacity = bytes_needed;
	}

	format_input_(&ctx->internal_buf, signal, channels, samples, bytes_per_sample);

	FLAC__MD5Update(ctx, ctx->internal_buf.p8, bytes_needed);

	return true;
}
