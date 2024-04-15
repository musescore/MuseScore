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

#ifndef MU_ENGRAVING_DEADSLAPPED_H
#define MU_ENGRAVING_DEADSLAPPED_H

#include "engravingitem.h"

namespace mu::engraving {
//---------------------------------------------------------
//    @@ DeadSlapped
///     This class implements a dead slapped element.
//---------------------------------------------------------

class DeadSlapped : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, DeadSlapped)
    DECLARE_CLASSOF(ElementType::DEAD_SLAPPED)

public:

    ~DeadSlapped() {}

    DeadSlapped* clone() const override { return new DeadSlapped(*this); }

    struct LayoutData : public EngravingItem::LayoutData {
        muse::draw::PainterPath path1;
        muse::draw::PainterPath path2;
    };
    DECLARE_LAYOUTDATA_METHODS(DeadSlapped)

private:

    friend class Factory;

    DeadSlapped(Rest* parent);
};
} // namespace mu::engraving
#endif
