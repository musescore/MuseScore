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
#include <mutex>
#include <list>

#include "synthesizer/synthesizer.h"
#include "midievent.h"

class Voice;
class Channel;
class ZInstrument;
enum class Trigger;
enum class MidiEventType : unsigned char;

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
      void push(Voice* v) {
            buffer[writeIdx++] = v;
            writeIdx %= MAX_VOICES;
            ++n;
            }
      Voice* pop()  {
            if (n == 0)
                  abort();
            --n;
            Voice* v = buffer[readIdx++];
            readIdx %= MAX_VOICES;
            return v;
            }
      bool empty() const { return n == 0; }
      };

//---------------------------------------------------------
//   MidiEventFifo
//---------------------------------------------------------

class MidiEventFifo {
      MidiEvent buffer[MAX_TRIGGER];
      std::atomic<int> n;
      int writeIdx = 0;       // index of next slot to write
      int readIdx  = 0;       // index of slot to read

   public:
      MidiEventFifo() {
            n = 0;
            }
      void push(const MidiEvent& v) {
            buffer[writeIdx++] = v;
            writeIdx %= MAX_TRIGGER;
            ++n;
            }
      MidiEvent pop()  {
            if (n == 0)
                  abort();
            --n;
            MidiEvent v = buffer[readIdx++];
            readIdx %= MAX_TRIGGER;
            return v;
            }
      bool empty() const { return n == 0; }
      };

//---------------------------------------------------------
//   Zerberus
//---------------------------------------------------------

class Zerberus : public Synthesizer {
      static bool initialized;

      int silentBlocks = 0;

      std::atomic<bool> busy;

      std::list<ZInstrument*> instruments;
      Channel* _channel[MAX_CHANNEL];

      int allocatedVoices = 0;
      VoiceFifo freeVoices;
      Voice* activeVoices = 0;
      MidiEventFifo midiEvents;

      void programChange(int channel, int program);
      void trigger(Channel*, int key, int velo, Trigger);
      void processNoteOff(Channel*, int pitch);
      void processNoteOn(Channel* cp, int key, int velo);

   public:
      Zerberus();
      ~Zerberus();

      virtual void process(unsigned frames, float*, float*, float*);
      virtual void process(const MidiEvent& event);
      virtual void play(const MidiEvent& event);
      bool loadInstrument(const QString&);

      ZInstrument* instrument(int program) const;
      Voice* getActiveVoices() { return activeVoices; }
      Channel* channel(int n)  { return _channel[n]; }

      static double ct2hz(float c) { return 8.176 * pow(2.0, (double)c / 1200.0); }

      virtual const char* name() const;
      virtual bool loadSoundFonts(const QStringList&);
      virtual QStringList soundFonts() const;
      virtual void play(const Event&);
      virtual const QList<MidiPatch*>& getPatchInfo() const;
      virtual SynthesizerGroup state() const;
      virtual void setState(const SynthesizerGroup&);
      virtual void allSoundsOff(int channel);
      virtual void allNotesOff(int channel);
      virtual bool addSoundFont(const QString&);
      virtual bool removeSoundFont(const QString&);
      virtual void setParameter(int, double);
      virtual void setParameter(int, const QString&);

      virtual SynthesizerGui* gui();
      };

#endif

