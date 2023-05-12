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

#include "notedot.h"

#include "layout/tlayout.h"

#include "chord.h"
#include "note.h"
#include "rest.h"
#include "score.h"
#include "staff.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   NoteDot
//---------------------------------------------------------

NoteDot::NoteDot(Note* parent)
    : EngravingItem(ElementType::NOTEDOT, parent)
{
    setFlag(ElementFlag::MOVABLE, false);
}

NoteDot::NoteDot(Rest* parent)
    : EngravingItem(ElementType::NOTEDOT, parent)
{
    setFlag(ElementFlag::MOVABLE, false);
}

//---------------------------------------------------------
//   NoteDot::draw
//---------------------------------------------------------

void NoteDot::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    if (note() && note()->dotsHidden()) {     // don't draw dot if note is hidden
        return;
    } else if (rest() && rest()->isGap()) {  // don't draw dot for gap rests
        return;
    }
    Note* n = note();
    Fraction tick = n ? n->chord()->tick() : rest()->tick();
    // always draw dot for non-tab
    // for tab, draw if on a note and stems through staff or on a rest and rests shown
    if (!staff()->isTabStaff(tick)
        || (n && staff()->staffType(tick)->stemThrough())
        || (!n && staff()->staffType(tick)->showRests())) {
        painter->setPen(curColor());
        drawSymbol(SymId::augmentationDot, painter);
    }
}

//---------------------------------------------------------
<<<<<<< HEAD
<<<<<<< HEAD
=======
//   layout
//---------------------------------------------------------

void NoteDot::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
//   elementBase
//---------------------------------------------------------

EngravingItem* NoteDot::elementBase() const
{
    return parentItem();
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double NoteDot::mag() const
{
    return parentItem()->mag() * score()->styleD(Sid::dotMag);
}
}
