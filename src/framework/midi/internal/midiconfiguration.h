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
#ifndef MUSE_MIDI_MIDICONFIGURATION_H
#define MUSE_MIDI_MIDICONFIGURATION_H

#include "../imidiconfiguration.h"
#include "async/asyncable.h"

namespace muse::midi {
class MidiConfiguration : public IMidiConfiguration, public async::Asyncable
{
public:
    void init();

    bool midiPortIsAvalaible() const override;

    bool useRemoteControl() const override;
    void setUseRemoteControl(bool value) override;
    async::Channel<bool> useRemoteControlChanged() const override;

    MidiDeviceID midiInputDeviceId() const override;
    void setMidiInputDeviceId(const MidiDeviceID& deviceId) override;
    async::Notification midiInputDeviceIdChanged() const override;

    MidiDeviceID midiOutputDeviceId() const override;
    void setMidiOutputDeviceId(const MidiDeviceID& deviceId) override;
    async::Notification midiOutputDeviceIdChanged() const override;

    bool useMIDI20Output() const override;
    void setUseMIDI20Output(bool use) override;
    async::Channel<bool> useMIDI20OutputChanged() const override;

private:
    async::Notification m_midiInputDeviceIdChanged;
    async::Notification m_midiOutputDeviceIdChanged;
    async::Channel<bool> m_useRemoteControlChanged;
    async::Channel<bool> m_useMIDI20OutputChanged;
};
}

#endif // MUSE_MIDI_MIDICONFIGURATION_H
