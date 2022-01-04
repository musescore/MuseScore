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

#ifndef __STEMSLASH_H__
#define __STEMSLASH_H__

#include "engravingitem.h"
#include "stem.h"

namespace Ms {
//---------------------------------------------------------
//   @@ StemSlash
///    used for grace notes of type acciaccatura
//---------------------------------------------------------

class StemSlash final : public EngravingItem
{
    mu::LineF line;

    friend class mu::engraving::Factory;
    StemSlash(Chord* parent = 0);

public:

    qreal mag() const override { return parentItem()->mag(); }
    void setLine(const mu::LineF& l);

    StemSlash* clone() const override { return new StemSlash(*this); }
    void draw(mu::draw::Painter*) const override;
    void layout() override;
    Chord* chord() const { return (Chord*)explicitParent(); }
};
}     // namespace Ms
#endif
