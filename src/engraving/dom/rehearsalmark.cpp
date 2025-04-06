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

#include "rehearsalmark.h"

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
//   isEditAllowed
//---------------------------------------------------------

bool RehearsalMark::isEditAllowed(EditData& ed) const
{
    bool ctrlPressed  = ed.modifiers & ControlModifier;
    bool shiftPressed = ed.modifiers & ShiftModifier;
    bool altPressed = ed.modifiers & AltModifier;
    if (altPressed && !ctrlPressed && !shiftPressed && (ed.key == Key_Left || ed.key == Key_Right)) {
        return false;
    }

    return TextBase::isEditAllowed(ed);
}

void RehearsalMark::editDrag(EditData& ed)
{
    return EngravingItem::editDrag(ed);
}

bool RehearsalMark::moveSegment(const EditData& ed)
{
    assert(hasParentSegment());

    bool forward = ed.key == Key_Right;

    Segment* curSeg = toSegment(parent());
    IF_ASSERT_FAILED(curSeg) {
        return false;
    }

    Measure* newMeas = forward ? curSeg->measure()->nextMeasureMM() : curSeg->measure()->prevMeasureMM();
    if (!newMeas) {
        return false;
    }

    Segment* newSeg = newMeas->first(SegmentType::ChordRest);
    IF_ASSERT_FAILED(newSeg) {
        return false;
    }

    undoMoveSegment(newSeg, newSeg->tick() - curSeg->tick());

    return true;
}

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void RehearsalMark::setType(RehearsalMark::Type type)
{
    if (type == m_type) {
        return;
    }
    m_type = type;
    applyTypeStyle();
}

void RehearsalMark::applyTypeStyle()
{
    const auto& elemStyleMap = (m_type == Type::Main ? mainRehearsalMarkStyle : additionalRehearsalMarkStyle);
    for (const auto& elem : elemStyleMap) {
        if (propertyFlags(elem.pid) == PropertyFlags::STYLED) {
            setProperty(elem.pid, style().styleV(elem.sid));
        }
    }
}

void RehearsalMark::styleChanged()
{
    TextBase::styleChanged();
    applyTypeStyle();
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
        return style().styleV(Sid::rehearsalMarkPlacement);
    default:
        return TextBase::propertyDefault(id);
    }
}
} // namespace mu::engraving
