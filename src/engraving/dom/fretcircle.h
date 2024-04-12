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

#ifndef MU_ENGRAVING_FRETCIRCLE_H
#define MU_ENGRAVING_FRETCIRCLE_H

#include "engravingitem.h"

namespace mu::engraving {
class Chord;

class FretCircle final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, FretCircle)
    DECLARE_CLASSOF(ElementType::FRET_CIRCLE)

public:

    static constexpr double CIRCLE_WIDTH = 0.15;

    ~FretCircle();

    Chord* chord() const { return m_chord; }

    bool tabEllipseEnabled() const;
    RectF ellipseRect() const;

    struct LayoutData : public EngravingItem::LayoutData {
        RectF rect;
        double offsetFromUpNote = 0.0;
        double sideOffset = 0.0;
    };
    DECLARE_LAYOUTDATA_METHODS(FretCircle)

private:

    friend class Factory;

    FretCircle(Chord* ch = 0);
    FretCircle* clone() const override { return new FretCircle(*this); }

    Chord* m_chord = nullptr;
};
} // namespace mu::engraving
#endif
