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

#ifndef MU_ENGRAVING_SYSTEMDIVIDER_H
#define MU_ENGRAVING_SYSTEMDIVIDER_H

#include "symbol.h"

namespace mu::engraving {
//---------------------------------------------------------
//   SystemDivider
//---------------------------------------------------------
enum class SystemDividerType : unsigned char {
    LEFT, RIGHT
};

class SystemDivider final : public Symbol
{
    OBJECT_ALLOCATOR(engraving, SystemDivider)
    DECLARE_CLASSOF(ElementType::SYSTEM_DIVIDER)

public:
    SystemDivider(System* parent = 0);
    SystemDivider(const SystemDivider&);

    SystemDivider* clone() const override { return new SystemDivider(*this); }

    SystemDividerType dividerType() const { return m_dividerType; }
    void setDividerType(SystemDividerType v);

    RectF drag(EditData&) override;

    Segment* segment() const override { return nullptr; }
    System* system() const { return (System*)explicitParent(); }

    PropertyValue propertyDefault(Pid id) const override;

    void styleChanged() override;

    std::vector<LineF> dragAnchorLines() const override;

    void reset() override;

private:
    SystemDividerType m_dividerType = SystemDividerType::LEFT;
};
} // namespace mu::engraving

#endif
