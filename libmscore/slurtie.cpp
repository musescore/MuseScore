//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "measure.h"
#include "score.h"
#include "system.h"
#include "undo.h"
#include "slurtie.h"
#include "tie.h"
#include "chord.h"

namespace Ms {

//---------------------------------------------------------
//   SlurTieSegment
//---------------------------------------------------------

SlurTieSegment::SlurTieSegment(Score* score)
   : SpannerSegment(score)
      {
      setFlag(ElementFlag::ON_STAFF, true);
      }

SlurTieSegment::SlurTieSegment(const SlurTieSegment& b)
   : SpannerSegment(b)
      {
      for (int i = 0; i < int(Grip::GRIPS); ++i) {
            _ups[i]   = b._ups[i];
            _ups[i].p = QPointF();
            }
      path = b.path;
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF SlurTieSegment::gripAnchor(Grip grip) const
      {
      if (grip != Grip::START && grip != Grip::END)
            return QPointF();

      QPointF sp(system()->pagePos());
      QPointF pp(pagePos());
      QPointF p1(ups(Grip::START).p + pp);
      QPointF p2(ups(Grip::END).p + pp);

      switch (spannerSegmentType()) {
            case SpannerSegmentType::SINGLE:
                  return grip == Grip::START ? p1 : p2;

            case SpannerSegmentType::BEGIN:
                  return grip == Grip::START ? p1 : system()->abbox().topRight();

            case SpannerSegmentType::MIDDLE:
                  return grip == Grip::START ? sp : system()->abbox().topRight();

            case SpannerSegmentType::END:
                  return grip == Grip::START ? sp : p2;
            }
      return QPointF();
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SlurTieSegment::move(const QPointF& s)
      {
      Element::move(s);
      for (int k = 0; k < int(Grip::GRIPS); ++k)
            _ups[k].p += s;
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void SlurTieSegment::spatiumChanged(qreal oldValue, qreal newValue)
      {
      qreal diff = newValue / oldValue;
      for (UP& u : _ups)
            u.off *= diff;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void SlurTieSegment::startEdit(EditData& ed)
      {
      Element::startEdit(ed);
      ed.grips   = int(Grip::GRIPS);
      ed.curGrip = Grip::END;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void SlurTieSegment::endEdit(EditData&)
      {
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void SlurTieSegment::startEditDrag(EditData& ed)
      {
      ElementEditData* eed = ed.getData(this);
      for (auto i : { P_ID::SLUR_UOFF1, P_ID::SLUR_UOFF2, P_ID::SLUR_UOFF3, P_ID::SLUR_UOFF4, P_ID::USER_OFF })
            eed->pushProperty(i);
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void SlurTieSegment::endEditDrag(EditData& ed)
      {
      Element::endEditDrag(ed);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void SlurTieSegment::editDrag(EditData& ed)
      {
      Grip g     = ed.curGrip;
      ups(g).off += ed.delta;

      QPointF delta;

      switch (g) {
            case Grip::START:
            case Grip::END:
                  //
                  // move anchor for slurs/ties
                  //
                  if ((g == Grip::START && isSingleBeginType()) || (g == Grip::END && isSingleEndType())) {
                        Spanner* spanner = slurTie();
                        Qt::KeyboardModifiers km = qApp->keyboardModifiers();
                        Element* e = ed.view->elementNear(ed.pos);
                        if (e && e->isNote()) {
                              Note* note = toNote(e);
                              int tick = note->chord()->tick();
                              if ((g == Grip::END && tick > slurTie()->tick()) || (g == Grip::START && tick < slurTie()->tick2())) {
                                    if (km != (Qt::ShiftModifier | Qt::ControlModifier)) {
                                          Chord* c = note->chord();
                                          ed.view->setDropTarget(note);
                                          if (c != spanner->endChord())
                                                changeAnchor(ed, c);
                                          }
                                    }
                              }
                        else
                              ed.view->setDropTarget(0);
                        }
                  break;
            case Grip::BEZIER1:
                  break;
            case Grip::BEZIER2:
                  break;
            case Grip::SHOULDER:
                  ups(g).off = QPointF();
                  delta = ed.delta;
                  break;
            case Grip::DRAG:
                  ups(g).off = QPointF();
                  setUserOff(userOff() + ed.delta);
                  break;
            case Grip::NO_GRIP:
            case Grip::GRIPS:
                  break;
            }
      computeBezier(delta);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant SlurTieSegment::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::LINE_TYPE:
            case P_ID::SLUR_DIRECTION:
                  return slurTie()->getProperty(propertyId);
            case P_ID::SLUR_UOFF1:
                  return ups(Grip::START).off;
            case P_ID::SLUR_UOFF2:
                  return ups(Grip::BEZIER1).off;
            case P_ID::SLUR_UOFF3:
                  return ups(Grip::BEZIER2).off;
            case P_ID::SLUR_UOFF4:
                  return ups(Grip::END).off;
            default:
                  return SpannerSegment::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SlurTieSegment::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_ID::LINE_TYPE:
            case P_ID::SLUR_DIRECTION:
                  return slurTie()->setProperty(propertyId, v);
            case P_ID::SLUR_UOFF1:
                  ups(Grip::START).off = v.toPointF();
                  break;
            case P_ID::SLUR_UOFF2:
                  ups(Grip::BEZIER1).off = v.toPointF();
                  break;
            case P_ID::SLUR_UOFF3:
                  ups(Grip::BEZIER2).off = v.toPointF();
                  break;
            case P_ID::SLUR_UOFF4:
                  ups(Grip::END).off = v.toPointF();
                  break;
            default:
                  return SpannerSegment::setProperty(propertyId, v);
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SlurTieSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_TYPE:
            case P_ID::SLUR_DIRECTION:
                  return slurTie()->propertyDefault(id);
            case P_ID::SLUR_UOFF1:
            case P_ID::SLUR_UOFF2:
            case P_ID::SLUR_UOFF3:
            case P_ID::SLUR_UOFF4:
                  return QPointF();
            default:
                  return SpannerSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void SlurTieSegment::reset()
      {
      Element::reset();
      undoResetProperty(P_ID::SLUR_UOFF1);
      undoResetProperty(P_ID::SLUR_UOFF2);
      undoResetProperty(P_ID::SLUR_UOFF3);
      undoResetProperty(P_ID::SLUR_UOFF4);
      undoResetProperty(P_ID::AUTOPLACE);
      slurTie()->reset();
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SlurTieSegment::writeSlur(XmlWriter& xml, int no) const
      {
      if (autoplace() && visible() && (color() == Qt::black))
            return;

      xml.stag(QString("SlurSegment no=\"%1\"").arg(no));

      qreal _spatium = spatium();
      xml.tag("o1", ups(Grip::START).off   / _spatium);
      xml.tag("o2", ups(Grip::BEZIER1).off / _spatium);
      xml.tag("o3", ups(Grip::BEZIER2).off / _spatium);
      xml.tag("o4", ups(Grip::END).off     / _spatium);

      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   readSegment
//---------------------------------------------------------

void SlurTieSegment::read(XmlReader& e)
      {
      qreal _spatium = spatium();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "o1")
                  ups(Grip::START).off = e.readPoint() * _spatium;
            else if (tag == "o2")
                  ups(Grip::BEZIER1).off = e.readPoint() * _spatium;
            else if (tag == "o3")
                  ups(Grip::BEZIER2).off = e.readPoint() * _spatium;
            else if (tag == "o4")
                  ups(Grip::END).off = e.readPoint() * _spatium;
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      setAutoplace(false);
      }

//---------------------------------------------------------
//   drawEditMode
//---------------------------------------------------------

void SlurTieSegment::drawEditMode(QPainter* p, EditData& ed)
      {
      QPolygonF polygon(7);
      polygon[0] = QPointF(ed.grip[int(Grip::START)].center());
      polygon[1] = QPointF(ed.grip[int(Grip::BEZIER1)].center());
      polygon[2] = QPointF(ed.grip[int(Grip::SHOULDER)].center());
      polygon[3] = QPointF(ed.grip[int(Grip::BEZIER2)].center());
      polygon[4] = QPointF(ed.grip[int(Grip::END)].center());
      polygon[5] = QPointF(ed.grip[int(Grip::DRAG)].center());
      polygon[6] = QPointF(ed.grip[int(Grip::START)].center());
      p->setPen(QPen(MScore::frameMarginColor, 0.0));
      p->drawPolyline(polygon);

      p->setPen(QPen(MScore::defaultColor, 0.0));
      for (int i = 0; i < ed.grips; ++i) {
            p->setBrush(Grip(i) == ed.curGrip ? MScore::frameMarginColor : Qt::NoBrush);
            p->drawRect(ed.grip[i]);
            }
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::SlurTie(Score* s)
   : Spanner(s)
      {
      _slurDirection = Direction::AUTO;
      _up            = true;
      _lineType      = 0;     // default is solid
      }

SlurTie::SlurTie(const SlurTie& t)
   : Spanner(t)
      {
      _up            = t._up;
      _slurDirection = t._slurDirection;
      _lineType      = t._lineType;
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::~SlurTie()
      {
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SlurTie::writeProperties(XmlWriter& xml) const
      {
      Element::writeProperties(xml);
      if (track() != track2() && track2() != -1)
            xml.tag("track2", track2());
      int idx = 0;
      for (const SpannerSegment* ss : spannerSegments())
            ((SlurTieSegment*)ss)->writeSlur(xml, idx++);
      writeProperty(xml, P_ID::SLUR_DIRECTION);
      writeProperty(xml, P_ID::LINE_TYPE);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SlurTie::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (readProperty(tag, e, P_ID::SLUR_DIRECTION))
            ;
      else if (tag == "lineType")
            _lineType = e.readInt();
      else if (tag == "SlurSegment") {
            SlurTieSegment* s = newSlurTieSegment();
            s->read(e);
            add(s);
            }
      else if (!Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   undoSetLineType
//---------------------------------------------------------

void SlurTie::undoSetLineType(int t)
      {
      undoChangeProperty(P_ID::LINE_TYPE, t);
      }

//---------------------------------------------------------
//   undoSetSlurDirection
//---------------------------------------------------------

void SlurTie::undoSetSlurDirection(Direction d)
      {
      undoChangeProperty(P_ID::SLUR_DIRECTION, QVariant::fromValue<Direction>(d));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant SlurTie::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::LINE_TYPE:
                  return lineType();
            case P_ID::SLUR_DIRECTION:
                  return QVariant::fromValue<Direction>(slurDirection());
            default:
                  return Spanner::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SlurTie::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::LINE_TYPE:
                  setLineType(v.toInt());
                  break;
            case P_ID::SLUR_DIRECTION:
                  setSlurDirection(v.value<Direction>());
                  break;
            default:
                  return Spanner::setProperty(propertyId, v);
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SlurTie::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_TYPE:
                  return 0;
            case P_ID::SLUR_DIRECTION:
                  return QVariant::fromValue<Direction>(Direction::AUTO);
            default:
                  return Spanner::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   firstNoteRestSegmentX
//    in System() coordinates
//    returns the position just after the last non-chordrest segment
//---------------------------------------------------------

qreal SlurTie::firstNoteRestSegmentX(System* system)
      {
      for (const MeasureBase* mb : system->measures()) {
            if (mb->isMeasure()) {
                  const Measure* measure = static_cast<const Measure*>(mb);
                  for (const Segment* seg = measure->first(); seg; seg = seg->next()) {
                        if (seg->isChordRestType()) {
                              // first CR found; back up to previous segment
                              seg = seg->prev();
                              if (seg) {
                                    // find maximum width
                                    qreal width = 0.0;
                                    int n = score()->nstaves();
                                    for (int i = 0; i < n; ++i) {
                                          if (!system->staff(i)->show())
                                                continue;
                                          Element* e = seg->element(i * VOICES);
                                          if (e)
                                                width = qMax(width, e->width());
                                          }
                                    return seg->measure()->pos().x() + seg->pos().x() + width;
                                    }
                              else
                                    return 0.0;
                              }
                        }
                  }
            }
      qDebug("firstNoteRestSegmentX: did not find segment");
      return 0.0;
      }

//---------------------------------------------------------
//   fixupSegments
//---------------------------------------------------------

void SlurTie::fixupSegments(unsigned nsegs)
      {
      unsigned onsegs = spannerSegments().size();
      if (nsegs > onsegs) {
            for (unsigned i = onsegs; i < nsegs; ++i) {
                  SpannerSegment* s;
                  if (!delSegments.empty()) {
                        s = delSegments.dequeue();
                        }
                  else {
                        s = newSlurTieSegment();
                        }
                  s->setTrack(track());
                  add(s);
                  }
            }
      else if (nsegs < onsegs) {
            for (unsigned i = nsegs; i < onsegs; ++i) {
                  SpannerSegment* s = spannerSegments().takeLast();
                  s->setSystem(0);
                  delSegments.enqueue(s);  // cannot delete: used in SlurSegment->edit()
                  }
            }
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void SlurTie::reset()
      {
      Element::reset();
      undoResetProperty(P_ID::SLUR_DIRECTION);
      undoResetProperty(P_ID::LINE_TYPE);
      }

}

