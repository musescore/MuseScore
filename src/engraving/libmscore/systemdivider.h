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

#ifndef __SYSTEMDIVIDER_H__
#define __SYSTEMDIVIDER_H__

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
    enum class Type {
        LEFT, RIGHT
    };

private:
    Type _dividerType;

public:
    SystemDivider(System* parent = 0);
    SystemDivider(const SystemDivider&);

    SystemDivider* clone() const override { return new SystemDivider(*this); }

    Type dividerType() const { return _dividerType; }
    void setDividerType(Type v);

    mu::RectF drag(EditData&) override;

    void layout() override;

    Segment* segment() const override { return nullptr; }
    System* system() const { return (System*)explicitParent(); }
};
} // namespace mu::engraving

#endif
