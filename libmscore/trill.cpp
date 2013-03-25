//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: trill.cpp 5305 2012-02-09 12:09:20Z wschweer $
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

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TrillSegment::draw(QPainter* painter) const
      {
      qreal mag  = magS();
      int idx    = score()->symIdx();
      QRectF b2(symbols[idx][trillelementSym].bbox(mag));
      qreal w2   = symbols[idx][trillelementSym].width(mag);

      qreal x2   = pos2().x();

      painter->setPen(curColor());
      if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN) {
            int sym = 0;
            qreal x0 = 0.0, x1 = 0.0, y = 0.0;
            int n = 0;
            QRectF b1;

            switch(trill()->trillType()) {
                  case Trill::TRILL_LINE:
                        sym  = trillSym;
                        b1   = symbols[idx][sym].bbox(mag);
                        x0   = -b1.x();
                        x1   = x0 + b1.width();
                        n    = int(floor((x2-x1) / w2));
                        y    = 0.0;
                        break;

                  case Trill::UPPRALL_LINE:
                        sym  = upprallSym;
                        b1   = symbols[idx][sym].bbox(mag);
                        x0   = -b1.x();
                        x1   = b1.width();
                        n    = int(floor((x2-x1) / w2));
                        y    = -b1.height();
                        break;
                  case Trill::DOWNPRALL_LINE:
                        sym  = downprallSym;
                        b1   = symbols[idx][sym].bbox(mag);
                        x0   = -b1.x();
                        x1   = b1.width();
                        n    = int(floor((x2-x1) / w2));
                        y    = -b1.height();
                        break;
                  case Trill::PRALLPRALL_LINE:
                        sym  = prallprallSym;
                        b1   = symbols[idx][sym].bbox(mag);
                        x0   = -b1.x();
                        x1   = b1.width();
                        n    = int(floor((x2-x1) / w2));
                        y    = -b1.height();
                        break;
                  case Trill::PURE_LINE:
                        sym = noSym;
                        x0 = 0;
                        x1 = 0;
                        n    = int(floor((x2-x1) / w2));
                        y = 0.0;
                  }
            if (n <= 0)
                  n = 1;
            if (sym != noSym)
                  symbols[idx][sym].draw(painter, mag, QPointF(x0, y));
            symbols[idx][trillelementSym].draw(painter, mag,  QPointF(x1, b2.y() * .9), n);
            }
      else {
            qreal x1 = 0.0;
            int n = int(floor((x2-x1) / w2));
            symbols[idx][trillelementSym].draw(painter, mag,  QPointF(x1, b2.y() * .9), n);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TrillSegment::layout()
      {
      QRectF b1(symbols[score()->symIdx()][trillSym].bbox(magS()));
      QRectF rr(b1.translated(-b1.x(), 0.0));
      rr |= QRectF(0.0, rr.y(), pos2().x(), rr.height());
      setbbox(rr);
      if (parent())
            rypos() += score()->styleS(ST_trillY).val() * spatium();
      adjustReadPos();
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TrillSegment::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      if (e->type() == ACCIDENTAL)
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
            case ACCIDENTAL:
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
//   Trill
//---------------------------------------------------------

Trill::Trill(Score* s)
  : SLine(s)
      {
      _trillType = TRILL_LINE;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Trill::add(Element* e)
      {
      if (e->type() == ACCIDENTAL) {
            e->setParent(this);
            _el.push_back(e);
            }
      else
            SLine::add(e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Trill::remove(Element* e)
      {
      if (!_el.remove(e))
            Spanner::remove(e);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Trill::layout()
      {
      qreal _spatium = spatium();
//      setPos(0.0, yoff() * _spatium);

      SLine::layout();

      //
      // special case:
      // if end segment is first chord/rest segment in measure,
      // shorten trill line so it ends at end of previous measure
      //
      Segment* seg1  = static_cast<Segment*>(startElement());
      Segment* seg2  = static_cast<Segment*>(endElement());
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
                  TrillSegment* ls = static_cast<TrillSegment*>(frontSegment());
                  ls->setPos2(ls->ipos2() + QPointF(-dx, 0.0));
                  ls->layout();
                  }
            }
      foreach(Element* e, _el) {
            e->setMag(.6);
            e->layout();
            e->setPos(_spatium*1.3, -2.2*_spatium);
            e->adjustReadPos();
            }
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
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      xml.tag("subtype", trillTypeName());
      SLine::writeProperties(xml);
      foreach(Element* e, _el)
            e->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Trill::read
//---------------------------------------------------------

void Trill::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();

      setId(e.intAttribute("id", -1));
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setTrillType(e.readElementText());
            else if (tag == "Accidental") {
                  Accidental* a = new Accidental(score());
                  a->read(e);
                  add(a);
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
            _trillType = TRILL_LINE;
      else if (s == "upprall")
            _trillType = UPPRALL_LINE;
      else if (s == "downprall")
            _trillType = DOWNPRALL_LINE;
      else if (s == "prallprall")
            _trillType = PRALLPRALL_LINE;
      else if (s == "pure")
            _trillType = PURE_LINE;
      else
            qDebug("Trill::setSubtype: unknown <%s>", qPrintable(s));
      }

//---------------------------------------------------------
//   trillTypeName
//---------------------------------------------------------

QString Trill::trillTypeName() const
      {
      switch(trillType()) {
            case TRILL_LINE:
                  return "trill";
            case UPPRALL_LINE:
                  return "upprall";
            case DOWNPRALL_LINE:
                  return "downprall";
            case PRALLPRALL_LINE:
                  return "prallprall";
            case PURE_LINE:
                  return "pure";
            default:
                  qDebug("unknown Trill subtype %d", trillType());
                  return "?";
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Trill::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      foreach(Element* e, _el)
            e->scanElements(data, func, all);
      func(data, this);
      SLine::scanElements(data, func, all);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Trill::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_TRILL_TYPE:
                  return trillType();
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
            case P_TRILL_TYPE:
                  setTrillType(TrillType(val.toInt()));
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
            case P_TRILL_TYPE:
                  return 0;
            default:
                  return SLine::propertyDefault(propertyId);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   undoSetTrillType
//---------------------------------------------------------

void Trill::undoSetTrillType(TrillType val)
      {
      score()->undoChangeProperty(this, P_TRILL_TYPE, val);
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Trill::setYoff(qreal val)
      {
      rUserYoffset() += (val - score()->styleS(ST_trillY).val()) * spatium();
      }


