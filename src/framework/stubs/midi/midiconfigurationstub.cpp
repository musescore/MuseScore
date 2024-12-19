/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "midiconfigurationstub.h"

using namespace muse;
using namespace muse::midi;

bool MidiConfigurationStub::midiPortIsAvalaible() const
{
    return false;
}

bool MidiConfigurationStub::useRemoteControl() const
{
    return false;
}

void MidiConfigurationStub::setUseRemoteControl(bool)
{
}

async::Channel<bool> MidiConfigurationStub::useRemoteControlChanged() const
{
    static async::Channel<bool> ch;
    return ch;
}

MidiDeviceID MidiConfigurationStub::midiInputDeviceId() const
{
    return MidiDeviceID();
}

void MidiConfigurationStub::setMidiInputDeviceId(const MidiDeviceID&)
{
}

async::Notification MidiConfigurationStub::midiInputDeviceIdChanged() const
{
    static async::Notification n;
    return n;
}

MidiDeviceID MidiConfigurationStub::midiOutputDeviceId() const
{
    return MidiDeviceID();
}

void MidiConfigurationStub::setMidiOutputDeviceId(const MidiDeviceID&)
{
}

async::Notification MidiConfigurationStub::midiOutputDeviceIdChanged() const
{
    static async::Notification n;
    return n;
}

bool MidiConfigurationStub::useMIDI20Output() const
{
    return false;
}

void MidiConfigurationStub::setUseMIDI20Output(bool)
{
}

async::Channel<bool> MidiConfigurationStub::useMIDI20OutputChanged() const
{
    static async::Channel<bool> ch;
    return ch;
}
