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

#include "mscore/preferences.h"
#include "synthesizer/event.h"
#include "synthesizer/midipatch.h"

#include "zerberus.h"
#include "zerberusgui.h"
#include "voice.h"
#include "channel.h"
#include "instrument.h"
#include "zone.h"

#include <stdio.h>

bool Zerberus::initialized = false;
// instruments can be shared between several zerberus instances
std::list<ZInstrument*> Zerberus::globalInstruments;

//---------------------------------------------------------
//   Zerberus
//---------------------------------------------------------

Zerberus::Zerberus()
   : Synthesizer()
      {
      if (!initialized) {
            initialized = true;
            Voice::init();
            }
      for (int i = 0; i < MAX_VOICES; ++i)
            freeVoices.push(new Voice(this));
      for (int i = 0; i < MAX_CHANNEL; ++i)
            _channel[i] = new Channel(this, i);
      busy = true;      // no sf loaded yet
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
printf("delete sf <%s>\n", qPrintable(i->name()));
                  delete i;
                  auto it = find(globalInstruments.begin(), globalInstruments.end(), i);
                  if (it != globalInstruments.end())
                        globalInstruments.erase(it);
                  }
            }
      }

//---------------------------------------------------------
//   programChange
//---------------------------------------------------------

void Zerberus::programChange(int channel, int program)
      {
      printf("Zerberus programChange %d %d\n", channel, program);
      }

//---------------------------------------------------------
//   trigger
//    gui
//---------------------------------------------------------

