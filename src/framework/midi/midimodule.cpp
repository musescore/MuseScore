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
#include "midimodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "internal/midiconfiguration.h"

#include "ui/iuiengine.h"
#include "view/devtools/midiportdevmodel.h"

#include "log.h"

using namespace mu::midi;

static std::shared_ptr<MidiConfiguration> s_configuration = std::make_shared<MidiConfiguration>();

#ifdef Q_OS_LINUX
#include "internal/platform/lin/alsamidioutport.h"
#include "internal/platform/lin/alsamidiinport.h"
static std::shared_ptr<AlsaMidiOutPort> s_midiOutPort = std::make_shared<AlsaMidiOutPort>();
static std::shared_ptr<AlsaMidiInPort> s_midiInPort = std::make_shared<AlsaMidiInPort>();

#elif defined(Q_OS_WIN)
#include "internal/platform/win/winmidioutport.h"
#include "internal/platform/win/winmidiinport.h"
static std::shared_ptr<WinMidiOutPort> s_midiOutPort = std::make_shared<WinMidiOutPort>();
static std::shared_ptr<WinMidiInPort> s_midiInPort = std::make_shared<WinMidiInPort>();

#elif defined(Q_OS_MACOS)
#include "internal/platform/osx/coremidioutport.h"
#include "internal/platform/osx/coremidiinport.h"
static std::shared_ptr<CoreMidiOutPort> s_midiOutPort = std::make_shared<CoreMidiOutPort>();
static std::shared_ptr<CoreMidiInPort> s_midiInPort = std::make_shared<CoreMidiInPort>();

#else
#include "internal/dummymidioutport.h"
#include "internal/dummymidiinport.h"
static std::shared_ptr<DummyMidiOutPort> s_midiOutPort = std::make_shared<DummyMidiOutPort>();
static std::shared_ptr<DummyMidiInPort> s_midiInPort = std::make_shared<DummyMidiInPort>();
#endif

std::string MidiModule::moduleName() const
{
    return "midi";
}

void MidiModule::registerExports()
{
    modularity::ioc()->registerExport<IMidiConfiguration>(moduleName(), s_configuration);
    modularity::ioc()->registerExport<IMidiOutPort>(moduleName(), s_midiOutPort);
    modularity::ioc()->registerExport<IMidiInPort>(moduleName(), s_midiInPort);
}

void MidiModule::registerUiTypes()
{
    qmlRegisterType<MidiPortDevModel>("MuseScore.Midi", 1, 0, "MidiPortDevModel");
}

void MidiModule::onInit(const framework::IApplication::RunMode& mode)
{
    s_configuration->init();

    if (mode == framework::IApplication::RunMode::Editor) {
        s_midiOutPort->init();
        s_midiInPort->init();
    }
}

void MidiModule::onDeinit()
{
    s_midiOutPort->deinit();
    s_midiInPort->deinit();
}
