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

//---------------------------------------------------------
//   hairpinStyle
//---------------------------------------------------------

static const ElementStyle hairpinStyle {
      { Sid::hairpinFontFace,                    Pid::BEGIN_FONT_FACE            },
      { Sid::hairpinFontSize,                    Pid::BEGIN_FONT_SIZE            },
      { Sid::hairpinFontBold,                    Pid::BEGIN_FONT_BOLD            },
      { Sid::hairpinFontItalic,                  Pid::BEGIN_FONT_ITALIC          },
      { Sid::hairpinFontUnderline,               Pid::BEGIN_FONT_UNDERLINE       },
      { Sid::hairpinTextAlign,                   Pid::BEGIN_TEXT_ALIGN           },
      { Sid::hairpinFontFace,                    Pid::CONTINUE_FONT_FACE         },
      { Sid::hairpinFontSize,                    Pid::CONTINUE_FONT_SIZE         },
      { Sid::hairpinFontBold,                    Pid::CONTINUE_FONT_BOLD         },
      { Sid::hairpinFontItalic,                  Pid::CONTINUE_FONT_ITALIC       },
      { Sid::hairpinFontUnderline,               Pid::CONTINUE_FONT_UNDERLINE    },
      { Sid::hairpinTextAlign,                   Pid::CONTINUE_TEXT_ALIGN        },
      { Sid::hairpinFontFace,                    Pid::END_FONT_FACE              },
      { Sid::hairpinFontSize,                    Pid::END_FONT_SIZE              },
      { Sid::hairpinFontBold,                    Pid::END_FONT_BOLD              },
      { Sid::hairpinFontItalic,                  Pid::END_FONT_ITALIC            },
      { Sid::hairpinFontUnderline,               Pid::END_FONT_UNDERLINE         },
      { Sid::hairpinTextAlign,                   Pid::END_TEXT_ALIGN             },
      { Sid::hairpinLineWidth,                   Pid::LINE_WIDTH                 },
      { Sid::hairpinHeight,                      Pid::HAIRPIN_HEIGHT             },
      { Sid::hairpinContHeight,                  Pid::HAIRPIN_CONT_HEIGHT        },
      { Sid::hairpinPlacement,                   Pid::PLACEMENT                  },
      { Sid::hairpinPosAbove,                    Pid::OFFSET                     },
      };

//---------------------------------------------------------
//   HairpinSegment
//---------------------------------------------------------

