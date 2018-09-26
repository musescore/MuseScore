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

#include "trill.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "sym.h"
#include "score.h"
#include "accidental.h"
#include "segment.h"
#include "staff.h"

namespace Ms {


//---------------------------------------------------------
//   trillTable
//    must be in sync with Trill::Type
//---------------------------------------------------------

const TrillTableItem trillTable[] = {
      { Trill::Type::TRILL_LINE,      "trill",      QT_TRANSLATE_NOOP("trillType", "Trill line")          },
      { Trill::Type::UPPRALL_LINE,    "upprall",    QT_TRANSLATE_NOOP("trillType", "Upprall line")        },
      { Trill::Type::DOWNPRALL_LINE,  "downprall",  QT_TRANSLATE_NOOP("trillType", "Downprall line")      },
      { Trill::Type::PRALLPRALL_LINE, "prallprall", QT_TRANSLATE_NOOP("trillType", "Prallprall line")     }
      };

int trillTableSize() {
      return sizeof(trillTable)/sizeof(TrillTableItem);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TrillSegment::draw(QPainter* painter) const
      {
      painter->setPen(spanner()->curColor());
      drawSymbols(_symbols, painter);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void TrillSegment::add(Element* e)
      {
      e->setParent(this);
      if (e->type() == ElementType::ACCIDENTAL) {
            // accidental is part of trill
            trill()->setAccidental(toAccidental(e));
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TrillSegment::remove(Element* e)
      {
      if (trill()->accidental() == e) {
            // accidental is part of trill
            trill()->setAccidental(0);
            }
      }

//---------------------------------------------------------
//   symbolLine
//---------------------------------------------------------

void TrillSegment::symbolLine(SymId start, SymId fill)
      {
      qreal x1 = 0;
      qreal x2 = pos2().x();
      qreal w   = x2 - x1;
      qreal mag = magS();
      ScoreFont* f = score()->scoreFont();

      _symbols.clear();
      _symbols.push_back(start);
      qreal w1 = f->advance(start, mag);
      qreal w2 = f->advance(fill, mag);
      int n    = lrint((w - w1) / w2);
      for (int i = 0; i < n; ++i)
           _symbols.push_back(fill);
      QRectF r(f->bbox(_symbols, mag));
      setbbox(r);
      }

void TrillSegment::symbolLine(SymId start, SymId fill, SymId end)
      {
      qreal x1 = 0;
      qreal x2 = pos2().x();
      qreal w   = x2 - x1;
      qreal mag = magS();
      ScoreFont* f = score()->scoreFont();

      _symbols.clear();
      _symbols.push_back(start);
      qreal w1 = f->bbox(start, mag).width();
      qreal w2 = f->width(fill, mag);
      qreal w3 = f->width(end, mag);
      int n    = lrint((w - w1 - w3) / w2);
      for (int i = 0; i < n; ++i)
           _symbols.push_back(fill);
      _symbols.push_back(end);
      QRectF r(f->bbox(_symbols, mag));
      setbbox(r);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TrillSegment::layout()
      {
      if (staff())
            setMag(staff()->mag(tick()));
      if (isSingleType() || isBeginType()) {
            Accidental* a = trill()->accidental();
            if (a) {
                  a->layout();
                  a->setMag(a->mag() * .6);
                  qreal _spatium = spatium();
                  a->setPos(_spatium * 1.3, -2.2 * _spatium);
                  a->setParent(this);
                  }
            switch (trill()->trillType()) {
                  case Trill::Type::TRILL_LINE:
                        symbolLine(SymId::ornamentTrill, SymId::wiggleTrill);
                        break;
                  case Trill::Type::PRALLPRALL_LINE:
                        symbolLine(SymId::wiggleTrill, SymId::wiggleTrill);
                        break;
                  case Trill::Type::UPPRALL_LINE:
                              symbolLine(SymId::ornamentBottomLeftConcaveStroke,
                                 SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
                        break;
                  case Trill::Type::DOWNPRALL_LINE:
                              symbolLine(SymId::ornamentLeftVerticalStroke,
                                 SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
                        break;
                  }
            }
      else
            symbolLine(SymId::wiggleTrill, SymId::wiggleTrill);

      autoplaceSpannerSegment(spatium() * 1.0, Sid::trillPosBelow, Sid::trillPosAbove);
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape TrillSegment::shape() const
      {
      return Shape(bbox());
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TrillSegment::acceptDrop(EditData& data) const
      {
      if (data.element->isAccidental())
            return true;
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* TrillSegment::drop(EditData& data)
      {
      Element* e = data.element;
      switch (e->type()) {
            case ElementType::ACCIDENTAL:
                  e->setParent(trill());
                  score()->undoAddElement(e);
                  break;

            default:
                  delete e;
                  e = 0;
                  break;
            }
      return e;
      }

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

Element* TrillSegment::propertyDelegate(Pid pid)
      {
      if (pid == Pid::TRILL_TYPE || pid == Pid::ORNAMENT_STYLE || pid == Pid::PLACEMENT || pid == Pid::PLAY)
            return spanner();
      return LineSegment::propertyDelegate(pid);
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void TrillSegment::scanElements(void* data, void (*func)(void*, Element*), bool /*all*/)
      {
      func(data, this);
      if (isSingleType() || isBeginType()) {
            Accidental* a = trill()->accidental();
            if (a)
                  func(data, a);
            }
      }

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

Trill::Trill(Score* s)
  : SLine(s)
      {
      _trillType     = Type::TRILL_LINE;
      _accidental    = 0;
      _ornamentStyle = MScore::OrnamentStyle::DEFAULT;
      setPlayArticulation(true);
      setPlacement(Placement::ABOVE);
      }

Trill::~Trill()
      {
      delete _accidental;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Trill::add(Element* e)
      {
      if (e->type() == ElementType::ACCIDENTAL) {
            e->setParent(this);
            _accidental = toAccidental(e);
            }
      else
            SLine::add(e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Trill::remove(Element* e)
      {
      if (e == _accidental)
            _accidental = 0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Trill::layout()
      {
      SLine::layout();
      if (score() == gscore)
            return;
      if (spannerSegments().empty())
            return;
      TrillSegment* ls = toTrillSegment(frontSegment());
      if (spannerSegments().empty())
            qDebug("Trill: no segments");
      if (_accidental)
            _accidental->setParent(ls);
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Trill::createLineSegment()
      {
      TrillSegment* seg = new TrillSegment(score());
      seg->setTrack(track());
      seg->setColor(color());
      return seg;
      }

//---------------------------------------------------------
//   Trill::write
//---------------------------------------------------------

void Trill::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
      xml.tag("subtype", trillTypeName());
      writeProperty(xml, Pid::PLAY);
      writeProperty(xml, Pid::ORNAMENT_STYLE);
      SLine::writeProperties(xml);
      if (_accidental)
            _accidental->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Trill::read
//---------------------------------------------------------

void Trill::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setTrillType(e.readElementText());
            else if (tag == "Accidental") {
                  _accidental = new Accidental(score());
                  _accidental->read(e);
                  _accidental->setParent(this);
                  }
            else if ( tag == "ornamentStyle")
                  readProperty(e, Pid::ORNAMENT_STYLE);
            else if ( tag == "play")
                  setPlayArticulation(e.readBool());
            else if (!SLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   setTrillType
//---------------------------------------------------------

void Trill::setTrillType(const QString& s)
      {
      if (s == "0") {
            _trillType = Type::TRILL_LINE;
            return;
            }
      if (s == "pure") {
            _trillType = Type::PRALLPRALL_LINE; // obsolete, compatibility only
            return;
            }
      for (TrillTableItem i : trillTable) {
            if (s.compare(i.name) == 0) {
                  _trillType = i.type;
                  return;
                  }
            }
      qDebug("Trill::setSubtype: unknown <%s>", qPrintable(s));
      }

//---------------------------------------------------------
//   trillTypeName
//---------------------------------------------------------

QString Trill::trillTypeName() const
      {
      for (TrillTableItem i : trillTable) {
            if (i.type == trillType())
                  return i.name;
            }
      qDebug("unknown Trill subtype %d", int(trillType()));
            return "?";
      }

//---------------------------------------------------------
//   trillTypeName
//---------------------------------------------------------

QString Trill::trillTypeUserName() const
      {
      return qApp->translate("trillType", trillTable[static_cast<int>(trillType())].userName.toUtf8().constData());
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Trill::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (_accidental)
            _accidental->scanElements(data, func, all);
      func(data, this);       // ?
      SLine::scanElements(data, func, all);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Trill::getProperty(Pid propertyId) const
      {
      switch(propertyId) {
            case Pid::TRILL_TYPE:
                  return int(trillType());
            case Pid::ORNAMENT_STYLE:
                  return int(ornamentStyle());
            case Pid::PLAY:
                  return bool(playArticulation());
            default:
                  break;
            }
      return SLine::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Trill::setProperty(Pid propertyId, const QVariant& val)
      {
      switch(propertyId) {
            case Pid::TRILL_TYPE:
                  setTrillType(Type(val.toInt()));
                  break;
            case Pid::PLAY:
                  setPlayArticulation(val.toBool());
                  break;
            case Pid::ORNAMENT_STYLE:
                  setOrnamentStyle(MScore::OrnamentStyle(val.toInt()));
                  break;
            default:
                  if (!SLine::setProperty(propertyId, val))
                        return false;
                  break;
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Trill::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::TRILL_TYPE:
                  return 0;
            case Pid::ORNAMENT_STYLE:
                  //return int(score()->style()->ornamentStyle(_ornamentStyle));
                  return int(MScore::OrnamentStyle::DEFAULT);
            case Pid::PLAY:
                  return true;
            case Pid::PLACEMENT:
                  return int(Placement::ABOVE);
            default:
                  return SLine::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   undoSetTrillType
//---------------------------------------------------------

void Trill::undoSetTrillType(Type val)
      {
      undoChangeProperty(Pid::TRILL_TYPE, int(val));
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Trill::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(trillTypeUserName());
      }
}

