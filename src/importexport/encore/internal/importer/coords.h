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

// Shared tick-conversion and coordinate-anchoring helpers used by the spanner/ornament resolvers.

#pragma once

#include <array>

#include "engraving/types/fraction.h"

#include "../parser/elem.h"

namespace mu::iex::enc {
// Visits the note, and optionally rest, elements of one staff in stream order, with the LINE-slot
// remap when given one. fn returns false to stop; all comparison logic stays in the caller.
template<typename Fn>
inline void forEachStaffNoteXoff(const EncMeasure& meas, int staffIdx, bool includeRests,
                                 const std::array<int, 256>* lineSlotByRawByte, Fn fn)
{
    for (const auto& elem : meas.elements) {
        const EncMeasureElem* em = elem.get();
        const bool isNote = em->type == static_cast<quint8>(EncElemType::NOTE);
        const bool isRest = em->type == static_cast<quint8>(EncElemType::REST);
        if (!isNote && !(includeRests && isRest)) {
            continue;
        }
        int slot = static_cast<int>(em->staffIdx);
        if (lineSlotByRawByte) {
            const int mapped = (*lineSlotByRawByte)[static_cast<unsigned char>(em->rawStaffByte())];
            slot = (mapped >= 0) ? mapped : static_cast<int>(em->staffIdx);
        }
        if (slot != staffIdx) {
            continue;
        }
        if (!fn(em, static_cast<int>(em->xoffset))) {
            break;
        }
    }
}

// Ticks per whole note for a measure. Re-derived from the time signature (not beatTicks, which is
// wrong for compound meters), falling back to the fixed 960 grid. Single source of truth for the
// conversion used when snapping spanners and ornaments to Encore element ticks.
int encWholeNoteTicks(const EncMeasure& measure);

// A glyph's column origin differs from a note's by a per-file constant, so raw values cannot be
// compared directly. See ENCORE_IMPORTER.md 8.1 and ENCORE_FORMAT.md 7.7.

// START anchor. Trusts the element's own tick; only when the glyph is drawn to the left of the note
// at that tick does it snap back to the latest preceding chord/rest whose xoffset is <= the glyph's.
mu::engraving::Fraction snapStartTickByXoffset(
    mu::engraving::Fraction defaultTick, const EncMeasure& encMeas, int staffIdx, int ornXoffset, mu::engraving::Fraction measTick);
}
