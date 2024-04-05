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
#ifndef MUSE_SHORTCUTS_MIDIREMOTESTUB_H
#define MUSE_SHORTCUTS_MIDIREMOTESTUB_H

#include "shortcuts/imidiremote.h"

namespace muse::shortcuts {
class MidiRemoteStub : public IMidiRemote
{
public:
    const MidiMappingList& midiMappings() const override;
    Ret setMidiMappings(const MidiMappingList& midiMappings) override;
    void resetMidiMappings() override;
    async::Notification midiMappingsChanged() const override;

    // Setting
    void setIsSettingMode(bool arg) override;
    bool isSettingMode() const override;

    void setCurrentActionEvent(const muse::midi::Event& ev) override;

    // Process
    Ret process(const muse::midi::Event& ev) override;
};
}

#endif // MUSE_SHORTCUTS_MIDIREMOTESTUB_H
