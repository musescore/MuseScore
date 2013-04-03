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

struct MidiPatch;
class Event;
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

   public:
      static const int MAX_BUFFERSIZE = 4096;
      static const int MAX_EFFECTS = 2;

   private:
      std::atomic<bool> lock1;
      std::atomic<bool> lock2;
      std::vector<Synthesizer*> _synthesizer;
      std::vector<Effect*> _effectList[2];
      Effect* _effect[2];

      int _sampleRate;

      float effect1Buffer[MAX_BUFFERSIZE];
      float effect2Buffer[MAX_BUFFERSIZE];
      void init();
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

      int sampleRate()            { return _sampleRate; }
      void setSampleRate(int val);

      void process(unsigned, float*);
      void play(const Event&, unsigned);

      void setMasterTuning(double) {}
      double masterTuning() const { return 440.0; }

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

#endif

