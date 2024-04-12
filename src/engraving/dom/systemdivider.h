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

class SystemDivider final : public Symbol
{
    OBJECT_ALLOCATOR(engraving, SystemDivider)
    DECLARE_CLASSOF(ElementType::SYSTEM_DIVIDER)

public:
    SystemDivider(System* parent = 0);
    SystemDivider(const SystemDivider&);

    SystemDivider* clone() const override { return new SystemDivider(*this); }

    enum class Type {
        LEFT, RIGHT
    };

    Type dividerType() const { return m_dividerType; }
    void setDividerType(Type v);

    RectF drag(EditData&) override;

    Segment* segment() const override { return nullptr; }
    System* system() const { return (System*)explicitParent(); }

    void styleChanged() override;

private:
    Type m_dividerType = Type::LEFT;
};
} // namespace mu::engraving

#endif
