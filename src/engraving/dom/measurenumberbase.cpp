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

#include "measurenumberbase.h"

#include "measure.h"
#include "score.h"
#include "staff.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   MeasureNumberBase
//---------------------------------------------------------

MeasureNumberBase::MeasureNumberBase(const ElementType& type, Measure* parent, TextStyleType tid)
    : TextBase(type, parent, tid)
{
    setFlag(ElementFlag::ON_STAFF, true);
}

//---------------------------------------------------------
//   MeasureNumberBase
//     Copy constructor
//---------------------------------------------------------

MeasureNumberBase::MeasureNumberBase(const MeasureNumberBase& other)
    : TextBase(other)
{
    setFlag(ElementFlag::ON_STAFF, true);
    setHPlacement(other.hPlacement());
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

engraving::PropertyValue MeasureNumberBase::getProperty(Pid id) const
{
    switch (id) {
    case Pid::HPLACEMENT:
        return int(hPlacement());
    default:
        return TextBase::getProperty(id);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool MeasureNumberBase::setProperty(Pid id, const PropertyValue& val)
{
    switch (id) {
    case Pid::HPLACEMENT:
        setHPlacement(val.value<PlacementH>());
        mutldata()->layoutInvalid = true;
        triggerLayout();
        return true;
    default:
        return TextBase::setProperty(id, val);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue MeasureNumberBase::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::DEFAULT;
    default:
        return TextBase::propertyDefault(id);
    }
}
} // namespace MS
