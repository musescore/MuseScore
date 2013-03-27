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
#include "instrument.h"
#include "synthesizer/synthesizer.h"
#include "msynthesizer.h"
#include "xml.h"
#include "sparm_p.h"
#include "midipatch.h"

int MasterSynthesizer::_sampleRate;

//---------------------------------------------------------
//   MasterSynthesizer
//---------------------------------------------------------

MasterSynthesizer::~MasterSynthesizer()
      {
      foreach(Synthesizer* s, _synthesizer)
            delete s;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MasterSynthesizer::init()
      {
      foreach(Synthesizer* s, _synthesizer)
            s->init();
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void MasterSynthesizer::process(unsigned n, float* p)
      {
      foreach(Synthesizer* s, _synthesizer) {
            if (s->active())
                  s->process(n, p);
            }
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void MasterSynthesizer::reset()
      {
      foreach(Synthesizer* s, _synthesizer)
            s->reset();
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void MasterSynthesizer::play(const Event& event, int syntiIdx)
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
      foreach(Synthesizer* s, _synthesizer) {
            if (s->name() == name)
                  return idx;
            ++idx;
            }
      return -1;
      }

//---------------------------------------------------------
//   synthIndexToName
//---------------------------------------------------------

QString MasterSynthesizer::name(int idx) const
      {
      if (idx >= _synthesizer.size())
            return QString();
      return QString(_synthesizer[idx]->name());
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

QList<MidiPatch*> MasterSynthesizer::getPatchInfo() const
      {
      QList<MidiPatch*> pl;
      int idx = 0;
      foreach(Synthesizer* s, _synthesizer) {
            QList<MidiPatch*> ip = s->getPatchInfo();
            foreach(MidiPatch* mp, ip)
                  mp->synti = idx;
            pl += ip;
            ++idx;
            }
      return pl;
      }

//---------------------------------------------------------
//   parameter
//---------------------------------------------------------

SyntiParameter MasterSynthesizer::parameter(int id) const
      {
      SParmId spid(id);
      return _synthesizer[spid.syntiId]->parameter(id);
      }

//---------------------------------------------------------
//   setParameter
//---------------------------------------------------------

void MasterSynthesizer::setParameter(int id, double val)
      {
      SParmId spid(id);
      _synthesizer[spid.syntiId]->setParameter(id, val);
      }

//---------------------------------------------------------
//   state
//---------------------------------------------------------

SyntiState MasterSynthesizer::state() const
      {
      SyntiState ss;
      foreach(Synthesizer* s, _synthesizer) {
            SyntiState st = s->state();
            ss.append(st);
            }
      return ss;
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void MasterSynthesizer::setState(SyntiState& ss)
      {
      foreach(Synthesizer* s, _synthesizer)
            s->setState(ss);
      }

//---------------------------------------------------------
//   allSoundsOff
//---------------------------------------------------------

void MasterSynthesizer::allSoundsOff(int channel)
      {
      foreach(Synthesizer* s, _synthesizer)
            s->allSoundsOff(channel);
      }

//---------------------------------------------------------
//   allNotesOff
//---------------------------------------------------------

void MasterSynthesizer::allNotesOff(int channel)
      {
      foreach(Synthesizer* s, _synthesizer)
            s->allNotesOff(channel);
      }

//---------------------------------------------------------
//   synth
//---------------------------------------------------------

Synthesizer* MasterSynthesizer::synthesizer(const QString& name)
      {
      foreach(Synthesizer* s, _synthesizer) {
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

