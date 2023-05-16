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

#include "stemslash.h"

#include "layout/tlayout.h"

#include "beam.h"
#include "chord.h"
#include "hook.h"
#include "note.h"
#include "score.h"
#include "stem.h"

#include "draw/types/pen.h"

using namespace mu;

namespace mu::engraving {
StemSlash::StemSlash(Chord* parent)
    : EngravingItem(ElementType::STEM_SLASH, parent)
{
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StemSlash::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    painter->setPen(Pen(curColor(), m_width, PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->drawLine(m_line);
}

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StemSlash::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
=======
>>>>>>> cd79de8b507ce5e52931bbfbce650f3fc04e0ae2
KerningType StemSlash::doComputeKerningType(const EngravingItem* nextItem) const
{
    if (!this->chord() || !this->chord()->beam() || !nextItem || !nextItem->parentItem()) {
        return KerningType::KERNING;
    }
    EngravingItem* nextParent = nextItem->parentItem();
    Chord* nextChord = nullptr;
    if (nextParent->isChord()) {
        nextChord = toChord(nextParent);
    } else if (nextParent->isNote()) {
        nextChord = toChord(nextParent->parentItem());
    }
    if (!nextChord) {
        return KerningType::KERNING;
    }
    if (nextChord->beam() && nextChord->beam() == this->chord()->beam()) {
        // Stem slash is allowed to collide with items from the same grace notes group
        return KerningType::ALLOW_COLLISION;
    }
    return KerningType::KERNING;
}
} // namespace mu::engraving
