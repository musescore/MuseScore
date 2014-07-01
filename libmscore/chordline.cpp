//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "chordline.h"
#include "xml.h"
#include "chord.h"
#include "measure.h"
#include "system.h"
#include "note.h"

namespace Ms {

//---------------------------------------------------------
//   ChordLine
//---------------------------------------------------------

ChordLine::ChordLine(Score* s)
   : Element(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      modified = false;
      _chordLineType = ChordLineType::NOTYPE;
      _straight = false;
      }

ChordLine::ChordLine(const ChordLine& cl)
   : Element(cl)
      {
      path     = cl.path;
      modified = cl.modified;
      _chordLineType = cl._chordLineType;
      _straight = cl._straight;
      }

//---------------------------------------------------------
//   setChordLineType
//---------------------------------------------------------

void ChordLine::setChordLineType(ChordLineType st)
      {
      qreal x2 = 0;
      qreal y2 = 0;
      switch(st) {
            case ChordLineType::NOTYPE:
                  break;
            case ChordLineType::FALL:
                  x2 = 2;
                  y2 = 2;
                  break;
            case ChordLineType::PLOP:
                  x2 = -2;
                  y2 = -2;
                  break;
            case ChordLineType::SCOOP:
                  x2 = -2;
                  y2 = 2;
                  break;
            default:
            case ChordLineType::DOIT:
                  x2 = 2;
                  y2 = -2;
                  break;
            }
      if (st != ChordLineType::NOTYPE) {
            path = QPainterPath();
            // chordlines to the right of the note
            if (st == ChordLineType::FALL || st == ChordLineType::DOIT)
                  path.cubicTo(x2/2, 0.0, x2, y2/2, x2, y2);
            // chordlines to the left of the note
            if (st == ChordLineType::PLOP || st == ChordLineType::SCOOP)
                  path.cubicTo(0.0, y2/2, x2/2, y2, x2, y2);
            }
      _chordLineType = st;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void ChordLine::layout()
      {
      qreal _spatium = spatium();
      if (parent()) {
            Note* note = chord()->upNote();
            QPointF p(note->pos());
            // chordlines to the right of the note
            if (_chordLineType == ChordLineType::FALL || _chordLineType == ChordLineType::DOIT)
                  setPos(p.x() + note->headWidth() + _spatium * .2, p.y());
            // chordlines to the left of the note
            if (_chordLineType == ChordLineType::PLOP)
                  setPos(p.x() + note->headWidth() * .25, p.y() - note->headHeight() * .75);
            if (_chordLineType == ChordLineType::SCOOP) {
                  qreal x = p.x() + (chord()->up() ? note->headWidth() * .25 : _spatium * -.2);
                  setPos(x, p.y() + note->headHeight() * .75);
                  }
            }
      else
            setPos(0.0, 0.0);
      QRectF r(path.boundingRect());
      bbox().setRect(r.x() * _spatium, r.y() * _spatium, r.width() * _spatium, r.height() * _spatium);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordLine::read(XmlReader& e)
      {
      path = QPainterPath();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Path") {
                  path = QPainterPath();
                  QPointF curveTo;
                  QPointF p1;
                  int state = 0;
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "Element") {
                              int type = e.intAttribute("type");
                              qreal x  = e.doubleAttribute("x");
                              qreal y  = e.doubleAttribute("y");
                              switch(QPainterPath::ElementType(type)) {
                                    case QPainterPath::MoveToElement:
                                          path.moveTo(x, y);
                                          break;
                                    case QPainterPath::LineToElement:
                                          path.lineTo(x, y);
                                          break;
                                    case QPainterPath::CurveToElement:
                                          curveTo.rx() = x;
                                          curveTo.ry() = y;
                                          state = 1;
                                          break;
                                    case QPainterPath::CurveToDataElement:
                                          if (state == 1) {
                                                p1.rx() = x;
                                                p1.ry() = y;
                                                state = 2;
                                                }
                                          else if (state == 2) {
                                                path.cubicTo(curveTo, p1, QPointF(x, y));
                                                state = 0;
                                                }
                                          break;
                                    }
                              e.skipCurrentElement(); //needed to go to next Element in Path
                              }
                        else
                              e.unknown();
                        }
                  modified = true;
                  }
            else if (tag == "subtype")
                  setChordLineType(ChordLineType(e.readInt()));
             else if (tag == "straight")
                  setStraight(e.readInt());
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ChordLine::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("subtype", int(_chordLineType));
      xml.tag("straight", _straight, false);
      Element::writeProperties(xml);
      if (modified) {
            int n = path.elementCount();
            xml.stag("Path");
            for (int i = 0; i < n; ++i) {
                  const QPainterPath::Element& e = path.elementAt(i);
                  xml.tagE(QString("Element type=\"%1\" x=\"%2\" y=\"%3\"")
                     .arg(int(e.type)).arg(e.x).arg(e.y));
                  }
            xml.etag();
            }
      xml.etag();
      }

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void ChordLine::draw(QPainter* painter) const
      {
      qreal _spatium = spatium();

      if (this->isStraight()) {
            painter->save();
            QPen pen(curColor());
            pen.setWidthF(_spatium * .15);
            pen.setCapStyle(Qt::RoundCap);
            painter->setPen(pen);
            if (_chordLineType == ChordLineType::FALL)
                  painter->drawLine(QLineF(20.0, -4.0, 30.0, 4.0));
            else if (_chordLineType == ChordLineType::DOIT)
                  painter->drawLine(QLineF(20.0, 4.0, 30.0, -4.0));
            else if (_chordLineType == ChordLineType::SCOOP) {
                  painter->translate(-50.0, -5.0);
                  painter->drawLine(QLineF(20.0, -4.0, 30.0, 4.0));
                  }
            else if (_chordLineType == ChordLineType::PLOP) {
                  painter->translate(-50.0, 5.0);
                  painter->drawLine(QLineF(20.0, 4.0, 30.0, -4.0));
                  }
            painter->restore();
            }
      else  {
            painter->scale(_spatium, _spatium);
            painter->setPen(QPen(curColor(), .15, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(path);
            painter->scale(1.0/_spatium, 1.0/_spatium);
            }
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void ChordLine::editDrag(const EditData& ed)
      {
      int n = path.elementCount();
      QPainterPath p;
      qreal sp = spatium();
      qreal dx = ed.delta.x() / sp;
      qreal dy = ed.delta.y() / sp;
      for (int i = 0; i < n; ++i) {
            const QPainterPath::Element& e = path.elementAt(i);
            qreal x = e.x;
            qreal y = e.y;
            if (ed.curGrip == i) {
                  x += dx;
                  y += dy;
                  }
            switch(e.type) {
                  case QPainterPath::CurveToDataElement:
                        break;
                  case QPainterPath::MoveToElement:
                        p.moveTo(x, y);
                        break;
                  case QPainterPath::LineToElement:
                        p.lineTo(x, y);
                        break;
                  case QPainterPath::CurveToElement:
                        {
                        qreal x2 = path.elementAt(i+1).x;
                        qreal y2 = path.elementAt(i+1).y;
                        qreal x3 = path.elementAt(i+2).x;
                        qreal y3 = path.elementAt(i+2).y;
                        if (i + 1 == ed.curGrip) {
                              x2 += dx;
                              y2 += dy;
                              }
                        else if (i + 2 == ed.curGrip) {
                              x3 += dx;
                              y3 += dy;
                              }
                        p.cubicTo(x, y, x2, y2, x3, y3);
                        i += 2;
                        }
                        break;
                  }
            }
      path = p;
      modified = true;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void ChordLine::updateGrips(int* grips, int* defaultGrip, QRectF* grip) const
      {
      int n = path.elementCount();
      *grips = n;
      *defaultGrip = n - 1;
      QPointF cp(pagePos());
      qreal sp = spatium();
      for (int i = 0; i < n; ++i)
            grip[i].translate(cp + QPointF(path.elementAt(i).x * sp, path.elementAt(i).y * sp));
      }

}

