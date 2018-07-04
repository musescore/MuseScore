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
      Voice* buffer[MAX_VOICES];
      std::atomic<int> n;
      int writeIdx = 0;       // index of next slot to write
      int readIdx  = 0;       // index of slot to read

   public:
      VoiceFifo() {
            n = 0;
            }
      ~VoiceFifo() {
            for (Voice* v : buffer)
                  delete v;
            }
      void push(Voice* v) {
            buffer[writeIdx++] = v;
            writeIdx %= MAX_VOICES;
            ++n;
            }
      Voice* pop()  {
            Q_ASSERT(n != 0);
            --n;
            Voice* v = buffer[readIdx++];
            readIdx %= MAX_VOICES;
            return v;
            }
      bool empty() const { return n == 0; }
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

      QMutex mutex;

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
      virtual bool removeSoundFonts(const QStringList& fileNames);
      virtual QStringList soundFonts() const;

      virtual Ms::SynthesizerGui* gui();
      static QFileInfoList sfzFiles();
      };

#endif

