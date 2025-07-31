/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
class IndicatorIcon : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, IndicatorIcon)
    DECLARE_CLASSOF(ElementType::INDICATOR_ICON)

public:
    IndicatorIcon(const ElementType& type, System* parent = nullptr, ElementFlags = ElementFlag::NOTHING);
    IndicatorIcon* clone() const override { return new IndicatorIcon(*this); }

    const System* system() const { return toSystem(explicitParent()); }

    struct LayoutData : public EngravingItem::LayoutData {
        ld_field<RectF> rangeRect = { "[IndicatorIcon] rangeRect", RectF() };
    };
    DECLARE_LAYOUTDATA_METHODS(IndicatorIcon)

    muse::draw::Font font() const;

    virtual char16_t iconCode() const { return 0x000; }

    void undoChangeProperty(Pid, const PropertyValue&, PropertyFlags) override { return; } // not editable
};
}
