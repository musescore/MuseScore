/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "midiremoteconfiguration.h"
#include "global/settings.h"

#include "log.h"

using namespace muse;
using namespace muse::midiremote;

static const Settings::Key ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE("shortcuts", "io/midi/advanceOnRelease");

static const std::string MIDIMAPPINGS_FILE_NAME("/midi_mappings.xml");

void MidiRemoteConfiguration::init()
{
    settings()->setDefaultValue(ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE, Val(true));
    settings()->valueChanged(ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE).onReceive(this, [this](const Val& val) {
        m_advanceToNextNoteOnKeyReleaseChanged.send(val.toBool());
    });
}

io::path_t MidiRemoteConfiguration::midiMappingUserAppDataPath() const
{
    return globalConfiguration()->userAppDataPath() + MIDIMAPPINGS_FILE_NAME;
}

bool MidiRemoteConfiguration::advanceToNextNoteOnKeyRelease() const
{
    return settings()->value(ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE).toBool();
}

void MidiRemoteConfiguration::setAdvanceToNextNoteOnKeyRelease(bool value)
{
    settings()->setSharedValue(ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE, Val(value));
}

async::Channel<bool> MidiRemoteConfiguration::advanceToNextNoteOnKeyReleaseChanged() const
{
    return m_advanceToNextNoteOnKeyReleaseChanged;
}
