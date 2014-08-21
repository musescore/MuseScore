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

namespace Ms {

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void HairpinSegment::layout()
      {
      QTransform t;
      qreal _spatium = spatium();
      qreal h1 = hairpin()->hairpinHeight().val() * spatium() * .5;
      qreal h2 = hairpin()->hairpinContHeight().val() * spatium() * .5;

      qreal len;
      qreal x = pos2().x();
      if (x < _spatium)             // minimum size of hairpin
            x = _spatium;
      qreal y = pos2().y();
      len     = sqrt(x * x + y * y);
      t.rotateRadians(asin(y/len));

      drawCircledTip =  hairpin()->hairpinCircledTip();
      circledTipRadius = 0;
      if( drawCircledTip )
        circledTipRadius  = 0.6 * _spatium * .5;
      if (hairpin()->hairpinType() == Hairpin::Type::CRESCENDO) {
            // crescendo
            switch (spannerSegmentType()) {
                  case SpannerSegmentType::SINGLE:
                  case SpannerSegmentType::BEGIN:
                        l1.setLine(.0 + circledTipRadius*2, .0, len, h1);
                        l2.setLine(.0 + circledTipRadius*2, .0, len, -h1);
                        circledTip.setX( 0 + circledTipRadius );
                        circledTip.setY( 0 );
                        break;
                  case SpannerSegmentType::MIDDLE:
                  case SpannerSegmentType::END:
                        drawCircledTip = false;
                        l1.setLine(.0,  h2, len, h1);
                        l2.setLine(.0, -h2, len, -h1);
                        break;
                  }
            }
      else {
            // decrescendo
            switch(spannerSegmentType()) {
                  case SpannerSegmentType::SINGLE:
                  case SpannerSegmentType::END:
                        l1.setLine(.0,  h1, len - circledTipRadius*2, 0.0);
                        l2.setLine(.0, -h1, len - circledTipRadius*2, 0.0);
                        circledTip.setX( len - circledTipRadius );
                        circledTip.setY( 0 );
                        break;
                  case SpannerSegmentType::BEGIN:
                  case SpannerSegmentType::MIDDLE:
                        drawCircledTip = false;
                        l1.setLine(.0,  h1, len, + h2);
                        l2.setLine(.0, -h1, len, - h2);
                        break;
                  }
            }
// Do Coord rotation
      l1 = t.map(l1);
      l2 = t.map(l2);
      if( drawCircledTip )
        circledTip = t.map(circledTip);


      QRectF r = QRectF(l1.p1(), l1.p2()).normalized() | QRectF(l2.p1(), l2.p2()).normalized();
      qreal w = point(score()->styleS(StyleIdx::hairpinLineWidth));
      setbbox(r.adjusted(-w*.5, -w*.5, w, w));
      if (parent())
            rypos() += score()->styleS(StyleIdx::hairpinY).val() * _spatium;
      adjustReadPos();
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void HairpinSegment::updateGrips(int* grips, int* defaultGrip, QRectF* grip) const
      {
      *grips = 4;
      *defaultGrip = 2;

      QPointF pp(pagePos());
      qreal _spatium = spatium();
      qreal x = pos2().x();
      if (x < _spatium)             // minimum size of hairpin
            x = _spatium;
      qreal y = pos2().y();
      QPointF p(x, y);

// Calc QPointF for Grip Aperture
      QTransform doRotation;
      QPointF gripLineAperturePoint;
      qreal h1 = hairpin()->hairpinHeight().val() * spatium() * .5;
      qreal len = sqrt( x * x + y * y );
      doRotation.rotateRadians( asin(y/len) );
      qreal lineApertureX;
      qreal offsetX = 10;                               // Horizontal offset for x Grip
      if(len < offsetX * 3 )                            // For small hairpin, offset = 30% of len
          offsetX = len/3;                              // else offset is fixed to 10

      if( hairpin()->hairpinType() == Hairpin::Type::CRESCENDO )
            lineApertureX = len - offsetX;              // End of CRESCENDO - Offset
        else
            lineApertureX = offsetX;                    // Begin of DECRESCENDO + Offset
      qreal lineApertureH = ( len - offsetX ) * h1/len; // Vertical position for y grip
      gripLineAperturePoint.setX( lineApertureX );
      gripLineAperturePoint.setY( lineApertureH );
      gripLineAperturePoint = doRotation.map( gripLineAperturePoint );
// End calc position grip aperture

      grip[int(GripLine::START)].translate( pp );
      grip[int(GripLine::END)].translate( p + pp );
      grip[int(GripLine::MIDDLE)].translate( p * .5 + pp );
      grip[int(GripLine::APERTURE)].translate( gripLineAperturePoint + pp );
      }
//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void HairpinSegment::editDrag(const EditData& ed)
      {

    if( ed.curGrip == int(GripLine::APERTURE) ){
          qreal newHeight = hairpin()->hairpinHeight().val() + ed.delta.y()/spatium()/.5;
          if( newHeight < 0.5 )
              newHeight = 0.5;
          hairpin()->setHairpinHeight(Spatium(newHeight));
          score()->setLayoutAll(true);
    }

    LineSegment::editDrag( ed );

      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void HairpinSegment::draw(QPainter* painter) const
      {
      QColor color;
      if (selected() && !(score() && score()->printing()))
            color = MScore::selectColor[0];
      else if (!visible())
            color = Qt::gray;
      else
            color = hairpin()->curColor();
      QPen pen(color, point(hairpin()->lineWidth()), hairpin()->lineStyle());
      painter->setPen(pen);
      painter->drawLine(l1);
      painter->drawLine(l2);
      if( drawCircledTip ) {
          painter->setBrush(Qt::NoBrush);
          painter->drawEllipse( circledTip,circledTipRadius,circledTipRadius );
      }

      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant HairpinSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::HAIRPIN_CIRCLEDTIP:
            case P_ID::HAIRPIN_TYPE:
            case P_ID::VELO_CHANGE:
            case P_ID::DYNAMIC_RANGE:
            case P_ID::DIAGONAL:
            case P_ID::HAIRPIN_HEIGHT:
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return hairpin()->getProperty(id);
            default:
                  return LineSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool HairpinSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::HAIRPIN_CIRCLEDTIP:
            case P_ID::HAIRPIN_TYPE:
            case P_ID::VELO_CHANGE:
            case P_ID::DYNAMIC_RANGE:
            case P_ID::DIAGONAL:
            case P_ID::LINE_WIDTH:
            case P_ID::HAIRPIN_HEIGHT:
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return hairpin()->setProperty(id, v);
            default:
                  return LineSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant HairpinSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::HAIRPIN_CIRCLEDTIP:
            case P_ID::HAIRPIN_TYPE:
            case P_ID::VELO_CHANGE:
            case P_ID::DYNAMIC_RANGE:
            case P_ID::DIAGONAL:
            case P_ID::HAIRPIN_HEIGHT:
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return hairpin()->propertyDefault(id);
            default:
                  return LineSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle HairpinSegment::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::HAIRPIN_HEIGHT:
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return hairpin()->propertyStyle(id);

            default:
                  return LineSegment::propertyStyle(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void HairpinSegment::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::HAIRPIN_HEIGHT:
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return hairpin()->resetProperty(id);

            default:
                  return LineSegment::resetProperty(id);
            }
      }


//---------------------------------------------------------
//   Hairpin
//---------------------------------------------------------

Hairpin::Hairpin(Score* s)
   : SLine(s)
      {
      _hairpinType = Type::CRESCENDO;
      _hairpinCircledTip = false;
      _veloChange  = 10;
      _dynRange    = Dynamic::Range::PART;
      setLineWidth(score()->styleS(StyleIdx::hairpinLineWidth));
      lineWidthStyle         = PropertyStyle::STYLED;
      _hairpinHeight         = score()->styleS(StyleIdx::hairpinHeight);
      hairpinHeightStyle     = PropertyStyle::STYLED;
      _hairpinContHeight     = score()->styleS(StyleIdx::hairpinContHeight);
      hairpinContHeightStyle = PropertyStyle::STYLED;
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
      if (!xml.canWrite(this))
            return;
      int id = xml.spannerId(this);
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id));
      xml.tag("subtype", int(_hairpinType));
      xml.tag("veloChange", _veloChange);
      writeProperty(xml, P_ID::HAIRPIN_CIRCLEDTIP);
      writeProperty(xml, P_ID::DYNAMIC_RANGE);
      writeProperty(xml, P_ID::PLACEMENT);
      writeProperty(xml, P_ID::HAIRPIN_HEIGHT);
      writeProperty(xml, P_ID::HAIRPIN_CONT_HEIGHT);
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

      int id = e.intAttribute("id", -1);
      e.addSpanner(id, this);

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  _hairpinType = Type(e.readInt());
            else if (tag == "lineWidth") {
                  setLineWidth(Spatium(e.readDouble()));
                  lineWidthStyle = PropertyStyle::UNSTYLED;
                  }
            else if (tag == "hairpinHeight") {
                  setHairpinHeight(Spatium(e.readDouble()));
                  hairpinHeightStyle = PropertyStyle::UNSTYLED;
                  }
            else if (tag == "hairpinContHeight") {
                  setHairpinContHeight(Spatium(e.readDouble()));
                  hairpinContHeightStyle = PropertyStyle::UNSTYLED;
                  }
            else if (tag == "hairpinCircledTip")
                  _hairpinCircledTip = e.readInt();
            else if (tag == "veloChange")
                  _veloChange = e.readInt();
            else if (tag == "dynType")
                  _dynRange = Dynamic::Range(e.readInt());
            else if (!SLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   undoSetHairpinType
//---------------------------------------------------------

void Hairpin::undoSetHairpinType(Type val)
      {
      score()->undoChangeProperty(this, P_ID::HAIRPIN_TYPE, int(val));
      }

//---------------------------------------------------------
//   undoSetVeloChange
//---------------------------------------------------------

void Hairpin::undoSetVeloChange(int val)
      {
      score()->undoChangeProperty(this, P_ID::VELO_CHANGE, val);
      }

//---------------------------------------------------------
//   undoSetDynType
//---------------------------------------------------------

void Hairpin::undoSetDynRange(Dynamic::Range val)
      {
      score()->undoChangeProperty(this, P_ID::DYNAMIC_RANGE, int(val));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Hairpin::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::HAIRPIN_CIRCLEDTIP:
                return _hairpinCircledTip;
            case P_ID::HAIRPIN_TYPE:
                return int(_hairpinType);
            case P_ID::VELO_CHANGE:
                  return _veloChange;
            case P_ID::DYNAMIC_RANGE:
                  return int(_dynRange);
            case P_ID::HAIRPIN_HEIGHT:
                  return _hairpinHeight.val();
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return _hairpinContHeight.val();
            default:
                  return SLine::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Hairpin::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::HAIRPIN_CIRCLEDTIP:
                _hairpinCircledTip = v.toBool();
                break;
            case P_ID::HAIRPIN_TYPE:
                  _hairpinType = Type(v.toInt());
                  setGenerated(false);
                  break;
            case P_ID::VELO_CHANGE:
                  _veloChange = v.toInt();
                  break;
            case P_ID::DYNAMIC_RANGE:
                  _dynRange = Dynamic::Range(v.toInt());
                  break;
            case P_ID::LINE_WIDTH:
                  lineWidthStyle = PropertyStyle::UNSTYLED;
                  SLine::setProperty(id, v);
                  break;
            case P_ID::HAIRPIN_HEIGHT:
                  hairpinHeightStyle = PropertyStyle::UNSTYLED;
                  _hairpinHeight = Spatium(v.toDouble());
                  break;
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  hairpinContHeightStyle = PropertyStyle::UNSTYLED;
                  _hairpinContHeight = Spatium(v.toDouble());
                  break;
            default:
                  return SLine::setProperty(id, v);
            }
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Hairpin::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::HAIRPIN_CIRCLEDTIP:  return false;
            case P_ID::HAIRPIN_TYPE:        return int(Type::CRESCENDO);
            case P_ID::VELO_CHANGE:         return 10;
            case P_ID::DYNAMIC_RANGE:       return int(Dynamic::Range::PART);
            case P_ID::LINE_WIDTH:          return score()->styleS(StyleIdx::hairpinLineWidth).val();
            case P_ID::HAIRPIN_HEIGHT:      return score()->styleS(StyleIdx::hairpinHeight).val();
            case P_ID::HAIRPIN_CONT_HEIGHT: return score()->styleS(StyleIdx::hairpinContHeight).val();

            default:
                  return SLine::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle Hairpin::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:            return lineWidthStyle;
            case P_ID::HAIRPIN_HEIGHT:        return hairpinHeightStyle;
            case P_ID::HAIRPIN_CONT_HEIGHT:   return hairpinContHeightStyle;
            default:
                  return SLine::propertyStyle(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Hairpin::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
                  setLineWidth(score()->styleS(StyleIdx::hairpinLineWidth));
                  lineWidthStyle = PropertyStyle::STYLED;
                  break;

            case P_ID::HAIRPIN_HEIGHT:
                  setHairpinHeight(score()->styleS(StyleIdx::hairpinHeight));
                  hairpinHeightStyle = PropertyStyle::STYLED;
                  break;

            case P_ID::HAIRPIN_CONT_HEIGHT:
                  setHairpinContHeight(score()->styleS(StyleIdx::hairpinContHeight));
                  hairpinContHeightStyle = PropertyStyle::STYLED;
                  break;

            default:
                  return SLine::resetProperty(id);
            }
      score()->setLayoutAll(true);
      }


//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Hairpin::setYoff(qreal val)
      {
      rUserYoffset() += (val - score()->styleS(StyleIdx::hairpinY).val()) * spatium();
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Hairpin::styleChanged()
      {
      if (lineWidthStyle == PropertyStyle::STYLED)
            setLineWidth(score()->styleS(StyleIdx::hairpinLineWidth));
      if (hairpinHeightStyle == PropertyStyle::STYLED)
            setHairpinHeight(score()->styleS(StyleIdx::hairpinHeight));
      if (hairpinContHeightStyle == PropertyStyle::STYLED)
            setHairpinContHeight(score()->styleS(StyleIdx::hairpinContHeight));
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Hairpin::reset()
      {
      if (lineWidthStyle == PropertyStyle::UNSTYLED)
            score()->undoChangeProperty(this, P_ID::LINE_WIDTH, propertyDefault(P_ID::LINE_WIDTH), PropertyStyle::STYLED);
      if (hairpinHeightStyle == PropertyStyle::UNSTYLED)
            score()->undoChangeProperty(this, P_ID::HAIRPIN_HEIGHT, propertyDefault(P_ID::HAIRPIN_HEIGHT), PropertyStyle::STYLED);
      if (hairpinContHeightStyle == PropertyStyle::UNSTYLED)
            score()->undoChangeProperty(this, P_ID::HAIRPIN_CONT_HEIGHT, propertyDefault(P_ID::HAIRPIN_CONT_HEIGHT), PropertyStyle::STYLED);
      SLine::reset();
      }

QString Hairpin::accessibleInfo()
      {
      QString rez = SLine::accessibleInfo();
      switch (hairpinType()) {
            case Type::CRESCENDO:
                  rez += ": " + tr("Crescendo");
                  break;
            case Type::DECRESCENDO:
                  rez += ": " + tr("Descrescendo");
                  break;
            default:
                  rez += ": " + tr("Custom");
            }
      return rez;
      }
}

