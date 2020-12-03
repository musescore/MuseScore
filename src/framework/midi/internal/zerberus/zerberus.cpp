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

#include <stdio.h>

#include <QFileInfo>

#include "log.h"
#include "zerberus.h"
#include "voice.h"
#include "channel.h"
#include "instrument.h"
#include "zone.h"

using namespace mu::zerberus;

bool Zerberus::initialized = false;
// instruments can be shared between several zerberus instances
std::list<ZInstrument*> Zerberus::globalInstruments;

//---------------------------------------------------------
//   Zerberus
//---------------------------------------------------------

Zerberus::Zerberus()
{
    if (!initialized) {
        initialized = true;
        Voice::init();
    }

    freeVoices.init(this);
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        _channel[i] = new Channel(this, i);
    }
    busy = true;        // no sf loaded yet
}

//---------------------------------------------------------
//   ~Zerberus
//---------------------------------------------------------

Zerberus::~Zerberus()
{
    busy = true;
    while (!instruments.empty()) {
        auto i  = instruments.front();
        auto it = instruments.begin();
        instruments.erase(it);

        i->setRefCount(i->refCount() - 1);
        if (i->refCount() <= 0) {
            delete i;
            auto it1 = find(globalInstruments.begin(), globalInstruments.end(), i);
            if (it1 != globalInstruments.end()) {
                globalInstruments.erase(it1);
            }
        }
    }
    for (Channel* c : _channel) {
        delete c;
    }

    // qDeleteAll(patches);
}

//---------------------------------------------------------
//   trigger
//    gui
//---------------------------------------------------------

void Zerberus::trigger(Channel* channel, int key, int velo, Trigger trigger, int cc, int ccVal, double durSinceNoteOn)
{
    ZInstrument* i = channel->instrument();
    double random = (double)rand() / (double)RAND_MAX;
    for (Zone* z : i->zones()) {
        if (z->match(channel, key, velo, trigger, random, cc, ccVal)) {
            //
            // handle offBy voices
            //
            if (z->group) {
                for (Voice* v = activeVoices; v; v = v->next()) {
                    if (v->offBy() == z->group) {
                        if (v->offMode() == OffMode::FAST) {
                            v->stop(1);
                        } else {
                            v->stop();
                        }
                    }
                }
            }

            if (freeVoices.empty()) {
                qDebug("Zerberus: out of voices...");
                return;
            }

            Voice* voice = freeVoices.pop();
            Q_ASSERT(voice->isOff());
            voice->start(channel, key, velo, z, durSinceNoteOn);
            voice->setNext(activeVoices);
            activeVoices = voice;
        }
    }
}

//---------------------------------------------------------
//   processNoteOff
//---------------------------------------------------------

void Zerberus::processNoteOff(Channel* cp, int key)
{
    for (Voice* v = activeVoices; v; v = v->next()) {
        if ((v->channel() == cp)
            && (v->key() == key)
            && (v->loopMode() != LoopMode::ONE_SHOT)
            ) {
            if (cp->sustain() < 0x40 && !v->isStopped()) {
                v->stop();
                double durSinceNoteOn = v->getSamplesSinceStart() / sampleRate();
                trigger(cp, key, v->velocity(), Trigger::RELEASE, -1, -1, durSinceNoteOn);
            } else {
                if (v->isPlaying()) {
                    v->sustained();
                }
            }
        }
    }
}

//---------------------------------------------------------
//   processNoteOn
//---------------------------------------------------------

void Zerberus::processNoteOn(Channel* cp, int key, int velo)
{
    for (Voice* v = activeVoices; v; v = v->next()) {
        if (v->channel() == cp && v->key() == key) {
            if (v->isSustained()) {
//if (v->isPlaying())
//printf("retrigger (stop) %p\n", v);
                v->stop(100);             // fast stop
            }
        }
    }
    trigger(cp, key, velo, Trigger::ATTACK, -1, -1, 0);
}

//---------------------------------------------------------
//   process
//---------------------------------------------------------

bool Zerberus::noteOn(int channel, int key, int velo)
{
    Channel* cp = _channel[channel];
    if (cp->instrument() == 0) {
        // no instrument
        return false;
    }

    if (velo) {
        processNoteOn(cp, key, velo);
    } else {
        processNoteOff(cp, key);
    }

    return true;
}

bool Zerberus::noteOff(int channel, int key)
{
    Channel* cp = _channel[channel];
    if (cp->instrument() == 0) {
        // no instrument
        return false;
    }

    processNoteOff(cp, key);

    return true;
}

bool Zerberus::controller(int channel, int num, int val)
{
    Channel* cp = _channel[channel];
    if (cp->instrument() == 0) {
        // no instrument
        return false;
    }

    cp->controller(num, val);
    trigger(cp, -1, -1, Trigger::CC, num, val, 0);

    return true;
}

