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

#ifndef MU_ENGRAVING_SYSTEMTEXT_H
#define MU_ENGRAVING_SYSTEMTEXT_H

#include "stafftextbase.h"

namespace mu::engraving {
//---------------------------------------------------------
//   SystemText
//---------------------------------------------------------

class SystemText : public StaffTextBase
{
    OBJECT_ALLOCATOR(engraving, SystemText)
    DECLARE_CLASSOF(ElementType::SYSTEM_TEXT)

public:
    SystemText(Segment* parent, TextStyleType = TextStyleType::SYSTEM, ElementType type = ElementType::SYSTEM_TEXT);

    bool isEditAllowed(EditData&) const override;

    SystemText* clone() const override { return new SystemText(*this); }
    Segment* segment() const { return (Segment*)explicitParent(); }

    bool canBeExcludedFromOtherParts() const override { return true; }

protected:
    PropertyValue propertyDefault(Pid id) const override;
};
} // namespace mu::engraving
#endif
