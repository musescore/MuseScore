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

#include "changeMap.h"
#include "dynamichairpingroup.h"
#include "hairpin.h"
#include "measure.h"
#include "mscore.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "style.h"
#include "system.h"
#include "undo.h"
#include "utils.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   hairpinStyle
//---------------------------------------------------------

static const ElementStyle hairpinStyle {
      { Sid::hairpinFontFace,                    Pid::BEGIN_FONT_FACE            },
      { Sid::hairpinFontSize,                    Pid::BEGIN_FONT_SIZE            },
      { Sid::hairpinFontStyle,                   Pid::BEGIN_FONT_STYLE           },
      { Sid::hairpinText,                        Pid::BEGIN_TEXT                 },
      { Sid::hairpinTextAlign,                   Pid::BEGIN_TEXT_ALIGN           },
      { Sid::hairpinFontFace,                    Pid::CONTINUE_FONT_FACE         },
      { Sid::hairpinFontSize,                    Pid::CONTINUE_FONT_SIZE         },
      { Sid::hairpinFontStyle,                   Pid::CONTINUE_FONT_STYLE        },
      { Sid::hairpinText,                        Pid::CONTINUE_TEXT              },
      { Sid::hairpinTextAlign,                   Pid::CONTINUE_TEXT_ALIGN        },
      { Sid::hairpinFontFace,                    Pid::END_FONT_FACE              },
      { Sid::hairpinFontSize,                    Pid::END_FONT_SIZE              },
      { Sid::hairpinFontStyle,                   Pid::END_FONT_STYLE             },
      { Sid::hairpinTextAlign,                   Pid::END_TEXT_ALIGN             },
      { Sid::hairpinLineWidth,                   Pid::LINE_WIDTH                 },
      { Sid::hairpinHeight,                      Pid::HAIRPIN_HEIGHT             },
      { Sid::hairpinContHeight,                  Pid::HAIRPIN_CONT_HEIGHT        },
      { Sid::hairpinPlacement,                   Pid::PLACEMENT                  },
      { Sid::hairpinPosBelow,                    Pid::OFFSET                     },
      { Sid::hairpinLineStyle,                   Pid::LINE_STYLE                 },
      };

//---------------------------------------------------------
//   HairpinSegment
//---------------------------------------------------------

HairpinSegment::HairpinSegment(Spanner* sp, Score* s)
   : TextLineBaseSegment(sp, s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      }

bool HairpinSegment::acceptDrop(EditData& data) const
      {
      Element* e = data.dropElement;
      if (e->isDynamic())
            return true;
      return false;
      }

