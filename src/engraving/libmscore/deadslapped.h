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

#ifndef __DEADSLAPPED_H__
#define __DEADSLAPPED_H__

#include "durationelement.h"

namespace mu::engraving {
//---------------------------------------------------------
//    @@ DeadSlapped
///     This class implements a dead slapped element.
//---------------------------------------------------------

class DeadSlapped : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, DeadSlapped)
public:

    ~DeadSlapped() {}

    DeadSlapped* clone() const override { return new DeadSlapped(*this); }

    void draw(mu::draw::Painter*) const override;

private:

    friend class Factory;
    friend class layout::v0::TLayout;

    DeadSlapped(Rest* parent);

    mu::draw::PainterPath m_path1;
    mu::draw::PainterPath m_path2;
};
} // namespace mu::engraving
#endif
