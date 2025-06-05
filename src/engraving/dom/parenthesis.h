/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#pragma once
#include "engravingitem.h"

namespace mu::engraving {
class Parenthesis : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Parenthesis)
    M_PROPERTY2(DirectionH, direction, setDirection, DirectionH::LEFT)

public:
    static constexpr double PARENTHESIS_END_WIDTH = 0.1;

    Parenthesis(Segment* parent);
    Parenthesis(const Parenthesis& p);

    Parenthesis* clone() const override { return new Parenthesis(*this); }
    Segment* segment() const { return (Segment*)explicitParent(); }

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    String accessibleInfo() const override;

    struct LayoutData : public EngravingItem::LayoutData
    {
        ld_field<muse::draw::PainterPath> path = "path";
        ld_field<double> startY = "startY";
        ld_field<double> height = "height";
        ld_field<double> thickness = "thickness";
    };
    DECLARE_LAYOUTDATA_METHODS(Parenthesis);
};
} // namespace engraving
