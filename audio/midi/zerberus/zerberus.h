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
#include <queue>

#include "voice.h"

#include "audio/midi/synthesizer.h"
#include "audio/midi/event.h"
#include "audio/midi/midipatch.h"

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

class Zerberus : public Ms::Synthesizer
{
    static bool initialized;
    static std::list<ZInstrument*> globalInstruments;
    QList<Ms::MidiPatch*> patches;

    double _masterTuning = 440.0;
    std::atomic<bool> busy;

    std::list<ZInstrument*> instruments;
    Channel* _channel[MAX_CHANNELS];

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

    virtual void process(unsigned frames, float*, float*, float*) override;
    virtual void play(const Ms::PlayEvent& event) override;

    bool loadInstrument(const QString&);

    ZInstrument* instrument(int program) const;
    Voice* getActiveVoices() { return activeVoices; }
    Channel* channel(int n) { return _channel[n]; }
    int loadProgress() { return _loadProgress; }
    void setLoadProgress(int val) { _loadProgress = val; }
    bool loadWasCanceled() { return _loadWasCanceled; }
    void setLoadWasCanceled(bool status) { _loadWasCanceled = status; }

    virtual void setMasterTuning(double val) override { _masterTuning = val; }
    virtual double masterTuning() const override { return _masterTuning; }

    double ct2hz(double c) const { return pow(2.0, (c - 6900.0) / 1200.0) * masterTuning(); }

    virtual const char* name() const override;

    virtual Ms::SynthesizerGroup state() const override;
    virtual bool setState(const Ms::SynthesizerGroup&) override;

    virtual void allSoundsOff(int channel) override;
    virtual void allNotesOff(int channel) override;

    virtual bool addSoundFont(const QString&) override;
    virtual bool removeSoundFont(const QString&) override;
    virtual bool loadSoundFonts(const QStringList&) override;
    virtual bool removeSoundFonts(const QStringList& fileNames);
    QStringList soundFonts() const;
    std::vector<Ms::SoundFontInfo> soundFontsInfo() const override;

    virtual const QList<Ms::MidiPatch*>& getPatchInfo() const override { return patches; }

    void updatePatchList();

    virtual Ms::SynthesizerGui* gui() override;
    static QFileInfoList sfzFiles();
};

#endif
