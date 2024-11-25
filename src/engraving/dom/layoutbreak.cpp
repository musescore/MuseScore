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

#include "log.h"

#include "types/typesconv.h"

#include "layoutbreak.h"
#include "measurebase.h"
#include "score.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   sectionBreakStyle
//---------------------------------------------------------

static const ElementStyle sectionBreakStyle {
    { Sid::sectionPause, Pid::PAUSE }
};

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

LayoutBreak::LayoutBreak(MeasureBase* parent)
    : EngravingItem(ElementType::LAYOUT_BREAK, parent, ElementFlag::SYSTEM | ElementFlag::HAS_TAG)
{
    m_pause = 0.;
    m_startWithLongNames = false;
    m_startWithMeasureOne = false;
    m_firstSystemIndentation = false;
    m_layoutBreakType = LayoutBreakType(propertyDefault(Pid::LAYOUT_BREAK).toInt());

    initElementStyle(&sectionBreakStyle);

    resetProperty(Pid::PAUSE);
    resetProperty(Pid::START_WITH_LONG_NAMES);
    resetProperty(Pid::START_WITH_MEASURE_ONE);
    resetProperty(Pid::FIRST_SYSTEM_INDENTATION);
}

LayoutBreak::LayoutBreak(const LayoutBreak& lb)
    : EngravingItem(lb)
{
    m_layoutBreakType        = lb.m_layoutBreakType;
    m_pause                  = lb.m_pause;
    m_startWithLongNames     = lb.m_startWithLongNames;
    m_startWithMeasureOne    = lb.m_startWithMeasureOne;
    m_firstSystemIndentation = lb.m_firstSystemIndentation;
}

void LayoutBreak::setParent(MeasureBase* parent)
{
    EngravingItem::setParent(parent);
}

char16_t LayoutBreak::iconCode() const
{
    switch (m_layoutBreakType) {
    case LayoutBreakType::LINE:
        return 0xF483;
    case LayoutBreakType::PAGE:
        return 0xF484;
    case LayoutBreakType::SECTION:
        return 0xF485;
    case LayoutBreakType::NOBREAK:
        return 0xF486;
    default:
        return 0x000;
    }
}

//---------------------------------------------------------
//   setLayoutBreakType
//---------------------------------------------------------

void LayoutBreak::setLayoutBreakType(LayoutBreakType val)
{
    m_layoutBreakType = val;
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool LayoutBreak::acceptDrop(EditData& data) const
{
    return data.dropElement->type() == ElementType::LAYOUT_BREAK
           && toLayoutBreak(data.dropElement)->layoutBreakType() != layoutBreakType();
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* LayoutBreak::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    score()->undoChangeElement(this, e);
    return e;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue LayoutBreak::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LAYOUT_BREAK:
        return m_layoutBreakType;
    case Pid::PAUSE:
        return m_pause;
    case Pid::START_WITH_LONG_NAMES:
        return m_startWithLongNames;
    case Pid::START_WITH_MEASURE_ONE:
        return m_startWithMeasureOne;
    case Pid::FIRST_SYSTEM_INDENTATION:
        return m_firstSystemIndentation;
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool LayoutBreak::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::LAYOUT_BREAK:
        setLayoutBreakType(v.value<LayoutBreakType>());
        break;
    case Pid::PAUSE:
        setPause(v.toDouble());
        score()->setUpTempoMapLater();
        break;
    case Pid::START_WITH_LONG_NAMES:
        setStartWithLongNames(v.toBool());
        break;
    case Pid::START_WITH_MEASURE_ONE:
        setStartWithMeasureOne(v.toBool());
        break;
    case Pid::FIRST_SYSTEM_INDENTATION:
        setFirstSystemIndentation(v.toBool());
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }

    if (propertyId == Pid::START_WITH_MEASURE_ONE) {
        triggerLayoutToEnd();
    } else {
        triggerLayout();
        if (explicitParent() && measure()->next()) {
            measure()->next()->triggerLayout();
        }
    }

    setGenerated(false);
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue LayoutBreak::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::LAYOUT_BREAK:
        return PropertyValue();           // LAYOUT_BREAK_LINE;
    case Pid::PAUSE:
        return style().styleD(Sid::sectionPause);
    case Pid::START_WITH_LONG_NAMES:
        return true;
    case Pid::START_WITH_MEASURE_ONE:
        return true;
    case Pid::FIRST_SYSTEM_INDENTATION:
        return true;
    default:
        return EngravingItem::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString LayoutBreak::subtypeUserName() const
{
    return TConv::userName(layoutBreakType());
}

void LayoutBreak::added()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMapLater();
}

void LayoutBreak::removed()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMapLater();
}

Font LayoutBreak::font() const
{
    Font font(configuration()->iconsFontFamily(), Font::Type::Icon);
    static constexpr double STANDARD_POINT_SIZE = 12.0;
    double scaling = spatium() / SPATIUM20;
    font.setPointSizeF(STANDARD_POINT_SIZE * scaling);
    return font;
}
}
