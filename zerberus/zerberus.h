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

#ifndef __ZERBERUS_H__
#define __ZERBERUS_H__

#include <math.h>
#include <atomic>
// #include <mutex>
#include <list>
#include <memory>
#include <stack>

#include "synthesizer/synthesizer.h"
#include "synthesizer/event.h"
#include "voice.h"


class Channel;
class ZInstrument;
enum class Trigger : char;

static const int MAX_VOICES  = 512;
static const int MAX_CHANNEL = 64;
static const int MAX_TRIGGER = 512;

//---------------------------------------------------------
//   VoiceFifo
//---------------------------------------------------------

class VoiceFifo {
      std::stack<Voice*> buffer;
      std::vector< std::unique_ptr<Voice> > voices;

   public:
      VoiceFifo() {
            voices.resize(MAX_VOICES);
            }

      void init(Zerberus* z) {
            for (int i = 0; i < MAX_VOICES; ++i) {
                  voices.push_back(std::unique_ptr<Voice>(new Voice(z)));
                  buffer.push(voices.back().get());
                  }
            }

      void push(Voice* v) {
            buffer.push(v);
            }
    
      Voice* pop() {
            Q_ASSERT(!buffer.empty());
            Voice* v = buffer.top();
            buffer.pop();
            return v;
            }

      bool empty() const { return buffer.empty(); }
      };

//---------------------------------------------------------
//   Zerberus
//---------------------------------------------------------

class Zerberus : public Ms::Synthesizer {
      static bool initialized;
      static std::list<ZInstrument*> globalInstruments;

      double _masterTuning = 440.0;
      std::atomic<bool> busy;

      std::list<ZInstrument*> instruments;
      Channel* _channel[MAX_CHANNEL];

      int allocatedVoices = 0;
      VoiceFifo freeVoices;
      Voice* activeVoices = 0;
      int _loadProgress = 0;
      bool _loadWasCanceled = false;

      void programChange(int channel, int program);
      void trigger(Channel*, int key, int velo, Trigger, int cc, int ccVal, double durSinceNoteOn);
      void processNoteOff(Channel*, int pitch);
      void processNoteOn(Channel* cp, int key, int velo);

   public:
      Zerberus();
      ~Zerberus();

      virtual void process(unsigned frames, float*, float*, float*);
      virtual void play(const Ms::PlayEvent& event);

      bool loadInstrument(const QString&);

      ZInstrument* instrument(int program) const;
      Voice* getActiveVoices()      { return activeVoices; }
      Channel* channel(int n)       { return _channel[n]; }
      int loadProgress()            { return _loadProgress; }
      void setLoadProgress(int val) { _loadProgress = val; }
      bool loadWasCanceled()        { return _loadWasCanceled; }
      void setLoadWasCanceled(bool status)     { _loadWasCanceled = status; }

      virtual void setMasterTuning(double val) { _masterTuning = val;  }
      virtual double masterTuning() const      { return _masterTuning; }

      double ct2hz(double c) const { return pow(2.0, (c-6900.0) / 1200.0) * masterTuning(); }

      virtual const char* name() const;
      virtual const QList<Ms::MidiPatch*>& getPatchInfo() const;

      virtual Ms::SynthesizerGroup state() const;
      virtual bool setState(const Ms::SynthesizerGroup&);

      virtual void allSoundsOff(int channel);
      virtual void allNotesOff(int channel);

      virtual bool addSoundFont(const QString&);
      virtual bool removeSoundFont(const QString&);
      virtual bool loadSoundFonts(const QStringList&);
      virtual QStringList soundFonts() const;

      virtual Ms::SynthesizerGui* gui();
      static QFileInfoList sfzFiles();
      };

#endif

