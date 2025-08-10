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

#include "spacer.h"

#include "draw/types/pen.h"

#include "measure.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

Spacer::Spacer(Measure* parent)
    : EngravingItem(ElementType::SPACER, parent)
{
    m_spacerType = SpacerType::UP;
    m_gap = Spatium(0.0);
    m_z = -10; // Ensure behind notation
}

Spacer::Spacer(const Spacer& s)
    : EngravingItem(s)
{
    m_gap        = s.m_gap;
    m_spacerType = s.m_spacerType;
}

//---------------------------------------------------------
//   setGap
//---------------------------------------------------------

void Spacer::setGap(Spatium sp)
{
    m_gap = sp;
    renderer()->layoutItem(this);
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void Spacer::startEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    eed->pushProperty(Pid::SPACE);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Spacer::editDrag(EditData& ed)
{
    double s = ed.delta.y();

    switch (spacerType()) {
    case SpacerType::DOWN:
    case SpacerType::FIXED:
        m_gap += Spatium::fromMM(s, spatium());
        break;
    case SpacerType::UP:
        m_gap -= Spatium::fromMM(s, spatium());
        break;
    }
    m_gap = std::max(m_gap, Spatium(2.0));
    renderer()->layoutItem(this);
    triggerLayout();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Spacer::gripsPositions(const EditData&) const
{
    double _spatium = spatium();
    PointF p;
    switch (spacerType()) {
    case SpacerType::DOWN:
    case SpacerType::FIXED:
        p = PointF(_spatium * .5, absoluteGap());
        break;
    case SpacerType::UP:
        p = PointF(_spatium * .5, 0.0);
        break;
    }
    return { pagePos() + p };
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Spacer::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SPACE:
        return gap();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Spacer::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SPACE:
        setGap(v.value<Spatium>());
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    renderer()->layoutItem(this);
    triggerLayout();
    setGenerated(false);
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Spacer::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SPACE:
        return Spatium(0.0);
    default:
        return EngravingItem::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Spacer::subtypeUserName() const
{
    switch (m_spacerType) {
    case SpacerType::UP:
        return TranslatableString("engraving/spacertype", "Staff spacer up");
    case SpacerType::DOWN:
        return TranslatableString("engraving/spacertype", "Staff spacer down");
    case SpacerType::FIXED:
        return TranslatableString("engraving/spacertype", "Staff spacer fixed down");
    }
    return TranslatableString::untranslatable("Unknown spacer");
}
}
