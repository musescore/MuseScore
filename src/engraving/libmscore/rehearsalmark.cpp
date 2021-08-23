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

#include "score.h"
#include "rehearsalmark.h"
#include "measure.h"
#include "system.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   rehearsalMarkStyle
//---------------------------------------------------------

static const ElementStyle rehearsalMarkStyle {
    { Sid::rehearsalMarkPlacement, Pid::PLACEMENT },
    { Sid::rehearsalMarkMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   RehearsalMark
//---------------------------------------------------------

RehearsalMark::RehearsalMark(Score* s)
    : TextBase(s, Tid::REHEARSAL_MARK)
{
    initElementStyle(&rehearsalMarkStyle);
    setSystemFlag(true);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void RehearsalMark::layout()
{
    TextBase::layout();

    Segment* s = segment();
    if (s) {
        if (s->rtick().isZero()) {
            // first CR of measure, alignment is hcenter or right (the usual cases)
            // align with barline, point just after header, or start of measure depending on context

            Measure* m = s->measure();
            Segment* header = s->prev();        // possibly just a start repeat
            qreal measureX = -s->x();
            Segment* repeat = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
            qreal barlineX = repeat ? repeat->x() - s->x() : measureX;
            System* sys = m->system();
            bool systemFirst = (sys && m->isFirstInSystem());

            if (!header || repeat || !systemFirst) {
                // no header, or header with repeat, or header mid-system - align with barline
                rxpos() = barlineX;
            } else {
                // header at start of system
                // align to a point just after the header
                Element* e = header->element(track());
                qreal w = e ? e->width() : header->width();
                rxpos() = header->x() + w - s->x();

                // special case for right aligned rehearsal marks at start of system
                // left align with start of measure if that is further left
                if (align() & Align::RIGHT) {
                    rxpos() = qMin(rpos().x(), measureX + width());
                }
            }
        }
        autoplaceSegmentElement();
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant RehearsalMark::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SUB_STYLE:
        return int(Tid::REHEARSAL_MARK);
    case Pid::PLACEMENT:
        return score()->styleV(Sid::rehearsalMarkPlacement);
    default:
        return TextBase::propertyDefault(id);
    }
}
} // namespace Ms
