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

#ifndef MU_ZERBERUS_ZERBERUS_H
#define MU_ZERBERUS_ZERBERUS_H

#include <math.h>
#include <atomic>
#include <list>
#include <memory>
#include <queue>
#include <QString>

#include "voice.h"

namespace mu {
namespace zerberus {
class Channel;
class ZInstrument;
enum class Trigger : char;

static const int MAX_VOICES   = 512;
static const int MAX_CHANNELS = 256;
static const int MAX_TRIGGER  = 512;

//---------------------------------------------------------
//   VoiceFifo
//---------------------------------------------------------

class VoiceFifo
{
    std::queue<Voice*> buffer;
    std::vector< std::unique_ptr<Voice> > voices;

public:
    VoiceFifo()
    {
        voices.resize(MAX_VOICES);
    }

    void init(Zerberus* z)
    {
        for (int i = 0; i < MAX_VOICES; ++i) {
            voices.push_back(std::unique_ptr<Voice>(new Voice(z)));
            buffer.push(voices.back().get());
        }
    }

    void push(Voice* v)
    {
        buffer.push(v);
    }

    Voice* pop()
    {
        Q_ASSERT(!buffer.empty());
        Voice* v = buffer.front();
        buffer.pop();
        return v;
    }

    bool empty() const { return buffer.empty(); }
};

//---------------------------------------------------------
//   Zerberus
//---------------------------------------------------------

class Zerberus //: public Ms::Synthesizer
{
    static bool initialized;
    static std::list<ZInstrument*> globalInstruments;

    double _masterTuning = 440.0;
    std::atomic<bool> busy;

    std::list<ZInstrument*> instruments;
    Channel* _channel[MAX_CHANNELS];

    int allocatedVoices = 0;
    VoiceFifo freeVoices;
    Voice* activeVoices = 0;
    int _loadProgress = 0;
    bool _loadWasCanceled = false;

    float _sampleRate = 0.0f;

    bool loadInstrument(const QString& path);

    void trigger(Channel*, int key, int velo, Trigger, int cc, int ccVal, double durSinceNoteOn);
    void processNoteOff(Channel*, int pitch);
    void processNoteOn(Channel* cp, int key, int velo);

public:
    Zerberus();
    virtual ~Zerberus();

    float sampleRate() const { return _sampleRate; }
    void setSampleRate(float sr) { _sampleRate = sr; }

    bool addSoundFont(const QString& path);
    bool removeSoundFont(const QString& path);
    QStringList soundFonts() const;

    bool noteOn(int channel, int key, int velo);
    bool noteOff(int channel, int key);
    bool controller(int channel, int num, int val);

    void process(unsigned frames, float*, float*, float*);

    ZInstrument* instrument(int program) const;
    Voice* getActiveVoices() { return activeVoices; }
    Channel* channel(int n) { return _channel[n]; }
    int loadProgress() { return _loadProgress; }
    void setLoadProgress(int val) { _loadProgress = val; }
    bool loadWasCanceled() { return _loadWasCanceled; }
    void setLoadWasCanceled(bool status) { _loadWasCanceled = status; }

    void setMasterTuning(double val) { _masterTuning = val; }
    double masterTuning() const { return _masterTuning; }

    double ct2hz(double c) const { return pow(2.0, (c - 6900.0) / 1200.0) * masterTuning(); }

    void allSoundsOff(int channel);
    void allNotesOff(int channel);
};
}
}

#endif //MU_ZERBERUS_ZERBERUS_H
