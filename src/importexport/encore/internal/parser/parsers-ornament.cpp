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
bool EncOrnament::read(QDataStream& ds)
{
    const qint64 elemPos = ds.device()->pos();   // first byte after the type/voice byte; see ENCORE_FORMAT.md §8.2 Ornament subtypes
    EncMeasureElem::read(ds);
    ds >> tipo;
    // Everything from +8 onward moved two bytes later at format 3.07; bodyShift folds the older
    // layout into this one read. See ENCORE_FORMAT.md §6.8 Ornament.
    ds.skipRawData(4 + bodyShift);
    ds >> xoffset;
    ds.skipRawData(1);
    ds >> yoffset;
    ds.skipRawData(2);
    ds >> altMezuro;   // +16: v0xC2 spanning measure-count
    ds.skipRawData(1);
    ds >> alMezuro;    // +18: v0xC4 spanning measure-count
    ds.skipRawData(1);
    ds >> xoffset2;
    ds.skipRawData(5);
    ds >> speguleco;
    speguleco &= 3;
    ds.skipRawData(1);
    ds >> noto;
    ds.skipRawData(1);
    ds >> tempo;
    // The text index has its own slot only in a long enough element; otherwise it shares the tempo
    // byte. The threshold moves with the body layout, so a pre-3.07 size-32 staff text still reaches
    // its own slot. See ENCORE_FORMAT.md §8.2 Ornament subtypes.
    if (static_cast<int>(size) >= 33 + bodyShift) {
        ds.skipRawData(1);
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
