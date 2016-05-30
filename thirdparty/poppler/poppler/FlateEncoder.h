//========================================================================
//
// FlateEncoder.h
//
// Copyright (C) 2016, William Bader <williambader@hotmail.com>
//
// This file is under the GPLv2 or later license
//
//========================================================================

#ifndef FLATEENCODE_H
#define FLATEENCODE_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "poppler-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <ctype.h>
#include "goo/gmem.h"
#include "goo/gfile.h"
#include "Error.h"
#include "Object.h"
#include "Decrypt.h"

#include "Stream.h"

extern "C" {
#include <zlib.h>
}

//------------------------------------------------------------------------
// FlateEncoder
//------------------------------------------------------------------------

class FlateEncoder: public FilterStream {
public:

  FlateEncoder(Stream *strA);
  virtual ~FlateEncoder();
  virtual StreamKind getKind() { return strWeird; }
  virtual void reset();
  virtual int getChar()
    { return (outBufPtr >= outBufEnd && !fillBuf()) ? EOF : (*outBufPtr++ & 0xff); }
  virtual int lookChar()
    { return (outBufPtr >= outBufEnd && !fillBuf()) ? EOF : (*outBufPtr & 0xff); }
  virtual GooString *getPSFilter(int psLevel, const char *indent) { return NULL; }
  virtual GBool isBinary(GBool last = gTrue) { return gTrue; }
  virtual GBool isEncoder() { return gTrue; }

private:

  static const int inBufSize = 16384;
  static const int outBufSize = inBufSize;
  Guchar inBuf[ inBufSize ];
  Guchar outBuf[ outBufSize ];
  Guchar *outBufPtr;
  Guchar *outBufEnd;
  GBool inBufEof;
  GBool outBufEof;
  z_stream zlib_stream;

  GBool fillBuf();
};

#endif
