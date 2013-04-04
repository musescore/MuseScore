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

#ifndef __MCHANNEL_H__
#define __MCHANNEL_H__

class Zerberus;
class ZInstrument;

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

class Channel {
      Zerberus* _msynth;
      ZInstrument* _instrument;
      float _gain;
      float _panLeftGain;
      float _panRightGain;
      float _midiVolume;
      char ctrl[128];

      int _idx;               // channel index
      int _sustain;

   public:
      Channel(Zerberus*, int idx);

      void pitchBend(int);
      void controller(int ctrl, int val);
      ZInstrument* instrument() const    { return _instrument; }
      void setInstrument(ZInstrument* i) { _instrument = i; }
      Zerberus* msynth() const          { return _msynth; }
      int sustain() const;
      float gain() const                { return _gain * _midiVolume;  }
//      void setGain(float val)           { _gain = val;     }
      float panLeftGain() const  { return _panLeftGain; }
      float panRightGain() const { return _panRightGain; }
      };


#endif

