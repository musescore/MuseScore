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
#include "midimodule.h"

#include "modularity/ioc.h"
#include "internal/fluidlitesynth.h"
#include "internal/sequencer.h"
#include "internal/sffproviderlocalfile.h"

using namespace mu::audio::midi;

std::string MidiModule::moduleName() const
{
    return "audio_midi";
}

void MidiModule::registerExports()
{
    framework::ioc()->registerExport<ISynthesizer>(moduleName(), new FluidLiteSynth());
    framework::ioc()->registerExport<ISequencer>(moduleName(), new Sequencer());
    framework::ioc()->registerExport<ISoundFontFileProvider>(moduleName(), new SFFProviderLocalFile());
}
