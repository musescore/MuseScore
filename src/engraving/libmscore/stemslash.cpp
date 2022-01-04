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

#include "stemslash.h"
#include "score.h"
#include "chord.h"
#include "draw/pen.h"

using namespace mu;

namespace Ms {
StemSlash::StemSlash(Chord* parent)
    : EngravingItem(ElementType::STEM_SLASH, parent)
{
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StemSlash::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    qreal lw = score()->styleMM(Sid::stemWidth);
    painter->setPen(Pen(curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->drawLine(line);
}

//---------------------------------------------------------
//   setLine
//---------------------------------------------------------

void StemSlash::setLine(const LineF& l)
{
    line = l;
    qreal w = score()->styleMM(Sid::stemWidth) * .5;
    setbbox(RectF(line.p1(), line.p2()).normalized().adjusted(-w, -w, 2.0 * w, 2.0 * w));
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StemSlash::layout()
{
    Stem* stem = chord()->stem();
    qreal h2;
    qreal _spatium = spatium();
    qreal l = chord()->up() ? _spatium : -_spatium;
    PointF p(stem->flagPosition());
    qreal x = p.x() + _spatium * .1;
    qreal y = p.y();

    if (chord()->beam()) {
        y += l * .3;
        h2 = l * .8;
    } else {
        y += l * 1.2;
        h2 = l * .4;
    }
    qreal w  = chord()->upNote()->bboxRightPos() * .7;
    setLine(LineF(PointF(x + w, y - h2), PointF(x - w, y + h2)));
}
}
