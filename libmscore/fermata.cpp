//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "fermata.h"
#include "score.h"
#include "chordrest.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "stafftype.h"
#include "undo.h"
#include "page.h"
#include "barline.h"
#include "sym.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   Fermata
//---------------------------------------------------------

Fermata::Fermata(Score* s)
   : Element(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      setPlacement(Placement::ABOVE);

      _symId         = SymId::noSym;
      _timeStretch   = 1.0;
      setPlay(true);
      }

Fermata::Fermata(SymId id, Score* s)
   : Fermata(s)
      {
      setSymId(id);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Fermata::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Fermata::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "subtype") {
            QString s = e.readElementText();
            SymId id = Sym::name2id(s);
            setSymId(id);
            }
      else if ( tag == "play")
            setPlay(e.readBool());
      else if (tag == "timeStretch")
            _timeStretch = e.readDouble();
      else if (tag == "offset") {
            if (score()->mscVersion() > 114)
                  Element::readProperties(e);
            else
                  e.skipCurrentElement(); // ignore manual layout in older scores
            }
      else if (Element::readProperties(e))
            ;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Fermata::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this)) {
            qDebug("%s not written", name());
            return;
            }
      xml.stag("Fermata");
      xml.tag("subtype", Sym::id2name(_symId));
      writeProperty(xml, P_ID::TIME_STRETCH);
      writeProperty(xml, P_ID::PLAY);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   userName
//---------------------------------------------------------

QString Fermata::userName() const
      {
      return Sym::id2userName(symId());
      }

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Fermata::draw(QPainter* painter) const
      {
#if 0
      SymId sym = symId();
      FermataShowIn flags = articulationList[int(articulationType())].flags;
      if (staff()) {
            if (staff()->staffGroup() == StaffGroup::TAB) {
                  if (!(flags & FermataShowIn::TABLATURE))
                        return;
                  }
            else {
                  if (!(flags & FermataShowIn::PITCHED_STAFF))
                        return;
                  }
            }
#endif
      painter->setPen(curColor());
      drawSymbol(_symId, painter, QPointF(-0.5 * width(), 0.0));
      }

//---------------------------------------------------------
//   chordRest
//---------------------------------------------------------

ChordRest* Fermata::chordRest() const
      {
      if (parent() && parent()->isChordRest())
            return toChordRest(parent());
      return 0;
      }

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure* Fermata::measure() const
      {
      Segment* s = segment();
      return toMeasure(s ? s->parent() : 0);
      }

//---------------------------------------------------------
//   system
//---------------------------------------------------------

System* Fermata::system() const
      {
      Measure* m = measure();
      return toSystem(m ? m->parent() : 0);
      }

//---------------------------------------------------------
//   page
//---------------------------------------------------------

Page* Fermata::page() const
      {
      System* s = system();
      return toPage(s ? s->parent() : 0);
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Fermata::layout()
      {
      QRectF b(symBbox(_symId));
      setbbox(b.translated(-0.5 * b.width(), 0.0));

      Segment* s = segment();
      if (!s) {          // for use in palette
            setPos(QPointF());      // for palette
            return;
            }

      qreal _spatium = spatium();
      qreal dist = _spatium * 2;
      qreal noteHeadWidth = score()->noteHeadWidth() * staff()->mag(0);
      qreal    x = noteHeadWidth * .5;
      qreal    y;

      if (placeAbove())
            y = -dist;
      else
            y = dist + staff()->height();
      setPos(QPointF(x, y));

      if (!autoplace()) {
            adjustReadPos();
            return;
            }
      setUserOff(QPointF());

      // check for collisions

      qreal minDistance = _spatium * .5;        // score()->styleP(StyleIdx::dynamicsMinDistance);
      const Shape& s1   = s->measure()->staffShape(staffIdx());
      Shape s2          = shape().translated(s->pos() + pos());

      if (placeAbove()) {
            qreal d = s2.minVerticalDistance(s1);
            if (d > -minDistance)
                  rUserYoffset() = -d - minDistance;
            }
      else {
            qreal d = s1.minVerticalDistance(s2);
            if (d > -minDistance)
                  rUserYoffset() = d + minDistance;
            }
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Fermata::reset()
      {
      Element::reset();
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Fermata::dragAnchor() const
      {
      return QLineF(canvasPos(), parent()->canvasPos());
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Fermata::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::TIME_STRETCH:
                  return timeStretch();
            case P_ID::PLAY:
                  return play();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Fermata::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::PLACEMENT: {
                  Placement p = Placement(v.toInt());
                  if (p != placement()) {
                        QString s = Sym::id2name(_symId);
                        bool up = placeAbove();
                        if (s.endsWith(up ? "Above" : "Below")) {
                              QString s2 = s.left(s.size() - 5) + (up ? "Below" : "Above");
                              _symId = Sym::name2id(s2);
                              }
                        setPlacement(p);
                        }
                  }
                  break;
            case P_ID::PLAY:
                  setPlay(v.toBool());
                  break;
            case P_ID::TIME_STRETCH:
                  setTimeStretch(v.toDouble());
                  score()->fixTicks();
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Fermata::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::PLACEMENT:
                  return int(Placement::ABOVE);
            case P_ID::TIME_STRETCH:
                  return 1.0; // articulationList[int(articulationType())].timeStretch;
            case P_ID::PLAY:
                  return true;
            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyFlags& Fermata::propertyFlags(P_ID id)
      {
#if 0
      switch (id) {
            case P_ID::TIME_STRETCH:
                  return PropertyFlags::NOSTYLE;

            default:
                  break;
            }
#endif
      return Element::propertyFlags(id);
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Fermata::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            default:
                  break;
            }
      return StyleIdx::NOSTYLE;
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Fermata::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::TIME_STRETCH:
                  setProperty(id, propertyDefault(id));
                  return;

            default:
                  break;
            }
      Element::resetProperty(id);
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Fermata::mag() const
      {
      return parent() ? parent()->mag() * score()->styleD(StyleIdx::articulationMag): 1.0;
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Fermata::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(userName());
      }

}
