//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: tremolo.cpp 5532 2012-04-12 13:27:53Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "tremolo.h"
#include "score.h"
#include "style.h"
#include "chord.h"
#include "note.h"
#include "measure.h"
#include "segment.h"
#include "stem.h"

//---------------------------------------------------------
//   Tremolo
//---------------------------------------------------------

Tremolo::Tremolo(Score* score)
   : Element(score)
      {
      _subtype = TREMOLO_R8;
      _chord1  = 0;
      _chord2  = 0;
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Tremolo::draw(QPainter* painter) const
      {
      painter->setBrush(QBrush(curColor()));
      painter->drawPath(path);
      if ((parent() == 0) && !twoNotes()) {
            qreal x = 0.0; // bbox().width() * .25;
            QPen pen(curColor(), point(score()->styleS(ST_stemWidth)));
            painter->setPen(pen);
            qreal _spatium = spatium();
            painter->drawLine(QLineF(x, -_spatium*.5, x, bbox().height() + _spatium));
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tremolo::layout()
      {
      qreal sp  = spatium();
      qreal w   = sp * 1.2;
      qreal h   = sp * .8;
      qreal lw  = sp * .35;
      qreal d   = sp * 0.8;
      path      = QPainterPath();

      qreal y   = 0.0;
      int lines;
      switch(subtype()) {
            case TREMOLO_R16:
            case TREMOLO_C16:
                  lines = 2;
                  break;
            case TREMOLO_R32:
            case TREMOLO_C32:
                  lines = 3;
                  break;
            case TREMOLO_R64:
            case TREMOLO_C64:
                  lines = 4;
                  break;
            default:
                  lines = 1;
                  break;
            }
      for (int i = 0; i < lines; ++i) {
            path.moveTo(-w*.5, y + h - lw);
            path.lineTo(w*.5,  y);
            path.lineTo(w*.5,  y + lw);
            path.lineTo(-w*.5, y + h);
            path.closeSubpath();
            y += d;
            }
      setbbox(path.boundingRect());

      _chord1 = static_cast<Chord*>(parent());
      if (_chord1 == 0)
            return;
      Note* anchor1 = _chord1->upNote();
      Stem* stem    = _chord1->stem();
      qreal x;
      if (stem) {
            x  = stem->pos().x();
            y  = stem->pos().y();
            h  = stem->stemLen();
            }
      else {
            // center tremolo above note
            x = anchor1->x() + anchor1->headWidth() * .5;
            y = anchor1->y();
            h = 2.0 * spatium() + bbox().height();
            if (anchor1->line() > 4)
                  h *= -1;
            }
      if (!twoNotes()) {
            if (_chord1->hook()) {
                  y += (h - bbox().height()) * .5;
                  y -= spatium() * .5 * (_chord1->up() ? -1.0 : 1.0);
                  }
            else {
                  bool up  = _chord1->up();
                  qreal d = _chord1->downNote()->y() - _chord1->upNote()->y();
                  if (_chord1->beam()) {
                        qreal bd  = score()->styleD(ST_beamDistance);
                        qreal bw = score()->styleS(ST_beamWidth).val() * sp;
                        int n = _chord1->durationType().hooks();
                        qreal beamHeight = bw * n + bw * bd * (n-1);
                        if (up)
                              h += beamHeight;
                        else
                              h -= beamHeight;
                        }
                  if (!up)
                        d = -d;
                  y = y - d - ((bbox().height() - d - h) * .5);
                  }
            setPos(x, y);
            _chord1->setTremoloChordType(TremoloSingle);
            return;
            }
      y += (h - bbox().height()) * .5;
      //
      // two chord tremolo
      //
      Segment* s = _chord1->segment()->next();
      while (s) {
            if (s->element(track()) && (s->element(track())->type() == CHORD))
                  break;
            s = s->next();
            }
      if (s == 0) {
            qDebug("no second note of tremolo found\n");
            return;
            }

      _chord1->setTremoloChordType(TremoloFirstNote);
      _chord2 = static_cast<Chord*>(s->element(track()));
      _chord2->setTremolo(this);
      _chord2->setTremoloChordType(TremoloSecondNote);

      Note* note = _chord2->up() ? _chord2->upNote() : _chord2->downNote();
      int x2     = note->stemPos(_chord2->up()).x();

      note    = _chord1->up() ? _chord1->upNote() : _chord1->downNote();
      int x1  = note->stemPos(_chord1->up()).x();

      // qreal x2     = _chord2->_chord2->up()stemPos(_chord2->up(), true).x();
      // qreal x1     = _chord1->stemPos(_chord1->up(), true).x();
      x             = x1 - _chord1->pagePos().x() + (x2 - x1) * .5;
      setPos(x, y);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tremolo::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("subtype", subtypeName());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tremolo::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "subtype")
                  setSubtype(e.text());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString Tremolo::subtypeName() const
      {
      switch(subtype()) {
            case TREMOLO_R8:  return QString("r8");
            case TREMOLO_R16: return QString("r16");
            case TREMOLO_R32: return QString("r32");
            case TREMOLO_R64: return QString("r64");
            case TREMOLO_C8:  return QString("c8");
            case TREMOLO_C16: return QString("c16");
            case TREMOLO_C32: return QString("c32");
            case TREMOLO_C64: return QString("c64");
            default:
                  break;
            }
      return QString("??");
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Tremolo::setSubtype(const QString& s)
      {
      TremoloType t;
      if (s == "r8")
            t = TREMOLO_R8;
      else if (s == "r16")
            t = TREMOLO_R16;
      else if (s == "r32")
            t = TREMOLO_R32;
      else if (s == "r64")
            t = TREMOLO_R64;
      else if (s == "c8")
            t = TREMOLO_C8;
      else if (s == "c16")
            t = TREMOLO_C16;
      else if (s == "c32")
            t = TREMOLO_C32;
      else if (s == "c64")
            t = TREMOLO_C64;
      else
            t = TremoloType(s.toInt());    // for compatibility with old tremolo type
      _subtype = t;
      }

//---------------------------------------------------------
//   tremoloLen
//---------------------------------------------------------

Fraction Tremolo::tremoloLen() const
      {
      switch(subtype()) {
            default:
            case TREMOLO_R8:
            case TREMOLO_C8:
                  return Fraction(1,8);

            case TREMOLO_R16:
            case TREMOLO_C16:
                  return Fraction(1,16);

            case TREMOLO_R32:
            case TREMOLO_C32:
                  return Fraction(1,32);

            case TREMOLO_R64:
            case TREMOLO_C64:
                  return Fraction(1,64);
            }
      }

