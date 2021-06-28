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
#include "midiconfiguration.h"

#include "settings.h"

using namespace mu::midi;
using namespace mu::framework;

static const std::string module_name("midi");

static const Settings::Key USE_REMOTE_CONTROL_KEY(module_name, "io/midi/useRemoteControl");
static const Settings::Key MIDI_INPUT_DEVICE_ID(module_name, "io/portMidi/inputDevice");
static const Settings::Key MIDI_OUTPUT_DEVICE_ID(module_name, "io/portMidi/outputDevice");

void MidiConfiguration::init()
{
    settings()->setDefaultValue(USE_REMOTE_CONTROL_KEY, Val(true));
    settings()->setDefaultValue(MIDI_INPUT_DEVICE_ID, Val(""));
    settings()->setDefaultValue(MIDI_OUTPUT_DEVICE_ID, Val(""));
}

bool MidiConfiguration::useRemoteControl() const
{
    return settings()->value(USE_REMOTE_CONTROL_KEY).toBool();
}

void MidiConfiguration::setUseRemoteControl(bool value)
{
    settings()->setSharedValue(USE_REMOTE_CONTROL_KEY, Val(value));
}

MidiDeviceID MidiConfiguration::midiInputDeviceId() const
{
    return settings()->value(MIDI_INPUT_DEVICE_ID).toString();
}

void MidiConfiguration::setMidiInputDeviceId(const MidiDeviceID& deviceId)
{
    settings()->setSharedValue(MIDI_INPUT_DEVICE_ID, Val(deviceId));
}

MidiDeviceID MidiConfiguration::midiOutputDeviceId() const
{
    return settings()->value(MIDI_OUTPUT_DEVICE_ID).toString();
}

void MidiConfiguration::setMidiOutputDeviceId(const MidiDeviceID& deviceId)
{
    settings()->setSharedValue(MIDI_OUTPUT_DEVICE_ID, Val(deviceId));
}
