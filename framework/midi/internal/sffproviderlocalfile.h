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
#ifndef SYNTHESIZERSETUP_H
#define SYNTHESIZERSETUP_H

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "audio/iaudioengine.h"
#include "async/asyncable.h"
#include "../isynthesizer.h"

namespace mu {
namespace midi {
class SynthesizerSetup : public async::Asyncable
{
    INJECT(midi, ISynthesizer, synth)
    INJECT(midi, audio::IAudioEngine, audioEngine)
    INJECT(midi, framework::IGlobalConfiguration, globalConfiguration)

public:

    void setup();
};
}
}

#endif // SYNTHESIZERSETUP_H
