//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: arpeggio.cpp 5149 2011-12-29 08:38:43Z wschweer $
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

//---------------------------------------------------------
//   Arpeggio
//---------------------------------------------------------

Arpeggio::Arpeggio(Score* s)
  : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _subtype = ARP_NORMAL;
      setHeight(spatium() * 4);      // for use in palettes
      _span = 1;
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
      xml.stag("Arpeggio");
      Element::writeProperties(xml);
      xml.tag("subtype", _subtype);
      if (_userLen1.val() != 0.0)
            xml.sTag("userLen1", _userLen1);
      if (_userLen2.val() != 0.0)
            xml.sTag("userLen2", _userLen2);
      if (_span != 1)
            xml.tag("span", _span);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Arpeggio::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "subtype")
                  _subtype = ArpeggioType(val.toInt());
            else if (tag == "userLen1")
                  _userLen1 = Spatium(val.toDouble());
            else if (tag == "userLen2")
                  _userLen2 = Spatium(val.toDouble());
            else if (tag == "span")
                  _span = val.toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Arpeggio::layout()
      {
      qreal _spatium = spatium();
      qreal y1 = -_userLen1.val() * _spatium;
      qreal y2 = _height + _userLen2.val() * _spatium;
      switch (subtype()) {
            case ARP_NORMAL:
            case ARP_UP:
            case ARP_DOWN:
            default:
                  setbbox(QRectF(0.0, y1, symbols[score()->symIdx()][arpeggioSym].width(magS()), y2-y1));
                  return;
            case ARP_UP_STRAIGHT:
            case ARP_DOWN_STRAIGHT:
                  setbbox(QRectF(0.0, y1, symbols[score()->symIdx()][close11arrowHeadSym].width(magS()), y2-y1));
                  return;
            case ARP_BRACKET:
                  {
                  qreal lw = score()->styleS(ST_ArpeggioLineWidth).val() * _spatium;
                  qreal w = score()->styleS(ST_ArpeggioHookLen).val() * _spatium;
                  setbbox(QRectF(-lw * .5, y1 - lw * .5, w + lw, y2 - y1 + lw));
                  return;
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
      qreal y1 = _spatium - _userLen1.val() * _spatium;
      qreal y2 = _height + (_userLen2.val() + .5) * _spatium;
      qreal x1;
      switch (subtype()) {
            case ARP_NORMAL:
                  for (qreal y = y1; y < y2; y += _spatium)
                        symbols[score()->symIdx()][arpeggioSym].draw(p, 1.0, QPointF(0.0, y));
                  break;
            case ARP_UP:
                  symbols[score()->symIdx()][arpeggioarrowupSym].draw(p, 1.0, QPointF(0.0, y1));
                  for (qreal y = y1 + _spatium; y < y2; y += _spatium)
                        symbols[score()->symIdx()][arpeggioSym].draw(p, 1.0, QPointF(0.0, y));
                  break;
            case ARP_DOWN:
                  {
                  qreal y = y1;
                  for (; y < y2 - _spatium; y += _spatium)
                        symbols[score()->symIdx()][arpeggioSym].draw(p, 1.0, QPointF(0.0, y));
                  symbols[score()->symIdx()][arpeggioarrowdownSym].draw(p, 1.0, QPointF(0.0, y));
                  }
                  break;
            case ARP_UP_STRAIGHT:
                  y1-= _spatium * .5;
                  x1 = _spatium * .5;
                  symbols[score()->symIdx()][close11arrowHeadSym].draw(p, 1.0, QPointF(x1, y1 - (_spatium * .5)));
                  p->save();
                  p->setPen(QPen(curColor(),
                     score()->styleS(ST_ArpeggioLineWidth).val() * _spatium,
                     Qt::SolidLine, Qt::RoundCap));
                  p->drawLine(QLineF(x1, y1, x1, y2));
                  p->restore();
                  break;
            case ARP_DOWN_STRAIGHT:
                  y1-= _spatium;
                  y2-= _spatium * .5;
                  x1 = _spatium * .5;
                  symbols[score()->symIdx()][close1M1arrowHeadSym].draw(p, 1.0, QPointF(x1, y2 + (_spatium * .5)));
                  p->save();
                  p->setPen(QPen(curColor(),
                     score()->styleS(ST_ArpeggioLineWidth).val() * _spatium,
                     Qt::SolidLine, Qt::RoundCap));
                  p->drawLine(QLineF(x1, y1, x1, y2));
                  p->restore();
                  break;
            case ARP_BRACKET:
                  {
                  y1 = - _userLen1.val() * _spatium;
                  y2 = _height + _userLen2.val() * _spatium;
                  p->save();

                  p->setPen(QPen(curColor(),
                     score()->styleS(ST_ArpeggioLineWidth).val() * _spatium,
                     Qt::SolidLine, Qt::RoundCap));

                  qreal w = score()->styleS(ST_ArpeggioHookLen).val() * _spatium;
                  p->drawLine(QLineF(0.0, y1, 0.0, y2));
                  p->drawLine(QLineF(0.0, y1, w, y1));
                  p->drawLine(QLineF(0.0, y2, w, y2));
                  p->restore();
                  }
                  break;
            }
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Arpeggio::updateGrips(int* grips, QRectF* grip) const
      {
      qreal _spatium = spatium();
      *grips   = 2;
      QPointF p1(0.0, -_userLen1.val() * _spatium);
      QPointF p2(0.0, _height + _userLen2.val() * _spatium);
      grip[0].translate(pagePos() + p1);
      grip[1].translate(pagePos() + p2);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Arpeggio::editDrag(const EditData& ed)
      {
      Spatium d(ed.delta.y() / spatium());
      if (ed.curGrip == 0)
            _userLen1 -= d;
      else if (ed.curGrip == 1)
            _userLen2 += d;
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
            if (bchord && bchord->type() == CHORD)
                  dnote = static_cast<Chord*>(bchord)->downNote();
            return dnote->pagePos();
            }
      return QPointF();
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
