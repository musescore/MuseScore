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

#ifndef __SYNTHESIZER_H__
#define __SYNTHESIZER_H__

struct MidiPatch;
class Event;
class Synth;

#include "libmscore/sparm.h"

//---------------------------------------------------------
//   Synthesizer
//---------------------------------------------------------

class Synthesizer {
      bool _active;

   protected:

   public:
      Synthesizer() : _active(false) {}
      virtual ~Synthesizer() {}
      virtual void init()      {}

      virtual const char* name() const = 0;

      virtual void setMasterTuning(double) {}
      virtual double masterTuning() const { return 440.0; }

      virtual bool loadSoundFonts(const QStringList&) = 0;
      virtual bool addSoundFont(const QString&)    { return false; }
      virtual bool removeSoundFont(const QString&) { return false; }

      virtual QStringList soundFonts() const = 0;

      virtual void process(unsigned, float*, float) = 0;
      virtual void play(const Event&) = 0;

      virtual const QList<MidiPatch*>& getPatchInfo() const = 0;

      // set/get a single parameter
      virtual SyntiParameter parameter(int /*id*/) const { return SyntiParameter(); }
      virtual void setParameter(int /*id*/, double /*val*/) {}
      virtual void setParameter(int /*id*/, const QString&) {}

      // get/set synthesizer state
      virtual SyntiState state() const = 0;
      virtual void setState(SyntiState&) {}
      void reset() { _active = false; }
      bool active() const             { return _active; }
      void setActive(bool val = true) { _active = val;  }

      virtual void allSoundsOff(int /*channel*/) {}
      virtual void allNotesOff(int /*channel*/) {}
      };

#endif

