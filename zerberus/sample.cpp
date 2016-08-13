//=============================================================================
//  Zerberus
//  Zample player
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "sample.h"
#include "samplepool.h"
#include <QString>

Sample::Sample(QString f, bool diskStreaming, unsigned int streamBufferSize)
      {
      _filename = f;

      memset(&info, 0, sizeof(info));
      memset(&inst, 0, sizeof(inst));
      sf = 0;

      sf  = sf_open(_filename.toLocal8Bit().constData(), SFM_READ, &info);
      hasInstrument = sf_command(sf, SFC_GET_INSTRUMENT, &inst, sizeof(inst)) == SF_TRUE;

      if (sf == 0) {
            printf("open <%s> failed: %s\n", qPrintable(_filename), sf_strerror(sf));
            throw ERROR_OPENING_FILE;
            }

      if (diskStreaming && info.frames > streamBufferSize) {
            _data = new short[streamBufferSize * info.channels];
            if (sf_readf_short(sf, _data, streamBufferSize) != streamBufferSize) {
                  qDebug("Sample read failed: %s\n", sf_strerror(sf));
                  throw ERROR_READING_FILE;
                  }
            _needsStreaming = true;
            }
      else {
            _data = new short[info.frames * info.channels];
            if (sf_readf_short(sf, _data, info.frames) != info.frames) {
                  qDebug("Sample read failed: %s\n", sf_strerror(sf));
                  throw ERROR_READING_FILE;
                  }
            _needsStreaming = false;
            }

      sf_close(sf);
      }

