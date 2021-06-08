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

#include "element.h"
#include "stem.h"

namespace Ms {
//---------------------------------------------------------
//   @@ StemSlash
///    used for grace notes of type acciaccatura
//---------------------------------------------------------

class StemSlash final : public Element
{
    mu::LineF line;

public:
    StemSlash(Score* s = 0)
        : Element(s) {}

    qreal mag() const override { return parent()->mag(); }
    void setLine(const mu::LineF& l);

    StemSlash* clone() const override { return new StemSlash(*this); }
    ElementType type() const override { return ElementType::STEM_SLASH; }
    void draw(mu::draw::Painter*) const override;
    void layout() override;
    Chord* chord() const { return (Chord*)parent(); }
};
}     // namespace Ms
#endif
