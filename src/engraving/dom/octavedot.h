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

#ifndef MU_ENGRAVING_OCTAVEDOT_H
#define MU_ENGRAVING_OCTAVEDOT_H

#include "engravingitem.h"

namespace mu::engraving {
class Chord;

//---------------------------------------------------------
//    @@ OctaveDot
///     Graphic representation of a dot abover or under jianpu note.
//!
//!    parent:     Chord
//!    x-origin:   Chord
//!    y-origin:   SStaff
//---------------------------------------------------------

class OctaveDot final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, OctaveDot)
    DECLARE_CLASSOF(ElementType::OCTAVE_DOT)

public:
    OctaveDot(EngravingItem*);
    ~OctaveDot();
    OctaveDot& operator=(const OctaveDot&) = delete;

    OctaveDot* clone() const override { return new OctaveDot(*this); }

    PointF pagePos() const override;        ///< position in page coordinates
    Note* note() const;

    double len() const { return m_len; }
    void setLen(double v) { m_len = v; }

    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;

    struct LayoutData : public EngravingItem::LayoutData {
        double radius = 0.0;
    };
    DECLARE_LAYOUTDATA_METHODS(OctaveDot);

private:

    double m_len = 0.0;
};
} // namespace mu::engraving
#endif
