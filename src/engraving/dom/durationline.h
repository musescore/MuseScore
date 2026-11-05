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

#ifndef MU_ENGRAVING_DURATIONLINE_H
#define MU_ENGRAVING_DURATIONLINE_H

#include "engravingitem.h"

namespace mu::engraving {
//---------------------------------------------------------
//    @@ DurationLine
///     Graphic representation of a duration line.
//!
//!    parent:     ChordRest
//!    x-origin:   Chord
//!    y-origin:   SStaff
//---------------------------------------------------------

class DurationLine final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, DurationLine)
    DECLARE_CLASSOF(ElementType::DURATION_LINE)

public:
    DurationLine(DummyParentOr<ChordRest>);
    ~DurationLine() override = default;
    DurationLine& operator=(const DurationLine&) = delete;

    DurationLine* clone() const override { return new DurationLine(*this); }

    PointF pagePos() const override;        ///< position in page coordinates
    ChordRest* chordRest() const;
    double mag() const;

    struct LayoutData : public EngravingItem::LayoutData {
        double lineWidth = 0.0;
        double len = 0.0;
    };
    DECLARE_LAYOUTDATA_METHODS(DurationLine);
};
} // namespace mu::engraving
#endif
