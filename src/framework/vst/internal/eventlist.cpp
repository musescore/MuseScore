//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "eventlist.h"

using namespace mu::vst;
using namespace Steinberg;
using namespace Vst;

DEF_CLASS_IID(IEventList)
IMPLEMENT_FUNKNOWN_METHODS(EventList, IEventList, IEventList::iid)

EventList::EventList()
{
}

void EventList::addMidiEvent(const midi::Event& e)
{
    m_events.push_back(e);
}

void EventList::clear()
{
    m_events.clear();
}

int32 EventList::getEventCount()
{
    return m_events.size();
}

tresult EventList::getEvent(int32 index, Event& e)
{
    if (index >= static_cast<int32>(m_events.size())) {
        return kOutOfMemory;
    }

    auto& midiEvent = m_events[index];
    if (!midiEvent.isChannelVoice()) {
        return kResultFalse;
    }
    e.busIndex = midiEvent.group();
    e.sampleOffset = 0;//NOTE ???
    e.ppqPosition = 0; //NOTE ???
    e.flags = Event::kIsLive;

    switch (midiEvent.opcode()) {
    case midi::Event::Opcode::NoteOn:
        e.type = Event::kNoteOnEvent;
        e.noteOn.noteId = midiEvent.note();
        e.noteOn.channel = midiEvent.channel();
        e.noteOn.pitch = midiEvent.pitchNote();
        e.noteOn.tuning = midiEvent.pitchTuningCents();
        e.noteOn.velocity = midiEvent.velocityFraction();
        break;

    case midi::Event::Opcode::NoteOff:
        e.type = Event::kNoteOffEvent;
        e.noteOff.noteId = midiEvent.note();
        e.noteOff.channel = midiEvent.channel();
        e.noteOff.pitch = midiEvent.pitchNote();
        e.noteOff.tuning = midiEvent.pitchTuningCents();
        e.noteOff.velocity = midiEvent.velocityFraction();
        break;

    case midi::Event::Opcode::PerNotePitchBend:
        break;

    default:
        return kResultFalse;
    }

    return kResultOk;
}

tresult EventList::addEvent(Event& e)
{
    Q_UNUSED(e)
    return kNotImplemented;
}
