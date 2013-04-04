//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "config.h"
#include "event.h"
#include "synthesizer.h"
#include "msynthesizer.h"
#include "synthesizergui.h"
#include "libmscore/xml.h"
#include "midipatch.h"

//---------------------------------------------------------
//   MasterSynthesizer
//---------------------------------------------------------

MasterSynthesizer::MasterSynthesizer()
   : QObject(0)
      {
      lock1 = false;
      lock2 = true;
      _synthesizer.reserve(4);
      _gain = 1.0;
      for (int i = 0; i < MAX_EFFECTS; ++i)
            _effect[i] = 0;
      }

//---------------------------------------------------------
//   MasterSynthesizer
//---------------------------------------------------------

MasterSynthesizer::~MasterSynthesizer()
      {
      for (Synthesizer* s : _synthesizer)
            delete s;
      for (int i = 0; i < MAX_EFFECTS; ++i)
            delete _effect[i];
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void MasterSynthesizer::reset()
      {
      for (Synthesizer* s : _synthesizer)
            s->reset();
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void MasterSynthesizer::play(const Event& event, unsigned syntiIdx)
      {
      if (syntiIdx >= _synthesizer.size())
            return;
      _synthesizer[syntiIdx]->setActive(true);
      _synthesizer[syntiIdx]->play(event);
      }

//---------------------------------------------------------
//   synthNameToIndex
//---------------------------------------------------------

int MasterSynthesizer::index(const QString& name) const
      {
      int idx = 0;
      for (Synthesizer* s : _synthesizer) {
            if (s->name() == name)
                  return idx;
            ++idx;
            }
      qDebug("MasterSynthesizer::index for <%s> not found", qPrintable(name));
      return 0;
      }

//---------------------------------------------------------
//   synthIndexToName
//---------------------------------------------------------

QString MasterSynthesizer::name(unsigned idx) const
      {
      if (idx >= _synthesizer.size()) {
            qDebug("MasterSynthesizer::name() bad index %d", idx);
            return QString();
            }
      return QString(_synthesizer[idx]->name());
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

QList<MidiPatch*> MasterSynthesizer::getPatchInfo() const
      {
      QList<MidiPatch*> pl;
      for (Synthesizer* s : _synthesizer)
            pl += s->getPatchInfo();
      return pl;
      }

//---------------------------------------------------------
//   allSoundsOff
//---------------------------------------------------------

void MasterSynthesizer::allSoundsOff(int channel)
      {
      for (Synthesizer* s : _synthesizer)
            s->allSoundsOff(channel);
      }

//---------------------------------------------------------
//   allNotesOff
//---------------------------------------------------------

void MasterSynthesizer::allNotesOff(int channel)
      {
      for (Synthesizer* s : _synthesizer)
            s->allNotesOff(channel);
      }

//---------------------------------------------------------
//   synth
//---------------------------------------------------------

Synthesizer* MasterSynthesizer::synthesizer(const QString& name)
      {
      for (Synthesizer* s : _synthesizer) {
            if (s->name() == name)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   registerSynthesizer
//    ownership of s moves to MasterSynthesizer
//---------------------------------------------------------

void MasterSynthesizer::registerSynthesizer(Synthesizer* s)
      {
      _synthesizer.push_back(s);
      }

//---------------------------------------------------------
//   registerEffect
//---------------------------------------------------------

void MasterSynthesizer::registerEffect(int ab, Effect* e)
      {
      _effectList[ab].push_back(e);
      }

//---------------------------------------------------------
//   setEffect
//---------------------------------------------------------

void MasterSynthesizer::setEffect(int ab, int idx)
      {
      if (idx < 0 || idx >= int(_effectList[ab].size())) {
            qDebug("MasterSynthesizer::setEffect: bad idx %d %d", ab, idx);
            return;
            }
      lock2 = true;
      while (lock1)
            sleep(1);
      _effect[ab] = _effectList[ab][idx];
      lock2 = false;
      }

//---------------------------------------------------------
//   effect
//---------------------------------------------------------

Effect* MasterSynthesizer::effect(int idx)
      {
      return _effect[idx];
      }

//---------------------------------------------------------
//   setSampleRate
//---------------------------------------------------------

void MasterSynthesizer::setSampleRate(float val)
      {
      _sampleRate = val;
      for (Synthesizer* s : _synthesizer) {
            s->init(_sampleRate);
            connect(s->gui(), SIGNAL(sfChanged()), SLOT(sfChanged()));
            }
      for (Effect* e : _effectList[0])
            e->init(_sampleRate);
      for (Effect* e : _effectList[1])
            e->init(_sampleRate);
      lock2 = false;
      lock1 = false;
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void MasterSynthesizer::process(unsigned n, float* p)
      {
//      memset(effect1Buffer, 0, n * sizeof(float) * 2);
//      memset(effect2Buffer, 0, n * sizeof(float) * 2);

      if (lock2) {
            printf("lock2\n");
            return;
            }
      lock1 = true;
      if (lock2) {
            lock1 = false;
            return;
            }
      for (Synthesizer* s : _synthesizer) {
            if (s->active())
                  s->process(n, p, effect1Buffer, effect2Buffer);
            }
      if (_effect[0] && _effect[1]) {
            memset(effect1Buffer, 0, n * sizeof(float) * 2);
            _effect[0]->process(n, p, effect1Buffer);
            _effect[1]->process(n, effect1Buffer, p);
            }
      else if (_effect[0] || _effect[1]) {
            memcpy(effect1Buffer, p, n * sizeof(float) * 2);
            if (_effect[0])
                  _effect[0]->process(n, effect1Buffer, p);
            else
                  _effect[1]->process(n, effect1Buffer, p);
            }
      for (unsigned i = 0; i < n * 2; ++i)
            *p++ *= _gain;
      lock1 = false;
      }

//---------------------------------------------------------
//   indexOfEffect
//---------------------------------------------------------

int MasterSynthesizer::indexOfEffect(int ab, const QString& name)
      {
      int idx = 0;
      for (Effect* e : _effectList[ab]) {
            if (e && e->name() == name)
                  return idx;
            ++idx;
            }
      qDebug("indexOfEffect %d %s not found", ab, qPrintable(name));
      return -1;
      }

int MasterSynthesizer::indexOfEffect(int ab)
      {
      return indexOfEffect(ab, _effect[ab]->name());
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void MasterSynthesizer::setState(const SynthesizerState& ss)
      {
      for (const SynthesizerGroup& g : ss) {
            if (g.name() == "master") {
                  for (const IdValue& v : g) {
                        switch (v.id) {
                              case 0:
                                    setEffect(0, indexOfEffect(0, v.data));
                                    break;
                              case 1:
                                    setEffect(1, indexOfEffect(1, v.data));
                                    break;
                              case 2: {
                                    float f = v.data.toDouble();
                                    setGain(f);
                                    }
                                    break;
                              }
                        }
                  }
            else {
                  Synthesizer* s = synthesizer(g.name());
                  if (s)
                        s->setState(g);
                  else {
                        if (effect(0) && effect(0)->name() == g.name())
                              effect(0)->setState(g);
                        else if (effect(1) && effect(1)->name() == g.name())
                              effect(1)->setState(g);
                        }
                  }
            }

      }

//---------------------------------------------------------
//   state
//---------------------------------------------------------

SynthesizerState MasterSynthesizer::state() const
      {
      SynthesizerState ss;
      SynthesizerGroup g;
      g.setName("master");
      g.push_back(IdValue(0, QString("%1").arg(_effect[0] ? _effect[0]->name() : "none")));
      g.push_back(IdValue(1, QString("%1").arg(_effect[1] ? _effect[1]->name() : "none")));
      g.push_back(IdValue(2, QString("%1").arg(gain())));
      ss.push_back(g);
      for (Synthesizer* s : _synthesizer)
            ss.push_back(s->state());
      if (_effect[0])
            ss.push_back(_effect[0]->state());
      if (_effect[1])
            ss.push_back(_effect[1]->state());
      return ss;
      }

//---------------------------------------------------------
//   setGain
//---------------------------------------------------------

void MasterSynthesizer::setGain(float f)
      {
      if (_gain != f) {
            _gain = f;
            emit gainChanged(_gain);
            }
      }

