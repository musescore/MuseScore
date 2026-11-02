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

#ifndef MU_ENGRAVING_OCTAVEDOT_H
#define MU_ENGRAVING_OCTAVEDOT_H

#include "engravingitem.h"

namespace mu::engraving {
class Chord;

//---------------------------------------------------------
//    @@ OctaveDot
///     Graphic representation of a dot abover or under jianpu note.
//!
//!    parent:     Node
//!    x-origin:   Chord
//!    y-origin:   SStaff
//---------------------------------------------------------

class OctaveDot final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, OctaveDot)
    DECLARE_CLASSOF(ElementType::OCTAVE_DOT)

public:
    OctaveDot(DummyParentOr<Note>);
    ~OctaveDot() override = default;
    OctaveDot& operator=(const OctaveDot&) = delete;

    OctaveDot* clone() const override { return new OctaveDot(*this); }

    Note* note() const;

    struct LayoutData : public EngravingItem::LayoutData {
        double radius = 0.0;
        double len = 0.0;
        bool above = false;
    };
    DECLARE_LAYOUTDATA_METHODS(OctaveDot);
};
} // namespace mu::engraving
#endif
