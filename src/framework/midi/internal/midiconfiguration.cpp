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

#include "miditypes.h"

#include "settings.h"

using namespace muse;
using namespace muse::midi;

static const std::string module_name("midi");

static const Settings::Key USE_REMOTE_CONTROL_KEY(module_name, "io/midi/useRemoteControl");
static const Settings::Key MIDI_INPUT_DEVICE_ID(module_name, "io/inputDeviceId");
static const Settings::Key MIDI_OUTPUT_DEVICE_ID(module_name, "io/outputDeviceId");
static const Settings::Key USE_MIDI20_OUTPUT_KEY(module_name, "io/midi/useMIDI20Output");

void MidiConfiguration::init()
{
    settings()->setDefaultValue(USE_REMOTE_CONTROL_KEY, Val(true));
    settings()->valueChanged(USE_REMOTE_CONTROL_KEY).onReceive(this, [this](const Val& val) {
        m_useRemoteControlChanged.send(val.toBool());
    });

    settings()->setDefaultValue(MIDI_INPUT_DEVICE_ID, Val("")); // "" makes MuseScore select the first available device
    settings()->valueChanged(MIDI_INPUT_DEVICE_ID).onReceive(nullptr, [this](const Val&) {
        m_midiInputDeviceIdChanged.notify();
    });

    settings()->setDefaultValue(MIDI_OUTPUT_DEVICE_ID, Val(NONE_DEVICE_ID));
    settings()->valueChanged(MIDI_OUTPUT_DEVICE_ID).onReceive(nullptr, [this](const Val&) {
        m_midiOutputDeviceIdChanged.notify();
    });

    settings()->setDefaultValue(USE_MIDI20_OUTPUT_KEY, Val(true));
    settings()->valueChanged(USE_MIDI20_OUTPUT_KEY).onReceive(this, [this](const Val& val) {
        m_useMIDI20OutputChanged.send(val.toBool());
    });
}

bool MidiConfiguration::midiPortIsAvalaible() const
{
    return true;
}

bool MidiConfiguration::useRemoteControl() const
{
    return settings()->value(USE_REMOTE_CONTROL_KEY).toBool();
}

void MidiConfiguration::setUseRemoteControl(bool value)
{
    settings()->setSharedValue(USE_REMOTE_CONTROL_KEY, Val(value));
}

async::Channel<bool> MidiConfiguration::useRemoteControlChanged() const
{
    return m_useRemoteControlChanged;
}

MidiDeviceID MidiConfiguration::midiInputDeviceId() const
{
    return settings()->value(MIDI_INPUT_DEVICE_ID).toString();
}

void MidiConfiguration::setMidiInputDeviceId(const MidiDeviceID& deviceId)
{
    settings()->setSharedValue(MIDI_INPUT_DEVICE_ID, Val(deviceId));
}

async::Notification MidiConfiguration::midiInputDeviceIdChanged() const
{
    return m_midiInputDeviceIdChanged;
}

MidiDeviceID MidiConfiguration::midiOutputDeviceId() const
{
    return settings()->value(MIDI_OUTPUT_DEVICE_ID).toString();
}

void MidiConfiguration::setMidiOutputDeviceId(const MidiDeviceID& deviceId)
{
    settings()->setSharedValue(MIDI_OUTPUT_DEVICE_ID, Val(deviceId));
}

async::Notification MidiConfiguration::midiOutputDeviceIdChanged() const
{
    return m_midiOutputDeviceIdChanged;
}

bool MidiConfiguration::useMIDI20Output() const
{
    return settings()->value(USE_MIDI20_OUTPUT_KEY).toBool();
}

void MidiConfiguration::setUseMIDI20Output(bool use)
{
    settings()->setSharedValue(USE_MIDI20_OUTPUT_KEY, Val(use));
}

async::Channel<bool> MidiConfiguration::useMIDI20OutputChanged() const
{
    return m_useMIDI20OutputChanged;
}
