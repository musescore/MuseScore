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
#include "bracketItem.h"

namespace Ms {

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

Bracket::Bracket(Score* s)
   : Element(s)
      {
      h2           = 3.5 * spatium();
      _firstStaff  = 0;
      _lastStaff   = 0;
      _bi          = 0;
      setGenerated(true);     // brackets are not saved
      }

Bracket::~Bracket()
      {
      }

#if 0
//---------------------------------------------------------
//   setSpan
//---------------------------------------------------------

void Bracket::setSpan(int v)
      {
      _span = v;
      if (bracketType() == BracketType::BRACE) {
            // total default height of a system of n staves / height of a 5 line staff
            _magx = v + ((v - 1) * score()->styleS(StyleIdx::akkoladeDistance).val() / 4.0);
            if (v == 1)
                  _braceSymbol = SymId::braceSmall;
            else if (v <= 2)
                  _braceSymbol = SymId::brace;
            else if (v <= 3)
                  _braceSymbol = SymId::braceLarge;
            else
                  _braceSymbol = SymId::braceLarger;
            if (!symIsValid(_braceSymbol))
                  _braceSymbol = SymId::brace;
            }
      }
#endif

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
      qreal w;
      switch (bracketType()) {
            case BracketType::BRACE:
                  if (score()->styleSt(StyleIdx::MusicalSymbolFont) == "Emmentaler" || score()->styleSt(StyleIdx::MusicalSymbolFont) == "Gonville")
                        w = score()->styleP(StyleIdx::akkoladeWidth) + score()->styleP(StyleIdx::akkoladeBarDistance);
                  else
                        w = (symWidth(_braceSymbol) * _magx) + score()->styleP(StyleIdx::akkoladeBarDistance);
                  break;
            case BracketType::NORMAL:
                  w = score()->styleP(StyleIdx::bracketWidth) + score()->styleP(StyleIdx::bracketDistance);
                  break;
            case BracketType::SQUARE:
                  w = score()->styleP(StyleIdx::staffLineWidth) + spatium() * .5;
                  break;
            case BracketType::LINE:
                  w = 0.67f * score()->styleP(StyleIdx::bracketWidth) + score()->styleP(StyleIdx::bracketDistance);
                  break;
            case BracketType::NO_BRACKET:
            default:
                  w = 0.0;
                  break;
            }
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

