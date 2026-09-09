/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

// Read LYRIC (syllable text in its size slot) and TIE (arc span decides forward-tie direction).

#include "elem-text.h"
#include "parsers-encoding.h"

namespace mu::iex::enc {
bool EncLyric::read(QDataStream& ds)
{
    EncMeasureElem::read(ds);

    // Syllable text lives inside the size*spacingFactor slot at a format-supplied offset.
    const int textOffset = 5 + static_cast<int>(preKieSkip) + 1 + static_cast<int>(textGapAfterKie);
    const int slot = static_cast<int>(size) * static_cast<int>(spacingFactor);
    int remaining = slot - textOffset;
    if (remaining <= 0) {
        return true;   // too short to carry text; element loop reseeks past it
    }

    ds.skipRawData(preKieSkip);
    ds >> kie;
    ds.skipRawData(textGapAfterKie);

    text = readEncodedStringRemaining(ds, remaining);
    // No trailing skip: the element loop reseeks to the element end after read().
    return true;
}

bool EncTie::read(QDataStream& ds)
{
    EncMeasureElem::read(ds);
    quint8 dirByte = 0;
    quint8 startFlag = 0;
    if (size > 5) {
        ds >> dirByte;          // offset +5
    }
    if (size > 6) {
        ds >> startFlag;        // offset +6
    }
    // Dir/startFlag bit layout: see ENCORE_FORMAT.md §6.7 Tie.
    isTieStart = ((dirByte & 0x80) != 0) || ((startFlag & 0x80) != 0) || ((dirByte & 0x02) != 0);

    // The arc x-span is the authoritative forward-tie signal, overriding the +5 curvature byte.
    // arcX1<arcX2 is a real forward tie; arcX1==arcX2 is either an intra-chord decorative arc
    // (drop) or a cross-measure placeholder (keep, startFlag bit 7 distinguishes them). The pair
    // sits two bytes lower before format 3.07, which is why the shorter 16-byte form carries it
    // too. See ENCORE_FORMAT.md §6.7 Tie.
    if (static_cast<int>(size) >= 18 + bodyShift) {
        ds.skipRawData(3 + bodyShift);   // to offset +10, or +8 before format 3.07
        // Both endpoints are uint16, read through the stream so the file's byte order applies.
        // Taking only their low byte silently yields zero on a big-endian file.
        ds >> arcX1 >> arcX2;
        if (arcX1 < arcX2) {
            isTieStart = true;
        } else if (arcX1 == arcX2 && (startFlag & 0x80) == 0) {
            isTieStart = false;
        }
        quint8 sp = 0;              // offset +14: staff position of source note
        ds >> sp;
        sourcePosition = static_cast<qint8>(sp);
    }
    // No trailing skip: the element loop reseeks to the element end after read().
    return true;
}
} // namespace mu::iex::enc
