/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 */

#include "footnote.h"

#include "score.h"
#include "segment.h"

namespace mu::engraving {
static const ElementStyle footnoteStyle {
    { Sid::footnotePlacement, Pid::PLACEMENT },
    { Sid::footnoteMinDistance, Pid::MIN_DISTANCE },
};

Footnote::Footnote(Segment* parent)
    : TextBase(ElementType::FOOTNOTE, parent, TextStyleType::FOOTNOTE, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&footnoteStyle);
}

Footnote::Footnote(const Footnote& footnote)
    : TextBase(footnote)
{
}

bool Footnote::isEditAllowed(EditData& ed) const
{
    return TextBase::isEditAllowed(ed);
}

PropertyValue Footnote::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::FOOTNOTE;
    default:
        return TextBase::propertyDefault(id);
    }
}

std::unique_ptr<ElementGroup> Footnote::getDragGroup(std::function<bool(const EngravingItem*)> isDragged)
{
    return TextBase::getDragGroup(isDragged);
}

bool Footnote::acceptDrop(EditData& ed) const
{
    return TextBase::acceptDrop(ed);
}

EngravingItem* Footnote::drop(EditData& ed)
{
    return TextBase::drop(ed);
}

PropertyValue Footnote::getProperty(Pid propertyId) const
{
    return TextBase::getProperty(propertyId);
}

bool Footnote::setProperty(Pid propertyId, const PropertyValue& v)
{
    if (!TextBase::setProperty(propertyId, v)) {
        return false;
    }
    triggerLayout();
    return true;
}

void Footnote::reset()
{
    TextBase::reset();
}
} // namespace mu::engraving
