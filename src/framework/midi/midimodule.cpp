/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "midimodule.h"

#include "internal/midiconfiguration.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#include "internal/platform/lin/alsamidioutport.h"
#include "internal/platform/lin/alsamidiinport.h"
#elif defined(Q_OS_WIN)
#include "internal/platform/win/winmidioutport.h"
#include "internal/platform/win/winmidiinport.h"
#elif defined(Q_OS_MACOS)
#include "internal/platform/osx/coremidioutport.h"
#include "internal/platform/osx/coremidiinport.h"
#else
#include "internal/dummymidioutport.h"
#include "internal/dummymidiinport.h"
#endif

using namespace muse::midi;

std::string MidiModule::moduleName() const
{
    return "midi";
}

void MidiModule::registerExports()
{
    m_configuration = std::make_shared<MidiConfiguration>();

    #if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    m_midiOutPort = std::make_shared<AlsaMidiOutPort>();
    m_midiInPort = std::make_shared<AlsaMidiInPort>();
    #elif defined(Q_OS_WIN)
    m_midiOutPort = std::make_shared<WinMidiOutPort>();
    m_midiInPort = std::make_shared<WinMidiInPort>();
    #elif defined(Q_OS_MACOS)
    m_midiOutPort = std::make_shared<CoreMidiOutPort>();
    m_midiInPort = std::make_shared<CoreMidiInPort>();
    #else
    m_midiOutPort = std::make_shared<DummyMidiOutPort>();
    m_midiInPort = std::make_shared<DummyMidiInPort>();
    #endif

    globalIoc()->registerExport<IMidiConfiguration>(moduleName(), m_configuration);
    globalIoc()->registerExport<IMidiOutPort>(moduleName(), m_midiOutPort);
    globalIoc()->registerExport<IMidiInPort>(moduleName(), m_midiInPort);
}

void MidiModule::onInit(const IApplication::RunMode& mode)
{
    m_configuration->init();

    if (mode == IApplication::RunMode::GuiApp) {
        m_midiOutPort->init();
        m_midiInPort->init();
    }
}

void MidiModule::onDeinit()
{
    m_midiOutPort->deinit();
    m_midiInPort->deinit();
}