void Zerberus::trigger(Channel* channel, int key, int velo, Trigger trigger)
      {
      ZInstrument* i = channel->instrument();
// printf("trigger %d %d type %d\n", key, velo, trigger);
      for (Zone* z : i->zones()) {
            if (z->match(channel, key, velo, trigger)) {
//                  printf("  match %d\n", z->trigger);
                  if (freeVoices.empty()) {
                        printf("out of voices...\n");
                        return;
                        }
                  Voice* voice = freeVoices.pop();
                  if (!voice->isOff())
                        abort();
                  voice->start(channel, key, velo, z);
                  if (trigger == Trigger::RELEASE)
                        voice->stop();    // start voice in stop mode
                  voice->setNext(activeVoices);
                  activeVoices = voice;

                  //
                  // handle offBy voices
                  //
                  if (z->group) {
                        for (Voice* v = activeVoices; v; v = v->next()) {
                              if (v->offBy() == z->group) {
                                    if (v->offMode() == OffMode::FAST)
                                          v->stop(1);
                                    else
                                          v->stop();
                                    }
                              }
                        }
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
                  if (cp->sustain() < 0x40) {
                        v->stop();
                        trigger(cp, key, v->velocity(), Trigger::RELEASE);
                        }
                  else {
                        if (v->isPlaying()) {
                              // printf("sustain %p\n", v);
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
                        // if (v->isPlaying())
                        // printf("retrigger (stop) %p\n", v);
                        v->stop(100);     // fast stop
                        }
                  }
            }
      trigger(cp, key, velo, Trigger::ATTACK);
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Zerberus::play(const PlayEvent& event)
      {
      if (busy)
            return;
      Channel* cp = _channel[int(event.channel())];
      if (cp->instrument() == 0) {
//            printf("Zerberus::play(): no instrument for channel %d\n", event.channel());
            return;
            }

      switch(event.type()) {
            case ME_NOTEOFF:
                  processNoteOff(cp, event.dataA());
                  break;

            case ME_NOTEON: {
                  int key = event.dataA();
                  int vel = event.dataB();
                  if (vel)
                        processNoteOn(cp, key, vel);
                  else
                        processNoteOff(cp, key);
                  }
                  break;

            case ME_CONTROLLER:
                  cp->controller(event.dataA(), event.dataB());
                  break;

            default:
                  printf("event type 0x%02x\n", event.type());
                  break;
            }
      }

//---------------------------------------------------------
//   process
//    realtime
//---------------------------------------------------------

void Zerberus::process(unsigned frames, float* p, float*, float*)
      {
      if (busy) {
            printf("busy\n");
            return;
            }
      Voice* v = activeVoices;
      Voice* pv = 0;
      while (v) {
            v->process(frames, p);
            if (v->isOff()) {
                  if (pv)
                        pv->setNext(v->next());
                  else
                        activeVoices = v->next();
                  freeVoices.push(v);
                  }
            else
                  pv = v;
            v = v->next();
            }
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* Zerberus::name() const
      {
      return "Zerberus";
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

const QList<MidiPatch*>& Zerberus::getPatchInfo() const
      {
      static QList<MidiPatch*> pl;
      qDeleteAll(pl);
      pl.clear();
      int idx = 0;
      for (ZInstrument* i : instruments) {
            MidiPatch* p = new MidiPatch { false, name(), 0, idx, i->name() };
            pl.append(p);
            ++idx;
            }
      return pl;
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
            if (channel == -1 || (v->channel()->idx() == channel))
                  v->stop();
            }
      busy = false;
      }

//---------------------------------------------------------
//   loadSoundFonts
//---------------------------------------------------------

bool Zerberus::loadSoundFonts(const QStringList& sl)
      {
      foreach (const QString& s, sl) {
            if (!loadInstrument(s))
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   soundFonts
//---------------------------------------------------------

QStringList Zerberus::soundFonts() const
      {
      QStringList sl;
      for (ZInstrument* i : instruments)
            sl.append(QFileInfo(i->path()).fileName());
      return sl;
      }

//---------------------------------------------------------
//   addSoundFont
//---------------------------------------------------------

bool Zerberus::addSoundFont(const QString& s)
      {
      return loadInstrument(s);
      }

//---------------------------------------------------------
//   removeSoundFont
//---------------------------------------------------------

bool Zerberus::removeSoundFont(const QString& s)
      {
      for (ZInstrument* i : instruments) {
            if (i->path() == s) {
                  auto it = find(instruments.begin(), instruments.end(), i);
                  if (it == instruments.end())
                        return false;
                  instruments.erase(it);
                  for (int k = 0; k < MAX_CHANNEL; ++k) {
                        if (_channel[k]->instrument() == i)
                              _channel[k]->setInstrument(0);
                        }
                  if (!instruments.empty()) {
                        for (int i = 0; i < MAX_CHANNEL; ++i) {
                              if (_channel[i]->instrument() == 0)
                                    _channel[i]->setInstrument(instruments.front());
                              }
                        }
                  i->setRefCount(i->refCount() - 1);
                  if (i->refCount() <= 0) {
                        auto it = find(globalInstruments.begin(), globalInstruments.end(), i);
                        if (it == globalInstruments.end())
                              return false;
                        globalInstruments.erase(it);
printf("delete sf <%s>\n", qPrintable(i->name()));
                        delete i;
                        }
                  return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   state
//---------------------------------------------------------

SynthesizerGroup Zerberus::state() const
      {
      SynthesizerGroup g;
      g.setName(name());

      QStringList sfl = soundFonts();
      foreach(QString sf, sfl)
            g.push_back(IdValue(0, sf));
      return g;
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void Zerberus::setState(const SynthesizerGroup& sp)
      {
      QStringList sfs;
      for (const IdValue& v : sp)
            sfs.append(v.data);
      loadSoundFonts(sfs);
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

ZInstrument* Zerberus::instrument(int n) const
      {
      int idx = 0;
      for (auto i = instruments.begin(); i != instruments.end(); ++i) {
            if (idx == n)
                  return *i;
            ++idx;
            }
      return 0;
      }

//---------------------------------------------------------
//   loadInstrument
//    return true on success
//---------------------------------------------------------

bool Zerberus::loadInstrument(const QString& s)
      {
      printf("Zerberus::loadInstrument(%s)\n", qPrintable(s));
      if (s.isEmpty())
            return false;
      for (ZInstrument* instr : instruments) {
            if (QFileInfo(instr->path()).fileName() == s) {   // already loaded?
                  printf("sf already loaded\n");
                  return true;
                  }
            }
      for (ZInstrument* instr : globalInstruments) {
            if (QFileInfo(instr->path()).fileName() == s) {
                  instruments.push_back(instr);
                  instr->setRefCount(instr->refCount() + 1);
                  printf("2: sf already loaded\n");
                  if (instruments.size() == 1) {
                        for (int i = 0; i < MAX_CHANNEL; ++i)
                              _channel[i]->setInstrument(instr);
                        }
                  busy = false;
                  return true;
                  }
            }

      QFileInfoList l = Zerberus::sfzFiles();
      QString path;
      foreach (const QFileInfo& fi, l) {
            if (fi.fileName() == s) {
                  path = fi.absoluteFilePath();
                  break;
                  }
            }
      busy = true;
      ZInstrument* instr = new ZInstrument();
      connect(instr, SIGNAL(progress(int)), SLOT(setLoadProgress(int)));

      if (instr->load(path)) {
            globalInstruments.push_back(instr);
            instruments.push_back(instr);
            instr->setRefCount(1);
            //
            // set default instrument for all channels:
            //
            if (instruments.size() == 1) {
                  for (int i = 0; i < MAX_CHANNEL; ++i)
                        _channel[i]->setInstrument(instr);
                  }
            busy = false;
            return true;
            }
      qDebug("Zerberus::loadInstrument failed");
      busy = false;
      delete instr;
      return false;
      }
