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

#include "bagpembell.h"

#include "draw/types/pen.h"

#include "types/typesconv.h"

#include "iengravingfont.h"

#include "score.h"
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
//   BEDrawingDataX
//      BagpipeEmbellishment drawing data in the x direction
//      shared between ::draw() and ::layout()
//---------------------------------------------------------

BagpipeEmbellishment::BEDrawingDataX::BEDrawingDataX(SymId hs, SymId fs, const double m, const double s, const int nn)
    : headsym(hs), flagsym(fs), mags(0.75 * m), spatium(s), lw(0.1 * s), xcorr(0.1 * s)
{
    double w = Score::paletteScore()->engravingFont()->width(hs, mags);
    headw = 1.2 * w;         // using 1.0 the stem xpos is off
    headp = 1.6 * w;
    xl    = (1 - 1.6 * (nn - 1)) * w / 2;
}

//---------------------------------------------------------
//   BEDrawingDataY
//      BagpipeEmbellishment drawing data in the y direction
//      shared between ::draw() and ::layout()
//---------------------------------------------------------

BagpipeEmbellishment::BEDrawingDataY::BEDrawingDataY(const int l, const double s)
    : y1b(-8 * s / 2),
    y1f((l - 6) * s / 2),
    y2(l * s / 2),
    ycorr(0.8 * s),
    bw(0.3 * s) {}

//---------------------------------------------------------
//   mag
//      return fixed magnification
//---------------------------------------------------------

double BagpipeEmbellishment::mag() const
{
    return 0.7;
}

void BagpipeEmbellishment::draw(mu::draw::Painter*) const
{
    UNREACHABLE;
}

void BagpipeEmbellishment::setLayoutData(const LayoutData& data)
{
    m_layoutData = data;
    setbbox(data.bbox);
}
