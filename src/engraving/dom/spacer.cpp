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

#include "../editing/editdata.h"
#include "../editing/elementeditdata.h"

#include "measure.h"
#include "score.h"
#include "system.h"

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
    m_gap = 0.0_sp;
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
}

//---------------------------------------------------------
//   startDragGrip
//---------------------------------------------------------

void Spacer::startDragGrip(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    eed->pushProperty(Pid::SPACE);
}

//---------------------------------------------------------
//   dragGrip
//---------------------------------------------------------

void Spacer::dragGrip(EditData& ed)
{
    double s = ed.delta.y();

    switch (spacerType()) {
    case SpacerType::DOWN:
    case SpacerType::FIXED:
        m_gap += Spatium::fromAbsolute(s, spatium());
        break;
    case SpacerType::UP:
        m_gap -= Spatium::fromAbsolute(s, spatium());
        break;
    }
    m_gap = std::max(m_gap, 2.0_sp);
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
        return 0.0_sp;
    default:
        return EngravingItem::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Spacer::triggerLayout() const
{
    if (!explicitParent()) {
        return;
    }

    Measure* m = measure();
    if (!m) {
        EngravingItem::triggerLayout();
        return;
    }

    Score* s = score();
    System* system = m->system();
    if (!s || !system || s->nstaves() == 0) {
        m->triggerLayout();
        return;
    }

    // Most spacer edits can stay local to the measure and are much cheaper.
    // Escalate only for down/fixed spacers on the last non-vbox system of the page,
    // where edits can repaginate and affect page-end spacing.
    if (spacerType() == SpacerType::UP) {
        m->triggerLayout();
        return;
    }

    System* nextSystem = m->nextNonVBoxSystem();
    if (nextSystem && nextSystem->page() == system->page()) {
        m->triggerLayout();
        return;
    }

    // For the last notation system on a page, include the next system boundary
    // so page-end spacing can be recomputed without relaying out the whole score.
    Measure* firstMeasure = system->firstMeasure();
    if (!firstMeasure) {
        m->triggerLayout();
        return;
    }

    Fraction startTick = firstMeasure->tick();
    Fraction endTick = s->endTick();
    if (nextSystem) {
        if (Measure* nextFirstMeasure = nextSystem->firstMeasure()) {
            endTick = nextFirstMeasure->tick();
        }
    }

    s->setLayout(startTick, endTick, 0, s->nstaves() - 1, this);
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
