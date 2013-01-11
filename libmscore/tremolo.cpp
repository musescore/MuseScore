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
      setSubtype(TREMOLO_R8);
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
      painter->setPen(Qt::NoPen);
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
//   setSubtype
//---------------------------------------------------------

void Tremolo::setSubtype(TremoloType t)
      {
      _subtype = t;
      switch (subtype()) {
            case TREMOLO_R16:
            case TREMOLO_C16:
                  _lines = 2;
                  break;
            case TREMOLO_R32:
            case TREMOLO_C32:
                  _lines = 3;
                  break;
            case TREMOLO_R64:
            case TREMOLO_C64:
                  _lines = 4;
                  break;
            default:
                  _lines = 1;
                  break;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tremolo::layout()
      {
      qreal _spatium  = spatium();

      qreal w2  = _spatium * score()->styleS(ST_tremoloWidth).val() * .5;
      qreal h2  = _spatium * score()->styleS(ST_tremoloBoxHeight).val()  * .5;
      qreal lw  = _spatium * score()->styleS(ST_tremoloStrokeWidth).val();
      qreal td  = _spatium * score()->styleS(ST_tremoloDistance).val();
      path      = QPainterPath();

      qreal ty   = 0.0;
      for (int i = 0; i < _lines; ++i) {
            path.moveTo(-w2,  ty + h2 - lw);
            path.lineTo( w2,  ty - h2);
            path.lineTo( w2,  ty - h2 + lw);
            path.lineTo(-w2,  ty + h2);

            path.closeSubpath();
            ty += td;
            }
      setbbox(path.boundingRect());

      _chord1 = static_cast<Chord*>(parent());
      if (_chord1 == 0)
            return;
      Note* anchor1 = _chord1->upNote();
      Stem* stem    = _chord1->stem();
      qreal x, y, h;
      if (stem) {
            x  = stem->pos().x();
            y  = stem->pos().y();
            h  = stem->stemLen();
            }
      else {
            // center tremolo above note
            x = anchor1->x() + anchor1->headWidth() * .5;
            y = anchor1->y();
            h = 2.0 * _spatium + bbox().height();
            if (anchor1->line() > 4)
                  h *= -1;
            }
      if (!twoNotes()) {
            //
            // single note tremolos
            //
            bool up = _chord1->up();
            int line = up ? _chord1->upLine() : _chord1->downLine();
            static const qreal t[3][2][4][2] = {
                  // normal stem
                  {
                     // DOWN
                     {
                        // even line   odd line
                        { 6,           5          },  // line 1
                        { 6 - 2 * .8,  5 - 2 * .8 },  // line 2
                        { 6 - 4 * .8,  3          },  // line 3
                        { 2         ,  3          }   // line 4
                        },
                     // UP
                     {
                        // even line   odd line
                        { -6,          -5          },  // line 1
                        { -6,          -5          },  // line 2
                        { -6,          -3 - 4 * .8 },  // line 3
                        { -2 - 6 * .8, -3 - 6 * .8 }   // line 4
                        }
                     },
                  // stem with hook
                  {
                     // DOWN
                     {
                        // even line   odd line
                        { 3,           3          },  // line 1
                        { 2,           2          },  // line 2
                        { 2,           2          },  // line 3
                        { 2,           2          }   // line 4
                        },
                     // UP
                     {
                        // even line   odd line
                        { -3,          -3          },  // line 1
                        { -2 - 2 * .8, -2 - 2 * .8 },  // line 2
                        { -2 - 4 * .8, -2 - 4 * .8 },  // line 3
                        { -2 - 6 * .8, -2 - 6 * .8 }   // line 4
                        }
                     },
                  // stem with beam
                  {
                     // DOWN
                     {
                        // even line   odd line
                        { 3,           3          },  // line 1
                        { 2,           2          },  // line 2
                        { 2,           2          },  // line 3
                        { 2,           2          }   // line 4
                        },
                     // UP
                     {
                        // even line   odd line
                        { -3,          -3          },  // line 1
                        { -2 - 2 * .8, -2 - 2 * .8 },  // line 2
                        { -2 - 4 * .8, -2 - 4 * .8 },  // line 3
                        { -2 - 6 * .8, -2 - 6 * .8 }   // line 4
                        }
                     },
                  };
            int idx = _chord1->hook() ? 1 : (_chord1->beam() ? 2 : 0);
            y = (line + t[idx][up][_lines-1][line & 1]) * _spatium * .5;
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
      x             = x1 - _chord1->pagePos().x() + (x2 - x1 + note->headWidth()) * .5;
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

void Tremolo::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "subtype")
                  setSubtype(e.readElementText());
            else if (!Element::readProperties(e))
                  e.unknown();
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
      setSubtype(t);
      }

//---------------------------------------------------------
//   tremoloLen
//---------------------------------------------------------

Fraction Tremolo::tremoloLen() const
      {
      Fraction f;
      switch(lines()) {
            case 1: f.set(1,8); break;
            case 2: f.set(1,16); break;
            case 3: f.set(1,32); break;
            case 4: f.set(1,64); break;
            }
      return f;
      }

