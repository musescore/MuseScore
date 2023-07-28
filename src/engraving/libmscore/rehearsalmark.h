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

#ifndef __REHEARSALMARK_H__
#define __REHEARSALMARK_H__

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
    enum class Type {
        Main = 0,
        Additional
    };

    RehearsalMark(Segment* parent);

    RehearsalMark* clone() const override { return new RehearsalMark(*this); }

    Segment* segment() const { return (Segment*)explicitParent(); }

    PropertyValue propertyDefault(Pid id) const override;

    void setType(Type type);
    Type type() const { return _type; }

    void styleChanged() override;

private:
    void applyTypeStyle();

    Type _type = Type::Main;
};
} // namespace mu::engraving
#endif
