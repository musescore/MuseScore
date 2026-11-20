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

// Read ORNAMENT elements (slurs, wedges, staff text, etc.): geometry, spanning measure counts, tind.

#include "elem-ornament.h"
#include "readers.h"

namespace mu::iex::enc {
// These fields are words whose value fits a byte. Reading the word takes the right byte in either byte
// order, where reading the first byte returns the high one in a big-endian file. See ENCORE_FORMAT.md 6.8.
static quint8 wordLowByte(QDataStream& ds)
{
    quint16 word = 0;
    ds >> word;
    return static_cast<quint8>(word & 0xFF);
}

bool EncOrnament::read(QDataStream& ds)
{
    const qint64 elemPos = ds.device()->pos();   // first byte after the type/voice byte; see ENCORE_FORMAT.md §8.2 Ornament subtypes
    EncMeasureElem::read(ds);
    ds >> tipo;
    // Everything from +8 onward moved two bytes later at format 3.07; bodyShift folds the older
    // layout into this one read. See ENCORE_FORMAT.md §6.8 Ornament.
    ds.skipRawData(4 + bodyShift);
    ds >> xoffset;
    ds >> yoffset;
    ds.skipRawData(2);
    altMezuro = wordLowByte(ds);   // +16: v0xC2 spanning measure-count
    alMezuro = wordLowByte(ds);    // +18: v0xC4 spanning measure-count
    xoffset2 = wordLowByte(ds);
    ds.skipRawData(4);
    speguleco = wordLowByte(ds) & 3;
    noto = wordLowByte(ds);
    const int tempoEnd = 32 + bodyShift;
    if (static_cast<int>(size) >= tempoEnd) {
        tempo = wordLowByte(ds);
    } else {
        ds >> tempo;
    }
    // The text index has its own slot only in a long enough element; otherwise it shares the tempo
    // byte. The threshold moves with the body layout, so a pre-3.07 size-32 staff text still reaches
    // its own slot, a whole word there. See ENCORE_FORMAT.md §8.2 Ornament subtypes.
    if (static_cast<int>(size) >= tempoEnd + 2) {
        tind = wordLowByte(ds);
    } else if (static_cast<int>(size) >= tempoEnd + 1) {
        ds >> tind;
    } else {
        tind = tempo;
    }
    // The compact ornament keeps these three elsewhere. See ENCORE_FORMAT.md §6.8 Ornament.
    const qint64 elemStart = elemPos - 3;   // elemPos sits just past the type/voice byte, at +3
    if (tindOffset >= 0 && ornType() == EncOrnamentType::STAFFTEXT) {
        tind = byteAt(ds, elemStart + tindOffset);
    }
    if (yByteOffset >= 0) {
        yoffset = static_cast<qint8>(byteAt(ds, elemStart + yByteOffset));
    }
    if (measCountOffset >= 0) {
        alMezuro = byteAt(ds, elemStart + measCountOffset);
    }
    // No trailing skip: the element loop reseeks to the element end after read().
    return true;
}
} // namespace mu::iex::enc
