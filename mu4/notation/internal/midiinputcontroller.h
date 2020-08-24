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
#ifndef MU_NOTATION_MIDIINPUTCONTROLLER_H
#define MU_NOTATION_MIDIINPUTCONTROLLER_H

#include "modularity/ioc.h"
#include "midi/imidiinport.h"
#include "context/iglobalcontext.h"
#include "shortcuts/imidiremote.h"
#include "inotationconfiguration.h"
#include "async/asyncable.h"

namespace mu {
namespace notation {
class MidiInputController : public async::Asyncable
{
    INJECT(notation, midi::IMidiInPort, midiInPort)
    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, INotationConfiguration, configuration)
    INJECT(notation, shortcuts::IMidiRemote, midiRemote)

public:

    void init();

private:

    void onMidiEventReceived(midi::tick_t tick, const midi::Event& event);
};
}
}

#endif // MU_NOTATION_MIDIINPUTCONTROLLER_H
