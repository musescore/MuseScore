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
#include "log.h"

#include "modularity/ioc.h"
#include "internal/fluidsynth.h"
#include "internal/zerberussynth.h"
#include "internal/synthesizersregister.h"
#include "internal/midiconfiguration.h"
#include "internal/soundfontsprovider.h"

#include "internal/midiportdatasender.h"

#include "view/synthssettingsmodel.h"

#include "internal/platform/lin/alsamidioutport.h"

#include "internal/synthesizercontroller.h"
#include "ui/iuiengine.h"
#include "devtools/midiportdevmodel.h"

using namespace mu::midi;

#ifdef Q_OS_LINUX
#include "internal/platform/lin/alsamidioutport.h"
#include "internal/platform/lin/alsamidiinport.h"
static std::shared_ptr<IMidiOutPort> midiOutPort = std::make_shared<AlsaMidiOutPort>();
static std::shared_ptr<IMidiInPort> midiInPort = std::make_shared<AlsaMidiInPort>();

#elif defined(Q_OS_WIN)
#include "internal/platform/win/winmidioutport.h"
#include "internal/platform/win/winmidiinport.h"
static std::shared_ptr<IMidiOutPort> midiOutPort = std::make_shared<WinMidiOutPort>();
static std::shared_ptr<IMidiInPort> midiInPort = std::make_shared<WinMidiInPort>();

#elif defined(Q_OS_MACOS)
#include "internal/platform/osx/coremidioutport.h"
#include "internal/platform/osx/coremidiinport.h"
static std::shared_ptr<IMidiOutPort> midiOutPort = std::make_shared<CoreMidiOutPort>();
static std::shared_ptr<IMidiInPort> midiInPort = std::make_shared<CoreMidiInPort>();

#else
#include "internal/dummymidioutport.h"
#include "internal/dummymidiinport.h"
static std::shared_ptr<IMidiOutPort> midiOutPort = std::make_shared<DummyMidiOutPort>();
static std::shared_ptr<IMidiInPort> midiInPort = std::make_shared<DummyMidiInPort>();
#endif

static SynthesizerController s_synthesizerController;

std::string MidiModule::moduleName() const
{
    return "midi";
}

void MidiModule::registerExports()
{
    std::shared_ptr<ISynthesizersRegister> sreg = std::make_shared<SynthesizersRegister>();
    sreg->registerSynthesizer("Zerberus", std::make_shared<ZerberusSynth>());
    sreg->registerSynthesizer("Fluid", std::make_shared<FluidSynth>());
    sreg->setDefaultSynthesizer("Fluid");

    framework::ioc()->registerExport<ISynthesizersRegister>(moduleName(), sreg);
    framework::ioc()->registerExport<IMidiConfiguration>(moduleName(), new MidiConfiguration());
    framework::ioc()->registerExport<ISoundFontsProvider>(moduleName(), new SoundFontsProvider());
    framework::ioc()->registerExport<IMidiPortDataSender>(moduleName(), new MidiPortDataSender());
    framework::ioc()->registerExport<IMidiOutPort>(moduleName(), midiOutPort);
    framework::ioc()->registerExport<IMidiInPort>(moduleName(), midiInPort);
}

void MidiModule::registerUiTypes()
{
    qmlRegisterType<SynthsSettingsModel>("MuseScore.Midi", 1, 0, "SynthsSettingsModel");
    qmlRegisterType<MidiPortDevModel>("MuseScore.Midi", 1, 0, "MidiPortDevModel");
}

void MidiModule::onInit(const framework::IApplication::RunMode&)
{
    s_synthesizerController.init();
}
