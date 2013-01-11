//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: hairpin.cpp 5305 2012-02-09 12:09:20Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "hairpin.h"
#include "style.h"
#include "xml.h"
#include "utils.h"
#include "score.h"
#include "measure.h"
#include "segment.h"
#include "system.h"
#include "undo.h"
#include "staff.h"
#include "mscore.h"

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void HairpinSegment::layout()
      {
      QTransform t;
      qreal _spatium = spatium();
      qreal h1 = score()->styleS(ST_hairpinHeight).val() * _spatium * .5;
      qreal h2 = score()->styleS(ST_hairpinContHeight).val() * _spatium * .5;

      qreal len;
      qreal x = pos2().x();
      if (x < _spatium)             // minimum size of hairpin
            x = _spatium;
      qreal y = pos2().y();
      len     = sqrt(x * x + y * y);
      t.rotateRadians(asin(y/len));

      if (hairpin()->subtype() == 0) {
            // crescendo
            switch (subtype()) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_BEGIN:
                        l1.setLine(.0, .0, len, h1);
                        l2.setLine(.0, .0, len, - h1);
                        break;
                  case SEGMENT_MIDDLE:
                  case SEGMENT_END:
                        l1.setLine(.0,  h2, len, h1);
                        l2.setLine(.0, -h2, len, - h1);
                        break;
                  }
            }
      else {
            // decrescendo
            switch(subtype()) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_END:
                        l1.setLine(.0,  h1, len, 0.0);
                        l2.setLine(.0, -h1, len, 0.0);
                        break;
                  case SEGMENT_BEGIN:
                  case SEGMENT_MIDDLE:
                        l1.setLine(.0,  h1, len, + h2);
                        l2.setLine(.0, -h1, len, - h2);
                        break;
                  }
            }
      l1 = t.map(l1);
      l2 = t.map(l2);

      QRectF r = QRectF(l1.p1(), l1.p2()).normalized() | QRectF(l2.p1(), l2.p2()).normalized();
      qreal w = point(score()->styleS(ST_hairpinWidth));
      setbbox(r.adjusted(-w*.5, -w*.5, w, w));
      if (parent())
            rypos() += score()->styleS(ST_hairpinY).val() * _spatium;
      adjustReadPos();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void HairpinSegment::draw(QPainter* painter) const
      {
      QPen pen(curColor(), point(score()->styleS(ST_hairpinWidth)));
      painter->setPen(pen);
      painter->drawLine(l1);
      painter->drawLine(l2);
      }

//---------------------------------------------------------
//   Hairpin
//---------------------------------------------------------

Hairpin::Hairpin(Score* s)
   : SLine(s)
      {
      _subtype     = CRESCENDO;
      _veloChange  = 10;
      _dynRange    = DYNAMIC_PART;
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick() to _tick2
//---------------------------------------------------------

void Hairpin::layout()
      {
      setPos(0.0, 0.0);
      SLine::layout();
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Hairpin::createLineSegment()
      {
      return new HairpinSegment(score());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Hairpin::write(Xml& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      xml.tag("subtype", _subtype);
      xml.tag("veloChange", _veloChange);
      writeProperty(xml, P_DYNAMIC_RANGE);
      writeProperty(xml, P_PLACEMENT);
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Hairpin::read(XmlReader& e)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();

      setId(e.intAttribute("id", -1));
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  _subtype = HairpinType(e.readInt());
            else if (tag == "veloChange")
                  _veloChange = e.readInt();
            else if (tag == "dynType")
                  _dynRange = DynamicRange(e.readInt());
            else if (!SLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   undoSetSubtype
//---------------------------------------------------------

void Hairpin::undoSetSubtype(HairpinType val)
      {
      score()->undoChangeProperty(this, P_HAIRPIN_TYPE, val);
      }

//---------------------------------------------------------
//   undoSetVeloChange
//---------------------------------------------------------

void Hairpin::undoSetVeloChange(int val)
      {
      score()->undoChangeProperty(this, P_VELO_CHANGE, val);
      }

//---------------------------------------------------------
//   undoSetDynType
//---------------------------------------------------------

void Hairpin::undoSetDynRange(DynamicRange val)
      {
      score()->undoChangeProperty(this, P_DYNAMIC_RANGE, val);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Hairpin::getProperty(P_ID id) const
      {
      switch(id) {
            case P_HAIRPIN_TYPE:
                  return _subtype;
            case P_VELO_CHANGE:
                  return _veloChange;
            case P_DYNAMIC_RANGE:
                  return _dynRange;
            default:
                  return SLine::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Hairpin::setProperty(P_ID id, const QVariant& v)
      {
      switch(id) {
            case P_HAIRPIN_TYPE:
                  _subtype = HairpinType(v.toInt());
                  setGenerated(false);
                  break;
            case P_VELO_CHANGE:
                  _veloChange = v.toInt();
                  break;
            case P_DYNAMIC_RANGE:
                  _dynRange = DynamicRange(v.toInt());
                  break;
            default:
                  return SLine::setProperty(id, v);
            }
      return true;
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Hairpin::setYoff(qreal val)
      {
      rUserYoffset() += (val - score()->styleS(ST_hairpinY).val()) * spatium();
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Hairpin::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_DYNAMIC_RANGE: return DYNAMIC_PART;
            default:              return SLine::propertyDefault(id);
            }
      }


