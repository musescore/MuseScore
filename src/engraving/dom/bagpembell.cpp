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

#include "bagpembell.h"

#include "types/typesconv.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

// Staff line and pitch for every bagpipe note
const BagpipeNoteInfo BagpipeEmbellishment::BAGPIPE_NOTEINFO_LIST[] = {
    { "LG",  6,  65 },
    { "LA",  5,  67 },
    { "B",   4,  69 },
    { "C",   3,  71 },  // actually C#
    { "D",   2,  72 },
    { "E",   1,  74 },
    { "F",   0,  76 },  // actually F#
    { "HG", -1,  77 },
    { "HA", -2,  79 }
};

//---------------------------------------------------------
//   resolveNoteList
//     return notes as list of indices in BagpipeNoteInfoList
//---------------------------------------------------------

BagpipeNoteList BagpipeEmbellishment::resolveNoteList() const
{
    BagpipeNoteList nl;

    StringList notes = TConv::embellishmentNotes(m_embelType);
    int noteInfoSize = sizeof(BAGPIPE_NOTEINFO_LIST) / sizeof(*BAGPIPE_NOTEINFO_LIST);
    for (const String& note : notes) {
        // search for note in BagpipeNoteInfoList
        for (int i = 0; i < noteInfoSize; ++i) {
            if (String::fromAscii(BAGPIPE_NOTEINFO_LIST[i].name.ascii()) == note) {
                // found it, append to list
                nl.push_back(i);
                break;
            }
        }
    }

    return nl;
}

//---------------------------------------------------------
//   mag
//      return fixed magnification
//---------------------------------------------------------

double BagpipeEmbellishment::mag() const
{
    return 0.7;
}
