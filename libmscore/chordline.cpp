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

const char* scorelineNames[] = {
      QT_TRANSLATE_NOOP("Ms", "Fall"),
      QT_TRANSLATE_NOOP("Ms", "Doit"),
      QT_TRANSLATE_NOOP("Ms", "Plop"),
      QT_TRANSLATE_NOOP("Ms", "Scoop"),
      };

//---------------------------------------------------------
//   ChordLine
//---------------------------------------------------------

ChordLine::ChordLine(Score* s)
   : Element(s, ElementFlag::MOVABLE)
      {
      modified = false;
      _chordLineType = ChordLineType::NOTYPE;
      _straight = false;
      _lengthX = 0.0;
      _lengthY = 0.0;
      }

ChordLine::ChordLine(const ChordLine& cl)
   : Element(cl)
      {
      path     = cl.path;
      modified = cl.modified;
      _chordLineType = cl._chordLineType;
      _straight = cl._straight;
      _lengthX = cl._lengthX;
      _lengthY = cl._lengthY;
      }

//---------------------------------------------------------
//   setChordLineType
//---------------------------------------------------------

void ChordLine::setChordLineType(ChordLineType st)
      {
      _chordLineType = st;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void ChordLine::layout()
      {
      if (!modified) {
            qreal x2 = 0;
            qreal y2 = 0;
            switch(_chordLineType) {
                  case ChordLineType::NOTYPE:
                        break;
                  case ChordLineType::FALL:
                        x2 = _initialLength;
                        y2 = _initialLength;
                        break;
                  case ChordLineType::PLOP:
                        x2 = -_initialLength;
                        y2 = -_initialLength;
                        break;
                  case ChordLineType::SCOOP:
                        x2 = -_initialLength;
                        y2 = _initialLength;
                        break;
                  default:
                  case ChordLineType::DOIT:
                        x2 = _initialLength;
                        y2 = -_initialLength;
                        break;
                  }
            if (_chordLineType != ChordLineType::NOTYPE) {
                  path = QPainterPath();
                  // chordlines to the right of the note
                  if (_chordLineType == ChordLineType::FALL || _chordLineType == ChordLineType::DOIT) {
                        if (_straight)
                              path.lineTo(x2, y2);
                        else
                              path.cubicTo(x2/2, 0.0, x2, y2/2, x2, y2);
                        }
                  // chordlines to the left of the note
                  else if (_chordLineType == ChordLineType::PLOP || _chordLineType == ChordLineType::SCOOP) {
                        if (_straight)
                              path.lineTo(x2, y2);
                        else
                              path.cubicTo(0.0, y2/2, x2/2, y2, x2, y2);
                        }
                  }
            }

      qreal _spatium = spatium();
      if (parent()) {
            Note* note = chord()->upNote();
            QPointF p(note->pos());
            // chordlines to the right of the note
            if (_chordLineType == ChordLineType::FALL || _chordLineType == ChordLineType::DOIT)
                  setPos(p.x() + note->bboxRightPos() + _spatium * .2, p.y());
            // chordlines to the left of the note
            if (_chordLineType == ChordLineType::PLOP)
                  setPos(p.x() + note->bboxRightPos() * .25, p.y() - note->headHeight() * .75);
            if (_chordLineType == ChordLineType::SCOOP) {
                  qreal x = p.x() + (chord()->up() ? note->bboxRightPos() * .25 : _spatium * -.2);
                  setPos(x, p.y() + note->headHeight() * .75);
                  }
            }
      else
            setPos(0.0, 0.0);
      QRectF r(path.boundingRect());
      int x1, y1, width, height = 0;

      x1 = r.x() * _spatium;
      y1 = r.y() * _spatium;
      width = r.width() * _spatium;
      height = r.height() * _spatium;
      bbox().setRect(x1, y1, width, height);
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
             else if (tag == "lengthX")
                  setLengthX(e.readInt());
             else if (tag == "lengthY")
                  setLengthY(e.readInt());

            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ChordLine::write(XmlWriter& xml) const
      {
      xml.stag(name());
      xml.tag("subtype", int(_chordLineType));
      xml.tag("straight", _straight, false);
      xml.tag("lengthX", _lengthX, 0.0);
      xml.tag("lengthY", _lengthY, 0.0);
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
            painter->scale(_spatium, _spatium);
            painter->setPen(QPen(curColor(), .15, Qt::SolidLine));
            painter->setBrush(Qt::NoBrush);

            QPainterPath pathOffset = path;
            qreal offset = 0.5;

            if (_chordLineType == ChordLineType::FALL)
                  pathOffset.translate(offset, -offset);
            else if (_chordLineType == ChordLineType::DOIT)
                  pathOffset.translate(offset, offset);
            else if (_chordLineType == ChordLineType::SCOOP)
                  pathOffset.translate(-offset, offset);
            else if (_chordLineType == ChordLineType::PLOP)
                  pathOffset.translate(-offset, -offset);

            painter->drawPath(pathOffset);
            painter->scale(1.0/_spatium, 1.0/_spatium);
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

void ChordLine::editDrag(EditData& ed)
      {
      int n = path.elementCount();
      QPainterPath p;
      qreal sp = spatium();
      _lengthX += ed.delta.x();
      _lengthY += ed.delta.y();

      // used to limit how grips can affect the slide, stops the user from being able to turn one kind of slide into another
      int slideBoundary = 5;
      if ((_chordLineType == ChordLineType::PLOP || _chordLineType == ChordLineType::FALL) && _lengthY < -slideBoundary)
            _lengthY = -slideBoundary;
      else if ((_chordLineType == ChordLineType::FALL || _chordLineType == ChordLineType::DOIT) && _lengthX < -slideBoundary)
            _lengthX = -slideBoundary;
      else if ((_chordLineType == ChordLineType::DOIT || _chordLineType == ChordLineType::SCOOP) && _lengthY > slideBoundary)
            _lengthY = slideBoundary;
      else if ((_chordLineType == ChordLineType::SCOOP || _chordLineType == ChordLineType::PLOP)  && _lengthX > slideBoundary)
            _lengthX = slideBoundary;

      qreal dx = ed.delta.x() / sp;
      qreal dy = ed.delta.y() / sp;
      for (int i = 0; i < n; ++i) {
            const QPainterPath::Element& e = (_straight ? path.elementAt(1) : path.elementAt(i));
            if (_straight) {
                  if (i > 0)
                        break;
                  // check the gradient of the line
                  const QPainterPath::Element& startPoint = path.elementAt(0);
                  if ( (_chordLineType == ChordLineType::FALL && (e.x + dx < startPoint.x || e.y + dy < startPoint.y))  ||
                       (_chordLineType == ChordLineType::DOIT && (e.x + dx < startPoint.x || e.y + dy > startPoint.y))  ||
                       (_chordLineType == ChordLineType::SCOOP && (e.x + dx > startPoint.x || e.y + dy < startPoint.y)) ||
                       (_chordLineType == ChordLineType::PLOP && (e.x + dx > startPoint.x || e.y + dy > startPoint.y)) )
                              return;
                  }

            qreal x = e.x;
            qreal y = e.y;
            if (ed.curGrip == Grip(i)) {
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
                        if (Grip(i + 1) == ed.curGrip) {
                              x2 += dx;
                              y2 += dy;
                              }
                        else if (Grip(i + 2) == ed.curGrip) {
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

void ChordLine::updateGrips(EditData& ed) const
      {
      qreal sp = spatium();
      int n    = path.elementCount();
      QPointF cp(pagePos());
      if (_straight) {
            // limit the number of grips to one
            qreal offset = 0.5 * sp;

            if (_chordLineType == ChordLineType::FALL)
                  ed.grip[0].translate(QPointF(offset, -offset));
            else if (_chordLineType == ChordLineType::DOIT)
                   ed.grip[0].translate(QPointF(offset, offset));
            else if (_chordLineType == ChordLineType::SCOOP)
                   ed.grip[0].translate(QPointF(-offset, offset));
            else if (_chordLineType == ChordLineType::PLOP)
                   ed.grip[0].translate(QPointF(-offset, -offset));

            // translate on the length and height - stops the grips from going past boundaries of slide
            ed.grip[0].translate(cp + QPointF(path.elementAt(1).x * sp, path.elementAt(1).y * sp));
            }
      else  {
            for (int i = 0; i < n; ++i)
                  ed.grip[i].translate(cp + QPointF(path.elementAt(i).x * sp, path.elementAt(i).y * sp));
            }
      }

//---------------------------------------------------------
//   grips
//---------------------------------------------------------

void ChordLine::startEdit(EditData& ed)
      {
      Element::startEdit(ed);
      if (_straight) {
            ed.curGrip = Grip(0);
            ed.grips   = 1;
            }
      else {
            ed.grips   = path.elementCount();
            ed.curGrip = Grip(ed.grips-1);
            }
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString ChordLine::accessibleInfo() const
      {
      QString rez = Element::accessibleInfo();
      if(chordLineType() != ChordLineType::NOTYPE)
            rez = QString("%1: %2").arg(rez).arg(scorelineNames[static_cast<int>(chordLineType()) - 1]);
      return rez;
      }

}

