//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "stem.h"
#include "staff.h"
#include "chord.h"
#include "score.h"
#include "stafftype.h"
#include "hook.h"
#include "tremolo.h"
#include "note.h"

// TEMPORARY HACK!!
#include "sym.h"
// END OF HACK

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

Stem::Stem(Score* s)
   : Element(s)
      {
      _len     = 0.0;
      _userLen = 0.0;
      setFlags(ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   up
//---------------------------------------------------------

bool Stem::up() const
      {
      return chord() ? chord()->up() : true;
      }

//---------------------------------------------------------
//   stemLen
//---------------------------------------------------------

qreal Stem::stemLen() const
      {
      return up() ? -_len : _len;
      }

//---------------------------------------------------------
//   hookPos
//---------------------------------------------------------

QPointF Stem::hookPos() const
      {
      QPointF p(pos());
      if (up()) {
            p.ry() -= _len + _userLen;
            }
      else {
            p.rx() += score()->styleP(ST_stemWidth);
            p.ry() += _len + _userLen;
            }
      return p;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Stem::layout()
      {
      qreal l = _len + _userLen;
      if (up())
            l = -l;
      Staff* st = staff();
      qreal lw5  = point(score()->styleS(ST_stemWidth)) * .5;
      QPointF p1(0.0, 0.0);
      QPointF p2(0.0, l);
      if (st) {
            if (!st->useTablature() && chord()) {
                  // adjust P1 for note head
                  Chord* c = chord();
                  if (c->up()) {
                        Note* n   = c->downNote();
                        p1 = symbols[score()->symIdx()][n->noteHead()].attach(n->magS());
                        p1.rx() = -lw5;
                        p2.rx() = -lw5;
                        }
                  else {
                        Note* n = c->upNote();
                        p1 = -symbols[score()->symIdx()][n->noteHead()].attach(n->magS());
                        p1.rx() = lw5;
                        p2.rx() = lw5;
                        }
                  }
            else if (st->useTablature()) {
                  p1.rx() = -lw5;
                  p2.rx() = -lw5;
                  }
            }
      line.setP1(p1);
      line.setP2(p2);

      // compute bounding rectangle
      QRectF r(line.p1(), line.p2());
      setbbox(r.normalized().adjusted(-lw5, -lw5, lw5, lw5));
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void Stem::setLen(qreal v)
      {
      if (v < 0.0)
            v = -v;
      _len = v;
      layout();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Stem::spatiumChanged(qreal oldValue, qreal newValue)
      {
      _userLen = (_userLen / oldValue) * newValue;
      layout();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Stem::draw(QPainter* painter) const
      {
      bool useTab = false;
      Staff* st = staff();
      if (st && st->useTablature()) {     // stems used in palette do not have a staff
            if (st->staffType()->slashStyle())
                  return;
            useTab = true;
            }
      qreal lw = point(score()->styleS(ST_stemWidth));
      painter->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::RoundCap));
      painter->drawLine(line);

      // NOT THE BEST PLACE FOR THIS?
      // with tablatures, dots are not drawn near 'notes', but near stems
      // TODO: adjust bounding rectangle in layout()
      if (useTab) {
            int nDots = chord()->dots();
            if (nDots > 0)
                  symbols[score()->symIdx()][dotSym].draw(painter, magS(), QPointF(spatium(), stemLen()), nDots);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Stem::write(Xml& xml) const
      {
      xml.stag("Stem");
      Element::writeProperties(xml);
      if (_userLen != 0.0)
            xml.tag("userLen", _userLen / spatium());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Stem::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "userLen")
                  _userLen = e.text().toDouble() * spatium();
            else if (e.tagName() == "subtype")        // obsolete
                  ;
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Stem::updateGrips(int* grips, QRectF* grip) const
      {
      *grips   = 1;
      grip[0].translate(pagePos() + line.p2());
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Stem::editDrag(const EditData& ed)
      {
      qreal yDelta = ed.delta.y();
      _userLen += up() ? -yDelta : yDelta;
      layout();
      Chord* c = static_cast<Chord*>(parent());
      if (c->hook())
            c->hook()->move(0.0, ed.delta.y());
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Stem::toDefault()
      {
      score()->undoChangeProperty(this, P_USER_LEN, 0.0);
      Element::toDefault();
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Stem::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      if ((e->type() == TREMOLO) && (static_cast<Tremolo*>(e)->subtype() <= TREMOLO_R64)) {
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Stem::drop(const DropData& data)
      {
      Element* e = data.element;
      Chord* ch = chord();
      switch(e->type()) {
            case TREMOLO:
                  e->setParent(ch);
                  score()->setLayout(ch->measure());
                  score()->undoAddElement(e);
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

QVariant Stem::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_USER_LEN:            return userLen();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Stem::setProperty(P_ID propertyId, const QVariant& v)
      {
      score()->addRefresh(canvasBoundingRect());
      switch(propertyId) {
            case P_USER_LEN:  setUserLen(v.toDouble()); break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      score()->addRefresh(canvasBoundingRect());
      layout();
      score()->addRefresh(canvasBoundingRect());
      score()->setLayoutAll(false);       //DEBUG
      return true;
      }

