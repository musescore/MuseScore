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

#include "slide.h"

#include "chord.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   Slide
//---------------------------------------------------------

Slide::Slide(Chord* parent)
    : ChordLine(parent, ElementType::SLIDE)
{
}

Slide::Slide(const Slide& cl)
    : ChordLine(cl)
{
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Slide::layout()
{
    if (!modified) {
        qreal x2 = 0;
        qreal y2 = 0;
        qreal halfLength = _initialLength / 2;

        if (_chordLineType != ChordLineType::NOTYPE) {
            path = PainterPath();
            x2 = halfLength;
            y2 = halfLength;
            if (onTheRight()) {
                if (_chordLineType == ChordLineType::DOIT) {
                    y2 *= -1;
                }
                path.lineTo(x2, y2);
            } else {
                x2 *= -1;
                if (_chordLineType == ChordLineType::PLOP) {
                    y2 *= -1;
                }
                path.lineTo(x2, y2);
            }
        }
    }

    qreal sp = spatium();
    if (_note && explicitParent() && _chordLineType != ChordLineType::NOTYPE) {
        PointF p(_note->pos());
        qreal offset = 0.5;
        setPos(p.x() + offset * sp * (onTheRight() ? 4 : -1), p.y() + offset * sp * (topToBottom() ? 1 : -1));
    } else {
        setPos(0.0, 0.0);
    }
    RectF r = path.boundingRect();

    bbox().setRect(r.x() * sp, r.y() * sp, r.width() * sp, r.height() * sp);
}

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Slide::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;

    qreal sp = spatium();
    painter->scale(sp, sp);

    Pen pen(curColor(), .15, PenStyle::SolidLine);

    painter->setPen(pen);
    painter->setBrush(BrushStyle::NoBrush);
    painter->drawPath(path);

    painter->scale(1.0 / sp, 1.0 / sp);
}
}
