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

#ifndef MU_ENGRAVING_STICKING_H
#define MU_ENGRAVING_STICKING_H

#include "textbase.h"

namespace mu::engraving {
//-----------------------------------------------------------------------------
//   @@ Sticking
///    Drum sticking
//-----------------------------------------------------------------------------

class Sticking final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, Sticking)
    DECLARE_CLASSOF(ElementType::STICKING)

    PropertyValue propertyDefault(Pid id) const override;

public:
    Sticking(Segment* parent);

    Sticking* clone() const override { return new Sticking(*this); }

    Segment* segment() const { return (Segment*)explicitParent(); }
    Measure* measure() const { return (Measure*)explicitParent()->explicitParent(); }

    bool isEditAllowed(EditData&) const override;
};
} // namespace mu::engraving
#endif
