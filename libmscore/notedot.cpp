//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "notedot.h"
#include "score.h"
#include "staff.h"
#include "sym.h"
#include "xml.h"
#include "chord.h"
#include "rest.h"

namespace Ms {
//---------------------------------------------------------
//   NoteDot
//---------------------------------------------------------

NoteDot::NoteDot(Score* s)
    : Element(s)
{
    setFlag(ElementFlag::MOVABLE, false);
}

//---------------------------------------------------------
//   NoteDot::draw
//---------------------------------------------------------

void NoteDot::draw(QPainter* p) const
{
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
        p->setPen(curColor());
        drawSymbol(SymId::augmentationDot, p);
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

Element* NoteDot::elementBase() const
{
    return parent();
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
        } else if (!Element::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal NoteDot::mag() const
{
    return parent()->mag() * score()->styleD(Sid::dotMag);
}
}
