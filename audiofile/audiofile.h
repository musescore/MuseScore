//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2007-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __AUDIOFILE_H__
#define __AUDIOFILE_H__

#include <sndfile.h>

//---------------------------------------------------------
//   AudioFile
//---------------------------------------------------------

class AudioFile {
      SF_INFO info;
      SNDFILE* sf;
      SF_INSTRUMENT inst;
      bool hasInstrument;
      QByteArray buf;  // used during read of Sample
      int idx;

   public:
      AudioFile();
      ~AudioFile();

      bool open(const QByteArray&);
      const char* error() const     { return sf_strerror(sf); }
      int read(short*, int);

      int channels() const   { return info.channels; }
      int frames() const     { return info.frames; }
      int samplerate() const { return info.samplerate; }

      sf_count_t getFileLen() const { return buf.size(); }
      sf_count_t tell() const       { return idx; }
      sf_count_t read(void* ptr, sf_count_t count);
      sf_count_t write(const void* ptr, sf_count_t count);
      sf_count_t seek(sf_count_t offset, int whence);
      unsigned int loopStart(int v = 0) { return hasInstrument ? inst.loops[v].start : -1; }
      unsigned int loopEnd(int v = 0)   { return hasInstrument ? inst.loops[v].end : -1; }
      int loopMode(int v = 0)   { return hasInstrument ? inst.loops[v].mode : -1; }
      };

#endif

