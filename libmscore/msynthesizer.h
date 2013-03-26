//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SYNTI_H__
#define __SYNTI_H__

struct MidiPatch;
class Event;

#include "sparm.h"

class Synthesizer;

//---------------------------------------------------------
//   MasterSynthesizer
//    hosts several synthesizers
//---------------------------------------------------------

class MasterSynthesizer {
      QList<Synthesizer*> _synthesizer;
      float _gain;
      static int _sampleRate;

   public:
      MasterSynthesizer();
      ~MasterSynthesizer();
      void registerSynthesizer(Synthesizer*);
      void init();

      static int sampleRate()            { return _sampleRate; }
      static void setSampleRate(int val) { _sampleRate = val;  }

      void process(unsigned, float*);
      void play(const Event&, int);

      double gain() const     { return _gain; }
      void setGain(float val) { _gain = val;  }

      void setMasterTuning(double) {}
      double masterTuning() const { return 440.0; }

      int index(const QString&) const;
      QString name(int) const;

      QList<MidiPatch*> getPatchInfo() const;

      // set/get a single parameter
      SyntiParameter parameter(int id) const;
      void setParameter(int id, double val);

      // get/set synthesizer state
      SyntiState state() const;
      void setState(SyntiState&);

      Synthesizer* synthesizer(const QString& name);
      void reset();
      void allSoundsOff(int channel);
      void allNotesOff(int channel);
      };

#endif

