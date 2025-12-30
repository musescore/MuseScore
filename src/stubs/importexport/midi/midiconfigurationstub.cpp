/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "log.h"

using namespace mu;
using namespace muse;
using namespace mu::iex::midi;

int MidiConfigurationStub::midiShortestNote() const
{
    return 0;
}

void MidiConfigurationStub::setMidiShortestNote(int)
{
    NOT_IMPLEMENTED;
}

async::Channel<int> MidiConfigurationStub::midiShortestNoteChanged() const
{
    return {};
}

bool MidiConfigurationStub::roundTempo() const
{
    return false;
}

void MidiConfigurationStub::setRoundTempo(bool)
{
    NOT_IMPLEMENTED;
}

async::Channel<bool> MidiConfigurationStub::roundTempoChanged() const
{
    return {};
}

void MidiConfigurationStub::setMidiImportOperationsFile(const std::optional<muse::io::path_t>&) const
{
    NOT_IMPLEMENTED;
}

bool MidiConfigurationStub::isExpandRepeats() const
{
    return false;
}

void MidiConfigurationStub::setExpandRepeats(bool)
{
    NOT_IMPLEMENTED;
}

bool MidiConfigurationStub::isMidiExportRpns() const
{
    return false;
}

void MidiConfigurationStub::setIsMidiExportRpns(bool)
{
    NOT_IMPLEMENTED;
}
