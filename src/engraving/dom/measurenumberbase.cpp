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
}
} // namespace MS