      _shape.clear();
      switch (bracketType()) {
            case BracketType::BRACE: {
                  if (score()->styleSt(StyleIdx::MusicalSymbolFont) == "Emmentaler" || score()->styleSt(StyleIdx::MusicalSymbolFont) == "Gonville") {
                        _braceSymbol = SymId::noSym;
                        qreal w = score()->styleP(StyleIdx::akkoladeWidth);

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
                        path.cubicTo(XM( -136), YM( -624), XM(  -8), YM(-1320), XM(   -8), YM(-2048)); // c 0*/
                        setbbox(path.boundingRect());
                        _shape.add(bbox());
                        }
                  else {
                        _braceSymbol = SymId::brace;
                        qreal h = h2 * 2;
                        qreal w = symWidth(_braceSymbol) * _magx;
                        bbox().setRect(0, 0, w, h);
                        _shape.add(bbox());
                        }
                  }
                  break;
            case BracketType::NORMAL: {
                  qreal _spatium = spatium();
                  qreal w = score()->styleP(StyleIdx::bracketWidth) * .5;
                  qreal x = -w;

                  qreal bd   = _spatium * .25;
                  _shape.add(QRectF(x, -bd, w * 2, 2 * (h2+bd)));
                  _shape.add(symBbox(SymId::bracketTop).translated(QPointF(-w, -bd)));
                  _shape.add(symBbox(SymId::bracketBottom).translated(QPointF(-w, bd + 2*h2)));

                  w      += symWidth(SymId::bracketTop);
                  qreal y = - symHeight(SymId::bracketTop) - bd;
                  qreal h = (-y + h2) * 2;
                  bbox().setRect(x, y, w, h);
                  }
                  break;
            case BracketType::SQUARE: {
                  qreal w = score()->styleP(StyleIdx::staffLineWidth) * .5;
                  qreal x = -w;
                  qreal y = -w;
                  qreal h = (h2 + w) * 2 ;
                  w      += (.5 * spatium() + 3* w);
                  bbox().setRect(x, y, w, h);
                  _shape.add(bbox());
                  }
                  break;
            case BracketType::LINE: {
                  qreal _spatium = spatium();
                  qreal w = 0.67 * score()->styleP(StyleIdx::bracketWidth) * .5;
                  qreal x = -w;
                  qreal bd = _spatium * .25;
                  qreal y = -bd;
                  qreal h = (-y + h2) * 2;
                  bbox().setRect(x, y, w, h);
                  _shape.add(bbox());
                  }
                  break;
            case BracketType::NO_BRACKET:
                  break;
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bracket::draw(QPainter* painter) const
      {
      if (h2 == 0.0)
            return;
      switch (bracketType()) {
            case BracketType::BRACE: {
                  if (_braceSymbol == SymId::noSym) {
                        painter->setPen(Qt::NoPen);
                        painter->setBrush(QBrush(curColor()));
                        painter->drawPath(path);
                        }
                  else {
                        qreal h        = 2 * h2;
                        qreal _spatium = spatium();
                        qreal mag      = h / (4 *_spatium);
                        painter->setPen(curColor());
                        painter->save();
                        painter->scale(_magx, mag);
                        drawSymbol(_braceSymbol, painter, QPointF(0, h/mag));
                        painter->restore();
                        }
                  }
                  break;
            case BracketType::NORMAL: {
                  qreal h        = 2 * h2;
                  qreal _spatium = spatium();
                  qreal w        = score()->styleP(StyleIdx::bracketWidth);
                  qreal bd       = _spatium * .25;
                  QPen pen(curColor(), w, Qt::SolidLine, Qt::FlatCap);
                  painter->setPen(pen);
                  painter->drawLine(QLineF(0.0, -bd - w * .5, 0.0, h + bd + w * .5));
                  qreal x    =  -w * .5;
                  qreal y1   = -bd;
                  qreal y2   = h + bd;
                  drawSymbol(SymId::bracketTop, painter, QPointF(x, y1));
                  drawSymbol(SymId::bracketBottom, painter, QPointF(x, y2));
                  }
                  break;
            case BracketType::SQUARE: {
                  qreal h = 2 * h2;
                  qreal _spatium = spatium();
                  qreal w = score()->styleP(StyleIdx::staffLineWidth);
                  QPen pen(curColor(), w, Qt::SolidLine, Qt::SquareCap);
                  painter->setPen(pen);
                  painter->drawLine(QLineF(0.0, 0.0, 0.0, h));
                  painter->drawLine(QLineF(0.0, 0.0, w + .5 *_spatium, 0.0));
                  painter->drawLine(QLineF(0.0, h  , w + .5 *_spatium, h));
                  }
                  break;
            case BracketType::LINE: {
                  qreal h = 2 * h2;
                  qreal _spatium = spatium();
                  qreal w = 0.67 * score()->styleP(StyleIdx::bracketWidth);
                  QPen pen(curColor(), w, Qt::SolidLine, Qt::FlatCap);
                  painter->setPen(pen);
                  qreal bd = _spatium * .25;
                  painter->drawLine(QLineF(0.0, -bd, 0.0, h + bd));
                  }
                  break;
            case BracketType::NO_BRACKET:
                  break;
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Bracket::startEdit(EditData& ed)
      {
      ed.grips   = 1;
      ed.curGrip = Grip::START;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Bracket::updateGrips(EditData& ed) const
      {
      ed.grip[0].translate(QPointF(0.0, h2 * 2) + pagePos());
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Bracket::endEdit(EditData& ed)
      {
      endEditDrag(ed);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Bracket::editDrag(EditData& ed)
      {
      h2 += ed.delta.y() * .5;
      layout();
      }

//---------------------------------------------------------
//   endEditDrag
//    snap to nearest staff
//---------------------------------------------------------

void Bracket::endEditDrag(EditData&)
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
      bracketItem()->undoChangeProperty(P_ID::BRACKET_SPAN, staffIdx2 - staffIdx1 + 1);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Bracket::acceptDrop(EditData& data) const
      {
      return data.element->type() == ElementType::BRACKET;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Bracket::drop(EditData& data)
      {
      Element* e = data.element;
      Bracket* b = 0;
      if (e->isBracket()) {
            b = toBracket(e);
            undoChangeProperty(P_ID::SYSTEM_BRACKET, int(b->bracketType()));
            }
      delete e;
      return b;
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Bracket::edit(EditData& ed)
      {
      if (!(ed.modifiers & Qt::ShiftModifier))
            return false;

      if (ed.key == Qt::Key_Left) {
            bracketItem()->undoChangeProperty(P_ID::BRACKET_COLUMN, bracketItem()->column()+1);
            return true;
            }
      if (ed.key == Qt::Key_Right) {
            if (bracketItem()->column() == 0)
                  return true;
            bracketItem()->undoChangeProperty(P_ID::BRACKET_COLUMN, bracketItem()->column()-1);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Bracket::getProperty(P_ID id) const
      {
      return _bi->getProperty(id);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Bracket::setProperty(P_ID id, const QVariant& v)
      {
      return _bi->setProperty(id, v);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Bracket::propertyDefault(P_ID id) const
      {
      return _bi->propertyDefault(id);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Bracket::setSelected(bool f)
      {
      _bi->setSelected(f);
      Element::setSelected(f);
      }

//---------------------------------------------------------
//   Bracket::write
//    used only for palettes
//---------------------------------------------------------

void Bracket::write(XmlWriter& xml) const
      {
      switch (_bi->bracketType()) {
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
      if (_bi->column())
            xml.tag("level", _bi->column());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Bracket::read
//    used only for palettes
//---------------------------------------------------------

void Bracket::read(XmlReader& e)
      {
      QString t(e.attribute("type", "Normal"));
      _bi = new BracketItem(score());

      if (t == "Normal")
            _bi->setBracketType(BracketType::NORMAL);
      else if (t == "Akkolade")  //compatibility, not used anymore
            _bi->setBracketType(BracketType::BRACE);
      else if (t == "Brace")
            _bi->setBracketType(BracketType::BRACE);
      else if (t == "Square")
            _bi->setBracketType(BracketType::SQUARE);
      else if (t == "Line")
            _bi->setBracketType(BracketType::LINE);
      else
            qDebug("unknown brace type <%s>", qPrintable(t));

      while (e.readNextStartElement()) {
            if (e.name() == "level")
                  _bi->setColumn(e.readInt());
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }


}

