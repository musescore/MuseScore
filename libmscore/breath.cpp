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

#include "breath.h"
#include "sym.h"
#include "system.h"
#include "segment.h"
#include "measure.h"
#include "score.h"
#include "xml.h"

namespace Ms {

SymId Breath::symList[Breath::breathSymbols] = {
      SymId::breathMarkComma,
      SymId::breathMarkComma,      // TODO-smufl SymId(lcommaSym),
      SymId::caesuraCurved,
      SymId::caesura
      };

//---------------------------------------------------------
//   Breath
//---------------------------------------------------------

Breath::Breath(Score* s)
  : Element(s)
      {
      _breathType = 0;
      _pause = 0.0;
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Breath::layout()
      {
      setbbox(symBbox(symList[breathType()]));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Breath::write(Xml& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag("Breath");
      xml.tag("subtype", _breathType);
      writeProperty(xml, P_ID::PAUSE);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Breath::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  _breathType = e.readInt();
            else if (tag == "pause")
                  _pause = e.readDouble();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Breath::draw(QPainter* p) const
      {
      p->setPen(curColor());
      drawSymbol(symList[_breathType], p);
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

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Breath::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::PAUSE:
                  return _pause;
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Breath::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_ID::PAUSE:
                  setPause(v.toDouble());
                  score()->addLayoutFlags(LayoutFlag::FIX_TICKS);
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Breath::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_ID::PAUSE:
                  return 0.0;
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* Breath::nextElement()
      {
      return segment()->firstInNextSegments(staffIdx());
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* Breath::prevElement()
      {
      return segment()->lastInPrevSegments(staffIdx());
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Breath::accessibleInfo()
      {
      switch (breathType()) {
            case 2:
            case 3:
                  return tr("Caesura");
            default:
                  return tr("Breath");
            }
      }
}

