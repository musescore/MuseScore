/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef __FINGERING_H__
#define __FINGERING_H__

#include "textbase.h"

#include "types/types.h"

namespace mu::engraving {
class Note;

//---------------------------------------------------------
//   @@ Fingering
//---------------------------------------------------------

class Fingering final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, Fingering)
    DECLARE_CLASSOF(ElementType::FINGERING)

public:
    Fingering(Note* parent, TextStyleType tid, ElementFlags ef = ElementFlag::HAS_TAG);
    Fingering(Note* parent, ElementFlags ef = ElementFlag::HAS_TAG);

    Fingering* clone() const override { return new Fingering(*this); }

    Note* note() const { return toNote(explicitParent()); }
    ElementType layoutType() const;
    PlacementV calculatePlacement() const;

    bool isEditAllowed(EditData&) const override;

    PropertyValue propertyDefault(Pid id) const override;

    String accessibleInfo() const override;

    bool isOnCrossBeamSide() const;
};
} // namespace mu::engraving
#endif
