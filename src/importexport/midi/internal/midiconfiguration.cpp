/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "engraving/types/constants.h"

#include "midiimport/importmidi_operations.h"

using namespace mu;
using namespace muse;
using namespace mu::iex::midi;

static const Settings::Key SHORTEST_NOTE_KEY("iex_midi", "io/midi/shortestNote");
static const Settings::Key EXPORTRPNS_KEY("iex_midi", "io/midi/exportRPNs");
static const Settings::Key EXPAND_REPEATS_KEY("iex_midi", "io/midi/expandRepeats");

void MidiConfiguration::init()
{
    settings()->setDefaultValue(SHORTEST_NOTE_KEY, Val(mu::engraving::Constants::DIVISION / 4));
    settings()->valueChanged(SHORTEST_NOTE_KEY).onReceive(this, [this](const Val& val) {
        m_midiShortestNoteChanged.send(val.toInt());
    });

    settings()->setDefaultValue(EXPAND_REPEATS_KEY, Val(true));
    settings()->setDefaultValue(EXPORTRPNS_KEY, Val(true));
}

int MidiConfiguration::midiShortestNote() const
{
    return settings()->value(SHORTEST_NOTE_KEY).toInt();
}

void MidiConfiguration::setMidiShortestNote(int ticks)
{
    settings()->setSharedValue(SHORTEST_NOTE_KEY, Val(ticks));
}

async::Channel<int> MidiConfiguration::midiShortestNoteChanged() const
{
    return m_midiShortestNoteChanged;
}

void MidiConfiguration::setMidiImportOperationsFile(const std::optional<muse::io::path_t>& filePath) const
{
    if (filePath) {
        midiImportOperations.setOperationsFile(filePath.value().toQString());
    }
}

bool MidiConfiguration::isExpandRepeats() const
{
    return settings()->value(EXPAND_REPEATS_KEY).toBool();
}

void MidiConfiguration::setExpandRepeats(bool expand)
{
    settings()->setSharedValue(EXPAND_REPEATS_KEY, Val(expand));
}

bool MidiConfiguration::isMidiExportRpns() const
{
    return settings()->value(EXPORTRPNS_KEY).toBool();
}

void MidiConfiguration::setIsMidiExportRpns(bool exportRpns)
{
    settings()->setSharedValue(EXPORTRPNS_KEY, Val(exportRpns));
}
