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
#include "midiimportconfiguration.h"

#include "settings.h"

#include "libmscore/mscore.h"

using namespace mu::framework;
using namespace mu::iex::midiimport;

static const Settings::Key SHORTEST_NOTE_KEY("iex_midiimport", "io/midi/shortestNote");

void MidiImportConfiguration::init()
{
    settings()->setDefaultValue(SHORTEST_NOTE_KEY, Val(Ms::MScore::division / 4));
}

int MidiImportConfiguration::midiShortestNote() const
{
    return settings()->value(SHORTEST_NOTE_KEY).toInt();
}

void MidiImportConfiguration::setMidiShortestNote(int ticks)
{
    settings()->setValue(SHORTEST_NOTE_KEY, Val(ticks));
}
