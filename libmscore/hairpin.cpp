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

Spatium Hairpin::editHairpinHeight;

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void HairpinSegment::layout()
      {
      if (hairpin()->useTextLine()) {
            // layout as textline rather than true hairpin
            // use dynamics text style for position, so the text aligns with dynamics
            // TODO: new style setting specifically for vertical offset of textline hairpins?
            // or, use hairpinY but adjust by 0.5sp, which currently yields same vertical position as dynamics
            if (parent())
                  rypos() += score()->textStyle(TextStyleType::DYNAMICS).offset(spatium()).y();
            TextLineSegment::layout();
            return;
            }

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

void HairpinSegment::updateGrips(Grip* defaultGrip, QVector<QRectF>& grip) const
      {
      *defaultGrip = Grip::END;

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

      grip[int(Grip::START)].translate( pp );
      grip[int(Grip::END)].translate( p + pp );
      grip[int(Grip::MIDDLE)].translate( p * .5 + pp );
      grip[int(Grip::APERTURE)].translate( gripLineAperturePoint + pp );
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void HairpinSegment::editDrag(const EditData& ed)
      {
      if (ed.curGrip == Grip::APERTURE) {
            qreal newHeight = hairpin()->hairpinHeight().val() + ed.delta.y()/spatium()/.5;
            if (newHeight < 0.5)
                  newHeight = 0.5;
            hairpin()->setHairpinHeight(Spatium(newHeight));
            triggerLayout();
            }
      LineSegment::editDrag(ed);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void HairpinSegment::draw(QPainter* painter) const
      {
      if (hairpin()->useTextLine()) {
            TextLineSegment::draw(painter);
            return;
            }

      QColor color;
      if (selected() && !(score() && score()->printing()))
            color = (track() > -1) ? MScore::selectColor[voice()] : MScore::selectColor[0];
      else if (!hairpin()->visible())     // || !hairpin()->lineVisible()
            color = Qt::gray;
      else
            color = hairpin()->lineColor();
      QPen pen(color, point(hairpin()->lineWidth()), hairpin()->lineStyle());
      if (hairpin()->lineStyle() == Qt::CustomDashLine) {
            QVector<qreal> pattern;
            pattern << 5.0 << 20.0;
            pen.setDashPattern(pattern);
            }

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
            case P_ID::HAIRPIN_TEXTLINE:
            case P_ID::HAIRPIN_CIRCLEDTIP:
            case P_ID::HAIRPIN_TYPE:
            case P_ID::VELO_CHANGE:
            case P_ID::DYNAMIC_RANGE:
            case P_ID::DIAGONAL:
            case P_ID::HAIRPIN_HEIGHT:
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return hairpin()->getProperty(id);
            default:
                  return TextLineSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool HairpinSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::HAIRPIN_TEXTLINE:
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
                  return TextLineSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant HairpinSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::HAIRPIN_TEXTLINE:
            case P_ID::TEXT_STYLE_TYPE:
            case P_ID::HAIRPIN_CIRCLEDTIP:
            case P_ID::HAIRPIN_TYPE:
            case P_ID::VELO_CHANGE:
            case P_ID::DYNAMIC_RANGE:
            case P_ID::DIAGONAL:
            case P_ID::HAIRPIN_HEIGHT:
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return hairpin()->propertyDefault(id);
            default:
                  return TextLineSegment::propertyDefault(id);
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
                  return TextLineSegment::propertyStyle(id);
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
                  return TextLineSegment::resetProperty(id);
            }
      }


//---------------------------------------------------------
//   Hairpin
//---------------------------------------------------------

Hairpin::Hairpin(Score* s)
   : TextLine(s)
      {
      _hairpinType = Type::CRESCENDO;
      _useTextLine = false;
      _hairpinCircledTip = false;
      _veloChange  = 0;
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
      TextLine::layout();
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
      if (_useTextLine)
            xml.tag("useTextLine", true);
      writeProperty(xml, P_ID::VELO_CHANGE);
      writeProperty(xml, P_ID::HAIRPIN_CIRCLEDTIP);
      writeProperty(xml, P_ID::DYNAMIC_RANGE);
      writeProperty(xml, P_ID::PLACEMENT);
      writeProperty(xml, P_ID::HAIRPIN_HEIGHT);
      writeProperty(xml, P_ID::HAIRPIN_CONT_HEIGHT);
      TextLine::writeProperties(xml);
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
            else if (tag == "useTextLine")
                  _useTextLine = e.readInt();
            else if (!TextLine::readProperties(e))
                  e.unknown();
            }

      // add default text to legacy hairpins
      if (score()->mscVersion() <= 206 && !_useTextLine) {
            bool cresc = _hairpinType == Hairpin::Type::CRESCENDO;
            if (!_beginText)
                  setBeginText(cresc ? "cresc." : "dim.");

            if (!_continueText)
                  setContinueText(cresc ? "(cresc.)" : "(dim.)");
            }
      }

//---------------------------------------------------------
//   undoSetHairpinType
//---------------------------------------------------------

void Hairpin::undoSetHairpinType(Type val)
      {
      undoChangeProperty(P_ID::HAIRPIN_TYPE, int(val));
      }

//---------------------------------------------------------
//   undoSetVeloChange
//---------------------------------------------------------

void Hairpin::undoSetVeloChange(int val)
      {
      undoChangeProperty(P_ID::VELO_CHANGE, val);
      }

//---------------------------------------------------------
//   undoSetDynType
//---------------------------------------------------------

void Hairpin::undoSetDynRange(Dynamic::Range val)
      {
      undoChangeProperty(P_ID::DYNAMIC_RANGE, int(val));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Hairpin::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::HAIRPIN_TEXTLINE:
                return _useTextLine;
            case P_ID::HAIRPIN_CIRCLEDTIP:
                return _hairpinCircledTip;
            case P_ID::HAIRPIN_TYPE:
                return int(_hairpinType);
            case P_ID::VELO_CHANGE:
                  return _veloChange;
            case P_ID::DYNAMIC_RANGE:
                  return int(_dynRange);
            case P_ID::HAIRPIN_HEIGHT:
                  return _hairpinHeight;
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return _hairpinContHeight;
            default:
                  return TextLine::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Hairpin::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::HAIRPIN_TEXTLINE:
                _useTextLine = v.toBool();
                break;
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
                  TextLine::setProperty(id, v);
                  break;
            case P_ID::HAIRPIN_HEIGHT:
                  hairpinHeightStyle = PropertyStyle::UNSTYLED;
                  _hairpinHeight = v.value<Spatium>();
                  break;
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  hairpinContHeightStyle = PropertyStyle::UNSTYLED;
                  _hairpinContHeight = v.value<Spatium>();
                  break;
            default:
                  return TextLine::setProperty(id, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Hairpin::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::HAIRPIN_TEXTLINE:    return _useTextLine;  // HACK: treat current setting as default
            case P_ID::TEXT_STYLE_TYPE:     return int(TextStyleType::HAIRPIN);
            case P_ID::HAIRPIN_CIRCLEDTIP:  return false;
            case P_ID::HAIRPIN_TYPE:        return int(Type::CRESCENDO);
            case P_ID::VELO_CHANGE:         return 0;
            case P_ID::DYNAMIC_RANGE:       return int(Dynamic::Range::PART);
            case P_ID::LINE_WIDTH:          return score()->style(StyleIdx::hairpinLineWidth);
            case P_ID::HAIRPIN_HEIGHT:      return score()->style(StyleIdx::hairpinHeight);
            case P_ID::HAIRPIN_CONT_HEIGHT: return score()->style(StyleIdx::hairpinContHeight);
            case P_ID::LINE_STYLE:          return _useTextLine ? int(Qt::CustomDashLine) : int(Qt::SolidLine);

            default:
                  return TextLine::propertyDefault(id);
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
                  return TextLine::propertyStyle(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Hairpin::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
                  setProperty(id, propertyDefault(id));
                  lineWidthStyle = PropertyStyle::STYLED;
                  break;

            case P_ID::HAIRPIN_HEIGHT:
                  setProperty(id, propertyDefault(id));
                  hairpinHeightStyle = PropertyStyle::STYLED;
                  break;

            case P_ID::HAIRPIN_CONT_HEIGHT:
                  setLineWidth(score()->styleS(StyleIdx::hairpinLineWidth));
                  hairpinContHeightStyle = PropertyStyle::STYLED;
                  break;

            default:
                  return TextLine::resetProperty(id);
            }
      triggerLayout();
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
            undoChangeProperty(P_ID::LINE_WIDTH, propertyDefault(P_ID::LINE_WIDTH), PropertyStyle::STYLED);
      if (hairpinHeightStyle == PropertyStyle::UNSTYLED)
            undoChangeProperty(P_ID::HAIRPIN_HEIGHT, propertyDefault(P_ID::HAIRPIN_HEIGHT), PropertyStyle::STYLED);
      if (hairpinContHeightStyle == PropertyStyle::UNSTYLED)
            undoChangeProperty(P_ID::HAIRPIN_CONT_HEIGHT, propertyDefault(P_ID::HAIRPIN_CONT_HEIGHT), PropertyStyle::STYLED);
      TextLine::reset();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Hairpin::accessibleInfo() const
      {
      QString rez = TextLine::accessibleInfo();
      switch (hairpinType()) {
            case Type::CRESCENDO:
                  rez += ": " + tr("Crescendo");
                  break;
            case Type::DECRESCENDO:
                  rez += ": " + tr("Decrescendo");
                  break;
            default:
                  rez += ": " + tr("Custom");
            }
      return rez;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Hairpin::startEdit(MuseScoreView* view, const QPointF& p)
      {
      editHairpinHeight = _hairpinHeight;
      TextLine::startEdit(view, p);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Hairpin::endEdit()
      {
      if (editHairpinHeight != _hairpinHeight)
            score()->undoPropertyChanged(this, P_ID::HAIRPIN_HEIGHT, editHairpinHeight);
      TextLine::endEdit();
      }

}

