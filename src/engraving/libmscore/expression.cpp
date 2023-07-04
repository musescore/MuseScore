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
#include "expression.h"

#include "chord.h"
#include "dynamic.h"
#include "dynamichairpingroup.h"
#include "note.h"
#include "segment.h"
#include "score.h"
#include "stafftext.h"

namespace mu::engraving {
static const ElementStyle expressionStyle {
    { Sid::expressionPlacement, Pid::PLACEMENT },
    { Sid::expressionMinDistance, Pid::MIN_DISTANCE },
    { Sid::snapToDynamics, Pid::SNAP_TO_DYNAMICS },
};

Expression::Expression(Segment* parent)
    : TextBase(ElementType::EXPRESSION, parent, TextStyleType::EXPRESSION, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&expressionStyle);
}

Expression::Expression(const Expression& expression)
    : TextBase(expression)
{
    _snapToDynamics = expression._snapToDynamics;
}

PropertyValue Expression::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::EXPRESSION;
    default:
        return TextBase::propertyDefault(id);
    }
}

double Expression::computeDynamicExpressionDistance() const
{
    if (!m_snappedDynamic) {
        return 0.0;
    }
    // We are essentially faking the kerning behaviour of dynamic VS expression text
    // There's no other way to do this because the dynamic is a different font.
    String dynamicTextString = m_snappedDynamic->xmlText();
    String f = String::fromStdString("<sym>dynamicForte</sym>");
    double distance = (dynamicTextString.endsWith(f) ? 0.2 : 0.5) * spatium();
    distance *= 0.5 * (m_snappedDynamic->dynamicsSize() + (size() / 10));
    return distance;
}

std::unique_ptr<ElementGroup> Expression::getDragGroup(std::function<bool(const EngravingItem*)> isDragged)
{
    if (auto g = DynamicExpressionDragGroup::detectFor(this, isDragged)) {
        return g;
    }
    return TextBase::getDragGroup(isDragged);
}

void Expression::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    TextBase::undoChangeProperty(id, v, ps);
    if (m_snappedDynamic) {
        if ((id == Pid::OFFSET && m_snappedDynamic->offset() != v.value<PointF>())
            || (id == Pid::PLACEMENT && m_snappedDynamic->placement() != v.value<PlacementV>())) {
            m_snappedDynamic->undoChangeProperty(id, v, ps);
        }
    }
}

bool Expression::acceptDrop(EditData& ed) const
{
    return ed.dropElement->type() == ElementType::DYNAMIC || TextBase::acceptDrop(ed);
}

EngravingItem* Expression::drop(EditData& ed)
{
    EngravingItem* item = ed.dropElement;
    if (item->isDynamic()) {
        if (m_snappedDynamic) {
            return m_snappedDynamic->drop(ed);
        }

        item->setTrack(track());
        item->setParent(segment());
        score()->undoAddElement(item);
        item->undoChangeProperty(Pid::PLACEMENT, placement(), PropertyFlags::UNSTYLED);
        return item;
    }

    return TextBase::drop(ed);
}

PropertyValue Expression::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SNAP_TO_DYNAMICS:
        return _snapToDynamics;
    default:
        return TextBase::getProperty(propertyId);
    }
}

bool Expression::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SNAP_TO_DYNAMICS:
        if (_snapToDynamics == false && v.toBool() == true) {
            resetProperty(Pid::OFFSET);
        }
        setSnapToDynamics(v.toBool());
        break;
    default:
        if (!TextBase::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

void Expression::mapPropertiesFromOldExpressions(StaffText* staffText)
{
    if (staffText->minDistance() != propertyDefault(Pid::MIN_DISTANCE).value<Spatium>()) {
        setMinDistance(staffText->minDistance());
        setPropertyFlags(Pid::MIN_DISTANCE, PropertyFlags::UNSTYLED);
    }
    if (staffText->placement() != propertyDefault(Pid::PLACEMENT).value<PlacementV>()) {
        setPlacement(staffText->placement());
        setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
    }
    if (staffText->offset() != propertyDefault(Pid::OFFSET).value<PointF>()) {
        setOffset(staffText->offset());
        setSnapToDynamics(false);
        setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
        setPropertyFlags(Pid::SNAP_TO_DYNAMICS, PropertyFlags::UNSTYLED);
    }
    if (staffText->frameType() != FrameType::NO_FRAME) {
        setFrameType(staffText->frameType());
        setFrameWidth(staffText->frameWidth());
        setFrameColor(staffText->frameColor());
        setBgColor(staffText->bgColor());
        setFrameRound(staffText->frameRound());
    }
}
} // namespace mu::engraving
