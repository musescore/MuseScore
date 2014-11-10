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

#include "tremolo.h"
#include "score.h"
#include "style.h"
#include "chord.h"
#include "note.h"
#include "measure.h"
#include "segment.h"
#include "stem.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   Tremolo
//---------------------------------------------------------

static const char* tremoloName[] = {
      QT_TRANSLATE_NOOP("Tremolo", "Eighth through stem"),
      QT_TRANSLATE_NOOP("Tremolo", "16th through stem"),
      QT_TRANSLATE_NOOP("Tremolo", "32nd through stem"),
      QT_TRANSLATE_NOOP("Tremolo", "64th through stem"),
      QT_TRANSLATE_NOOP("Tremolo", "Eighth between notes"),
      QT_TRANSLATE_NOOP("Tremolo", "16th between notes"),
      QT_TRANSLATE_NOOP("Tremolo", "32nd between notes"),
      QT_TRANSLATE_NOOP("Tremolo", "64th between notes")
      };

Tremolo::Tremolo(Score* score)
   : Element(score)
      {
      setTremoloType(TremoloType::R8);
      _chord1  = 0;
      _chord2  = 0;
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      }

Tremolo::Tremolo(const Tremolo& t)
   : Element(t)
      {
      setTremoloType(t.tremoloType());
      _chord1  = t.chord1();
      _chord2  = t.chord2();
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
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
            QPen pen(curColor(), point(score()->styleS(StyleIdx::stemWidth)));
            painter->setPen(pen);
            qreal _spatium = spatium();
            painter->drawLine(QLineF(x, -_spatium*.5, x, path.boundingRect().height() + _spatium));
            }
      }

//---------------------------------------------------------
//   setTremoloType
//---------------------------------------------------------

void Tremolo::setTremoloType(TremoloType t)
      {
      _tremoloType = t;
      switch (tremoloType()) {
            case TremoloType::R16:
            case TremoloType::C16:
                  _lines = 2;
                  break;
            case TremoloType::R32:
            case TremoloType::C32:
                  _lines = 3;
                  break;
            case TremoloType::R64:
            case TremoloType::C64:
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

      qreal w2  = _spatium * score()->styleS(StyleIdx::tremoloWidth).val() * .5;
      qreal h2  = _spatium * score()->styleS(StyleIdx::tremoloBoxHeight).val()  * .5;
      qreal lw  = _spatium * score()->styleS(StyleIdx::tremoloStrokeWidth).val();
      qreal td  = _spatium * score()->styleS(StyleIdx::tremoloDistance).val();
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

      QRectF rect = path.boundingRect();
      if ((parent() == 0) && !twoNotes())
            rect.setHeight(rect.height() + _spatium);
      setbbox(rect);

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
            return;
            }
      y += (h - bbox().height()) * .5;
      //
      // two chord tremolo
      //
      Segment* s = _chord1->segment()->next();
      while (s) {
            if (s->element(track()) && (s->element(track())->type() == Element::Type::CHORD))
                  break;
            s = s->next();
            }
      if (s == 0) {
            qDebug("no second note of tremolo found");
            return;
            }

      _chord2 = static_cast<Chord*>(s->element(track()));
      _chord2->setTremolo(this);

      int x2  = _chord2->stemPosBeam().x();
      int x1  = _chord1->stemPosBeam().x();

      // qreal x2     = _chord2->_chord2->up()stemPos(_chord2->up(), true).x();
      // qreal x1     = _chord1->stemPos(_chord1->up(), true).x();
      x             = x1 - _chord1->pagePos().x() + (x2 - x1 + _chord1->upNote()->headWidth()) * .5;
      setPos(x, y);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tremolo::write(Xml& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(name());
      xml.tag("subtype", tremoloTypeName());
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
                  setTremoloType(e.readElementText());
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   tremoloTypeName
//---------------------------------------------------------

QString Tremolo::tremoloTypeName() const
      {
      switch(tremoloType()) {
            case TremoloType::R8:  return QString("r8");
            case TremoloType::R16: return QString("r16");
            case TremoloType::R32: return QString("r32");
            case TremoloType::R64: return QString("r64");
            case TremoloType::C8:  return QString("c8");
            case TremoloType::C16: return QString("c16");
            case TremoloType::C32: return QString("c32");
            case TremoloType::C64: return QString("c64");
            default:
                  break;
            }
      return QString("??");
      }

//---------------------------------------------------------
//   setTremoloType
//---------------------------------------------------------

void Tremolo::setTremoloType(const QString& s)
      {
      TremoloType t;
      if (s == "r8")
            t = TremoloType::R8;
      else if (s == "r16")
            t = TremoloType::R16;
      else if (s == "r32")
            t = TremoloType::R32;
      else if (s == "r64")
            t = TremoloType::R64;
      else if (s == "c8")
            t = TremoloType::C8;
      else if (s == "c16")
            t = TremoloType::C16;
      else if (s == "c32")
            t = TremoloType::C32;
      else if (s == "c64")
            t = TremoloType::C64;
      else
            t = TremoloType(s.toInt());    // for compatibility with old tremolo type
      setTremoloType(t);
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

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString Tremolo::subtypeName() const
      {
      return qApp->translate("Tremolo", tremoloName[subtype() - int(TremoloType::R8)]);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Tremolo::accessibleInfo()
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(subtypeName());
      }

}

