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
#ifndef MU_NOTATION_MIDIINPUTOUTOUTCONTROLLER_H
#define MU_NOTATION_MIDIINPUTOUTOUTCONTROLLER_H

#include "modularity/ioc.h"
#include "midi/imidiinport.h"
#include "midi/imidioutport.h"
#include "midi/imidiconfiguration.h"
#include "context/iglobalcontext.h"
#include "shortcuts/imidiremote.h"
#include "inotationconfiguration.h"
#include "async/asyncable.h"

namespace mu::notation {
class MidiInputOutputController : public async::Asyncable
{
    INJECT(notation, midi::IMidiInPort, midiInPort)
    INJECT(notation, midi::IMidiOutPort, midiOutPort)
    INJECT(notation, midi::IMidiConfiguration, midiConfiguration)
    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, INotationConfiguration, configuration)
    INJECT(notation, shortcuts::IMidiRemote, midiRemote)

public:
    void init();

private:
    void checkInputConnection();
    void checkOutputConnection();

    void checkConnection(const midi::MidiDeviceID& preferredDeviceId, const midi::MidiDeviceID& currentDeviceId,
                         const midi::MidiDeviceList& availableDevices, const std::function<Ret(
                                                                                               const midi::MidiDeviceID&)>& connectCallback);

    void onMidiEventsReceived(const std::vector<std::pair<midi::tick_t, midi::Event> >& events);

    midi::MidiDeviceID firstAvailableDeviceId(const midi::MidiDeviceList& devices) const;
};
}

#endif // MU_NOTATION_MIDIINPUTOUTOUTCONTROLLER_H