Element* HairpinSegment::drop(EditData& data)
      {
      Element* e = data.dropElement;
      if (e->isDynamic()) {
            Dynamic* d = toDynamic(e);
            hairpin()->undoChangeProperty(Pid::END_TEXT, d->xmlText());
            }
      return 0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void HairpinSegment::layout()
      {
      const qreal _spatium = spatium();
      const int _trck = track();
      Dynamic* sd = nullptr;
      Dynamic* ed = nullptr;
      qreal dymax = hairpin()->placeBelow() ? -10000.0 : 10000.0;
      if (autoplace() && !score()->isPalette()) {
            Segment* start = hairpin()->startSegment();
            Segment* end = hairpin()->endSegment();
            // Try to fit between adjacent dynamics
            qreal minDynamicsDistance = score()->styleP(Sid::autoplaceHairpinDynamicsDistance) * staff()->mag(tick());
            const System* sys = system();
            if (isSingleType() || isBeginType()) {
                  if (start && start->system() == sys) {
                        sd = toDynamic(start->findAnnotation(ElementType::DYNAMIC, _trck, _trck));
                        if (!sd) {
                              // Dynamics might have been added to the previous
                              // segment rather than exactly to hairpin start,
                              // search in that segment too.
                              start = start->prev(SegmentType::ChordRest);
                              if (start && start->system() == sys)
                                    sd = toDynamic(start->findAnnotation(ElementType::DYNAMIC, _trck, _trck));
                              }
                        }
                  if (sd && sd->addToSkyline() && sd->placement() == hairpin()->placement()
                      && (hairpin()->lineVisible() || !_text->empty())) {
                        const qreal sdRight = sd->bbox().right() + sd->pos().x()
                                              + sd->segment()->pos().x() + sd->measure()->pos().x();
                        const qreal dist    = qMax(sdRight - pos().x() + minDynamicsDistance, 0.0);
                        rxpos()  += dist;
                        rxpos2() -= dist;
                        // prepare to align vertically
                        dymax = sd->pos().y();
                        }
                  }
            if (isSingleType() || isEndType()) {
                  if (end && end->tick() < sys->endTick() && start != end) {
                        // checking ticks rather than systems
                        // systems may be unknown at layout stage.
                        ed = toDynamic(end->findAnnotation(ElementType::DYNAMIC, _trck, _trck));
                        }
                  if (ed && ed->addToSkyline() && ed->placement() == hairpin()->placement()
                      && (hairpin()->lineVisible() || !_endText->empty())) {
                        const qreal edLeft  = ed->bbox().left() + ed->pos().x()
                                              + ed->segment()->pos().x() + ed->measure()->pos().x();
                        const qreal dist    = edLeft - pos2().x() - pos().x() - minDynamicsDistance;
                        const qreal extendThreshold = 3.0 * _spatium;   // TODO: style setting
                        if (dist < 0.0)
                              rxpos2() += dist;       // always shorten
                        else if (dist >= extendThreshold && hairpin()->endText().isEmpty() && minDynamicsDistance > 0.0)
                              rxpos2() += dist;       // lengthen only if appropriate
                        // prepare to align vertically
                        if (hairpin()->placeBelow())
                              dymax = qMax(dymax, ed->pos().y());
                        else
                              dymax = qMin(dymax, ed->pos().y());
                        }
                  }
            }

      HairpinType type = hairpin()->hairpinType();
      if (hairpin()->isLineType()) {
            twoLines = false;
            TextLineBaseSegment::layout();
            drawCircledTip   = false;
            circledTipRadius = 0.0;
            }
      else {
            twoLines  = true;

            hairpin()->setBeginTextAlign(Align::LEFT | Align::VCENTER);
            hairpin()->setEndTextAlign(Align::RIGHT | Align::VCENTER);

            qreal x1 = 0.0;
            TextLineBaseSegment::layout();
            if (!_text->empty())
                  x1 = _text->width() + _spatium * .5;

            QTransform t;
            qreal h1 = hairpin()->hairpinHeight().val()     * _spatium * .5;
            qreal h2 = hairpin()->hairpinContHeight().val() * _spatium * .5;

            qreal x = pos2().x();
            if (!_endText->empty())
                  x -= (_endText->width() + _spatium * .5);       // 0.5 spatium distance
            if (x < _spatium)             // minimum size of hairpin
                  x = _spatium;
            qreal y = pos2().y();
            qreal len = sqrt(x * x + y * y);
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
                                    circledTip.setX(x1 + circledTipRadius );
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

            joinedHairpin.clear();
            if (spannerSegmentType() != SpannerSegmentType::MIDDLE) {
                  if (type == HairpinType::DECRESC_HAIRPIN && spannerSegmentType() != SpannerSegmentType::BEGIN)
                        joinedHairpin << points[0] << points[1] << points[2]; // [top-left, joint, bottom-left]
                  else if (type == HairpinType::CRESC_HAIRPIN && spannerSegmentType() != SpannerSegmentType::END)
                        joinedHairpin << points[1] << points[0] << points[3]; // [top-right, joint, bottom-right]
                  }

            QRectF r = QRectF(l1.p1(), l1.p2()).normalized() | QRectF(l2.p1(), l2.p2()).normalized();
            if (!_text->empty())
                  r |= _text->bbox();
            if (!_endText->empty())
                  r |= _endText->bbox().translated(x + _endText->bbox().width(), 0.0);
            qreal w  = point(score()->styleS(Sid::hairpinLineWidth));
            setbbox(r.adjusted(-w*.5, -w*.5, w, w));
            }
      if (!parent()) {
            rpos() = QPointF();
            roffset() = QPointF();
            return;
            }

      if (isStyled(Pid::OFFSET))
            roffset() = hairpin()->propertyDefault(Pid::OFFSET).toPointF();

      // rebase vertical offset on drag
      qreal rebase = 0.0;
      if (offsetChanged() != OffsetChange::NONE)
            rebase = rebaseOffset();

      if (autoplace()) {
            qreal ymax = pos().y();
            qreal d;
            qreal ddiff = hairpin()->isLineType() ? 0.0 : _spatium * 0.5;

            qreal sp = spatium();
            qreal md = minDistance().val() * sp;

            bool above = spanner()->placeAbove();
            SkylineLine sl(!above);
            Shape sh = shape();
            sl.add(sh.translated(pos()));
            if (above) {
                  d  = system()->topDistance(staffIdx(), sl);
                  if (d > -md)
                        ymax -= d + md;
                  // align hairpin with dynamics
                  if (!hairpin()->diagonal())
                        ymax = qMin(ymax, dymax - ddiff);
                  }
            else {
                  d  = system()->bottomDistance(staffIdx(), sl);
                  if (d > -md)
                        ymax += d + md;
                  // align hairpin with dynamics
                  if (!hairpin()->diagonal())
                        ymax = qMax(ymax, dymax - ddiff);
                  }
            qreal yd = ymax - pos().y();
            if (!qFuzzyIsNull(yd)) {
                  if (offsetChanged() != OffsetChange::NONE) {
                        // user moved element within the skyline
                        // we may need to adjust minDistance, yd, and/or offset
                        qreal adj = pos().y() + rebase;
                        bool inStaff = above ? sh.bottom() + adj > 0.0 : sh.top() + adj < staff()->height();
                        rebaseMinDistance(md, yd, sp, rebase, above, inStaff);
                        }
                  rypos() += yd;
                  }

            if (hairpin()->addToSkyline() && !hairpin()->diagonal()) {
                  // align dynamics with hairpin
                  if (sd && sd->autoplace() && sd->placement() == hairpin()->placement()
                      && (hairpin()->lineVisible() || !_text->empty())){
                        qreal ny = y() + ddiff - sd->offset().y();
                        if (sd->placeAbove())
                              ny = qMin(ny, sd->ipos().y());
                        else
                              ny = qMax(ny, sd->ipos().y());
                        if (sd->ipos().y() != ny) {
                              sd->rypos() = ny;
                              if (sd->addToSkyline()) {
                                    Segment* s = sd->segment();
                                    Measure* m = s->measure();
                                    QRectF r = sd->bbox().translated(sd->pos());
                                    s->staffShape(sd->staffIdx()).add(r);
                                    r = sd->bbox().translated(sd->pos() + s->pos() + m->pos());
                                    m->system()->staff(sd->staffIdx())->skyline().add(r);
                                    }
                              }
                        }
                  if (ed && ed->autoplace() && ed->placement() == hairpin()->placement()
                      && (hairpin()->lineVisible() || !_endText->empty())) {
                        qreal ny = y() + ddiff - ed->offset().y();
                        if (ed->placeAbove())
                              ny = qMin(ny, ed->ipos().y());
                        else
                              ny = qMax(ny, ed->ipos().y());
                        if (ed->ipos().y() != ny) {
                              ed->rypos() = ny;
                              if (ed->addToSkyline()) {
                                    Segment* s = ed->segment();
                                    Measure* m = s->measure();
                                    QRectF r = ed->bbox().translated(ed->pos());
                                    s->staffShape(ed->staffIdx()).add(r);
                                    r = ed->bbox().translated(ed->pos() + s->pos() + m->pos());
                                    m->system()->staff(ed->staffIdx())->skyline().add(r);
                                    }
                              }
                        }
                  }
            }
      setOffsetChanged(false);
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
//   gripsPositions
//---------------------------------------------------------

std::vector<QPointF> HairpinSegment::gripsPositions(const EditData&) const
      {
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
      gripLineAperturePoint = doRotation.map(gripLineAperturePoint);

      std::vector<QPointF> grips(gripsCount());

      // End calc position grip aperture
      QPointF pp(pagePos());
      grips[int(Grip::START)] = pp;
      grips[int(Grip::END)] = p + pp;
      grips[int(Grip::MIDDLE)] = p * .5 + pp;
      grips[int(Grip::APERTURE)] = gripLineAperturePoint + pp;

      return grips;
      }

//---------------------------------------------------------
//   getDragGroup
//---------------------------------------------------------

std::unique_ptr<ElementGroup> HairpinSegment::getDragGroup(std::function<bool(const Element*)> isDragged)
      {
      if (auto g = HairpinWithDynamicsDragGroup::detectFor(this, isDragged))
            return g;
      return TextLineBaseSegment::getDragGroup(isDragged);
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void HairpinSegment::startEditDrag(EditData& ed)
      {
      TextLineBaseSegment::startEditDrag(ed);
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

#if 0
      QColor color;
      if ((selected() && !(score() && score()->printing())) || !hairpin()->visible())
            color = curColor();
      else
            color = hairpin()->lineColor();
#endif
      QColor color = curColor(hairpin()->visible(), hairpin()->lineColor());
      qreal w = hairpin()->lineWidth();
      if (staff())
            w *= staff()->mag(hairpin()->tick());
      QPen pen(color, w, hairpin()->lineStyle());
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
         || pid == Pid::VELO_CHANGE_METHOD
         || pid == Pid::SINGLE_NOTE_DYNAMICS
         || pid == Pid::HAIRPIN_CIRCLEDTIP
         || pid == Pid::HAIRPIN_HEIGHT
         || pid == Pid::HAIRPIN_CONT_HEIGHT
         || pid == Pid::DYNAMIC_RANGE
         || pid == Pid::LINE_STYLE
            )
            return spanner();
      return TextLineBaseSegment::propertyDelegate(pid);
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid HairpinSegment::getPropertyStyle(Pid pid) const
      {
      switch (pid) {
            case Pid::OFFSET:
                  if (hairpin()->isLineType())
                        return spanner()->placeAbove() ? Sid::hairpinLinePosAbove : Sid::hairpinLinePosBelow;
                  return spanner()->placeAbove() ? Sid::hairpinPosAbove : Sid::hairpinPosBelow;
            case Pid::BEGIN_TEXT:
                  switch (hairpin()->hairpinType()) {
                        default:
                              return Sid::hairpinText;
                        case HairpinType::CRESC_LINE:
                              return Sid::hairpinCrescText;
                        case HairpinType::DECRESC_LINE:
                              return Sid::hairpinDecrescText;
                        }
                  break;
            case Pid::CONTINUE_TEXT:
                  switch (hairpin()->hairpinType()) {
                        default:
                              return Sid::hairpinText;
                        case HairpinType::CRESC_LINE:
                              return Sid::hairpinCrescContText;
                        case HairpinType::DECRESC_LINE:
                              return Sid::hairpinDecrescContText;
                        }
                  break;
            case Pid::LINE_STYLE:
                  return hairpin()->isLineType() ? Sid::hairpinLineLineStyle : Sid::hairpinLineStyle;
            default:
                  break;
            }
      return TextLineBaseSegment::getPropertyStyle(pid);
      }

Sid Hairpin::getPropertyStyle(Pid pid) const
      {
      switch (pid) {
            case Pid::OFFSET:
                  if (isLineType())
                        return placeAbove() ? Sid::hairpinLinePosAbove : Sid::hairpinLinePosBelow;
                  return placeAbove() ? Sid::hairpinPosAbove : Sid::hairpinPosBelow;
            case Pid::BEGIN_TEXT:
                  switch (hairpinType()) {
                        default:
                              return Sid::hairpinText;
                        case HairpinType::CRESC_LINE:
                              return Sid::hairpinCrescText;
                        case HairpinType::DECRESC_LINE:
                              return Sid::hairpinDecrescText;
                        }
                  break;
            case Pid::CONTINUE_TEXT:
                  switch (hairpinType()) {
                        default:
                              return Sid::hairpinText;
                        case HairpinType::CRESC_LINE:
                              return Sid::hairpinCrescContText;
                        case HairpinType::DECRESC_LINE:
                              return Sid::hairpinDecrescContText;
                        }
                  break;
            case Pid::LINE_STYLE:
                  return isLineType() ? Sid::hairpinLineLineStyle : Sid::hairpinLineStyle;
            default:
                  break;
            }
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
      resetProperty(Pid::END_TEXT_PLACE);
      resetProperty(Pid::CONTINUE_TEXT_PLACE);
      resetProperty(Pid::BEGIN_HOOK_HEIGHT);
      resetProperty(Pid::END_HOOK_HEIGHT);
      resetProperty(Pid::HAIRPIN_TYPE);
      resetProperty(Pid::LINE_VISIBLE);

      _hairpinCircledTip     = false;
      _veloChange            = 0;
      _dynRange              = Dynamic::Range::PART;
      _singleNoteDynamics    = true;
      _veloChangeMethod      = ChangeMethod::NORMAL;
      }

//---------------------------------------------------------
//   setHairpinType
//---------------------------------------------------------

void Hairpin::setHairpinType(HairpinType val)
      {
      if (_hairpinType == val)
            return;
      _hairpinType = val;
#if 0
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
#endif
      styleChanged();
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
      { Sid::hairpinMinDistance, Pid::MIN_DISTANCE },
      };

LineSegment* Hairpin::createLineSegment()
      {
      HairpinSegment* h = new HairpinSegment(this, score());
      h->setTrack(track());
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
      writeProperty(xml, Pid::SINGLE_NOTE_DYNAMICS);
      writeProperty(xml, Pid::VELO_CHANGE_METHOD);
      writeProperty(xml, Pid::PLACEMENT);

      TextLineBase::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Hairpin::read(XmlReader& e)
      {
      eraseSpannerSegments();

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setHairpinType(HairpinType(e.readInt()));
            else if (tag == "lineStyle" && score()->mscVersion() > MSCVERSION) { // 4.x compat
                  QString lineStyle = e.readElementText();
                  if (lineStyle == "dashed") { // closest guesses, better than loosing them entirely
                        if (isLineType())
                              setLineStyle(Qt::CustomDashLine);
                        else
                              setLineStyle(Qt::DashLine);
                        }
                  else if (lineStyle == "dotted")
                        setLineStyle(Qt::DotLine);
                  else
                        setLineStyle(Qt::SolidLine);
                  }
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
            else if (tag == "singleNoteDynamics")
                  _singleNoteDynamics = e.readBool();
            else if (tag == "veloChangeMethod")
                  _veloChangeMethod = ChangeMap::nameToChangeMethod(e.readElementText());
            else if (!TextLineBase::readProperties(e))
                  e.unknown();
            }
      styleChanged();
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
            case Pid::SINGLE_NOTE_DYNAMICS:
                  return _singleNoteDynamics;
            case Pid::VELO_CHANGE_METHOD:
                  return int(_veloChangeMethod);
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
            case Pid::SINGLE_NOTE_DYNAMICS:
                  _singleNoteDynamics = v.toBool();
                  break;
            case Pid::VELO_CHANGE_METHOD:
                  _veloChangeMethod = ChangeMethod(v.toInt());
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
                  if (isLineType())
                        return int(Qt::CustomDashLine);
                  return int(Qt::SolidLine);

            case Pid::BEGIN_TEXT:
                  if (_hairpinType == HairpinType::CRESC_LINE)
                        return QString("cresc.");
                  if (_hairpinType == HairpinType::DECRESC_LINE)
                        return QString("dim.");
                  return QString();

            case Pid::CONTINUE_TEXT:
            case Pid::END_TEXT:
                  if (_hairpinType == HairpinType::CRESC_LINE)
                        return QString("(cresc.)");
                  if (_hairpinType == HairpinType::DECRESC_LINE)
                        return QString("(dim.)");
                  return QString("");

            case Pid::BEGIN_TEXT_PLACE:
            case Pid::CONTINUE_TEXT_PLACE:
            case Pid::END_TEXT_PLACE:
                  return int(PlaceText::LEFT);

            case Pid::BEGIN_TEXT_OFFSET:
            case Pid::CONTINUE_TEXT_OFFSET:
            case Pid::END_TEXT_OFFSET:
                  return QPointF();

            case Pid::BEGIN_HOOK_TYPE:
            case Pid::END_HOOK_TYPE:
                  return int(HookType::NONE);

            case Pid::BEGIN_HOOK_HEIGHT:
            case Pid::END_HOOK_HEIGHT:
                  return Spatium(1.9);

            case Pid::LINE_VISIBLE:
                  return true;

            case Pid::HAIRPIN_TYPE:
                  return int(HairpinType::CRESC_HAIRPIN);

            case Pid::SINGLE_NOTE_DYNAMICS:
                  return true;

            case Pid::VELO_CHANGE_METHOD:
                  return int(ChangeMethod::NORMAL);

            case Pid::PLACEMENT:
                  return score()->styleV(Sid::hairpinPlacement);

            default:
                  return TextLineBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   Hairpin::propertyId
//---------------------------------------------------------

Pid Hairpin::propertyId(const QStringRef& name) const
      {
      if (name == "subtype")
            return Pid::HAIRPIN_TYPE;
      return TextLineBase::propertyId(name);
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

