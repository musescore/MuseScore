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
#include "rw/xml.h"
#include "score.h"
#include "staff.h"
#include "chord.h"
#include "rest.h"

using namespace mu;

namespace Ms {
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
    TRACE_OBJ_DRAW;
    if (note() && note()->dotsHidden()) {     // don't draw dot if note is hidden
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
//   layout
//---------------------------------------------------------

void NoteDot::layout()
{
    setbbox(symBbox(SymId::augmentationDot));
}

//---------------------------------------------------------
//   elementBase
//---------------------------------------------------------

EngravingItem* NoteDot::elementBase() const
{
    return parentItem();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NoteDot::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        if (e.name() == "name") {      // obsolete
            e.readElementText();
        } else if (e.name() == "subtype") {     // obsolete
            e.readElementText();
        } else if (!EngravingItem::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal NoteDot::mag() const
{
    return parentItem()->mag() * score()->styleD(Sid::dotMag);
}
}
