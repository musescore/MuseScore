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

#pragma once

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "midi/imidiconfiguration.h"
#include "midi/imidiinport.h"
#include "midi/imidioutport.h"
#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"
#include "shortcuts/imidiremote.h"

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