HairpinSegment::HairpinSegment(Score* s)
   : TextLineBaseSegment(s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      }

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
            d->ryoffset() = y;
            Skyline& sk = d->measure()->system()->staff(d->staffIdx())->skyline();
            Segment* s = d->segment();
            sk.add(d->shape().translated(s->pos() + s->measure()->pos()));
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

      if (isSingleType() || isBeginType()) {
            sd = lookupDynamic(hairpin()->startElement());
            if (sd) {
                  if (autoplace()) {
                        qreal dx        = sd->bbox().right() + sd->pos().x()
                                             + sd->segment()->pos().x() + sd->measure()->pos().x();
                        qreal dist      = dx - pos().x() + score()->styleP(Sid::autoplaceHairpinDynamicsDistance);
                        rxpos()  = dist;
                        rxpos2() = -dist;
                        }
                  else
                        sd->doAutoplace();
                  }
            }
      if (isSingleType() || isEndType()) {
            ed = lookupDynamic(hairpin()->endElement());
            if (ed) {
                  if (autoplace()) {
                        rxpos2() -= ed->bbox().width();
                        qreal dx = ed->bbox().left() + ed->pos().x()
                                    + ed->segment()->pos().x() + ed->measure()->pos().x();
                        ed->rxpos() = pos2().x() + pos().x() - dx + score()->styleP(Sid::autoplaceHairpinDynamicsDistance);
                        }
                  else
                        ed->doAutoplace();
                  int si = ed->staffIdx();
                  Segment* s = ed->segment();
                  s->staffShape(si).add(ed->shape().translated(ed->pos()));
                  }
            }

      HairpinType type = hairpin()->hairpinType();
      if (type == HairpinType::DECRESC_LINE || type == HairpinType::CRESC_LINE) {
            twoLines = false;
            TextLineBaseSegment::layout();
            drawCircledTip   = false;
            circledTipRadius = 0.0;
            }
      else {
            twoLines  = true;

            qreal x1 = 0.0;
            TextLineBaseSegment::layout();
            if (!_text->empty())
                  x1 = _text->width();

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

            switch (type) {
                  case HairpinType::CRESC_HAIRPIN: {
                        switch (spannerSegmentType()) {
                              case SpannerSegmentType::SINGLE:
                              case SpannerSegmentType::BEGIN:
                                    l1.setLine(x1 + circledTipRadius * 2.0, 0.0, len, h1);
                                    l2.setLine(x1 + circledTipRadius * 2.0, 0.0, len, -h1);
                                    circledTip.setX(circledTipRadius );
                                    circledTip.setY(0.0);
                                    break;

                              case SpannerSegmentType::MIDDLE:
                              case SpannerSegmentType::END:
                                    drawCircledTip = false;
                                    l1.setLine(x1,  h2, len, h1);
                                    l2.setLine(x1, -h2, len, -h1);
                                    break;
                              }
                        }
                        break;
                  case HairpinType::DECRESC_HAIRPIN: {
                        switch (spannerSegmentType()) {
                              case SpannerSegmentType::SINGLE:
                              case SpannerSegmentType::END:
                                    l1.setLine(x1,  h1, len - circledTipRadius * 2, 0.0);
                                    l2.setLine(x1, -h1, len - circledTipRadius * 2, 0.0);
                                    circledTip.setX(len - circledTipRadius);
                                    circledTip.setY(0.0);
                                    break;
                              case SpannerSegmentType::BEGIN:
                              case SpannerSegmentType::MIDDLE:
                                    drawCircledTip = false;
                                    l1.setLine(x1,  h1, len, + h2);
                                    l2.setLine(x1, -h1, len, - h2);
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
            if (!_text->empty())
                  r |= _text->bbox();
            qreal w  = score()->styleP(Sid::hairpinLineWidth);
            setbbox(r.adjusted(-w*.5, -w*.5, w, w));
            }
      if (!parent()) {
            rpos() = QPointF();
            roffset() = QPointF();
            return;
            }

      if (isStyled(Pid::OFFSET))
            roffset() = hairpin()->propertyDefault(Pid::OFFSET).toPointF();
      if (autoplace()) {
            qreal minDistance = spatium() * .7;
            qreal ymax = pos().y();

            SkylineLine sl(!hairpin()->placeAbove());
            sl.add(shape().translated(pos()));
            if (hairpin()->placeAbove()) {
                  qreal d  = system()->topDistance(staffIdx(), sl);
                  if (d > -minDistance)
                        ymax -= d + minDistance;
                  }
            else {
                  qreal d  = system()->bottomDistance(staffIdx(), sl);
                  if (d > -minDistance)
                        ymax += d + minDistance;

                  qreal sdy = 0.0;
                  if (sd) {
                        sdy = -sd->bbox().top() * .4;
                        sd->doAutoplace();
                        if (sd->pos().y() - sdy > ymax)
                              ymax = sd->pos().y() - sdy;
                        }
                  qreal edy = 0.0;
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
            rypos() += ymax - pos().y();
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

void HairpinSegment::updateGrips(EditData& ed) const
      {
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
      doRotation.rotateRadians(asin(y/len));
      qreal lineApertureX;
      qreal offsetX = 10;                               // Horizontal offset for x Grip
      if (len < offsetX * 3)                            // For small hairpin, offset = 30% of len
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
      ed.grip[int(Grip::START)].translate( pp );
      ed.grip[int(Grip::END)].translate( p + pp );
      ed.grip[int(Grip::MIDDLE)].translate( p * .5 + pp );
      ed.grip[int(Grip::APERTURE)].translate(gripLineAperturePoint + pp);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void HairpinSegment::startEdit(EditData& ed)
      {
      ed.grips   = 4;
      ed.curGrip = Grip::END;
      Element::startEdit(ed);
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void HairpinSegment::startEditDrag(EditData& ed)
      {
      TextLineBaseSegment::startEditDrag(ed);
      setAutoplace(false);
      ElementEditData* eed = ed.getData(this);

      eed->pushProperty(Pid::HAIRPIN_HEIGHT);
      eed->pushProperty(Pid::HAIRPIN_CONT_HEIGHT);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void HairpinSegment::editDrag(EditData& ed)
      {
      if (ed.curGrip == Grip::APERTURE) {
            qreal newHeight = hairpin()->hairpinHeight().val() + ed.delta.y()/spatium()/.5;
            if (newHeight < 0.5)
                  newHeight = 0.5;
            hairpin()->setHairpinHeight(Spatium(newHeight));
            triggerLayout();
            }
      TextLineBaseSegment::editDrag(ed);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void HairpinSegment::draw(QPainter* painter) const
      {
      TextLineBaseSegment::draw(painter);

      QColor color;
      if ((selected() && !(score() && score()->printing())) || !hairpin()->visible())
            color = curColor();
      else
            color = hairpin()->lineColor();
      QPen pen(color, hairpin()->lineWidth(), hairpin()->lineStyle());
      painter->setPen(pen);

      if (drawCircledTip) {
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse( circledTip,circledTipRadius,circledTipRadius );
            }
      }

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

Element* HairpinSegment::propertyDelegate(Pid pid)
      {
      if (pid == Pid::HAIRPIN_TYPE
         || pid == Pid::VELO_CHANGE
         || pid == Pid::HAIRPIN_CIRCLEDTIP
         || pid == Pid::HAIRPIN_HEIGHT
         || pid == Pid::HAIRPIN_CONT_HEIGHT
         || pid == Pid::DYNAMIC_RANGE)
            return spanner();
      return TextLineBaseSegment::propertyDelegate(pid);
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid HairpinSegment::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET)
            return spanner()->placeAbove() ? Sid::hairpinPosAbove : Sid::hairpinPosBelow;
      return TextLineBaseSegment::getPropertyStyle(pid);
      }

Sid Hairpin::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET)
            return placeAbove() ? Sid::hairpinPosAbove : Sid::hairpinPosBelow;
      return TextLineBase::getPropertyStyle(pid);
      }

//---------------------------------------------------------
//   Hairpin
//---------------------------------------------------------

Hairpin::Hairpin(Score* s)
   : TextLineBase(s)
      {
      initElementStyle(&hairpinStyle);

      resetProperty(Pid::BEGIN_TEXT_PLACE);
      resetProperty(Pid::HAIRPIN_TYPE);
      resetProperty(Pid::LINE_VISIBLE);
//      resetProperty(Pid::LINE_WITH);

      _hairpinCircledTip     = false;
      _veloChange            = 0;
      _dynRange              = Dynamic::Range::PART;
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
            case HairpinType::INVALID:
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

static const ElementStyle hairpinSegmentStyle {
      { Sid::hairpinPosBelow, Pid::OFFSET },
      };

LineSegment* Hairpin::createLineSegment()
      {
      HairpinSegment* h = new HairpinSegment(score());
      h->initElementStyle(&hairpinSegmentStyle);
      return h;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Hairpin::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
      xml.tag("subtype", int(_hairpinType));
      writeProperty(xml, Pid::VELO_CHANGE);
      writeProperty(xml, Pid::HAIRPIN_CIRCLEDTIP);
      writeProperty(xml, Pid::DYNAMIC_RANGE);
      writeProperty(xml, Pid::BEGIN_TEXT);
      writeProperty(xml, Pid::CONTINUE_TEXT);

      for (const StyledProperty& spp : *styledProperties()) {
            if (!isStyled(spp.pid))
                  writeProperty(xml, spp.pid);
            }
      Element::writeProperties(xml);
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

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setHairpinType(HairpinType(e.readInt()));
            else if (readStyledProperty(e, tag))
                  ;
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
      undoChangeProperty(Pid::HAIRPIN_TYPE, int(val));
      }

//---------------------------------------------------------
//   undoSetVeloChange
//---------------------------------------------------------

void Hairpin::undoSetVeloChange(int val)
      {
      undoChangeProperty(Pid::VELO_CHANGE, val);
      }

//---------------------------------------------------------
//   undoSetDynType
//---------------------------------------------------------

void Hairpin::undoSetDynRange(Dynamic::Range val)
      {
      undoChangeProperty(Pid::DYNAMIC_RANGE, int(val));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Hairpin::getProperty(Pid id) const
      {
      switch (id) {
            case Pid::HAIRPIN_CIRCLEDTIP:
                return _hairpinCircledTip;
            case Pid::HAIRPIN_TYPE:
                return int(_hairpinType);
            case Pid::VELO_CHANGE:
                  return _veloChange;
            case Pid::DYNAMIC_RANGE:
                  return int(_dynRange);
            case Pid::HAIRPIN_HEIGHT:
                  return _hairpinHeight;
            case Pid::HAIRPIN_CONT_HEIGHT:
                  return _hairpinContHeight;
            default:
                  return TextLineBase::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Hairpin::setProperty(Pid id, const QVariant& v)
      {
      switch (id) {
            case Pid::HAIRPIN_CIRCLEDTIP:
                _hairpinCircledTip = v.toBool();
                break;
            case Pid::HAIRPIN_TYPE:
                  setHairpinType(HairpinType(v.toInt()));
                  setGenerated(false);
                  break;
            case Pid::VELO_CHANGE:
                  _veloChange = v.toInt();
                  break;
            case Pid::DYNAMIC_RANGE:
                  _dynRange = Dynamic::Range(v.toInt());
                  break;
            case Pid::HAIRPIN_HEIGHT:
                  _hairpinHeight = v.value<Spatium>();
                  break;
            case Pid::HAIRPIN_CONT_HEIGHT:
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

QVariant Hairpin::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::HAIRPIN_CIRCLEDTIP:
                  return false;

            case Pid::VELO_CHANGE:
                  return 0;

            case Pid::DYNAMIC_RANGE:
                  return int(Dynamic::Range::PART);

            case Pid::LINE_STYLE:
                  if (_hairpinType == HairpinType::CRESC_HAIRPIN || _hairpinType == HairpinType::DECRESC_HAIRPIN)
                        return int(Qt::SolidLine);
                  return int(Qt::CustomDashLine);

            case Pid::BEGIN_TEXT:
            case Pid::CONTINUE_TEXT:
                  return QString("");

            case Pid::BEGIN_TEXT_PLACE:
                  return int(PlaceText::LEFT);

            case Pid::LINE_VISIBLE:
                  return true;

            case Pid::HAIRPIN_TYPE:
                  return int(HairpinType::CRESC_HAIRPIN);

            default:
                  return TextLineBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Hairpin::accessibleInfo() const
      {
      QString rez = TextLineBase::accessibleInfo();
      switch (hairpinType()) {
            case HairpinType::CRESC_HAIRPIN:
                  rez += ": " + QObject::tr("Crescendo");
                  break;
            case HairpinType::DECRESC_HAIRPIN:
                  rez += ": " + QObject::tr("Decrescendo");
                  break;
            default:
                  rez += ": " + QObject::tr("Custom");
            }
      return rez;
      }

}

