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
#ifndef MUSE_MIDI_MIDIMODULE_H
#define MUSE_MIDI_MIDIMODULE_H

#include <memory>

#include "modularity/imodulesetup.h"

namespace muse::midi {
class MidiConfiguration;
#if defined(Q_OS_LINUX)
class AlsaMidiOutPort;
class AlsaMidiInPort;
#elif defined(Q_OS_FREEBSD)
class AlsaMidiOutPort;
class AlsaMidiInPort;
#elif defined(Q_OS_WIN)
class WinMidiOutPort;
class WinMidiInPort;
#elif defined(Q_OS_MACOS)
class CoreMidiOutPort;
class CoreMidiInPort;
#else
class DummyMidiOutPort;
class DummyMidiInPort;
#endif
class MidiModule : public modularity::IModuleSetup
{
public:
    std::string moduleName() const override;

    void registerExports() override;
    void registerUiTypes() override;
    void onInit(const IApplication::RunMode& mode) override;
    void onDeinit() override;

private:
    std::shared_ptr<MidiConfiguration> m_configuration;

    #if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    std::shared_ptr<AlsaMidiOutPort> m_midiOutPort;
    std::shared_ptr<AlsaMidiInPort> m_midiInPort;

    #elif defined(Q_OS_WIN)
    std::shared_ptr<WinMidiOutPort> m_midiOutPort;
    std::shared_ptr<WinMidiInPort> m_midiInPort;

    #elif defined(Q_OS_MACOS)
    std::shared_ptr<CoreMidiOutPort> m_midiOutPort;
    std::shared_ptr<CoreMidiInPort> m_midiInPort;

    #else
    std::shared_ptr<DummyMidiOutPort> m_midiOutPort;
    std::shared_ptr<DummyMidiInPort> m_midiInPort;
    #endif
};
}

#endif // MUSE_MIDI_MIDIMODULE_H
