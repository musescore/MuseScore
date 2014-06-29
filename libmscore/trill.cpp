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

namespace Ms {

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
      if (e->type() == Element::Type::ACCIDENTAL) {
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
      _symbols.append(f->toString(start));
      qreal w1 = f->bbox(start, mag).width();
      qreal w2 = f->width(fill, mag);
      int n    = lrint((w - w1) / w2);
      for (int i = 0; i < n; ++i)
           _symbols.append(f->toString(fill));
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
      _symbols.append(f->toString(start));
      _symbols.append(f->toString(end));
      qreal w1 = f->bbox(start, mag).width();
      qreal w2 = f->width(fill, mag);
      qreal w3 = f->width(end, mag);
      int n    = lrint((w - w1 - w3) / w2);
      for (int i = 0; i < n; ++i)
           _symbols.insert(1, f->toString(fill));
      QRectF r(f->bbox(_symbols, mag));
      setbbox(r);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TrillSegment::layout()
      {
      if (parent())
            rypos() += score()->styleS(StyleIdx::trillY).val() * spatium();
      if (spannerSegmentType() == SpannerSegmentType::SINGLE || spannerSegmentType() == SpannerSegmentType::BEGIN) {
            Accidental* a = trill()->accidental();
            if (a) {
                  a->layout();
                  a->setMag(a->mag() * .6);
                  qreal _spatium = spatium();
                  a->setPos(_spatium * 1.3, -2.2 * _spatium);
                  a->adjustReadPos();
                  }
            switch (trill()->trillType()) {
                  case Trill::Type::TRILL_LINE:
                        symbolLine(SymId::ornamentTrill, SymId::wiggleTrill);
                        break;
                  case Trill::Type::PRALLPRALL_LINE:
                  case Trill::Type::PURE_LINE:
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
      adjustReadPos();
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TrillSegment::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      if (e->type() == Element::Type::ACCIDENTAL)
            return true;
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* TrillSegment::drop(const DropData& data)
      {
      Element* e = data.element;
      switch(e->type()) {
            case Element::Type::ACCIDENTAL:
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
      if (e->type() == Element::Type::ACCIDENTAL) {
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
      qreal _spatium = spatium();

      SLine::layout();
      if (score() == gscore)
            return;
      TrillSegment* ls = static_cast<TrillSegment*>(frontSegment());
      //
      // special case:
      // if end segment is first chord/rest segment in measure,
      // shorten trill line so it ends at end of previous measure
      //
      Segment* seg1  = startSegment();
      Segment* seg2  = endSegment();
      if (seg2
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

void Trill::write(Xml& xml) const
      {
      if (!xml.canWrite(this)) return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      xml.tag("subtype", trillTypeName());
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
            _trillType = Type::PURE_LINE;
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
            case Type::PURE_LINE:
                  return "pure";
            default:
                  qDebug("unknown Trill subtype %hhd", trillType());
                  return "?";
            }
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
            default:
                  if (!SLine::setProperty(propertyId, val))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
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
      score()->undoChangeProperty(this, P_ID::TRILL_TYPE, int(val));
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Trill::setYoff(qreal val)
      {
      rUserYoffset() += (val - score()->styleS(StyleIdx::trillY).val()) * spatium();
      }
}

