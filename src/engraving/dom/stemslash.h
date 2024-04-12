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

#ifndef MU_ENGRAVING_STEMSLASH_H
#define MU_ENGRAVING_STEMSLASH_H

#include "engravingitem.h"

namespace mu::engraving {
class Chord;

//---------------------------------------------------------
//   @@ StemSlash
///    used for grace notes of type acciaccatura
//---------------------------------------------------------

class StemSlash final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, StemSlash)
    DECLARE_CLASSOF(ElementType::STEM_SLASH)

public:

    double mag() const override { return parentItem()->mag(); }

    StemSlash* clone() const override { return new StemSlash(*this); }

    Chord* chord() const { return (Chord*)explicitParent(); }

    struct LayoutData : public EngravingItem::LayoutData {
        LineF line;
        double stemWidth = 0.0;
    };
    DECLARE_LAYOUTDATA_METHODS(StemSlash)

private:

    friend class Factory;
    StemSlash(Chord* parent = 0);
};
} // namespace mu::engraving
#endif
