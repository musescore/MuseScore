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

namespace mu::iex::enc {
bool EncOrnament::read(QDataStream& ds)
{
    const qint64 elemPos = ds.device()->pos();   // first byte after the type/voice byte; see ENCORE_FORMAT.md §Ornament subtypes
    EncMeasureElem::read(ds);
    ds >> tipo;
    ds.skipRawData(4);
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
    // v0xC2 size-32: tind overlaps tempo at byte 30. See ENCORE_FORMAT.md §Ornament subtypes.
    if (static_cast<int>(size) >= 33) {
        ds.skipRawData(1);
        ds >> tind;
    } else {
        tind = tempo;
    }
    // Compact v0xA6 STAFFTEXT stores tind at a fixed offset from the type/voice byte instead;
    // scope the seek to STAFFTEXT and to the device so an unrelated ornament near EOF cannot desync.
    if (tindOffset >= 0 && ornType() == EncOrnamentType::STAFFTEXT) {
        const qint64 tindPos = elemPos - 1 + tindOffset;
        if (tindPos >= 0 && tindPos < ds.device()->size()) {
            ds.device()->seek(tindPos);
            ds >> tind;
        }
    }
    // Same story for the placement y of compact v0xA6 STAFFTEXT: fixed offset, so the inline read
    // above landed on an unrelated byte. Same scoping as tind.
    if (yoffOffset >= 0 && ornType() == EncOrnamentType::STAFFTEXT) {
        const qint64 yoffPos = elemPos - 1 + yoffOffset;
        if (yoffPos >= 0 && yoffPos + 1 < ds.device()->size()) {
            ds.device()->seek(yoffPos);
            ds >> yoffset;
        }
    }
    // No trailing skip: the element loop reseeks to the element end after read().
    return true;
}
} // namespace mu::iex::enc
