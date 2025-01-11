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

#ifndef MU_ENGRAVING_REHEARSALMARK_H
#define MU_ENGRAVING_REHEARSALMARK_H

#include "textbase.h"

namespace mu::engraving {
//---------------------------------------------------------
//   @@ RehearsalMark
//---------------------------------------------------------

class RehearsalMark final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, RehearsalMark)
    DECLARE_CLASSOF(ElementType::REHEARSAL_MARK)

public:
    enum class Type : unsigned char {
        Main = 0,
        Additional
    };

    RehearsalMark(Segment* parent);

    bool isEditAllowed(EditData&) const override;

    RehearsalMark* clone() const override { return new RehearsalMark(*this); }

    Segment* segment() const { return (Segment*)explicitParent(); }

    PropertyValue propertyDefault(Pid id) const override;

    void setType(Type type);
    Type type() const { return m_type; }

    void styleChanged() override;

private:
    void applyTypeStyle();

    Type m_type = Type::Main;
};
} // namespace mu::engraving
#endif
