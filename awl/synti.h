//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: synti.h 3053 2010-05-06 13:55:19Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __SYNTI_H__
#define __SYNTI_H__

struct MidiPatch;
class Event;

//---------------------------------------------------------
//   Fparm
//---------------------------------------------------------

class Fparm {
   public:
      Fparm() {}
      Fparm(float a, float b, float c) : _val(a), _min(b), _max(c) {}
      float  _val;
      float  _min;
      float  _max;
      };

//---------------------------------------------------------
//   MidiPatch
//---------------------------------------------------------

struct MidiPatch {
      bool drum;
      int synti;
      int bank, prog;
      QString name;
      };

//---------------------------------------------------------
//   Synth
//---------------------------------------------------------

class Synth {

   protected:
      static const Fparm defaultParameter;

   public:
      Synth() {}
      virtual ~Synth() {}
      virtual void init(int sampleRate) = 0;

      virtual const char* name() const = 0;

      virtual void setMasterTuning(double) {}
      virtual double masterTuning() const { return 440.0; }

      virtual bool loadSoundFont(const QString&) = 0;
      virtual QString soundFont() const = 0;

      virtual void process(unsigned, float*, float*, int, float) = 0;
      virtual void play(const Event&) = 0;

      virtual const QList<MidiPatch*>& getPatchInfo() const = 0;

      virtual const Fparm& effectParameter(int /*effect*/, int /*param*/) const {
            return defaultParameter;
            }
      virtual double setEffectParameter(int /*effect*/, int /*param*/, double /*val*/ ) { return 0.0; }
      };

//---------------------------------------------------------
//   MasterSynth
//---------------------------------------------------------

class MasterSynth {
      QList<Synth*> syntis;
      float _gain;

   public:
      MasterSynth();
      ~MasterSynth();
      void init(int sampleRate);

      void process(unsigned, float*, float*, int);
      void play(const Event&, int);

      double gain() const     { return _gain; }
      void setGain(float val) { _gain = val;  }

      bool loadSoundFont(const QString&);
      QString soundFont() const;

      int synthNameToIndex(const QString&) const;
      QString synthIndexToName(int) const;

      QList<MidiPatch*> getPatchInfo() const;
      Synth* getSynth(int n);
      const QList<Synth*>& getSyntis() const;

      const Fparm& effectParameter(int synti, int effect, int param) const;
      double setEffectParameter(int synti, int effect, int param, double val);

      void setMasterTuning(double) {}
      double masterTuning() const { return 440.0; }
      };

#endif

