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

#pragma once

#include "engravingitem.h"

namespace mu::engraving {
class Chord;

//---------------------------------------------------------
//    @@ JianpuOctaveDot
///     Graphic representation of a dot abover or under jianpu note.
//!
//!    parent:     Node
//!    x-origin:   Chord
//!    y-origin:   SStaff
//---------------------------------------------------------

class JianpuOctaveDot final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, JianpuOctaveDot)
    DECLARE_CLASSOF(ElementType::JIANPU_OCTAVE_DOT)

public:
    JianpuOctaveDot(DummyParentOr<Note>);
    ~JianpuOctaveDot() override = default;
    JianpuOctaveDot& operator=(const JianpuOctaveDot&) = delete;

    JianpuOctaveDot* clone() const override { return new JianpuOctaveDot(*this); }

    Note* note() const;

    struct LayoutData : public EngravingItem::LayoutData {
        double radius = 0.0;
        double len = 0.0;
        bool above = false;
    };
    DECLARE_LAYOUTDATA_METHODS(JianpuOctaveDot);
};
} // namespace mu::engraving
