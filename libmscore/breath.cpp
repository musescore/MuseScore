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

const std::vector<BreathType> Breath::breathList {
      { SymId::breathMarkComma,      false, 0.0 },
      { SymId::breathMarkTick,       false, 0.0 },
      { SymId::breathMarkSalzedo,    false, 0.0 },
      { SymId::breathMarkUpbow,      false, 0.0 },
      { SymId::caesuraCurved,        true,  2.0 },
      { SymId::caesura,              true,  2.0 },
      { SymId::caesuraShort,         true,  2.0 },
      { SymId::caesuraThick,         true,  2.0 },
      };

//---------------------------------------------------------
//   Breath
//---------------------------------------------------------

Breath::Breath(Score* s)
   : Element(s, ElementFlag::MOVABLE)
      {
      _symId = SymId::breathMarkComma;
      _pause = 0.0;
      }

//---------------------------------------------------------
//   isCaesura
//---------------------------------------------------------

bool Breath::isCaesura() const
      {
      for (const BreathType& bt : breathList) {
            if (bt.id == _symId)
                  return bt.isCaesura;
            }
      return false;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Breath::layout()
      {
      if (isCaesura())
            setPos(x(), spatium());
      else
            setPos(x(), 0.5 * spatium());
      setbbox(symBbox(_symId));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Breath::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
      writeProperty(xml, Pid::SYMBOL);
      writeProperty(xml, Pid::PAUSE);
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
            if (tag == "subtype") {             // obsolete
                  switch (e.readInt()) {
                        case 0:
                        case 1:
                              _symId = SymId::breathMarkComma;
                              break;
                        case 2:
                              _symId = SymId::caesuraCurved;
                              break;
                        case 3:
                              _symId = SymId::caesura;
                              break;
                        }
                  }
            else if (tag == "symbol")
                  _symId = Sym::name2id(e.readElementText());
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
      drawSymbol(_symId, p);
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

QVariant Breath::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::SYMBOL:
                  return QVariant::fromValue(_symId);
            case Pid::PAUSE:
                  return _pause;
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Breath::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::SYMBOL:
                  setSymId(v.value<SymId>());
                  break;

            case Pid::PAUSE:
                  setPause(v.toDouble());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      triggerLayout();
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Breath::propertyDefault(Pid id) const
      {
      switch(id) {
            case Pid::PAUSE:
                  return 0.0;
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* Breath::nextSegmentElement()
      {
      return segment()->firstInNextSegments(staffIdx());
      }

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

Element* Breath::prevSegmentElement()
      {
      return segment()->lastInPrevSegments(staffIdx());
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Breath::accessibleInfo() const
      {
      return Sym::id2userName(_symId);
      }
}

