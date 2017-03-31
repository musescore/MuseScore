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


// must be in sync with Trill::Type
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
            trill()->setAccidental(static_cast<Accidental*>(e));
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
      if (autoplace())
            setUserOff(QPointF());

      if (staff())
            setMag(staff()->mag(tick()));
      if (isSingleType() || isBeginType()) {
            Accidental* a = trill()->accidental();
            if (a) {
                  a->layout();
                  a->setMag(a->mag() * .6);
                  qreal _spatium = spatium();
                  a->setPos(_spatium * 1.3, -2.2 * _spatium);
                  a->adjustReadPos();
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
                        if (score()->scoreFont()->isValid(SymId::ornamentBottomLeftConcaveStroke))
                              symbolLine(SymId::ornamentBottomLeftConcaveStroke,
                                 SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
                        else
                              symbolLine(SymId::ornamentUpPrall,
                                 // SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
                                 SymId::ornamentZigZagLineNoRightEnd);
                        break;
                  case Trill::Type::DOWNPRALL_LINE:
                        if (score()->scoreFont()->isValid(SymId::ornamentLeftVerticalStroke))
                              symbolLine(SymId::ornamentLeftVerticalStroke,
                                 SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
                        else
                              symbolLine(SymId::ornamentDownPrall,
                                 // SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
                                 SymId::ornamentZigZagLineNoRightEnd);
                        break;
                  }
            }
      else
            symbolLine(SymId::wiggleTrill, SymId::wiggleTrill);

      if (parent()) {
            qreal yo = score()->styleP(trill()->placeBelow() ? StyleIdx::trillPosBelow : StyleIdx::trillPosAbove);
            rypos() = yo;
            if (autoplace()) {
                  qreal minDistance = spatium();
                  Shape s1 = shape().translated(pos());
                  if (trill()->placeAbove()) {
                        qreal d  = system()->topDistance(staffIdx(), s1);
                        if (d > -minDistance)
                              rUserYoffset() = -d - minDistance;
                        }
                  else {
                        qreal d  = system()->bottomDistance(staffIdx(), s1);
                        if (d > -minDistance)
                              rUserYoffset() = d + minDistance;
                        }
                  }
            else
                  adjustReadPos();
            }
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
//   getProperty
//---------------------------------------------------------

QVariant TrillSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::TRILL_TYPE:
            case P_ID::ORNAMENT_STYLE:
            case P_ID::PLACEMENT:
            case P_ID::PLAY:
                  return trill()->getProperty(id);
            default:
                  return LineSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TrillSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::TRILL_TYPE:
            case P_ID::ORNAMENT_STYLE:
            case P_ID::PLACEMENT:
            case P_ID::PLAY:
                  return trill()->setProperty(id, v);
            default:
                  return LineSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TrillSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::TRILL_TYPE:
            case P_ID::ORNAMENT_STYLE:
            case P_ID::PLACEMENT:
            case P_ID::PLAY:
                  return trill()->propertyDefault(id);
            default:
                  return LineSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void TrillSegment::scanElements(void* data, void (*func)(void*, Element*), bool /*all*/)
      {
      func(data, this);
      if (spannerSegmentType() == SpannerSegmentType::SINGLE || spannerSegmentType() == SpannerSegmentType::BEGIN) {
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
      _trillType = Type::TRILL_LINE;
      _accidental = 0;
      _ornamentStyle    = MScore::OrnamentStyle::DEFAULT;
      setPlayArticulation(true);
      setPlacement(Element::Placement::ABOVE);
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
            _accidental = static_cast<Accidental*>(e);
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
      TrillSegment* ls = static_cast<TrillSegment*>(frontSegment());
#if 0
// this is now handled differently, in SLine::linePos
      //
      // special case:
      // if end segment is first chord/rest segment in measure,
      // shorten trill line so it ends at end of previous measure
      //
      qreal _spatium = spatium();
      Segment* seg1  = startSegment();
      Segment* seg2  = endSegment();
      if (seg1
         && seg2
         && (seg1->system() == seg2->system())
         && (spannerSegments().size() == 1)
         && (seg2->tick() == seg2->measure()->tick())
         ) {
            qreal x1   = seg2->pagePos().x();
            Measure* m = seg2->measure()->prevMeasure();
            if (m) {
                  Segment* s2      = m->last();
                  qreal x2         = s2->pagePos().x();
                  qreal dx         = x1 - x2 + _spatium * .3;
                  ls->setPos2(ls->ipos2() + QPointF(-dx, 0.0));
                  ls->layout();
                  }
            }
#endif
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
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      xml.tag("subtype", trillTypeName());
      writeProperty(xml, P_ID::PLAY);
      writeProperty(xml, P_ID::ORNAMENT_STYLE);
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

      e.addSpanner(e.intAttribute("id", -1), this);
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
                  setProperty(P_ID::ORNAMENT_STYLE, Ms::getProperty(P_ID::ORNAMENT_STYLE, e));
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
      if (s == "trill" || s == "0")
            _trillType = Type::TRILL_LINE;
      else if (s == "upprall")
            _trillType = Type::UPPRALL_LINE;
      else if (s == "downprall")
            _trillType = Type::DOWNPRALL_LINE;
      else if (s == "prallprall")
            _trillType = Type::PRALLPRALL_LINE;
      else if (s == "pure")
            _trillType = Type::PRALLPRALL_LINE; // obsolete, compatibility only
      else
            qDebug("Trill::setSubtype: unknown <%s>", qPrintable(s));
      }

//---------------------------------------------------------
//   trillTypeName
//---------------------------------------------------------

QString Trill::trillTypeName() const
      {
      switch(trillType()) {
            case Type::TRILL_LINE:
                  return "trill";
            case Type::UPPRALL_LINE:
                  return "upprall";
            case Type::DOWNPRALL_LINE:
                  return "downprall";
            case Type::PRALLPRALL_LINE:
                  return "prallprall";
            default:
                  qDebug("unknown Trill subtype %d", int(trillType()));
                  return "?";
            }
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

QVariant Trill::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::TRILL_TYPE:
                  return int(trillType());
            case P_ID::ORNAMENT_STYLE:
                  return int(ornamentStyle());
            case P_ID::PLAY:
                  return bool(playArticulation());
            default:
                  break;
            }
      return SLine::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Trill::setProperty(P_ID propertyId, const QVariant& val)
      {
      switch(propertyId) {
            case P_ID::TRILL_TYPE:
                  setTrillType(Type(val.toInt()));
                  break;
            case P_ID::PLAY:
                  setPlayArticulation(val.toBool());
                  break;
            case P_ID::ORNAMENT_STYLE:
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

QVariant Trill::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::TRILL_TYPE:
                  return 0;
            case P_ID::ORNAMENT_STYLE:
                  //return int(score()->style()->ornamentStyle(_ornamentStyle));
                  return int(MScore::OrnamentStyle::DEFAULT);
            case P_ID::PLAY:
                  return true;
            case P_ID::PLACEMENT:
                  return int(Element::Placement::ABOVE);
            default:
                  return SLine::propertyDefault(propertyId);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   undoSetTrillType
//---------------------------------------------------------

void Trill::undoSetTrillType(Type val)
      {
      undoChangeProperty(P_ID::TRILL_TYPE, int(val));
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Trill::setYoff(qreal val)
      {
      rUserYoffset() += val * spatium() - score()->styleP(StyleIdx::trillPosAbove);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Trill::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(trillTypeUserName());
      }
}

