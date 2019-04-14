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

#include "libmscore/synthesizerstate.h"

namespace Ms {

struct MidiPatch;
class PlayEvent;
class Synth;
class SynthesizerGui;

//---------------------------------------------------------
//   SoundFontInfo
//---------------------------------------------------------

struct SoundFontInfo {
      QString fileName;
      QString fontName;

      SoundFontInfo(QString _fileName) : fileName(_fileName), fontName(_fileName) {}
      SoundFontInfo(QString _fileName, QString _fontName) : fileName(_fileName), fontName(_fontName) {}
      };

//---------------------------------------------------------
//   Synthesizer
//---------------------------------------------------------

class Synthesizer {
      bool _active;

   protected:
      float _sampleRate;
      SynthesizerGui* _gui;

   public:
      Synthesizer() : _active(false) { _gui = 0; }
      virtual ~Synthesizer() {}
      virtual void init(float sr)    { _sampleRate = sr; }
      float sampleRate() const       { return _sampleRate; }

      virtual const char* name() const = 0;

      virtual void setMasterTuning(double) {}
      virtual double masterTuning() const { return 440.0; }

      virtual bool loadSoundFonts(const QStringList&) = 0;
      virtual bool addSoundFont(const QString&)    { return false; }
      virtual bool removeSoundFont(const QString&) { return false; }

      virtual std::vector<SoundFontInfo> soundFontsInfo() const = 0;

      virtual void process(unsigned, float*, float*, float*) = 0;
      virtual void play(const PlayEvent&) = 0;

      virtual const QList<MidiPatch*>& getPatchInfo() const = 0;

      // get/set synthesizer state
      virtual SynthesizerGroup state() const = 0;
      virtual bool setState(const SynthesizerGroup&) = 0;
      virtual void setValue(int, double) {}
      virtual double value(int) const { return 0.0; }

      void reset()                    { _active = false; }
      bool active() const             { return _active; }
      void setActive(bool val = true) { _active = val;  }

      virtual void allSoundsOff(int /*channel*/) {}
      virtual void allNotesOff(int /*channel*/) {}

      virtual SynthesizerGui* gui()  { return _gui; }
      };

}
#endif

