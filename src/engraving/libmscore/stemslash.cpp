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

#include "beam.h"
#include "chord.h"
#include "hook.h"
#include "note.h"
#include "score.h"
#include "stem.h"

#include "draw/types/pen.h"

using namespace mu;

namespace mu::engraving {
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
    painter->setPen(Pen(curColor(), _width, PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->drawLine(line);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StemSlash::layout()
{
    if (!chord() || !chord()->stem()) {
        return;
    }
    Chord* c = chord();
    Stem* stem = c->stem();
    Hook* hook = c->hook();
    Beam* beam = c->beam();

    static constexpr double heightReduction = 0.66;
    static constexpr double angleIncrease = 1.2;
    static constexpr double lengthIncrease = 1.1;

    double up = c->up() ? -1 : 1;
    double stemTipY = c->up() ? stem->bbox().translated(stem->pos()).top() : stem->bbox().translated(stem->pos()).bottom();
    double leftHang = score()->noteHeadWidth() * score()->styleD(Sid::graceNoteMag) / 2;
    double angle = score()->styleD(Sid::stemSlashAngle) * M_PI / 180; // converting to radians
    bool straight = score()->styleB(Sid::useStraightNoteFlags);
    double graceNoteMag = score()->styleD(Sid::graceNoteMag);

    double startX = stem->bbox().translated(stem->pos()).right() - leftHang;

    double startY;
    if (straight || beam) {
        startY = stemTipY - up * graceNoteMag * score()->styleMM(Sid::stemSlashPosition) * heightReduction;
    } else {
        startY = stemTipY - up * graceNoteMag * score()->styleMM(Sid::stemSlashPosition);
    }

    double endX = 0;
    double endY = 0;

    if (hook) {
        auto musicFont = score()->styleSt(Sid::MusicalSymbolFont);
        // HACK: adjust slash angle for fonts with "fat" hooks. In future, we must use smufl cutOut
        if (c->beams() >= 2 && !straight
            && (musicFont == "Bravura" || musicFont == "Finale Maestro" || musicFont == "Gonville")) {
            angle *= angleIncrease;
        }
        endX = hook->bbox().translated(hook->pos()).right(); // always ends at the right bbox margin of the hook
        endY = startY + up * (endX - startX) * tan(angle);
    }
    if (beam) {
        PointF p1 = beam->startAnchor();
        PointF p2 = beam->endAnchor();
        double beamAngle = p2.x() > p1.x() ? atan((p2.y() - p1.y()) / (p2.x() - p1.x())) : 0;
        angle += up * beamAngle / 2;
        double length = 2 * spatium();
        bool obtuseAngle = (c->up() && beamAngle < 0) || (!c->up() && beamAngle > 0);
        if (obtuseAngle) {
            length *= lengthIncrease; // needs to be slightly longer
        }
        endX = startX + length * cos(angle);
        endY = startY + up * length * sin(angle);
    }

    line = LineF(PointF(startX, startY), PointF(endX, endY));
    _width = score()->styleMM(Sid::stemSlashThickness) * graceNoteMag;
    setbbox(RectF(line.p1(), line.p2()).normalized().adjusted(-_width / 2, -_width / 2, _width, _width));
}
}
