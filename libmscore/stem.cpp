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
//   lineWidth
//---------------------------------------------------------

qreal Stem::lineWidth() const
      {
      return point(score()->styleS(ST_stemWidth));
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Stem::layout()
      {
      qreal l = _len + _userLen;
      qreal _up = up() ? -1.0 : 1.0;
      l *= _up;

      qreal y1 = 0.0;
      Staff* st = staff();
      if (chord() && st && !st->isTabStaff()) {
            Note* n  = up() ? chord()->downNote() : chord()->upNote();
            const Sym& sym = symbols[score()->symIdx()][n->noteHead()];
            if (n->mirror())
                  _up *= -1;
            y1 -= sym.attach(n->magS()).y() * _up;
            }

      line.setLine(0.0, y1, 0.0, l);

      // compute bounding rectangle
      QRectF r(line.p1(), line.p2());
      qreal lw5  = lineWidth() * .5;
      setbbox(r.normalized().adjusted(-lw5, -lw5, lw5, lw5));
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void Stem::setLen(qreal v)
      {
      _len = (v < 0.0) ? -v : v;
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
      Staff* st = staff();
      bool useTab = st && st->isTabStaff();

      if (useTab && st->staffType()->slashStyle())
            return;
      qreal lw = lineWidth();
      painter->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::RoundCap));
      painter->drawLine(line);
      if (!useTab)
            return;

      // TODO: adjust bounding rectangle in layout() for dots and for slash
      StaffTypeTablature* stt = static_cast<StaffTypeTablature*>(st->staffType());
      qreal sp = spatium();

      // slashed half note stem
      if (chord() && chord()->durationType().type() == TDuration::V_HALF
         && stt->minimStyle() == TAB_MINIM_SLASHED) {
            qreal wdt   = sp * STAFFTYPE_TAB_SLASH_WIDTH;
            qreal sln   = sp * STAFFTYPE_TAB_SLASH_SLANTY;
            qreal thk   = sp * STAFFTYPE_TAB_SLASH_THICK;
            qreal displ = sp * STAFFTYPE_TAB_SLASH_DISPL;
            QPainterPath path;

            qreal y = stt->stemsDown() ?
                         _len - STAFFTYPE_TAB_SLASH_2STARTY_DN*sp :
                        -_len + STAFFTYPE_TAB_SLASH_2STARTY_UP*sp;
            for (int i = 0; i < 2; ++i) {
                  path.moveTo( wdt*0.5-lw, y);        // top-right corner
                  path.lineTo( wdt*0.5-lw, y+thk);    // bottom-right corner
                  path.lineTo(-wdt*0.5,    y+thk+sln);// bottom-left corner
                  path.lineTo(-wdt*0.5,    y+sln);    // top-left corner
                  path.closeSubpath();
                  y += displ;
                  }
//            setbbox(path.boundingRect());
            painter->setBrush(QBrush(curColor()));
            painter->setPen(Qt::NoPen);
            painter->drawPath(path);
            }

      // dots
      // NOT THE BEST PLACE FOR THIS?
      // with tablatures, dots are not drawn near 'notes', but near stems
      int nDots = chord()->dots();
      if (nDots > 0) {
            qreal y = stemLen() - (stt->stemsDown() ?
                        (STAFFTYPE_TAB_DEFAULTSTEMLEN_DN - 0.75) * sp : 0.0 );
            symbols[score()->symIdx()][dotSym].draw(painter, magS(),
                        QPointF(STAFFTYPE_TAB_DEFAULTDOTDIST_X * sp, y), nDots);
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

void Stem::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "userLen")
                  _userLen = e.readDouble() * spatium();
            else if (tag == "subtype")        // obsolete
                  e.skipCurrentElement();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      if (_userLen < 0.0)
            _userLen = -_userLen;
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
//   reset
//---------------------------------------------------------

void Stem::reset()
      {
      score()->undoChangeProperty(this, P_USER_LEN, 0.0);
      Element::reset();
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
            case P_USER_LEN: return userLen();
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

