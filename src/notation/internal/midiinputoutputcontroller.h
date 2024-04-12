/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
class MidiInputOutputController : public muse::async::Asyncable
{
    INJECT(muse::midi::IMidiInPort, midiInPort)
    INJECT(muse::midi::IMidiOutPort, midiOutPort)
    INJECT(muse::midi::IMidiConfiguration, midiConfiguration)
    INJECT(context::IGlobalContext, globalContext)
    INJECT(INotationConfiguration, configuration)
    INJECT(muse::shortcuts::IMidiRemote, midiRemote)

public:
    void init();

private:
    void checkInputConnection();
    void checkOutputConnection();

    void checkConnection(const muse::midi::MidiDeviceID& preferredDeviceId, const muse::midi::MidiDeviceID& currentDeviceId,
                         const muse::midi::MidiDeviceList& availableDevices, const std::function<muse::Ret(
                                                                                                     const muse::midi::MidiDeviceID&)>& connectCallback);

    void onMidiEventReceived(const muse::midi::tick_t tick, const muse::midi::Event& event);

    muse::midi::MidiDeviceID firstAvailableDeviceId(const muse::midi::MidiDeviceList& devices) const;
};
}

#endif // MU_NOTATION_MIDIINPUTOUTOUTCONTROLLER_H
