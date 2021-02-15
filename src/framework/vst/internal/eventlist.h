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
#ifndef MU_VST_EVENTLIST_H
#define MU_VST_EVENTLIST_H

#include "framework/midi/miditypes.h"
#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/vst/ivstevents.h"

namespace mu {
namespace vst {
class EventList : public Steinberg::Vst::IEventList
{
public:
    EventList();
    virtual ~EventList() = default;

    DECLARE_FUNKNOWN_METHODS

    void addMidiEvent(const midi::Event& e);
    void clear();

    //methods for VST SDK:
    Steinberg::int32 getEventCount() override;
    Steinberg::tresult getEvent(Steinberg::int32 index, Steinberg::Vst::Event& e) override;
    Steinberg::tresult addEvent(Steinberg::Vst::Event& e) override;

private:
    std::vector<midi::Event> m_events = {};
};
} // namespace vst
} // namespace mu

#endif // MU_VST_EVENTLIST_H
