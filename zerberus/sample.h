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

//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

class Sample {
      int _channel;
      short* _data;
      long long _frames;
      int _sampleRate;
      long long _loopStart;
      long long _loopEnd;
      int _loopMode;

   public:
      Sample(int ch, short* val, int f, int sr)
         : _channel(ch), _data(val), _frames(f), _sampleRate(sr) {}
      ~Sample();
      bool read(const QString&);
      long long frames() const     { return _frames;          }
      short* data() const    { return _data + _channel; }
      int channel() const    { return _channel;         }
      int sampleRate() const { return _sampleRate;      }

      void setLoopStart (int v) { _loopStart = v; }
      void setLoopEnd (int v)   { _loopEnd = v; }
      void setLoopMode (int v)  { _loopMode = v; }
      long long loopStart()           { return _loopStart; }
      long long loopEnd()             { return _loopEnd; }
      int loopMode()            { return _loopMode; }
      };

#endif

