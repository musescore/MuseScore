//========================================================================
//
// JPEG2000Stream.h
//
// A JPX stream decoder using OpenJPEG
//
// Copyright 2008, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright 2011 Daniel Glöckner <daniel-gl@gmx.net>
// Copyright 2013, 2014 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2015 Adam Reichold <adam.reichold@t-online.de>
//
// Licensed under GPLv2 or later
//
//========================================================================


#ifndef JPEG2000STREAM_H
#define JPEG2000STREAM_H

#include "config.h"
#include "goo/gtypes.h"
#include "Object.h"
#include "Stream.h"

struct JPXStreamPrivate;

class JPXStream: public FilterStream {
public:

  JPXStream(Stream *strA);
  virtual ~JPXStream();
  virtual StreamKind getKind() { return strJPX; }
  virtual void reset();
  virtual void close();
  virtual Goffset getPos();
  virtual int getChar();
  virtual int lookChar();
  virtual GooString *getPSFilter(int psLevel, const char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  virtual void getImageParams(int *bitsPerComponent, StreamColorSpaceMode *csMode);

  int readStream(int nChars, Guchar *buffer) {
    return str->doGetChars(nChars, buffer);
  }
private:
  JPXStream(const JPXStream &other);
  JPXStream& operator=(const JPXStream &other);
  JPXStreamPrivate *priv;

  void init();
  virtual GBool hasGetChars() { return true; }
  virtual int getChars(int nChars, Guchar *buffer);
};

#endif
