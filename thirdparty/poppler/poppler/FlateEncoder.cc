//========================================================================
//
// FlateEncoder.cc
//
// Copyright (C) 2016, William Bader <williambader@hotmail.com>
//
// This file is under the GPLv2 or later license
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "FlateEncoder.h"

//------------------------------------------------------------------------
// FlateEncoder
//------------------------------------------------------------------------

FlateEncoder::FlateEncoder(Stream *strA):
  FilterStream(strA)
{
  int zlib_status;

  outBufPtr = outBufEnd = outBuf;
  inBufEof = outBufEof = gFalse;

  zlib_stream.zalloc = Z_NULL;
  zlib_stream.zfree = Z_NULL;
  zlib_stream.opaque = Z_NULL;

  zlib_status = deflateInit(&zlib_stream, Z_DEFAULT_COMPRESSION);

  if (zlib_status != Z_OK) {
    inBufEof = outBufEof = gTrue;
    error(errInternal, -1, "Internal: deflateInit() failed in FlateEncoder::FlateEncoder()");
  }

  zlib_stream.next_out = outBufEnd;
  zlib_stream.avail_out = 1; /* anything but 0 to trigger a read */
}

FlateEncoder::~FlateEncoder() {
  deflateEnd(&zlib_stream);
  if (str->isEncoder()) {
    delete str;
  }
}

void FlateEncoder::reset() {
  int zlib_status;

  str->reset();

  outBufPtr = outBufEnd = outBuf;
  inBufEof = outBufEof = gFalse;

  deflateEnd(&zlib_stream);

  zlib_status = deflateInit(&zlib_stream, Z_DEFAULT_COMPRESSION);

  if (zlib_status != Z_OK) {
    inBufEof = outBufEof = gTrue;
    error(errInternal, -1, "Internal: deflateInit() failed in FlateEncoder::reset()");
  }

  zlib_stream.next_out = outBufEnd;
  zlib_stream.avail_out = 1; /* anything but 0 to trigger a read */
}

GBool FlateEncoder::fillBuf() {
  int n;
  unsigned int starting_avail_out;
  int zlib_status;

  /* If the output is done, don't try to read more. */

  if (outBufEof) {
    return gFalse;
  }

  /* The output buffer should be empty. */
  /* If it is not empty, push any processed data to the start. */

  if (outBufPtr > outBuf && outBufPtr < outBufEnd) {
    n = outBufEnd - outBufPtr;
    memmove(outBuf, outBufPtr, n);
    outBufEnd = &outBuf[n];
  } else {
    outBufEnd = outBuf;
  }
  outBufPtr = outBuf;

  /* Keep feeding zlib until we get output. */
  /* zlib might consume a few input buffers */
  /* before it starts producing output. */

  do {

    /* avail_out > 0 means that zlib has depleted its input */
    /* and needs a new chunk of input in order to generate */
    /* more output. */

    if (zlib_stream.avail_out != 0) {

      /* Fill the input buffer */

      n = (inBufEof? 0: str->doGetChars(inBufSize, inBuf));

      if (n == 0) {
	inBufEof = gTrue;
      }

      zlib_stream.next_in = inBuf;
      zlib_stream.avail_in = n;
    }

    /* Ask zlib for output. */

    zlib_stream.next_out = outBufEnd;
    starting_avail_out = &outBuf[ outBufSize ] - outBufEnd;
    zlib_stream.avail_out = starting_avail_out;

    zlib_status = deflate(&zlib_stream, (inBufEof? Z_FINISH: Z_NO_FLUSH));

    if (zlib_status == Z_STREAM_ERROR ||
        zlib_stream.avail_out < 0 ||
        zlib_stream.avail_out > starting_avail_out) {
      /* Unrecoverable error */
      inBufEof = outBufEof = gTrue;
      error(errInternal, -1, "Internal: deflate() failed in FlateEncoder::fillBuf()");
      return gFalse;
    }

  } while (zlib_stream.avail_out == outBufSize && !inBufEof);

  outBufEnd = &outBuf[ outBufSize ] - zlib_stream.avail_out;

  if (inBufEof && zlib_stream.avail_out != 0) {
    outBufEof = gTrue;
  }

  return outBufPtr < outBufEnd;
}

