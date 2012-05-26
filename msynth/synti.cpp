//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: seq.cpp 3012 2010-04-28 17:12:41Z wschweer $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "config.h"
#include "mscore/preferences.h"
#include "libmscore/event.h"
#include "libmscore/instrument.h"
#include "synti.h"
#include "fluid/fluid.h"
#ifdef AEOLUS
#include "aeolus/aeolus/aeolus.h"
#endif
#include "libmscore/xml.h"
#include "sparm_p.h"

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Synth::reset()
      {
      _active = false;
      }

//---------------------------------------------------------
//   MasterSynth
//---------------------------------------------------------

MasterSynth::MasterSynth()
      {
      _gain = 1.0;
      }

//---------------------------------------------------------
//   MasterSynth
//---------------------------------------------------------

MasterSynth::~MasterSynth()
      {
      foreach(Synth* s, syntis)
            delete s;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MasterSynth::init(int sampleRate)
      {
      bool useJackFlag      = preferences.useJackAudio || preferences.useJackMidi;
      bool useAlsaFlag      = preferences.useAlsaAudio;
      bool usePortaudioFlag = preferences.usePortaudioAudio;

      if (useJackFlag || useAlsaFlag || usePortaudioFlag) {
            syntis.append(new FluidS::Fluid());
#ifdef AEOLUS
            syntis.append(new Aeolus());
#endif
            }
      if (syntis.isEmpty())
            return;
      foreach(Synth* s, syntis)
            s->init(sampleRate);
      foreach(Synth* s, syntis) {
            s->setMasterTuning(preferences.tuning);
            s->setParameter(SParmId(FLUID_ID, 1, 0).val, preferences.reverbRoomSize);
            s->setParameter(SParmId(FLUID_ID, 1, 1).val, preferences.reverbDamp);
            s->setParameter(SParmId(FLUID_ID, 1, 2).val, preferences.reverbWidth);
            s->setParameter(SParmId(FLUID_ID, 1, 3).val, preferences.reverbGain);
            s->setParameter(SParmId(FLUID_ID, 2, 4).val, preferences.chorusGain);
            }
      _gain = preferences.masterGain;
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void MasterSynth::process(unsigned n, float* p)
      {
      foreach(Synth* s, syntis) {
            if (s->active())
                  s->process(n, p, _gain);
            }
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void MasterSynth::reset()
      {
      foreach(Synth* s, syntis)
            s->reset();
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void MasterSynth::play(const Event& event, int syntiIdx)
      {
      syntis[syntiIdx]->setActive(true);
      syntis[syntiIdx]->play(event);
      }

//---------------------------------------------------------
//   synthNameToIndex
//---------------------------------------------------------

int MasterSynth::synthNameToIndex(const QString& name) const
      {
      int idx = 0;
      foreach(Synth* s, syntis) {
            if (s->name() == name)
                  return idx;
            ++idx;
            }
      return -1;
      }

//---------------------------------------------------------
//   synthIndexToName
//---------------------------------------------------------

QString MasterSynth::synthIndexToName(int idx) const
      {
      if (idx >= syntis.size())
            return QString();
      return QString(syntis[idx]->name());
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

QList<MidiPatch*> MasterSynth::getPatchInfo() const
      {
      QList<MidiPatch*> pl;
      int idx = 0;
      foreach(Synth* s, syntis) {
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

SyntiParameter MasterSynth::parameter(int id) const
      {
      SParmId spid(id);
      return syntis[spid.syntiId]->parameter(id);
      }

//---------------------------------------------------------
//   setParameter
//---------------------------------------------------------

void MasterSynth::setParameter(int id, double val)
      {
      SParmId spid(id);
      syntis[spid.syntiId]->setParameter(id, val);
      }

//---------------------------------------------------------
//   state
//---------------------------------------------------------

SyntiState MasterSynth::state() const
      {
      SyntiState ss;
      foreach(Synth* s, syntis) {
            SyntiState st = s->state();
            ss.append(st);
            }
      return ss;
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void MasterSynth::setState(SyntiState& ss)
      {
      foreach(Synth* synti, syntis)
            synti->setState(ss);
      }

//---------------------------------------------------------
//   allSoundsOff
//---------------------------------------------------------

void MasterSynth::allSoundsOff(int channel)
      {
      foreach(Synth* synti, syntis)
            synti->allSoundsOff(channel);
      }

//---------------------------------------------------------
//   allNotesOff
//---------------------------------------------------------

void MasterSynth::allNotesOff(int channel)
      {
      foreach(Synth* synti, syntis)
            synti->allNotesOff(channel);
      }

//---------------------------------------------------------
//   synth
//---------------------------------------------------------

Synth* MasterSynth::synth(const QString& s)
      {
      foreach(Synth* synti, syntis) {
            if (synti->name() == s)
                  return synti;
            }
      return 0;
      }

