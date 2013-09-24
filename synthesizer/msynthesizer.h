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

#ifndef __MSYNTHESIZER_H__
#define __MSYNTHESIZER_H__

#include <atomic>
#include "effects/effect.h"
#include "libmscore/synthesizerstate.h"

namespace Ms {

struct MidiPatch;
class NPlayEvent;
class Synthesizer;
class Effect;
class Xml;

//---------------------------------------------------------
//   MasterSynthesizer
//    hosts several synthesizers
//---------------------------------------------------------

class MasterSynthesizer : public QObject {
      Q_OBJECT

      float _gain;
      double _masterTuning;

   public:
      static const int MAX_BUFFERSIZE = 8192;
      static const int MAX_EFFECTS = 2;

   private:
      std::atomic<bool> lock1;
      std::atomic<bool> lock2;
      std::vector<Synthesizer*> _synthesizer;
      std::vector<Effect*> _effectList[2];
      Effect* _effect[2];

      float _sampleRate;

      float effect1Buffer[MAX_BUFFERSIZE];
      float effect2Buffer[MAX_BUFFERSIZE];
      int indexOfEffect(int ab, const QString& name);

   public slots:
      void sfChanged() { emit soundFontChanged(); }
      void setGain(float f);

   signals:
      void soundFontChanged();
      void gainChanged(float);

   public:
      MasterSynthesizer();
      ~MasterSynthesizer();
      void registerSynthesizer(Synthesizer*);

      void init();

      float sampleRate()            { return _sampleRate; }
      void setSampleRate(float val);

      void process(unsigned, float*);
      void play(const NPlayEvent&, unsigned);

      void setMasterTuning(double val);
      double masterTuning() const      { return _masterTuning; }

      int index(const QString&) const;
      QString name(unsigned) const;

      QList<MidiPatch*> getPatchInfo() const;

      SynthesizerState state() const;
      void setState(const SynthesizerState&);

      Synthesizer* synthesizer(const QString& name);
      const std::vector<Effect*>& effectList(int ab) const { return _effectList[ab]; }
      const std::vector<Synthesizer*> synthesizer() const { return _synthesizer; }
      void registerEffect(int ab, Effect*);

      void reset();
      void allSoundsOff(int channel);
      void allNotesOff(int channel);

      void setEffect(int ab, int idx);
      Effect* effect(int ab);
      int indexOfEffect(int ab);

      float gain() const    { return _gain; }
      };

}
#endif

