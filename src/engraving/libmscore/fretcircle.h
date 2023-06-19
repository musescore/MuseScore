/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef __FRETCIRCLE_H__
#define __FRETCIRCLE_H__

#include "engravingitem.h"

namespace mu::engraving {
class Chord;

class FretCircle final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, FretCircle)

public:

    static constexpr double CIRCLE_WIDTH = 0.15;

    ~FretCircle();

    void draw(mu::draw::Painter*) const override;

    double offsetFromUpNote() const { return m_offsetFromUpNote; }
    void setOffsetFromUpNote(double o) { m_offsetFromUpNote = o; }
    double sideOffset() const { return m_sideOffset; }
    void setSideOffset(double o) { m_sideOffset = o; }

    const mu::RectF& rect() const { return m_rect; }
    void setRect(const mu::RectF& r) { m_rect = r; }

    Chord* chord() const { return m_chord; }

    bool tabEllipseEnabled() const;
    RectF ellipseRect() const;

private:

    friend class Factory;

    FretCircle(Chord* ch = 0);
    FretCircle* clone() const override { return new FretCircle(*this); }

    Chord* m_chord = nullptr;
    mu::RectF m_rect;

    double m_offsetFromUpNote = 0;
    double m_sideOffset = 0;
};
} // namespace mu::engraving
#endif
