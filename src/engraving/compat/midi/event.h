/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_ENGRAVING_COMPAT_EVENT_H
#define MU_ENGRAVING_COMPAT_EVENT_H

#include <map>
#include <vector>

#include "midicoreevent.h"

namespace mu::engraving {
class Note;
class Harmony;

enum class BeatType : char;

//---------------------------------------------------------
//   PlayEvent
//    interface to Synthesizer
//---------------------------------------------------------

class PlayEvent : public MidiCoreEvent
{
    OBJECT_ALLOC(engraving, PlayEvent)
protected:
    float _tuning = .0f;

public:
    PlayEvent()
        : MidiCoreEvent() {}
    PlayEvent(const MidiCoreEvent& e)
        : MidiCoreEvent(e) {}
    PlayEvent(uint8_t t, uint8_t c, uint8_t a, uint8_t b)
        : MidiCoreEvent(t, c, a, b) {}
    float tuning() const { return _tuning; }
    void setTuning(float v) { _tuning = v; }
};

//---------------------------------------------------------
//   NPlayEvent
//    used for Sequencer interface
//---------------------------------------------------------

class NPlayEvent : public PlayEvent
{
    OBJECT_ALLOC(engraving, NPlayEvent)

    const Note* _note{ nullptr };
    const Harmony* _harmony{ nullptr };
    int _origin = -1;
    int _discard = 0;
    bool _portamento = false;

public:
    NPlayEvent()
        : PlayEvent() {}
    NPlayEvent(uint8_t t, uint8_t c, uint8_t a, uint8_t b)
        : PlayEvent(t, c, a, b) {}
    NPlayEvent(const MidiCoreEvent& e)
        : PlayEvent(e) {}
    NPlayEvent(BeatType beatType);

    const Note* note() const { return _note; }
    void setNote(const Note* v) { _note = v; }
    const Harmony* harmony() const { return _harmony; }
    void setHarmony(const Harmony* v) { _harmony = v; }

    int getOriginatingStaff() const { return _origin; }
    void setOriginatingStaff(int i) { _origin = i; }
    void setDiscard(int d) { _discard = d; }
    int discard() const { return _discard; }
    bool isMuted() const;
    void setPortamento(bool p) { _portamento = p; }
    bool portamento() const
    {
        return _portamento == true
               || (this->type() == ME_CONTROLLER
                   && (this->controller() == CTRL_PORTAMENTO || this->controller() == CTRL_PORTAMENTO_CONTROL
                       || this->controller() == CTRL_PORTAMENTO_TIME_MSB || this->controller() == CTRL_PORTAMENTO_TIME_LSB));
    }
};

//---------------------------------------------------------
//   Event
//---------------------------------------------------------

class Event : public PlayEvent
{
    OBJECT_ALLOC(engraving, Event)

    int _ontime;
    int _noquantOntime;
    int _noquantDuration;
    int _duration;
    int _tpc;                 // tonal pitch class
    int _voice;
    std::vector<Event> _notes;
    uint8_t* _edata;             // always zero terminated (_data[_len] == 0; )
    int _len;
    int _metaType;
    const Note* _note;

public:
    Event();
    Event(const Event&);
    Event(int t);
    ~Event();
    bool operator==(const Event&) const;

    void dump() const;

    int noquantOntime() const { return _noquantOntime; }
    void setNoquantOntime(int v) { _noquantOntime = v; }
    int noquantDuration() const { return _noquantDuration; }
    void setNoquantDuration(int v) { _noquantDuration = v; }

    int ontime() const { return _ontime; }
    void setOntime(int v) { _ontime = v; }

    int duration() const { return _duration; }
    void setDuration(int v) { _duration = v; }
    int voice() const { return _voice; }
    void setVoice(int val) { _voice = val; }
    int offtime() const { return _ontime + _duration; }
    std::vector<Event>& notes() { return _notes; }
    const uint8_t* edata() const { return _edata; }
    void setEData(uint8_t* d) { _edata = d; }
    int len() const { return _len; }
    void setLen(int l) { _len = l; }
    int metaType() const { return _metaType; }
    void setMetaType(int v) { _metaType = v; }
    int tpc() const { return _tpc; }
    void setTpc(int v) { _tpc = v; }
    const Note* note() const { return _note; }
    void setNote(const Note* v) { _note = v; }
};

//---------------------------------------------------------
//   EventList
//   EventMap
//---------------------------------------------------------

class EventList : public std::vector<Event>
{
    OBJECT_ALLOC(engraving, EventList)
public:
    void insert(const Event&);
    void insertNote(int channel, Note*);
};

class EventMap : public std::multimap<int, NPlayEvent>
{
    OBJECT_ALLOC(engraving, EventMap)

    int _highestChannel = 15;
public:
    void fixupMIDI();
    void registerChannel(int c)
    {
        if (c > _highestChannel) {
            _highestChannel = c;
        }
    }
};

typedef EventList::iterator iEvent;
typedef EventList::const_iterator ciEvent;
}
#endif
