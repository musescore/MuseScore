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
#ifndef MU_MIDI_SYNTHESIZERCONTROLLER_H
#define MU_MIDI_SYNTHESIZERCONTROLLER_H

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "audio/iaudioengine.h"
#include "async/asyncable.h"
#include "../isynthesizersregister.h"
#include "../isoundfontsprovider.h"

namespace mu {
namespace midi {
class SynthesizerController : public async::Asyncable
{
    INJECT(midi, audio::IAudioEngine, audioEngine)
    INJECT(midi, ISynthesizersRegister, synthRegister)
    INJECT(midi, ISoundFontsProvider, sfprovider)

public:

    void init();

private:

    void reloadSoundFonts(std::shared_ptr<ISynthesizer> synth);
};
}
}

#endif // MU_MIDI_SYNTHESIZERCONTROLLER_H
