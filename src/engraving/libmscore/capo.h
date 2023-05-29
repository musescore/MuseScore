/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#ifndef MU_ENGRAVING_CAPO_H
#define MU_ENGRAVING_CAPO_H

#include "stafftextbase.h"

namespace mu::engraving {
class Capo final : public StaffTextBase
{
    OBJECT_ALLOCATOR(engraving, Capo)
    DECLARE_CLASSOF(ElementType::CAPO)

public:
    Capo(Segment* parent = nullptr, TextStyleType textStyleType = TextStyleType::STAFF);

    Capo* clone() const override;

    PropertyValue getProperty(Pid id) const override;
    PropertyValue propertyDefault(Pid id) const override;
    bool setProperty(Pid id, const PropertyValue& val) override;
};
}

#endif // MU_ENGRAVING_CAPO_H
