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
  : Element(s, ElementFlag::MOVABLE)
      {
      _arpeggioType = ArpeggioType::NORMAL;
      setHeight(spatium() * 4);      // for use in palettes
      _span     = 1;
      _userLen1 = 0.0;
      _userLen2 = 0.0;
      _playArpeggio = true;
      _stretch = 1.0;
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

void Arpeggio::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag("Arpeggio");
      Element::writeProperties(xml);
      xml.tag("subtype", int(_arpeggioType));
      if (_userLen1 != 0.0)
            xml.tag("userLen1", _userLen1 / spatium());
      if (_userLen2 != 0.0)
            xml.tag("userLen2", _userLen2 / spatium());
      if (_span != 1)
            xml.tag("span", _span);
      writeProperty(xml, Pid::PLAY);
      writeProperty(xml, Pid::TIME_STRETCH);
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
            else if (tag == "play")
                 _playArpeggio = e.readBool();
            else if (tag == "timeStretch")
                  _stretch = e.readDouble();
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

      symbols.clear();
      qreal w1 = f->advance(end, mag);
      qreal w2 = f->advance(fill, mag);
      int n    = lrint((w - w1) / w2);
      for (int i = 0; i < n; ++i)
           symbols.push_back(fill);
      symbols.push_back(end);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Arpeggio::layout()
      {
      qreal y1 = -_userLen1;
      qreal y2 = _height + _userLen2;
      _hidden = false;
      if (score()->styleB(Sid::ArpeggioHiddenInStdIfTab)) {
            if (staff() && staff()->isPitchedStaff(tick())) {
                  for (Staff* s : staff()->staffList()) {
                        if (s->score() == score()  && s->isTabStaff(tick())) {
                              _hidden = true;
                               setbbox(QRect());
                               return;
                               }
                        }
                  }
            }
      if (staff())
            setMag(staff()->mag(tick()));
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
                  qreal lw = score()->styleS(Sid::ArpeggioLineWidth).val() * _spatium;
                  setbbox(QRectF(x1 - w * .5, y1, w, y2 - y1 + lw * .5));
                  }
                  break;

            case ArpeggioType::DOWN_STRAIGHT: {
                  qreal _spatium = spatium();
                  qreal x1 = _spatium * .5;
                  qreal w  = symBbox(SymId::arrowheadBlackDown).width();
                  qreal lw = score()->styleS(Sid::ArpeggioLineWidth).val() * _spatium;
                  setbbox(QRectF(x1 - w * .5, y1 - lw * .5, w, y2 - y1 + lw * .5));
                  }
                  break;

            case ArpeggioType::BRACKET: {
                  qreal _spatium = spatium();
                  qreal lw = score()->styleS(Sid::ArpeggioLineWidth).val() * _spatium * .5;
                  qreal w  = score()->styleS(Sid::ArpeggioHookLen).val() * _spatium;
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
      if (_hidden)
            return;
      qreal _spatium = spatium();

      qreal y1 = -_userLen1;
      qreal y2 = _height + _userLen2;

      p->setPen(QPen(curColor(),
         score()->styleS(Sid::ArpeggioLineWidth).val() * _spatium,
         Qt::SolidLine, Qt::RoundCap));

      p->save();
      switch (arpeggioType()) {
            case ArpeggioType::NORMAL:
            case ArpeggioType::UP:
                  {
                  QRectF r(symBbox(symbols));
                  qreal scale = p->worldTransform().m11();
                  p->rotate(-90.0);
                  score()->scoreFont()->draw(symbols, p, magS(), QPointF(-r.right() - y1, -r.bottom() + r.height()), scale);
                  }
                  break;

            case ArpeggioType::DOWN:
                  {
                  QRectF r(symBbox(symbols));
                  qreal scale = p->worldTransform().m11();
                  p->rotate(90.0);
                  score()->scoreFont()->draw(symbols, p, magS(), QPointF(-r.left() + y1, -r.top() - r.height()), scale);
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
                  qreal w = score()->styleS(Sid::ArpeggioHookLen).val() * _spatium;
                  p->drawLine(QLineF(0.0, y1, 0.0, y2));
                  p->drawLine(QLineF(0.0, y1, w, y1));
                  p->drawLine(QLineF(0.0, y2, w, y2));
                  }
                  break;
            }
      p->restore();
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Arpeggio::updateGrips(EditData& ed) const
      {
      QPointF p1(0.0, -_userLen1);
      QPointF p2(0.0, _height + _userLen2);
      ed.grip[0].translate(pagePos() + p1);
      ed.grip[1].translate(pagePos() + p2);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Arpeggio::editDrag(EditData& ed)
      {
      qreal d = ed.delta.y();
      if (ed.curGrip == Grip::START)
            _userLen1 -= d;
      else if (ed.curGrip == Grip::END)
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

QPointF Arpeggio::gripAnchor(Grip n) const
      {
      Chord* c = chord();
      if (c == 0)
            return QPointF();
      if (n == Grip::START)
            return c->upNote()->pagePos();
      else if (n == Grip::END) {
            Note* dnote = c->downNote();
            int btrack  = track() + (_span - 1) * VOICES;
            Element* e = c->segment()->element(btrack);
            if (e && e->isChord())
                  dnote = toChord(e)->downNote();
            return dnote->pagePos();
            }
      return QPointF();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Arpeggio::startEdit(EditData& ed)
      {
      Element::startEdit(ed);
      ed.grips   = 2;
      ed.curGrip = Grip::END;
      undoPushProperty(Pid::ARP_USER_LEN1);
      undoPushProperty(Pid::ARP_USER_LEN2);
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Arpeggio::edit(EditData& ed)
      {
      if (ed.curGrip != Grip::END || !(ed.modifiers & Qt::ShiftModifier))
            return false;

      if (ed.key == Qt::Key_Down) {
            Staff* s = staff();
            Part* part = s->part();
            int n = part->nstaves();
            int ridx = part->staves()->indexOf(s);
            if (ridx >= 0) {
                  if (_span + ridx < n)
                        ++_span;
                  }
            }
      else if (ed.key == Qt::Key_Up) {
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

bool Arpeggio::acceptDrop(EditData& data) const
      {
      return data.element->type() == ElementType::ARPEGGIO;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Arpeggio::drop(EditData& data)
      {
      Element* e = data.element;
      switch(e->type()) {
            case ElementType::ARPEGGIO:
                  {
                  Arpeggio* a = toArpeggio(e);
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

QVariant Arpeggio::getProperty(Pid propertyId) const
      {
      switch(propertyId) {
            case Pid::TIME_STRETCH:
                  return Stretch();
            case Pid::ARP_USER_LEN1:
                  return userLen1();
            case Pid::ARP_USER_LEN2:
                  return userLen2();
            case Pid::PLAY:
                  return _playArpeggio;
            default:
                  break;
            }
      return Element::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Arpeggio::setProperty(Pid propertyId, const QVariant& val)
      {
      switch(propertyId) {
            case Pid::TIME_STRETCH:
                  setStretch(val.toDouble());
                  break;
            case Pid::ARP_USER_LEN1:
                  setUserLen1(val.toDouble());
                  break;
            case Pid::ARP_USER_LEN2:
                  setUserLen2(val.toDouble());
                  break;
            case Pid::PLAY:
                  setPlayArpeggio(val.toBool());
                  break;
            default:
                  if (!Element::setProperty(propertyId, val))
                        return false;
                  break;
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Arpeggio::propertyDefault(Pid propertyId) const
      {
      switch(propertyId) {
            case Pid::ARP_USER_LEN1:
                  return 0.0;
            case Pid::ARP_USER_LEN2:
                  return 0.0;
            case Pid::TIME_STRETCH:
                  return 1.0;
            case Pid::PLAY:
                  return true;
            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }
}

