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
#include "stem.h"

#include <cmath>

#include "chord.h"
#include "hook.h"

#include "tremolosinglechord.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

static const ElementStyle stemStyle {
    { Sid::stemWidth, Pid::LINE_WIDTH }
};

Stem::Stem(Chord* parent)
    : EngravingItem(ElementType::STEM, parent)
{
    initElementStyle(&stemStyle);
    resetProperty(Pid::USER_LEN);
}

EngravingItem* Stem::elementBase() const
{
    return parentItem();
}

staff_idx_t Stem::vStaffIdx() const
{
    return staffIdx() + chord()->staffMove();
}

bool Stem::up() const
{
    return chord() ? chord()->up() : true;
}

void Stem::setBaseLength(Spatium baseLength)
{
    m_baseLength = Spatium(std::abs(baseLength.val()));
}

double Stem::lineWidthMag() const
{
    return absoluteFromSpatium(m_lineWidth) * chord()->intrinsicMag();
}

void Stem::spatiumChanged(double oldValue, double newValue)
{
    m_userLength = (m_userLength / oldValue) * newValue;
    m_baseLength = (m_baseLength / oldValue) * newValue;
    m_lineWidth = (m_lineWidth / oldValue) * newValue;
    renderer()->layoutItem(this);
}

//! In chord coordinates
PointF Stem::flagPosition() const
{
    return pos() + PointF(ldata()->bbox().left(), up() ? -length() : length());
}

std::vector<PointF> Stem::gripsPositions(const EditData&) const
{
    return { pagePos() + ldata()->line.p2() };
}

void Stem::startEdit(EditData& ed)
{
    EngravingItem::startEdit(ed);
    ElementEditDataPtr eed = ed.getData(this);
    eed->pushProperty(Pid::USER_LEN);
}

void Stem::startEditDrag(EditData& ed)
{
    EngravingItem::startEditDrag(ed);
    ElementEditDataPtr eed = ed.getData(this);
    eed->pushProperty(Pid::USER_LEN);
}

void Stem::editDrag(EditData& ed)
{
    double yDelta = up() ? -ed.delta.y() : ed.delta.y();
    m_userLength += Spatium::fromMM(yDelta, spatium());
    renderer()->layoutItem(this);
    Chord* c = chord();
    if (c->hook()) {
        c->hook()->move(PointF(0.0, ed.delta.y()));
    }
}

void Stem::reset()
{
    undoChangeProperty(Pid::USER_LEN, Spatium(0.0));
    EngravingItem::reset();
}

bool Stem::acceptDrop(EditData& data) const
{
    const EngravingItem* e = data.dropElement;
    switch (e->type()) {
    case ElementType::TREMOLO_SINGLECHORD:
        return item_cast<const TremoloSingleChord*>(e)->tremoloType() <= TremoloType::R64;
    default:
        break;
    }

    return false;
}

EngravingItem* Stem::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    Chord* ch  = chord();

    switch (e->type()) {
    case ElementType::TREMOLO_SINGLECHORD:
        item_cast<TremoloSingleChord*>(e)->setParent(ch);
        undoAddElement(e);
        return e;
    default:
        delete e;
        break;
    }
    return 0;
}

PropertyValue Stem::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        return lineWidth();
    case Pid::USER_LEN:
        return userLength();
    case Pid::STEM_DIRECTION:
        return PropertyValue::fromValue<DirectionV>(chord()->stemDirection());
    case Pid::APPEARANCE_LINKED_TO_MASTER:
        return chord() ? chord()->isPropertyLinkedToMaster(Pid::STEM_DIRECTION) : true;
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

bool Stem::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        setLineWidth(v.value<Spatium>());
        break;
    case Pid::USER_LEN:
        setUserLength(v.value<Spatium>());
        break;
    case Pid::STEM_DIRECTION:
        chord()->setStemDirection(v.value<DirectionV>());
        break;
    case Pid::APPEARANCE_LINKED_TO_MASTER:
        if (chord() && v.toBool() == true) {
            chord()->relinkPropertyToMaster(Pid::STEM_DIRECTION);
            break;
        }
    // fall through
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

PropertyValue Stem::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::USER_LEN:
        return 0.0;
    case Pid::STEM_DIRECTION:
        return PropertyValue::fromValue<DirectionV>(DirectionV::AUTO);
    default:
        return EngravingItem::propertyDefault(id);
    }
}