//---------------------------------------------------------
//   process
//    realtime
//---------------------------------------------------------

void Zerberus::process(unsigned frames, float* p, float*, float*)
{
    if (busy) {
        return;
    }
    Voice* v = activeVoices;
    Voice* pv = 0;
    while (v) {
        v->process(frames, p);
        if (v->isOff()) {
            if (pv) {
                pv->setNext(v->next());
            } else {
                activeVoices = v->next();
            }
            freeVoices.push(v);
        } else {
            pv = v;
        }
        v = v->next();
    }
}

//---------------------------------------------------------
//   allSoundsOff
//---------------------------------------------------------

void Zerberus::allSoundsOff(int channel)
{
    allNotesOff(channel);
}

//---------------------------------------------------------
//   allNotesOff
//---------------------------------------------------------

void Zerberus::allNotesOff(int channel)
{
    busy = true;
    for (Voice* v = activeVoices; v; v = v->next()) {
        if (channel == -1 || (v->channel()->idx() == channel)) {
            v->stop();
        }
    }
    busy = false;
}

//---------------------------------------------------------
//   soundFonts
//---------------------------------------------------------

QStringList Zerberus::soundFonts() const
{
    QStringList sl;
    for (ZInstrument* i : instruments) {
        sl.append(i->path());
    }
    return sl;
}

//---------------------------------------------------------
//   addSoundFont
//---------------------------------------------------------

bool Zerberus::addSoundFont(const QString& path)
{
    bool res = loadInstrument(path);
    return res;
}

//---------------------------------------------------------
//   removeSoundFont
//---------------------------------------------------------

bool Zerberus::removeSoundFont(const QString& path)
{
    for (ZInstrument* i : instruments) {
        if (i->path() == path) {
            auto it = find(instruments.begin(), instruments.end(), i);
            if (it == instruments.end()) {
                return false;
            }
            instruments.erase(it);
            for (int k = 0; k < MAX_CHANNELS; ++k) {
                if (_channel[k]->instrument() == i) {
                    _channel[k]->setInstrument(0);
                }
            }
            if (!instruments.empty()) {
                for (int ii = 0; ii < MAX_CHANNELS; ++ii) {
                    if (_channel[ii]->instrument() == 0) {
                        _channel[ii]->setInstrument(instruments.front());
                    }
                }
            }
            i->setRefCount(i->refCount() - 1);
            if (i->refCount() <= 0) {
                auto it1 = find(globalInstruments.begin(), globalInstruments.end(), i);
                if (it1 == globalInstruments.end()) {
                    return false;
                }
                globalInstruments.erase(it1);
                delete i;
            }

            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

ZInstrument* Zerberus::instrument(int n) const
{
    int idx = 0;
    for (auto i = instruments.begin(); i != instruments.end(); ++i) {
        if (idx == n) {
            return *i;
        }
        ++idx;
    }
    return 0;
}

//---------------------------------------------------------
//   loadInstrument
//    return true on success
//---------------------------------------------------------

bool Zerberus::loadInstrument(const QString& path)
{
    if (path.isEmpty()) {
        return false;
    }
    QFileInfo fis(path);
    QString fileName = fis.fileName();
    for (ZInstrument* instr : instruments) {
        if (QFileInfo(instr->path()).fileName() == fileName) {       // already loaded?
            return true;
        }
    }
    for (ZInstrument* instr : globalInstruments) {
        if (QFileInfo(instr->path()).fileName() == fileName) {
            instruments.push_back(instr);
            instr->setRefCount(instr->refCount() + 1);
            if (instruments.size() == 1) {
                for (int i = 0; i < MAX_CHANNELS; ++i) {
                    _channel[i]->setInstrument(instr);
                }
            }
            busy = false;
            return true;
        }
    }

    busy = true;
    ZInstrument* instr = new ZInstrument(this);

    try {
        if (instr->load(path)) {
            globalInstruments.push_back(instr);
            instruments.push_back(instr);
            instr->setRefCount(1);
            //
            // set default instrument for all channels:
            //
            if (instruments.size() == 1) {
                for (int i = 0; i < MAX_CHANNELS; ++i) {
                    _channel[i]->setInstrument(instr);
                }
            }
            busy = false;
            return true;
        }
    }
    catch (std::bad_alloc& a) {
        LOGE() << "Unable to allocate memory when loading Zerberus soundfont: " << path;

        // Prevent "Unreferenced local variable" warning for a
        Q_UNUSED(a);
    }
    catch (...) {
    }
    qDebug("Zerberus::loadInstrument failed");
    busy = false;
    delete instr;
    return false;
}
