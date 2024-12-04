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
#ifndef MUSE_MIDI_IMIDICONFIGURATION_H
#define MUSE_MIDI_IMIDICONFIGURATION_H

#include "modularity/imoduleinterface.h"
#include "miditypes.h"

//! NOTE There used to be synthesizer settings here, now nothing
//! but we will keep the interface, maybe something will appear,
//! for example, midi port settings

namespace muse::midi {
class IMidiConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiConfiguration)

public:
    virtual ~IMidiConfiguration() = default;

    virtual bool midiPortIsAvalaible() const = 0;

    virtual bool useRemoteControl() const = 0;
    virtual void setUseRemoteControl(bool value) = 0;
    virtual async::Channel<bool> useRemoteControlChanged() const = 0;

    virtual MidiDeviceID midiInputDeviceId() const = 0;
    virtual void setMidiInputDeviceId(const MidiDeviceID& deviceId) = 0;
    virtual async::Notification midiInputDeviceIdChanged() const = 0;

    virtual MidiDeviceID midiOutputDeviceId() const = 0;
    virtual void setMidiOutputDeviceId(const MidiDeviceID& deviceId) = 0;
    virtual async::Notification midiOutputDeviceIdChanged() const = 0;

    virtual bool useMIDI20Output() const = 0;
    virtual void setUseMIDI20Output(bool use) = 0;
    virtual async::Channel<bool> useMIDI20OutputChanged() const = 0;
};
}

#endif // MUSE_MIDI_IMIDICONFIGURATION_H
