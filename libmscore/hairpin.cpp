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
#include "chord.h"

namespace Ms {

Spatium Hairpin::editHairpinHeight;


//---------------------------------------------------------
//   lookupDynamic
//    return autoplace Dynamic at chord e position
//---------------------------------------------------------

Dynamic* lookupDynamic(Element* e)
      {
      Dynamic* d = 0;
      Segment* s = 0;
      if (e && e->isChord())
            s = toChord(e)->segment();
      if (s) {
            for (Element* ee : s->annotations()) {
                  if (ee->isDynamic() && ee->track() == e->track() && ee->placeBelow()) {
                        d = toDynamic(ee);
                        break;
                        }
                  }
            }
      if (d) {
            if (!d->autoplace())
                  d = 0;
            }
      return d;
      }

//---------------------------------------------------------
//   moveDynamic
//---------------------------------------------------------

static void moveDynamic(Dynamic* d, qreal y)
      {
      if (d && d->autoplace()) {
            int staffIdx = d->staffIdx();
            Shape& ss    = d->segment()->staffShape(staffIdx);
            Shape& ms    = d->measure()->staffShape(staffIdx);
            QPointF spos = d->segment()->pos();

            d->rUserYoffset() = y;
            ss.add(d->shape());
            ms.add(d->shape().translated(spos));
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void HairpinSegment::layout()
      {
      Dynamic* sd = 0;
      Dynamic* ed = 0;
      qreal _spatium = spatium();

      if (autoplace()) {
            setUserOff(QPointF());
            setUserOff2(QPointF());
            }
      if (isSingleType() || isBeginType()) {
            sd = lookupDynamic(hairpin()->startElement());
            if (sd) {
                  if (autoplace()) {
                        qreal dx        = sd->bbox().right() + sd->pos().x()
                                             + sd->segment()->pos().x() + sd->measure()->pos().x();
                        // hardcoded distance between Dynamic and Hairpin: 0.5sp
                        qreal dist      = dx - pos().x() + score()->styleP(StyleIdx::autoplaceHairpinDynamicsDistance);
                        rUserXoffset()  = dist;
                        rUserXoffset2() = -dist;
                        }
                  else
                        sd->doAutoplace();
                  }
            }
      if (isSingleType() || isEndType()) {
            ed = lookupDynamic(hairpin()->endElement());
            if (ed) {
                  if (autoplace()) {
                        rUserXoffset2() -= ed->bbox().width();
                        qreal dx         = ed->bbox().left() + ed->pos().x()
                                           + ed->segment()->pos().x() + ed->measure()->pos().x();
                        // hardcoded distance between Hairpin and Dynamic: 0.5sp
                        ed->rUserXoffset() = pos2().x() + pos().x() - dx + score()->styleP(StyleIdx::autoplaceHairpinDynamicsDistance);
                        }
                  else
                        ed->doAutoplace();
                  }
            }

      HairpinType type = hairpin()->hairpinType();
      if (type == HairpinType::DECRESC_LINE || type == HairpinType::CRESC_LINE) {
            twoLines = false;
            TextLineBaseSegment::layout();
            drawCircledTip = false;
            if (parent())
                  rypos() += score()->styleP(StyleIdx::hairpinY);
            }
      else {
            delete _text;
            delete _endText;
            _text    = 0;
            _endText = 0;

            QTransform t;
            qreal h1 = hairpin()->hairpinHeight().val()     * spatium() * .5;
            qreal h2 = hairpin()->hairpinContHeight().val() * spatium() * .5;

            qreal len;
            qreal x = pos2().x();
            if (x < _spatium)             // minimum size of hairpin
                  x = _spatium;
            qreal y = pos2().y();
            len     = sqrt(x * x + y * y);
            t.rotateRadians(asin(y/len));

            drawCircledTip   =  hairpin()->hairpinCircledTip();
            circledTipRadius = drawCircledTip ? 0.6 * _spatium * .5 : 0.0;

            QLine l1, l2;
            twoLines  = true;

            switch (type) {
                  case HairpinType::CRESC_HAIRPIN: {
                        switch (spannerSegmentType()) {
                              case SpannerSegmentType::SINGLE:
                              case SpannerSegmentType::BEGIN:
                                    l1.setLine(circledTipRadius * 2.0, 0.0, len, h1);
                                    l2.setLine(circledTipRadius * 2.0, 0.0, len, -h1);
                                    circledTip.setX(circledTipRadius );
                                    circledTip.setY(0.0);
                                    break;

                              case SpannerSegmentType::MIDDLE:
                              case SpannerSegmentType::END:
                                    drawCircledTip = false;
                                    l1.setLine(.0,  h2, len, h1);
                                    l2.setLine(.0, -h2, len, -h1);
                                    break;
                              }
                        }
                        break;
                  case HairpinType::DECRESC_HAIRPIN: {
                        switch (spannerSegmentType()) {
                              case SpannerSegmentType::SINGLE:
                              case SpannerSegmentType::END:
                                    l1.setLine(0.0,  h1, len - circledTipRadius * 2, 0.0);
                                    l2.setLine(0.0, -h1, len - circledTipRadius * 2, 0.0);
                                    circledTip.setX(len - circledTipRadius);
                                    circledTip.setY(0.0);
                                    break;
                              case SpannerSegmentType::BEGIN:
                              case SpannerSegmentType::MIDDLE:
                                    drawCircledTip = false;
                                    l1.setLine(.0,  h1, len, + h2);
                                    l2.setLine(.0, -h1, len, - h2);
                                    break;
                              }
                        }
                        break;
                  default:
                        break;
                  }

            // Do Coord rotation
            l1 = t.map(l1);
            l2 = t.map(l2);
            if (drawCircledTip )
                  circledTip = t.map(circledTip);

            points[0] = l1.p1();
            points[1] = l1.p2();
            points[2] = l2.p1();
            points[3] = l2.p2();
            npoints   = 4;

            QRectF r = QRectF(l1.p1(), l1.p2()).normalized() | QRectF(l2.p1(), l2.p2()).normalized();
            qreal w  = score()->styleP(StyleIdx::hairpinLineWidth);
            setbbox(r.adjusted(-w*.5, -w*.5, w, w));
            }
      if (parent()) {
            qreal yo = score()->styleP(StyleIdx::hairpinY);
            if (hairpin()->placeAbove())
                  yo = -yo + staff()->height() + bbox().height();
            rypos() += yo;
            if (autoplace()) {
                  qreal minDistance = spatium() * .7;
                  Shape s1 = shape().translated(pos());
                  qreal ymax = pos().y();
                  if (hairpin()->placeAbove()) {
                        qreal d  = system()->topDistance(staffIdx(), s1);
                        if (d > -minDistance)
                              ymax -= d + minDistance;
                        }
                  else {
                        qreal d  = system()->bottomDistance(staffIdx(), s1);

                        if (d > -minDistance)
                              ymax += d + minDistance;
                        qreal sdy;
                        if (sd) {
                              sdy = -sd->bbox().top() * .4;
                              sd->doAutoplace();
                              if (sd->pos().y() - sdy > ymax)
                                    ymax = sd->pos().y() - sdy;
                              }
                        qreal edy;
                        if (ed) {
                              edy = -ed->bbox().top() * .4;
                              ed->doAutoplace();
                              if (ed->pos().y() - edy > ymax)
                                    ymax = ed->pos().y() - edy;
                              }
                        if (sd)
                              moveDynamic(sd, ymax - sd->ipos().y() + sdy);
                        if (ed)
                              moveDynamic(ed, ymax - ed->ipos().y() + edy);
                        }
                  rUserYoffset() = ymax - pos().y();
                  }
            else
                  adjustReadPos();
            }
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape HairpinSegment::shape() const
      {
      switch (hairpin()->hairpinType()) {
            case HairpinType::CRESC_HAIRPIN:
            case HairpinType::DECRESC_HAIRPIN:
                  return Shape(bbox());
            case HairpinType::DECRESC_LINE:
            case HairpinType::CRESC_LINE:
            default:
                  return TextLineBaseSegment::shape();
            }
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

      if (hairpin()->hairpinType() == HairpinType::CRESC_HAIRPIN)
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
      TextLineBaseSegment::draw(painter);

      QColor color;
      if (selected() && !(score() && score()->printing()))
            color = (track() > -1) ? MScore::selectColor[voice()] : MScore::selectColor[0];
      else if (!hairpin()->visible())     // || !hairpin()->lineVisible()
            color = Qt::gray;
      else
            color = hairpin()->lineColor();
      QPen pen(color, point(hairpin()->lineWidth()), hairpin()->lineStyle());
      painter->setPen(pen);

      if (drawCircledTip) {
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
            case P_ID::PLACEMENT:
                  return hairpin()->getProperty(id);
            default:
                  return TextLineBaseSegment::getProperty(id);
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
            case P_ID::PLACEMENT:
                  return hairpin()->setProperty(id, v);
            default:
                  return TextLineBaseSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant HairpinSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::TEXT_STYLE_TYPE:
            case P_ID::HAIRPIN_CIRCLEDTIP:
            case P_ID::HAIRPIN_TYPE:
            case P_ID::VELO_CHANGE:
            case P_ID::DYNAMIC_RANGE:
            case P_ID::DIAGONAL:
            case P_ID::HAIRPIN_HEIGHT:
            case P_ID::HAIRPIN_CONT_HEIGHT:
            case P_ID::PLACEMENT:
                  return hairpin()->propertyDefault(id);
            default:
                  return TextLineBaseSegment::propertyDefault(id);
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
            case P_ID::PLACEMENT:
                  return hairpin()->propertyStyle(id);

            default:
                  return TextLineBaseSegment::propertyStyle(id);
            }
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx HairpinSegment::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::HAIRPIN_HEIGHT:
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return hairpin()->getPropertyStyle(id);
            default:
                  break;
            }
      return TextLineBaseSegment::getPropertyStyle(id);
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
                  return TextLineBaseSegment::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   Hairpin
//---------------------------------------------------------

Hairpin::Hairpin(Score* s)
   : TextLineBase(s)
      {
      _hairpinType       = HairpinType::CRESC_HAIRPIN;
      _hairpinCircledTip = false;
      _veloChange        = 0;
      _dynRange          = Dynamic::Range::PART;
      setLineWidth(score()->styleS(StyleIdx::hairpinLineWidth));
      lineWidthStyle         = PropertyStyle::STYLED;
      _hairpinHeight         = score()->styleS(StyleIdx::hairpinHeight);
      hairpinHeightStyle     = PropertyStyle::STYLED;
      _hairpinContHeight     = score()->styleS(StyleIdx::hairpinContHeight);
      hairpinContHeightStyle = PropertyStyle::STYLED;
      }

//---------------------------------------------------------
//   setHairpinType
//---------------------------------------------------------

void Hairpin::setHairpinType(HairpinType val)
      {
      if (_hairpinType == val)
            return;
      _hairpinType = val;
      switch (_hairpinType) {
            case HairpinType::CRESC_HAIRPIN:
            case HairpinType::DECRESC_HAIRPIN:
                  setBeginText("");
                  setContinueText("");
                  setLineStyle(Qt::SolidLine);
                  break;
            case HairpinType::CRESC_LINE:
                  setBeginText("cresc.");
                  setContinueText("(cresc.)");
                  setLineStyle(Qt::CustomDashLine);
                  break;
            case HairpinType::DECRESC_LINE:
                  setBeginText("dim.");
                  setContinueText("(dim.)");
                  setLineStyle(Qt::CustomDashLine);
                  break;
            };
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick() to _tick2
//---------------------------------------------------------

void Hairpin::layout()
      {
      setPos(0.0, 0.0);
      TextLineBase::layout();
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
      writeProperty(xml, P_ID::VELO_CHANGE);
      writeProperty(xml, P_ID::HAIRPIN_CIRCLEDTIP);
      writeProperty(xml, P_ID::DYNAMIC_RANGE);
      writeProperty(xml, P_ID::PLACEMENT);
      writeProperty(xml, P_ID::HAIRPIN_HEIGHT);
      writeProperty(xml, P_ID::HAIRPIN_CONT_HEIGHT);
      TextLineBase::writeProperties(xml);
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
                  setHairpinType(HairpinType(e.readInt()));
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
            else if (tag == "useTextLine") {      // obsolete
                  e.readInt();
                  if (hairpinType() == HairpinType::CRESC_HAIRPIN)
                        setHairpinType(HairpinType::CRESC_LINE);
                  else if (hairpinType() == HairpinType::DECRESC_HAIRPIN)
                        setHairpinType(HairpinType::DECRESC_LINE);
                  }
            else if (!TextLineBase::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   undoSetHairpinType
//---------------------------------------------------------

void Hairpin::undoSetHairpinType(HairpinType val)
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
                  return TextLineBase::getProperty(id);
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
                  setHairpinType(HairpinType(v.toInt()));
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
                  TextLineBase::setProperty(id, v);
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
                  return TextLineBase::setProperty(id, v);
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
            case P_ID::TEXT_STYLE_TYPE:
                  return int(TextStyleType::HAIRPIN);
            case P_ID::HAIRPIN_CIRCLEDTIP:
                  return false;
            case P_ID::VELO_CHANGE:
                  return 0;
            case P_ID::DYNAMIC_RANGE:
                  return int(Dynamic::Range::PART);
            case P_ID::LINE_WIDTH:
                  return score()->style(StyleIdx::hairpinLineWidth);
            case P_ID::HAIRPIN_HEIGHT:
                  return score()->style(StyleIdx::hairpinHeight);
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return score()->style(StyleIdx::hairpinContHeight);
            case P_ID::LINE_STYLE:
                  if (_hairpinType == HairpinType::CRESC_HAIRPIN || _hairpinType == HairpinType::DECRESC_HAIRPIN)
                        return int(Qt::SolidLine);
                  return int(Qt::CustomDashLine);
            case P_ID::PLACEMENT:
                  return int(Element::Placement::BELOW);
            default:
                  return TextLineBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle Hairpin::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
                  return lineWidthStyle;
            case P_ID::HAIRPIN_HEIGHT:
                  return hairpinHeightStyle;
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return hairpinContHeightStyle;
            default:
                  return TextLineBase::propertyStyle(id);
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
                  return TextLineBase::resetProperty(id);
            }
      triggerLayout();
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Hairpin::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
                  return StyleIdx::hairpinLineWidth;
            case P_ID::HAIRPIN_HEIGHT:
                  return StyleIdx::hairpinHeight;
            case P_ID::HAIRPIN_CONT_HEIGHT:
                  return StyleIdx::hairpinContHeight;
            default:
                  break;
            }
      return StyleIdx::NOSTYLE;
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
      TextLineBase::reset();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Hairpin::accessibleInfo() const
      {
      QString rez = TextLineBase::accessibleInfo();
      switch (hairpinType()) {
            case HairpinType::CRESC_HAIRPIN:
                  rez += ": " + tr("Crescendo");
                  break;
            case HairpinType::DECRESC_HAIRPIN:
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
      TextLineBase::startEdit(view, p);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Hairpin::endEdit()
      {
      if (editHairpinHeight != _hairpinHeight)
            score()->undoPropertyChanged(this, P_ID::HAIRPIN_HEIGHT, editHairpinHeight);
      TextLineBase::endEdit();
      }

}

