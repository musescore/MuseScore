//=============================================================================
//  Zerberus
//  Zample player
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SAMPLE_H__
#define __SAMPLE_H__

#include <sndfile.h>

//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

class Sample {
      short* _data;

      SF_INFO info;
      SNDFILE* sf;
      SF_INSTRUMENT inst;
      bool hasInstrument;
      bool _needsStreaming;
      QString _filename;

   public:
      Sample(QString f, bool diskStreaming=false);
      ~Sample();
      bool read(const QString&);
      int frames() const     { return info.frames;          }
      short* data() const    { return _data; }
      int channel() const    { return info.channels;        }
      int sampleRate() const { return info.samplerate;      }
      bool needsStreaming()  { return _needsStreaming;      }

      const QString filename() const    { return _filename; }

      unsigned int loopStart(int v = 0) { return hasInstrument ? inst.loops[v].start : -1; }
      unsigned int loopEnd(int v = 0)   { return hasInstrument ? inst.loops[v].end : -1; }
      int loopMode(int v = 0)   { return hasInstrument ? inst.loops[v].mode : -1; }
      };

#endif

