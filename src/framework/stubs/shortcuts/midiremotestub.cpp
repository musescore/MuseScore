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
#include "midiremotestub.h"

using namespace muse;
using namespace muse::shortcuts;

const MidiMappingList& MidiRemoteStub::midiMappings() const
{
    static MidiMappingList l;
    return l;
}

Ret MidiRemoteStub::setMidiMappings(const MidiMappingList&)
{
    return muse::make_ret(Ret::Code::NotImplemented);
}

void MidiRemoteStub::resetMidiMappings()
{
}

async::Notification MidiRemoteStub::midiMappingsChanged() const
{
    return async::Notification();
}

void MidiRemoteStub::setIsSettingMode(bool)
{
}

bool MidiRemoteStub::isSettingMode() const
{
    return false;
}

void MidiRemoteStub::setCurrentActionEvent(const muse::midi::Event&)
{
}

Ret MidiRemoteStub::process(const muse::midi::Event&)
{
    return muse::make_ret(Ret::Code::NotImplemented);
}
