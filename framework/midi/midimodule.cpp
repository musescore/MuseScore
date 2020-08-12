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

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "internal/fluidsynth.h"
#include "internal/zerberussynth.h"
#include "internal/sequencer.h"
#include "internal/synthesizersregister.h"
#include "internal/midiconfiguration.h"
#include "internal/soundfontsprovider.h"
#include "internal/dummymidiport.h"
#include "internal/midiportdatasender.h"

#include "view/synthssettingsmodel.h"

#include "internal/synthesizercontroller.h"

using namespace mu::midi;

static SynthesizerController s_synthesizerController;

std::string MidiModule::moduleName() const
{
    return "midi";
}

void MidiModule::registerExports()
{
    std::shared_ptr<ISynthesizersRegister> sreg = std::make_shared<SynthesizersRegister>();
    sreg->registerSynthesizer("Fluid", std::make_shared<FluidSynth>());
    sreg->registerSynthesizer("Zerberus", std::make_shared<ZerberusSynth>());
    sreg->setDefaultSynthesizer("Fluid");

    framework::ioc()->registerExport<ISynthesizersRegister>(moduleName(), sreg);
    framework::ioc()->registerExport<ISequencer>(moduleName(), new Sequencer());
    framework::ioc()->registerExport<IMidiConfiguration>(moduleName(), new MidiConfiguration());
    framework::ioc()->registerExport<ISoundFontsProvider>(moduleName(), new SoundFontsProvider());
    framework::ioc()->registerExport<IMidiPort>(moduleName(), new DummyMidiPort());
    framework::ioc()->registerExport<IMidiPortDataSender>(moduleName(), new MidiPortDataSender());
}

void MidiModule::registerUiTypes()
{
    qmlRegisterType<SynthsSettingsModel>("MuseScore.Midi", 1, 0, "SynthsSettingsModel");
}

void MidiModule::onInit()
{
    s_synthesizerController.init();
}
