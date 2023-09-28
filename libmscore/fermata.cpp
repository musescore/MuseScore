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
//   fermataStyle
//---------------------------------------------------------

static const ElementStyle fermataStyle {
      { Sid::fermataPosAbove, Pid::OFFSET },
      { Sid::fermataMinDistance, Pid::MIN_DISTANCE },
      };

//---------------------------------------------------------
//   Fermata
//---------------------------------------------------------

Fermata::Fermata(Score* s)
   : Element(s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      setPlacement(Placement::ABOVE);
      _symId         = SymId::noSym;
      _timeStretch   = 1.0;
      setPlay(true);
      initElementStyle(&fermataStyle);
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
      else if (tag == "play")
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
      xml.stag(this);
      xml.tag("subtype", Sym::id2name(_symId));
      writeProperty(xml, Pid::TIME_STRETCH);
      writeProperty(xml, Pid::PLAY);
      writeProperty(xml, Pid::MIN_DISTANCE);
      if (!isStyled(Pid::OFFSET))
            writeProperty(xml, Pid::OFFSET);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   subtype
//---------------------------------------------------------

int Fermata::subtype() const
      {
      QString s = Sym::id2name(_symId);
      if (s.endsWith("Below"))
            return int(Sym::name2id(s.left(s.size() - 5) + "Above"));
      else
            return int(_symId);
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
      Segment* s = segment();
      setPos(QPointF());
      if (!s) {          // for use in palette
            setOffset(0.0, 0.0);
            QRectF b(symBbox(_symId));
            setbbox(b.translated(-0.5 * b.width(), 0.0));
            return;
            }

      if (isStyled(Pid::OFFSET))
            setOffset(propertyDefault(Pid::OFFSET).toPointF());
      Element* e = s->element(track());
      if (e) {
            if (e->isChord())
                  rxpos() += score()->noteHeadWidth() * staff()->mag(Fraction(0, 1)) * .5;
            else if (e->isRest()) {
                  const Rest* rest = toRest(e);
                  rxpos() += e->x() + rest->centerX();
                  }
            else
                  rxpos() += e->x() - e->shape().left() + e->width() * staff()->mag(Fraction(0, 1)) * .5;
            }

      QString name = Sym::id2name(_symId);
      if (placeAbove()) {
            if (name.endsWith("Below"))
                  _symId = Sym::name2id(name.left(name.size() - 5) + "Above");
            }
      else {
            rypos() += staff()->height();
            if (name.endsWith("Above"))
                  _symId = Sym::name2id(name.left(name.size() - 5) + "Below");
            }
      QRectF b(symBbox(_symId));
      setbbox(b.translated(-0.5 * b.width(), 0.0));
      autoplaceSegmentElement();
      }

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

QVector<QLineF> Fermata::dragAnchorLines() const
      {
      QVector<QLineF> result;
      result << QLineF(canvasPos(), parent()->canvasPos());
      return result;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Fermata::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::SYMBOL:
                  return QVariant::fromValue(_symId);
            case Pid::TIME_STRETCH:
                  return timeStretch();
            case Pid::PLAY:
                  return play();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Fermata::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::SYMBOL:
                  setSymId(v.value<SymId>());
                  break;
            case Pid::PLACEMENT: {
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
            case Pid::PLAY:
                  setPlay(v.toBool());
                  break;
            case Pid::TIME_STRETCH:
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

QVariant Fermata::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::PLACEMENT:
                  return int(track() & 1 ? Placement::BELOW : Placement::ABOVE);
            case Pid::TIME_STRETCH:
                  return 1.0; // articulationList[int(articulationType())].timeStretch;
            case Pid::PLAY:
                  return true;
            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Fermata::resetProperty(Pid id)
      {
      switch (id) {
            case Pid::TIME_STRETCH:
                  setProperty(id, propertyDefault(id));
                  return;

            default:
                  break;
            }
      Element::resetProperty(id);
      }

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid Fermata::propertyId(const QStringRef& xmlName) const
      {
      if (xmlName == "subtype")
            return Pid::SYMBOL;
      return Element::propertyId(xmlName);
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid Fermata::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET)
            return placeAbove() ? Sid::fermataPosAbove : Sid::fermataPosBelow;
      return ScoreElement::getPropertyStyle(pid);
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Fermata::mag() const
      {
      return staff() ? staff()->mag(tick()) * score()->styleD(Sid::articulationMag) : 1.0;
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Fermata::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo(), userName());
      }

}
