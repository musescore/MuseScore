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

#include "global/log.h"

#include "measure.h"
#include "score.h"
#include "system.h"
#include "undo.h"
#include "slurtie.h"
#include "tie.h"
#include "chord.h"
#include "page.h"

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
//   gripAnchorLines
//---------------------------------------------------------

QVector<QLineF> SlurTieSegment::gripAnchorLines(Grip grip) const
      {
      QVector<QLineF> result;

      if (!system() || (grip != Grip::START && grip != Grip::END))
            return result;

      QPointF sp(system()->pagePos());
      QPointF pp(pagePos());
      QPointF p1(ups(Grip::START).p + pp);
      QPointF p2(ups(Grip::END).p + pp);

      QPointF anchorPosition;
      int gripIndex = static_cast<int>(grip);

      switch (spannerSegmentType()) {
            case SpannerSegmentType::SINGLE:
                  anchorPosition = (grip == Grip::START ? p1 : p2);
                  break;

            case SpannerSegmentType::BEGIN:
                  anchorPosition = (grip == Grip::START ? p1 : system()->abbox().topRight());
                  break;

            case SpannerSegmentType::MIDDLE:
                  anchorPosition = (grip == Grip::START ? sp : system()->abbox().topRight());
                  break;

            case SpannerSegmentType::END:
                  anchorPosition = (grip == Grip::START ? sp : p2);
                  break;
            }

      const Page* p = system()->page();
      const QPointF pageOffset = p ? p->pos() : QPointF();

      result << QLineF(anchorPosition, gripsPositions().at(gripIndex)).translated(pageOffset);

      return result;
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
      Element::spatiumChanged(oldValue, newValue);
      qreal diff = newValue / oldValue;
      for (UP& u : _ups)
            u.off *= diff;
      }

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<QPointF> SlurTieSegment::gripsPositions(const EditData&) const
      {
      const int ngrips = gripsCount();
      std::vector<QPointF> grips(ngrips);

      const QPointF p(pagePos());
      for (int i = 0; i < ngrips; ++i)
            grips[i] = _ups[i].p + _ups[i].off + p;

      return grips;
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void SlurTieSegment::startEditDrag(EditData& ed)
      {
      ElementEditData* eed = ed.getData(this);
      IF_ASSERT_FAILED(eed) {
            return;
            }
      for (auto i : { Pid::SLUR_UOFF1, Pid::SLUR_UOFF2, Pid::SLUR_UOFF3, Pid::SLUR_UOFF4, Pid::OFFSET })
            eed->pushProperty(i);
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void SlurTieSegment::endEditDrag(EditData& ed)
      {
      Element::endEditDrag(ed);
      triggerLayout();
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
                              Fraction tick = note->chord()->tick();
                              if ((g == Grip::END && tick > slurTie()->tick()) || (g == Grip::START && tick < slurTie()->tick2())) {
                                    if (km != (Qt::ShiftModifier | Qt::ControlModifier)) {
                                          Chord* c = note->chord();
                                          ed.view->setDropTarget(note);
                                          if (c->part() == spanner->part() && c != spanner->endCR())
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
                  setOffset(offset() + ed.delta);
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

QVariant SlurTieSegment::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::LINE_TYPE:
            case Pid::SLUR_DIRECTION:
                  return slurTie()->getProperty(propertyId);
            case Pid::SLUR_UOFF1:
                  return ups(Grip::START).off;
            case Pid::SLUR_UOFF2:
                  return ups(Grip::BEZIER1).off;
            case Pid::SLUR_UOFF3:
                  return ups(Grip::BEZIER2).off;
            case Pid::SLUR_UOFF4:
                  return ups(Grip::END).off;
            default:
                  return SpannerSegment::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SlurTieSegment::setProperty(Pid propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case Pid::LINE_TYPE:
            case Pid::SLUR_DIRECTION:
                  return slurTie()->setProperty(propertyId, v);
            case Pid::SLUR_UOFF1:
                  ups(Grip::START).off = v.toPointF();
                  break;
            case Pid::SLUR_UOFF2:
                  ups(Grip::BEZIER1).off = v.toPointF();
                  break;
            case Pid::SLUR_UOFF3:
                  ups(Grip::BEZIER2).off = v.toPointF();
                  break;
            case Pid::SLUR_UOFF4:
                  ups(Grip::END).off = v.toPointF();
                  break;
            default:
                  return SpannerSegment::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SlurTieSegment::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::LINE_TYPE:
            case Pid::SLUR_DIRECTION:
                  return slurTie()->propertyDefault(id);
            case Pid::SLUR_UOFF1:
            case Pid::SLUR_UOFF2:
            case Pid::SLUR_UOFF3:
            case Pid::SLUR_UOFF4:
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
      undoResetProperty(Pid::SLUR_UOFF1);
      undoResetProperty(Pid::SLUR_UOFF2);
      undoResetProperty(Pid::SLUR_UOFF3);
      undoResetProperty(Pid::SLUR_UOFF4);
      slurTie()->reset();
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void SlurTieSegment::undoChangeProperty(Pid pid, const QVariant& val, PropertyFlags ps)
      {
      if (pid == Pid::AUTOPLACE && (val.toBool() == true && !autoplace())) {
            // Switching autoplacement on. Save user-defined
            // placement properties to undo stack.
            undoPushProperty(Pid::SLUR_UOFF1);
            undoPushProperty(Pid::SLUR_UOFF2);
            undoPushProperty(Pid::SLUR_UOFF3);
            undoPushProperty(Pid::SLUR_UOFF4);
            // other will be saved in base classes.
            }
      SpannerSegment::undoChangeProperty(pid, val, ps);
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SlurTieSegment::writeSlur(XmlWriter& xml, int no) const
      {
      if (visible() && autoplace()
         && (color() == Qt::black)
         && offset().isNull()
         && ups(Grip::START).off.isNull()
         && ups(Grip::BEZIER1).off.isNull()
         && ups(Grip::BEZIER2).off.isNull()
         && ups(Grip::END).off.isNull()
         )
            return;

      xml.stag(this, QString("no=\"%1\"").arg(no));

      qreal _spatium = score()->spatium();
      if (!ups(Grip::START).off.isNull())
            xml.tag("o1", ups(Grip::START).off / _spatium);
      if (!ups(Grip::BEZIER1).off.isNull())
            xml.tag("o2", ups(Grip::BEZIER1).off / _spatium);
      if (!ups(Grip::BEZIER2).off.isNull())
            xml.tag("o3", ups(Grip::BEZIER2).off / _spatium);
      if (!ups(Grip::END).off.isNull())
            xml.tag("o4", ups(Grip::END).off / _spatium);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   readSegment
//---------------------------------------------------------

void SlurTieSegment::read(XmlReader& e)
      {
      qreal _spatium = score()->spatium();
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
            else if (!readProperties(e))
                  e.unknown();
            }
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
            // This must be done with an if-else statement rather than a ternary operator.
            // This is because there are two setBrush methods that take different types
            // of argument, either a Qt::BrushStyle or a QBrush. Since a QBrush can be
            // constructed from a QColour, passing Mscore::frameMarginColor works.
            // Qt::NoBrush is a Qt::BrushStyle, however, so if it is passed in a ternary
            // operator with a QColor, a new QColor will be created from it, and from that
            // a QBrush. Instead, what we really want to do is pass Qt::NoBrush as a
            // Qt::BrushStyle, therefore this requires two separate function calls:
            if (Grip(i) == ed.curGrip)
                  p->setBrush(MScore::frameMarginColor);
            else
                  p->setBrush(Qt::NoBrush);
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
      Spanner::writeProperties(xml);
      int idx = 0;
      for (const SpannerSegment* ss : spannerSegments())
            ((SlurTieSegment*)ss)->writeSlur(xml, idx++);
      writeProperty(xml, Pid::SLUR_DIRECTION);
      writeProperty(xml, Pid::LINE_TYPE);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SlurTie::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (readProperty(tag, e, Pid::SLUR_DIRECTION))
            ;
      else if (tag == "lineType")
            _lineType = e.readInt();
      else if (tag == "SlurSegment" || tag == "TieSegment") {
            const int idx = e.intAttribute("no", 0);
            const int n = int(spannerSegments().size());
            for (int i = n; i < idx; ++i)
                  add(newSlurTieSegment());
            SlurTieSegment* s = newSlurTieSegment();
            s->read(e);
            add(s);
            }
      else if (!Spanner::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SlurTie::read(XmlReader& e)
      {
      Spanner::read(e);
      }

//---------------------------------------------------------
//   undoSetLineType
//---------------------------------------------------------

void SlurTie::undoSetLineType(int t)
      {
      undoChangeProperty(Pid::LINE_TYPE, t);
      }

//---------------------------------------------------------
//   undoSetSlurDirection
//---------------------------------------------------------

void SlurTie::undoSetSlurDirection(Direction d)
      {
      undoChangeProperty(Pid::SLUR_DIRECTION, QVariant::fromValue<Direction>(d));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant SlurTie::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::LINE_TYPE:
                  return lineType();
            case Pid::SLUR_DIRECTION:
                  return QVariant::fromValue<Direction>(slurDirection());
            default:
                  return Spanner::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SlurTie::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::LINE_TYPE:
                  setLineType(v.toInt());
                  break;
            case Pid::SLUR_DIRECTION:
                  setSlurDirection(v.value<Direction>());
                  break;
            default:
                  return Spanner::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SlurTie::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::LINE_TYPE:
                  return 0;
            case Pid::SLUR_DIRECTION:
                  return QVariant::fromValue<Direction>(Direction::AUTO);
            default:
                  return Spanner::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   fixupSegments
//---------------------------------------------------------

void SlurTie::fixupSegments(unsigned nsegs)
      {
      Spanner::fixupSegments(nsegs, [this]() { return newSlurTieSegment(); });
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void SlurTie::reset()
      {
      Element::reset();
      undoResetProperty(Pid::SLUR_DIRECTION);
      undoResetProperty(Pid::LINE_TYPE);
      }

}

