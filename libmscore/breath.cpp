//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: breath.cpp 5230 2012-01-19 17:35:40Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "breath.h"
#include "sym.h"
#include "system.h"
#include "segment.h"
#include "measure.h"
#include "score.h"

int Breath::symList[Breath::breathSymbols] = {
      rcommaSym,
      lcommaSym,
      caesuraCurvedSym,
      caesuraStraight
      };

//---------------------------------------------------------
//   Breath
//---------------------------------------------------------

Breath::Breath(Score* s)
  : Element(s)
      {
      _subtype = 0;
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Breath::layout()
      {
      setbbox(symbols[score()->symIdx()][symList[subtype()]].bbox(magS()));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Breath::write(Xml& xml) const
      {
      xml.stag("Breath");
      xml.tag("subtype", _subtype);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Breath::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "subtype")
                  _subtype = e.text().toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Breath::draw(QPainter* p) const
      {
      p->setPen(curColor());
      symbols[score()->symIdx()][symList[subtype()]].draw(p, magS());
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space Breath::space() const
      {
      return Space(0.0, spatium() * 1.5);
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF Breath::pagePos() const
      {
      if (parent() == 0)
            return pos();
      System* system = segment()->measure()->system();
      qreal yp = y();
      if (system)
            yp += system->staff(staffIdx())->y() + system->y();
      return QPointF(pageX(), yp);
      }

