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

#include "instrumentname.h"

#include "measure.h"
#include "part.h"
#include "score.h"
#include "staff.h"
#include "style/style.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   longInstrumentStyle
//---------------------------------------------------------

static const ElementStyle longInstrumentStyle {
};

//---------------------------------------------------------
//   shortInstrumentStyle
//---------------------------------------------------------

static const ElementStyle shortInstrumentStyle {
};

//---------------------------------------------------------
//   InstrumentName
//---------------------------------------------------------

InstrumentName::InstrumentName(System* s)
    : TextBase(ElementType::INSTRUMENT_NAME, s, TextStyleType::INSTRUMENT_LONG, ElementFlag::NOTHING)
{
    setFlag(ElementFlag::MOVABLE, false);
    setInstrumentNameType(InstrumentNameType::LONG);
}

double InstrumentName::largestStaffSpatium() const
{
    if (systemFlag() || (explicitParent() && parentItem()->systemFlag())) {
        return style().spatium();
    }

    // Get spatium for instrument names from largest staff of part,
    // instead of staff it is attached to
    Part* p = part();
    if (!part()) {
        return style().spatium();
    }
    double largestSpatium = 0;
    for (Staff* s: p->staves()) {
        double sp = s->spatium(tick());
        if (sp > largestSpatium) {
            largestSpatium = sp;
        }
    }
    return largestSpatium;
}

//---------------------------------------------------------
//   setInstrumentNameType
//---------------------------------------------------------

void InstrumentName::setInstrumentNameType(InstrumentNameType st)
{
    m_instrumentNameType = st;
    if (st == InstrumentNameType::SHORT) {
        setTextStyleType(TextStyleType::INSTRUMENT_SHORT);
        initElementStyle(&shortInstrumentStyle);
    } else {
        setTextStyleType(TextStyleType::INSTRUMENT_LONG);
        initElementStyle(&longInstrumentStyle);
    }
}

bool InstrumentName::setProperty(Pid id, const PropertyValue& v)
{
    bool rv = true;
    switch (id) {
    case Pid::VISIBLE:
        // not supported
        break;
    default:
        rv = TextBase::setProperty(id, v);
        break;
    }
    return rv;
}

mu::engraving::staff_idx_t mu::engraving::InstrumentName::effectiveStaffIdx() const
{
    if (m_sysStaff->show() || m_instrumentNameRole == InstrumentNameRole::STAFF) {
        return staffIdx();
    }

    return system()->firstVisibleSysStaffOfPart(score()->staff(staffIdx())->part());
}
}
