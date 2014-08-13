//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "bracket.h"
#include "xml.h"
#include "style.h"
#include "utils.h"
#include "staff.h"
#include "score.h"
#include "system.h"
#include "sym.h"
#include "mscore.h"

namespace Ms {

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

Bracket::Bracket(Score* s)
   : Element(s)
      {
      _bracketType = BracketType::BRACE;
      h2           = 3.5 * spatium();
      _column      = 0;
      _span        = 0;
      _firstStaff  = 0;
      _lastStaff   = 0;
      setGenerated(true);     // brackets are not saved
      }

//---------------------------------------------------------
//   setHeight
//---------------------------------------------------------

void Bracket::setHeight(qreal h)
      {
      h2 = h * .5;
      }

//---------------------------------------------------------
//   width
//---------------------------------------------------------

qreal Bracket::width() const
      {
      qreal w = 0;
      if (bracketType() == BracketType::BRACE)
            w = point(score()->styleS(StyleIdx::akkoladeWidth) + score()->styleS(StyleIdx::akkoladeBarDistance));
      else if (bracketType() == BracketType::NORMAL)
            w = point(score()->styleS(StyleIdx::bracketWidth) + score()->styleS(StyleIdx::bracketDistance));
      else if (bracketType() == BracketType::SQUARE)
            w = point(score()->styleS(StyleIdx::staffLineWidth) + Spatium(0.5));
      else if (bracketType() == BracketType::LINE)
            w = point(0.67f * score()->styleS(StyleIdx::bracketWidth) + score()->styleS(StyleIdx::bracketDistance));
      return w;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Bracket::layout()
      {
      path = QPainterPath();
      if (h2 == 0.0)
            return;

      if (bracketType() == BracketType::BRACE) {
            qreal w = point(score()->styleS(StyleIdx::akkoladeWidth));

#define XM(a) (a+700)*w/700
#define YM(a) (a+7100)*h2/7100

            path.moveTo( XM(   -8), YM(-2048));
            path.cubicTo(XM(   -8), YM(-3192), XM(-360), YM(-4304), XM( -360), YM(-5400)); // c 0
            path.cubicTo(XM( -360), YM(-5952), XM(-264), YM(-6488), XM(   32), YM(-6968)); // c 1
            path.cubicTo(XM(   36), YM(-6974), XM(  38), YM(-6984), XM(   38), YM(-6990)); // c 0
            path.cubicTo(XM(   38), YM(-7008), XM(  16), YM(-7024), XM(    0), YM(-7024)); // c 0
            path.cubicTo(XM(   -8), YM(-7024), XM( -22), YM(-7022), XM(  -32), YM(-7008)); // c 1
            path.cubicTo(XM( -416), YM(-6392), XM(-544), YM(-5680), XM( -544), YM(-4960)); // c 0
            path.cubicTo(XM( -544), YM(-3800), XM(-168), YM(-2680), XM( -168), YM(-1568)); // c 0
            path.cubicTo(XM( -168), YM(-1016), XM(-264), YM( -496), XM( -560), YM(  -16)); // c 1
            path.lineTo( XM( -560), YM(    0));  //  l 1
            path.lineTo( XM( -560), YM(   16));  //  l 1
            path.cubicTo(XM( -264), YM(  496), XM(-168), YM( 1016), XM( -168), YM( 1568)); // c 0
            path.cubicTo(XM( -168), YM( 2680), XM(-544), YM( 3800), XM( -544), YM( 4960)); // c 0
            path.cubicTo(XM( -544), YM( 5680), XM(-416), YM( 6392), XM(  -32), YM( 7008)); // c 1
            path.cubicTo(XM(  -22), YM( 7022), XM(  -8), YM( 7024), XM(    0), YM( 7024)); // c 0
            path.cubicTo(XM(   16), YM( 7024), XM(  38), YM( 7008), XM(   38), YM( 6990)); // c 0
            path.cubicTo(XM(   38), YM( 6984), XM(  36), YM( 6974), XM(   32), YM( 6968)); // c 1
            path.cubicTo(XM( -264), YM( 6488), XM(-360), YM( 5952), XM( -360), YM( 5400)); // c 0
            path.cubicTo(XM( -360), YM( 4304), XM(  -8), YM( 3192), XM(   -8), YM( 2048)); // c 0
            path.cubicTo(XM( -  8), YM( 1320), XM(-136), YM(  624), XM( -512), YM(    0)); // c 1
            path.cubicTo(XM( -136), YM( -624), XM(  -8), YM(-1320), XM(   -8), YM(-2048)); // c 0
            setbbox(path.boundingRect());
            }
      else if (bracketType() == BracketType::NORMAL) {
            qreal _spatium = spatium();
            qreal w = score()->styleS(StyleIdx::bracketWidth).val() * _spatium * .5;
            qreal x = -w;
            w      += symWidth(SymId::bracketTop);
            qreal bd = _spatium * .25;
            qreal y = - symHeight(SymId::bracketTop) - bd;
            qreal h = (-y + h2) * 2;
            bbox().setRect(x, y, w, h);
            }
      else if (bracketType() == BracketType::SQUARE) {
            qreal _spatium = spatium();
            qreal w = score()->styleS(StyleIdx::staffLineWidth).val() * _spatium * .5;
            qreal x = -w;
            qreal y = -w;
            qreal h = (h2 + w) * 2 ;
            w      += (.5 * spatium() + 3* w);
            bbox().setRect(x, y, w, h);
            }
      else if (bracketType() == BracketType::LINE) {
            qreal _spatium = spatium();
            qreal w = 0.67 * score()->styleS(StyleIdx::bracketWidth).val() * _spatium * .5;
            qreal x = -w;
            qreal bd = _spatium * .25;
            qreal y = -bd;
            qreal h = (-y + h2) * 2;
            bbox().setRect(x, y, w, h);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bracket::draw(QPainter* painter) const
      {
      if (h2 == 0.0)
            return;
      if (bracketType() == BracketType::BRACE) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QBrush(curColor()));
            painter->drawPath(path);
            }
      else if (bracketType() == BracketType::NORMAL) {
            qreal h = 2 * h2;
            qreal _spatium = spatium();
            qreal w = score()->styleS(StyleIdx::bracketWidth).val() * _spatium;
            QPen pen(curColor(), w, Qt::SolidLine, Qt::FlatCap);
            painter->setPen(pen);
            qreal bd   = _spatium * .25;
            painter->drawLine(QLineF(0.0, -bd, 0.0, h + bd));
            qreal x    =  -w * .5;
            qreal y1   = -bd;
            qreal y2   = h + bd;
            drawSymbol(SymId::bracketTop, painter, QPointF(x, y1));
            drawSymbol(SymId::bracketBottom, painter, QPointF(x, y2));
            }
      else if (bracketType() == BracketType::SQUARE) {
            qreal h = 2 * h2;
            qreal _spatium = spatium();
            qreal w = score()->styleS(StyleIdx::staffLineWidth).val() * _spatium;
            QPen pen(curColor(), w, Qt::SolidLine, Qt::SquareCap);
            painter->setPen(pen);
            painter->drawLine(QLineF(0.0, 0.0, 0.0, h));
            painter->drawLine(QLineF(0.0, 0.0, w + .5 *_spatium, 0.0));
            painter->drawLine(QLineF(0.0, h  , w + .5 *_spatium, h));
            }
      else if (bracketType() == BracketType::LINE) {
            qreal h = 2 * h2;
            qreal _spatium = spatium();
            qreal w = 0.67 * score()->styleS(StyleIdx::bracketWidth).val() * _spatium;
            QPen pen(curColor(), w, Qt::SolidLine, Qt::FlatCap);
            painter->setPen(pen);
            qreal bd   = _spatium * .25;
            painter->drawLine(QLineF(0.0, -bd, 0.0, h + bd));
            }
      }

//---------------------------------------------------------
//   Bracket::write
//---------------------------------------------------------

void Bracket::write(Xml& xml) const
      {
      switch(bracketType()) {
            case BracketType::BRACE:
                  xml.stag("Bracket type=\"Brace\"");
                  break;
            case BracketType::NORMAL:
                  xml.stag("Bracket");
                  break;
            case BracketType::SQUARE:
                  xml.stag("Bracket type=\"Square\"");
                  break;
            case BracketType::LINE:
                  xml.stag("Bracket type=\"Line\"");
                  break;
            case BracketType::NO_BRACKET:
                  break;
            }
      if (_column)
            xml.tag("level", _column);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Bracket::read
//---------------------------------------------------------

void Bracket::read(XmlReader& e)
      {
      QString t(e.attribute("type", "Normal"));

      if (t == "Normal")
            setBracketType(BracketType::NORMAL);
      else if (t == "Akkolade")  //compatibility, not used anymore
            setBracketType(BracketType::BRACE);
      else if (t == "Brace")
            setBracketType(BracketType::BRACE);
      else if (t == "Square")
            setBracketType(BracketType::SQUARE);
      else if (t == "Line")
            setBracketType(BracketType::LINE);
      else
            qDebug("unknown brace type <%s>", qPrintable(t));

      while (e.readNextStartElement()) {
            if (e.name() == "level")
                  _column = e.readInt();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Bracket::startEdit(MuseScoreView*, const QPointF&)
      {

      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Bracket::updateGrips(int* grips, int* defaultGrip, QRectF* grip) const
      {
      *grips = 1;
      *defaultGrip = 0;
      grip[0].translate(QPointF(0.0, h2 * 2) + pagePos());
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF Bracket::gripAnchor(int) const
      {
      return QPointF();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Bracket::endEdit()
      {
      endEditDrag();
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Bracket::editDrag(const EditData& ed)
      {
      h2 += ed.delta.y() * .5;
      layout();
      score()->setLayoutAll(false);
      }

//---------------------------------------------------------
//   endEditDrag
//    snap to nearest staff
//---------------------------------------------------------

void Bracket::endEditDrag()
      {
      qreal ay1 = pagePos().y();
      qreal ay2 = ay1 + h2 * 2;

      int staffIdx1 = staffIdx();
      int staffIdx2;
      int n = system()->staves()->size();
      if (staffIdx1 + 1 >= n)
            staffIdx2 = staffIdx1;
      else {
            qreal ay  = parent()->pagePos().y();
            System* s = system();
            qreal y   = s->staff(staffIdx1)->y() + ay;
            qreal h1  = staff()->height();

            for (staffIdx2 = staffIdx1 + 1; staffIdx2 < n; ++staffIdx2) {
                  qreal h = s->staff(staffIdx2)->y() + ay - y;
                  if (ay2 < (y + (h + h1) * .5))
                        break;
                  y += h;
                  }
            staffIdx2 -= 1;
            }

      qreal sy = system()->staff(staffIdx1)->y();
      qreal ey = system()->staff(staffIdx2)->y() + score()->staff(staffIdx2)->height();
      h2 = (ey - sy) * .5;
      score()->undoChangeBracketSpan(staff(), _column, staffIdx2 - staffIdx1 + 1);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Bracket::acceptDrop(const DropData& data) const
      {
      return data.element->type() == Element::Type::BRACKET;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Bracket::drop(const DropData& data)
      {
      Element* e = data.element;
      if (e->type() == Element::Type::BRACKET) {
            Bracket* b = static_cast<Bracket*>(e);
            b->setParent(parent());
            b->setTrack(track());
            b->setSpan(span());
            b->setFirstStaff(firstStaff());
            b->setLastStaff(lastStaff());
            b->setLevel(level());
            score()->undoRemoveElement(this);
            score()->undoAddElement(b);
            return b;
            }
      delete e;
      return 0;
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Bracket::edit(MuseScoreView*, int, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      if (!(modifiers & Qt::ShiftModifier))
            return false;

      if (key == Qt::Key_Left) {
            BracketType bt = staff()->bracket(_column);
            // search empty level
            int oldColumn = _column;
            staff()->setBracket(_column, BracketType::NO_BRACKET);
            for (;;) {
                  ++_column;
                  if (staff()->bracket(_column) == BracketType::NO_BRACKET)
                        break;
                  }
            staff()->setBracket(_column, bt);
            staff()->setBracketSpan(_column, _lastStaff - _firstStaff + 1);
            score()->moveBracket(staffIdx(), oldColumn, _column);
            score()->setLayoutAll(true);
            return true;
            }
      if (key == Qt::Key_Right) {
            if (_column == 0)
                  return true;
            int l = _column - 1;
            for (; l >= 0; --l) {
                  if (staff()->bracket(l) != BracketType::NO_BRACKET)
                        continue;
                  BracketType bt = staff()->bracket(_column);
                  staff()->setBracket(_column, BracketType::NO_BRACKET);
                  staff()->setBracket(l, bt);
                  staff()->setBracketSpan(l, _lastStaff - _firstStaff + 1);
                  score()->moveBracket(staffIdx(), _column, l);
                  score()->setLayoutAll(true);
                  break;
                  }
            return true;
            }
      return false;
      }

}

