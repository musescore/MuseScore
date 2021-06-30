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
#ifndef MU_SHORTCUTS_MIDIREMOTE_H
#define MU_SHORTCUTS_MIDIREMOTE_H

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "ishortcutsconfiguration.h"
#include "shortcutstypes.h"
#include "../imidiremote.h"

namespace mu::framework {
class XmlReader;
class XmlWriter;
}

namespace mu::shortcuts {
class MidiRemote : public IMidiRemote
{
    INJECT(shortcuts, IShortcutsConfiguration, configuration)
    INJECT(shortcuts, actions::IActionsDispatcher, dispatcher)
    INJECT(shortcuts, mi::IMultiInstancesProvider, multiInstancesProvider)

public:
    MidiRemote() = default;

    void load();

    const MidiMappingList& midiMappings() const override;
    Ret setMidiMappings(const MidiMappingList& midiMappings) override;

    // Setting
    void setIsSettingMode(bool arg) override;
    bool isSettingMode() const override;

    void setCurrentActionEvent(const midi::Event& ev) override;

    // Process
    Ret process(const midi::Event& ev) override;

private:
    void readMidiMappings();
    MidiControlsMapping readMidiMapping(framework::XmlReader& reader) const;

    bool writeMidiMappings(const MidiMappingList& midiMappings) const;
    void writeMidiMapping(framework::XmlWriter& writer, const MidiControlsMapping& midiMapping) const;

    bool needIgnoreEvent(const midi::Event& event) const;

    RemoteEvent remoteEvent(const std::string& action) const;

    bool m_isSettingMode = false;

    MidiMappingList m_midiMappings;
};
}

#endif // MU_SHORTCUTS_MIDIREMOTE_H
