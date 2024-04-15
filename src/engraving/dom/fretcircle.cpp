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

#include "fretcircle.h"

#include "chord.h"
#include "note.h"
#include "staff.h"
#include "tie.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   FretCircle
//---------------------------------------------------------

FretCircle::FretCircle(Chord* ch)
    : EngravingItem(ElementType::FRET_CIRCLE, ch, ElementFlag::NOT_SELECTABLE), m_chord(ch)
{
}

FretCircle::~FretCircle()
{
}

//---------------------------------------------------------
//   tabEllipseEnabled
//---------------------------------------------------------

bool FretCircle::tabEllipseEnabled() const
{
    Tie* tieBack = m_chord->upNote()->tieBack();
    bool tiedAboveBarline = (tieBack && (m_chord->measure() != tieBack->startNote()->chord()->measure()));
    return !tiedAboveBarline && staff()->staffType(tick())->isCommonTabStaff() && (m_chord->ticks() >= Fraction(1, 2));
}

//---------------------------------------------------------
//   ellipseRect
//---------------------------------------------------------

RectF FretCircle::ellipseRect() const
{
    auto notes = m_chord->notes();
    Note* up = m_chord->upNote();
    Note* down = m_chord->downNote();
    RectF elRect = up->ldata()->bbox();

    double maxWidth = 0;
    double initialX = up->x();
    double initialWidth = up->width();

    for (const Note* note : notes) {
        elRect |= note->ldata()->bbox();
        maxWidth = std::max(note->width(), maxWidth);
    }

    elRect.setX(initialX - (maxWidth - initialWidth) * 0.5);

    /// handling offset
    constexpr double stretchFactorX = 0.2;
    constexpr double stretchFactorY = 1.0;
    double offsetX = spatium() * (1 + (notes.size() - 1) * stretchFactorX);
    double offsetY = spatium() * stretchFactorY;

    elRect.setWidth(elRect.width() + offsetX);

    if (notes.size() == 1) {
        // making a circle
        double deltaH = elRect.width() - elRect.height();
        elRect.setHeight(elRect.width());
        elRect.translate(0, -deltaH * 0.5);
    } else {
        // ellipse
        elRect.setHeight(elRect.height() + down->pos().y() - up->pos().y() + offsetY);
        elRect.translate(0, -offsetY * 0.5);
    }

    elRect.translate(-offsetX * 0.5, up->pos().y());
    return elRect;
}
}
