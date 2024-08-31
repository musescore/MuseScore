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
    m_gap = 0.0;
}

Spacer::Spacer(const Spacer& s)
    : EngravingItem(s)
{
    m_gap        = s.m_gap;
    m_path        = s.m_path;
    m_spacerType = s.m_spacerType;
}

//---------------------------------------------------------
//   layout0
//---------------------------------------------------------

void Spacer::layout0()
{
    double _spatium = spatium();

    m_path    = PainterPath();
    double w = _spatium;
    double b = w * .5;
    double h = explicitParent() ? m_gap : std::min(m_gap.val(), spatium() * 4.0);      // limit length for palette

    switch (spacerType()) {
    case SpacerType::DOWN:
        m_path.lineTo(w, 0.0);
        m_path.moveTo(b, 0.0);
        m_path.lineTo(b, h);
        m_path.lineTo(0.0, h - b);
        m_path.moveTo(b, h);
        m_path.lineTo(w, h - b);
        break;
    case SpacerType::UP:
        m_path.moveTo(b, 0.0);
        m_path.lineTo(0.0, b);
        m_path.moveTo(b, 0.0);
        m_path.lineTo(w, b);
        m_path.moveTo(b, 0.0);
        m_path.lineTo(b, h);
        m_path.moveTo(0.0, h);
        m_path.lineTo(w, h);
        break;
    case SpacerType::FIXED:
        m_path.lineTo(w, 0.0);
        m_path.moveTo(b, 0.0);
        m_path.lineTo(b, h);
        m_path.moveTo(0.0, h);
        m_path.lineTo(w, h);
        break;
    }
    double lw = _spatium * 0.4;
    RectF bb(0, 0, w, h);
    bb.adjust(-lw, -lw, lw, lw);
    setbbox(bb);
}

//---------------------------------------------------------
//   setGap
//---------------------------------------------------------

void Spacer::setGap(Millimetre sp)
{
    m_gap = sp;
    layout0();
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Spacer::spatiumChanged(double ov, double nv)
{
    m_gap = (m_gap / ov) * nv;
    layout0();
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
        m_gap += s;
        break;
    case SpacerType::UP:
        m_gap -= s;
        break;
    }
    if (m_gap.val() < spatium() * 2.0) {
        m_gap = Millimetre(spatium() * 2);
    }
    layout0();
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
        p = PointF(_spatium * .5, m_gap);
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
        setGap(v.value<Millimetre>());
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    layout0();
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
        return Millimetre(0.0);
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
