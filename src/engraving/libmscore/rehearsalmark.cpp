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

#include "rehearsalmark.h"

#include "layout/tlayout.h"

#include "measure.h"
#include "score.h"
#include "system.h"

using namespace mu;

namespace mu::engraving {
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
    applyTypeStyle();
}

void RehearsalMark::applyTypeStyle()
{
    const auto& elemStyleMap = (_type == Type::Main ? mainRehearsalMarkStyle : additionalRehearsalMarkStyle);
    for (const auto& elem : elemStyleMap) {
        setProperty(elem.pid, score()->styleV(elem.sid));
    }
}

void RehearsalMark::styleChanged()
{
    TextBase::styleChanged();
    applyTypeStyle();
}

//---------------------------------------------------------
<<<<<<< HEAD
<<<<<<< HEAD
=======
//   layout
//---------------------------------------------------------

void RehearsalMark::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
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
} // namespace mu::engraving
