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

#ifndef MU_ENGRAVING_MEASURENUMBERBASE_H
#define MU_ENGRAVING_MEASURENUMBERBASE_H

#include "textbase.h"

namespace mu::engraving {
//---------------------------------------------------------
//   MeasureNumberBase
///   The basic element making measure numbers.
///   Reimplemented by MMRestRange
//---------------------------------------------------------

class MeasureNumberBase : public TextBase
{
    OBJECT_ALLOCATOR(engraving, MeasureNumberBase)
public:
    MeasureNumberBase(const ElementType& type, Measure* parent = nullptr, TextStyleType = TextStyleType::DEFAULT);
    MeasureNumberBase(const MeasureNumberBase& other);

    Measure* measure() const { return toMeasure(explicitParent()); }

    bool isEditable() const override { return false; }    // The measure numbers' text should not be editable
};
} // namespace mu::engraving

#endif
