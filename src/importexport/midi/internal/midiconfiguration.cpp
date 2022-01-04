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

#include "libmscore/mscore.h"

#include "internal/midiimport/importmidi_operations.h"

using namespace mu::framework;
using namespace mu::iex::midi;

static const Settings::Key SHORTEST_NOTE_KEY("iex_midi", "io/midi/shortestNote");
static const Settings::Key EXPORTRPNS_KEY("iex_midi", "io/midi/exportRPNs");

void MidiConfiguration::init()
{
    settings()->setDefaultValue(SHORTEST_NOTE_KEY, Val(Ms::Constant::division / 4));
    settings()->setDefaultValue(EXPORTRPNS_KEY, Val(false));
}

int MidiConfiguration::midiShortestNote() const
{
    return settings()->value(SHORTEST_NOTE_KEY).toInt();
}

void MidiConfiguration::setMidiShortestNote(int ticks)
{
    settings()->setSharedValue(SHORTEST_NOTE_KEY, Val(ticks));
}

bool MidiConfiguration::isMidiExportRpns() const
{
    return settings()->value(EXPORTRPNS_KEY).toBool();
}

void MidiConfiguration::setIsMidiExportRpns(bool exportRpns) const
{
    settings()->setSharedValue(EXPORTRPNS_KEY, Val(exportRpns));
}

void MidiConfiguration::setMidiImportOperationsFile(const mu::io::path& filePath) const
{
    Ms::midiImportOperations.setOperationsFile(filePath.toQString());
}
