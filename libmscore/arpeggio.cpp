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
                  _subtype = ArpeggioType(e.readInt());
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
//   layout
//---------------------------------------------------------

void Arpeggio::layout()
      {
      qreal y1 = - _userLen1;
      qreal y2 = _height + _userLen2;
      switch (subtype()) {
            case ARP_NORMAL:
            case ARP_UP:
            case ARP_DOWN:
            default:
                  bbox().setRect(0.0, y1, symbols[score()->symIdx()][arpeggioSym].width(magS()), y2-y1);
                  return;
            case ARP_UP_STRAIGHT:
            case ARP_DOWN_STRAIGHT:
                  bbox().setRect(0.0, y1, symbols[score()->symIdx()][close11arrowHeadSym].width(magS()), y2-y1);
                  return;
            case ARP_BRACKET:
                  {
                  qreal _spatium = spatium();
                  qreal lw = score()->styleS(ST_ArpeggioLineWidth).val() * _spatium;
                  qreal w = score()->styleS(ST_ArpeggioHookLen).val() * _spatium;
                  bbox().setRect(-lw * .5, y1 - lw * .5, w + lw, y2 - y1 + lw);
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
      qreal y1 = _spatium - _userLen1;
      qreal y2 = _height  + _userLen2;
      qreal x1;
      qreal m = magS();
      switch (subtype()) {
            case ARP_NORMAL:
                  for (qreal y = y1; y < y2; y += _spatium)
                        symbols[score()->symIdx()][arpeggioSym].draw(p, m, QPointF(0.0, y));
                  break;
            case ARP_UP:
                  symbols[score()->symIdx()][arpeggioarrowupSym].draw(p, m, QPointF(0.0, y1));
                  for (qreal y = y1 + _spatium; y < y2; y += _spatium)
                        symbols[score()->symIdx()][arpeggioSym].draw(p, m, QPointF(0.0, y));
                  break;
            case ARP_DOWN:
                  {
                  qreal y = y1;
                  for (; y < y2 - _spatium; y += _spatium)
                        symbols[score()->symIdx()][arpeggioSym].draw(p, m, QPointF(0.0, y));
                  symbols[score()->symIdx()][arpeggioarrowdownSym].draw(p, m, QPointF(0.0, y));
                  }
                  break;
            case ARP_UP_STRAIGHT:
                  y1-= _spatium * .5;
                  x1 = _spatium * .5;
                  symbols[score()->symIdx()][close11arrowHeadSym].draw(p, m, QPointF(x1, y1 - (_spatium * .5)));
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
                  symbols[score()->symIdx()][close1M1arrowHeadSym].draw(p, m, QPointF(x1, y2 + (_spatium * .5)));
                  p->save();
                  p->setPen(QPen(curColor(),
                     score()->styleS(ST_ArpeggioLineWidth).val() * _spatium,
                     Qt::SolidLine, Qt::RoundCap));
                  p->drawLine(QLineF(x1, y1, x1, y2));
                  p->restore();
                  break;
            case ARP_BRACKET:
                  {
                  y1 = - _userLen1;
                  y2 = _height + _userLen2;
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
      *grips   = 2;
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
//   startEdit
//---------------------------------------------------------

void Arpeggio::startEdit(MuseScoreView*, const QPointF&)
      {
      undoPushProperty(P_ARP_USER_LEN1);
      undoPushProperty(P_ARP_USER_LEN2);
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
//   getProperty
//---------------------------------------------------------

QVariant Arpeggio::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ARP_USER_LEN1:
                  return userLen1();
            case P_ARP_USER_LEN2:
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
            case P_ARP_USER_LEN1:
                  setUserLen1(val.toDouble());
                  break;
            case P_ARP_USER_LEN2:
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


