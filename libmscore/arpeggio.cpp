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

#include "arpeggio.h"
#include "sym.h"
#include "chord.h"
#include "note.h"
#include "score.h"
#include "staff.h"
#include "part.h"
#include "segment.h"
#include "property.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   Arpeggio
//---------------------------------------------------------

Arpeggio::Arpeggio(Score* s)
  : Element(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      _arpeggioType = ArpeggioType::NORMAL;
      setHeight(spatium() * 4);      // for use in palettes
      _span     = 1;
      _userLen1 = 0.0;
      _userLen2 = 0.0;
      }

//---------------------------------------------------------
//   setHeight
//---------------------------------------------------------

void Arpeggio::setHeight(qreal h)
      {
      _height = h;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Arpeggio::write(Xml& xml) const
      {
      if (!xml.canWrite(this)) return;
      xml.stag("Arpeggio");
      Element::writeProperties(xml);
      xml.tag("subtype", int(_arpeggioType));
      if (_userLen1 != 0.0)
            xml.tag("userLen1", _userLen1 / spatium());
      if (_userLen2 != 0.0)
            xml.tag("userLen2", _userLen2 / spatium());
      if (_span != 1)
            xml.tag("span", _span);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Arpeggio::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  _arpeggioType = ArpeggioType(e.readInt());
            else if (tag == "userLen1")
                  _userLen1 = e.readDouble() * spatium();
            else if (tag == "userLen2")
                  _userLen2 = e.readDouble() * spatium();
            else if (tag == "span")
                  _span = e.readInt();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   symbolLine
//    construct a string of symbols aproximating width w
//---------------------------------------------------------

void Arpeggio::symbolLine(SymId end, SymId fill)
      {
      qreal y1 = -_userLen1;
      qreal y2 = _height + _userLen2;
      qreal w   = y2 - y1;
      qreal mag = magS();
      ScoreFont* f = score()->scoreFont();
      const QString& fillString = f->toString(fill);

      symbols.clear();
      symbols.append(f->toString(end));
      qreal w1 = f->bbox(end, mag).width();
      qreal w2 = f->width(fill, mag);
      int n    = lrint((w - w1) / w2);
      for (int i = 0; i < n; ++i)
           symbols.prepend(fillString);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Arpeggio::layout()
      {
      qreal y1 = -_userLen1;
      qreal y2 = _height + _userLen2;

      switch (arpeggioType()) {
            case ArpeggioType::NORMAL: {
                  symbolLine(SymId::wiggleArpeggiatoUp, SymId::wiggleArpeggiatoUp);
                  // string is rotated -90 degrees
                  QRectF r(symBbox(symbols));
                  setbbox(QRectF(0.0, -r.x() + y1, r.height(), r.width()));
                  }
                  break;

            case ArpeggioType::UP: {
                  symbolLine(SymId::wiggleArpeggiatoUpArrow, SymId::wiggleArpeggiatoUp);
                  // string is rotated -90 degrees
                  QRectF r(symBbox(symbols));
                  setbbox(QRectF(0.0, -r.x() + y1, r.height(), r.width()));
                  }
                  break;

            case ArpeggioType::DOWN: {
                  symbolLine(SymId::wiggleArpeggiatoDownArrow, SymId::wiggleArpeggiatoDown);
                  // string is rotated +90 degrees
                  QRectF r(symBbox(symbols));
                  setbbox(QRectF(0.0, r.x() + y1, r.height(), r.width()));
                  }
                  break;

            case ArpeggioType::UP_STRAIGHT: {
                  qreal _spatium = spatium();
                  qreal x1 = _spatium * .5;
                  qreal w  = symBbox(SymId::arrowheadBlackUp).width();
                  qreal lw = score()->styleS(StyleIdx::ArpeggioLineWidth).val() * _spatium;
                  setbbox(QRectF(x1 - w * .5, y1, w, y2 - y1 + lw * .5));
                  }
                  break;

            case ArpeggioType::DOWN_STRAIGHT: {
                  qreal _spatium = spatium();
                  qreal x1 = _spatium * .5;
                  qreal w  = symBbox(SymId::arrowheadBlackDown).width();
                  qreal lw = score()->styleS(StyleIdx::ArpeggioLineWidth).val() * _spatium;
                  setbbox(QRectF(x1 - w * .5, y1 - lw * .5, w, y2 - y1 + lw * .5));
                  }
                  break;

            case ArpeggioType::BRACKET: {
                  qreal _spatium = spatium();
                  qreal lw = score()->styleS(StyleIdx::ArpeggioLineWidth).val() * _spatium * .5;
                  qreal w  = score()->styleS(StyleIdx::ArpeggioHookLen).val() * _spatium;
                  setbbox(QRectF(0.0, y1, w, y2-y1).adjusted(-lw, -lw, lw, lw));
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Arpeggio::draw(QPainter* p) const
      {
      qreal _spatium = spatium();

      p->setPen(curColor());

      qreal y1 = -_userLen1;
      qreal y2 = _height + _userLen2;

      p->setPen(QPen(curColor(),
         score()->styleS(StyleIdx::ArpeggioLineWidth).val() * _spatium,
         Qt::SolidLine, Qt::RoundCap));

      switch (arpeggioType()) {
            case ArpeggioType::NORMAL:
            case ArpeggioType::UP:
                  {
                  QRectF r(symBbox(symbols));
                  p->rotate(-90.0);
                  drawSymbols(symbols, p, QPointF(-r.right() - y1, -r.bottom() + r.height()));
                  p->rotate(90.0);
                  }
                  break;

            case ArpeggioType::DOWN:
                  {
                  QRectF r(symBbox(symbols));
                  p->rotate(90.0);
                  drawSymbols(symbols, p, QPointF(-r.left() + y1, -r.top() - r.height()));
                  p->rotate(-90.0);
                  }
                  break;

            case ArpeggioType::UP_STRAIGHT:
                  {
                  QRectF r(symBbox(SymId::arrowheadBlackUp));
                  qreal x1 = _spatium * .5;
                  drawSymbol(SymId::arrowheadBlackUp, p, QPointF(x1 - r.width() * .5, y1 - r.top()));
                  y1 -= r.top() * .5;
                  p->drawLine(QLineF(x1, y1, x1, y2));
                  }
                  break;

            case ArpeggioType::DOWN_STRAIGHT:
                  {
                  QRectF r(symBbox(SymId::arrowheadBlackDown));
                  qreal x1 = _spatium * .5;

                  drawSymbol(SymId::arrowheadBlackDown, p, QPointF(x1 - r.width() * .5, y2 - r.bottom()));
                  y2 += r.top() * .5;
                  p->drawLine(QLineF(x1, y1, x1, y2));
                  }
                  break;

            case ArpeggioType::BRACKET:
                  {
                  qreal w = score()->styleS(StyleIdx::ArpeggioHookLen).val() * _spatium;
                  p->drawLine(QLineF(0.0, y1, 0.0, y2));
                  p->drawLine(QLineF(0.0, y1, w, y1));
                  p->drawLine(QLineF(0.0, y2, w, y2));
                  }
                  break;
            }
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Arpeggio::updateGrips(int* grips, int* defaultGrip, QRectF* grip) const
      {
      *grips   = 2;
      *defaultGrip = 1;
      QPointF p1(0.0, -_userLen1);
      QPointF p2(0.0, _height + _userLen2);
      grip[0].translate(pagePos() + p1);
      grip[1].translate(pagePos() + p2);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Arpeggio::editDrag(const EditData& ed)
      {
      qreal d = ed.delta.y();
      if (ed.curGrip == 0)
            _userLen1 -= d;
      else if (ed.curGrip == 1)
            _userLen2 += d;
      layout();
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Arpeggio::dragAnchor() const
      {
      Chord* c = chord();
      if (c)
            return QLineF(pagePos(), c->upNote()->pagePos());
      return QLineF();
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF Arpeggio::gripAnchor(int n) const
      {
      Chord* c = chord();
      if (c == 0)
            return QPointF();
      if (n == 0)
            return c->upNote()->pagePos();
      else if (n == 1) {
            Note* dnote = c->downNote();
            int btrack  = track() + (_span - 1) * VOICES;
            ChordRest* bchord = static_cast<ChordRest*>(c->segment()->element(btrack));
            if (bchord && bchord->type() == Element::Type::CHORD)
                  dnote = static_cast<Chord*>(bchord)->downNote();
            return dnote->pagePos();
            }
      return QPointF();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Arpeggio::startEdit(MuseScoreView*, const QPointF&)
      {
      undoPushProperty(P_ID::ARP_USER_LEN1);
      undoPushProperty(P_ID::ARP_USER_LEN2);
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Arpeggio::edit(MuseScoreView*, int curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      if (curGrip != 1 || !(modifiers & Qt::ShiftModifier))
            return false;

      if (key == Qt::Key_Down) {
            Staff* s = staff();
            Part* part = s->part();
            int n = part->nstaves();
            int ridx = part->staves()->indexOf(s);
            if (ridx >= 0) {
                  if (_span + ridx < n)
                        ++_span;
                  }
            }
      else if (key == Qt::Key_Up) {
            if (_span > 1)
                  --_span;
            }
      else
            return false;
      layout();
      Chord* c = chord();
      rxpos() = -(width() + spatium() * .5);
      c->layoutArpeggio2();
      return true;
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Arpeggio::spatiumChanged(qreal oldValue, qreal newValue)
      {
      _userLen1 *= (newValue / oldValue);
      _userLen2 *= (newValue / oldValue);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Arpeggio::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return e->type() == Element::Type::ARPEGGIO;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Arpeggio::drop(const DropData& data)
      {
      Element* e = data.element;
      switch(e->type()) {
            case Element::Type::ARPEGGIO:
                  {
                  Arpeggio* a = static_cast<Arpeggio*>(e);
                  if (parent())
                        score()->undoRemoveElement(this);
                  a->setTrack(track());
                  a->setParent(parent());
                  score()->undoAddElement(a);
                  }
                  return e;
            default:
                  delete e;
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Arpeggio::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::ARP_USER_LEN1:
                  return userLen1();
            case P_ID::ARP_USER_LEN2:
                  return userLen2();
            default:
                  break;
            }
      return Element::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Arpeggio::setProperty(P_ID propertyId, const QVariant& val)
      {
      switch(propertyId) {
            case P_ID::ARP_USER_LEN1:
                  setUserLen1(val.toDouble());
                  break;
            case P_ID::ARP_USER_LEN2:
                  setUserLen2(val.toDouble());
                  break;
            default:
                  if (!Element::setProperty(propertyId, val))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      return true;
      }


}

