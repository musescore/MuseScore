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

static const ElementStyle mainRehearsalMarkStyle {
    { Sid::rehearsalMarkFrameType, Pid::FRAME_TYPE },
    { Sid::rehearsalMarkFontSize, Pid::FONT_SIZE },
    { Sid::rehearsalMarkAlign, Pid::ALIGN },
};

static const ElementStyle additionalRehearsalMarkStyle {
    { Sid::tempoFrameType, Pid::FRAME_TYPE },
    { Sid::tempoFontSize, Pid::FONT_SIZE },
    { Sid::tempoAlign, Pid::ALIGN },
};

//---------------------------------------------------------
//   RehearsalMark
//---------------------------------------------------------

RehearsalMark::RehearsalMark(Segment* parent)
    : TextBase(ElementType::REHEARSAL_MARK, parent, TextStyleType::REHEARSAL_MARK, ElementFlag::ON_STAFF)
{
    initElementStyle(&rehearsalMarkStyle);
    setSystemFlag(true);
}

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void RehearsalMark::setType(RehearsalMark::Type type)
{
    if (type == _type) {
        return;
    }
    _type = type;

    const auto& elemStyleMap = (_type == Type::Main ? mainRehearsalMarkStyle : additionalRehearsalMarkStyle);
    for (const auto& elem : elemStyleMap) {
        setProperty(elem.pid, score()->styleV(elem.sid));
    }
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
                EngravingItem* e = header->element(track());
                qreal w = e ? e->width() : header->width();
                rxpos() = header->x() + w - s->x();

                // special case for right aligned rehearsal marks at start of system
                // left align with start of measure if that is further left
                if (align() == AlignH::RIGHT) {
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

engraving::PropertyValue RehearsalMark::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::REHEARSAL_MARK;
    case Pid::PLACEMENT:
        return score()->styleV(Sid::rehearsalMarkPlacement);
    default:
        return TextBase::propertyDefault(id);
    }
}
} // namespace Ms
