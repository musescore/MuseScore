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
#include "sym.h"
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
      QT_TRANSLATE_NOOP("Tremolo", "Buzz roll"),
      QT_TRANSLATE_NOOP("Tremolo", "Eighth between notes"),
      QT_TRANSLATE_NOOP("Tremolo", "16th between notes"),
      QT_TRANSLATE_NOOP("Tremolo", "32nd between notes"),
      QT_TRANSLATE_NOOP("Tremolo", "64th between notes")
      };

Tremolo::Tremolo(Score* score)
   : Element(score, ElementFlag::MOVABLE)
      {
      setTremoloType(TremoloType::R8);
      _chord1  = 0;
      _chord2  = 0;
      }

Tremolo::Tremolo(const Tremolo& t)
   : Element(t)
      {
      setTremoloType(t.tremoloType());
      _chord1  = t.chord1();
      _chord2  = t.chord2();
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Tremolo::mag() const
      {
      return parent() ? parent()->mag() : 1.0;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Tremolo::draw(QPainter* painter) const
      {
      if (tremoloType() == TremoloType::BUZZ_ROLL) {
            painter->setPen(curColor());
            drawSymbol(SymId::buzzRoll, painter);
            }
      else {
            painter->setBrush(QBrush(curColor()));
            painter->setPen(Qt::NoPen);
            painter->drawPath(path);
            }
      if ((parent() == 0) && !twoNotes()) {
            qreal x = 0.0; // bbox().width() * .25;
            QPen pen(curColor(), point(score()->styleS(Sid::stemWidth)));
            painter->setPen(pen);
            qreal _spatium = spatium() * mag();
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
      qreal _spatium  = spatium() * mag();

      qreal w2  = _spatium * score()->styleS(Sid::tremoloWidth).val() * .5;
      qreal lw  = _spatium * score()->styleS(Sid::tremoloStrokeWidth).val();
      qreal td  = _spatium * score()->styleS(Sid::tremoloDistance).val();

      path      = QPainterPath();
      qreal ty  = 0.0;

      for (int i = 0; i < _lines; i++) {
            path.addRect(-w2, ty, 2.0 * w2, lw);
            ty += td;
            }

      _chord1 = toChord(parent());
      if (_chord1 == 0) {
            // just for the palette
            QTransform shearTransform;
            shearTransform.shear(0.0, -(lw / 2.0) / w2);
            path = shearTransform.map(path);
            setbbox(path.boundingRect());
            addbbox(QRectF(bbox().x(), bbox().bottom(), bbox().width(), _spatium));
            return;
            }

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
            y = (line + t[idx][up][_lines-1][line & 1]) * spatium() * .5 / mag();

            QTransform shearTransform;
            shearTransform.shear(0.0, -(lw / 2.0) / w2);
            path = shearTransform.map(path);

            setbbox(path.boundingRect());
            setPos(x, y);
            return;
            }
      y += (h - bbox().height()) * .5;
      //
      // two chord tremolo
      //
      Segment* s = _chord1->segment()->next();
      while (s) {
            if (s->element(track()) && (s->element(track())->isChord()))
                  break;
            s = s->next();
            }
      if (s == 0) {
            qDebug("no second note of tremolo found");
            return;
            }

      _chord2 = toChord(s->element(track()));
      _chord2->setTremolo(this);

      Stem* stem1 = _chord1->stem();
      Stem* stem2 = _chord2->stem();

      // compute the y coordinates of the tips of the stems
      qreal y1, y2;
      qreal firstChordStaffY;

      if (stem2 && stem1) {
            // stemPageYOffset variable is used for the case when the first
            // chord is cross-staff
            firstChordStaffY = stem1->pagePos().y() - stem1->y();  // y coordinate of the staff of the first chord
            y1 = stem1->y() + stem1->p2().y();
            y2 = stem2->pagePos().y() - firstChordStaffY + stem2->p2().y();  // ->p2().y() is better than ->stemLen()
            }
      else {
            firstChordStaffY = _chord1->pagePos().y() - _chord1->y();  // y coordinate of the staff of the first chord
            y1 = _chord1->stemPosBeam().y() - firstChordStaffY + _chord1->defaultStemLength();
            y2 = _chord2->stemPosBeam().y() - firstChordStaffY + _chord2->defaultStemLength();
            }

      // improve the case when one stem is up and another is down
      if (_chord1->beams() == 0 && _chord2->beams() == 0 && _chord1->up() != _chord2->up()) {
            qreal meanNote1Y = .5 * (_chord1->upNote()->pagePos().y() - firstChordStaffY + _chord1->downNote()->pagePos().y() - firstChordStaffY);
            qreal meanNote2Y = .5 * (_chord2->upNote()->pagePos().y() - firstChordStaffY + _chord2->downNote()->pagePos().y() - firstChordStaffY);
            y1 = .5 * (y1 + meanNote1Y);
            y2 = .5 * (y2 + meanNote2Y);
            }

      y = (y1 + y2) * .5;
      if (!_chord1->up()) {
            y -= path.boundingRect().height() * .5;
            }
      if (!_chord2->up()) {
            y -= path.boundingRect().height() * .5;
            }

      // compute the x coordinates of the inner edge of the stems
      qreal x2  = _chord2->stemPosBeam().x();
      if (_chord2->up() && stem2)
            x2 -= stem2->lineWidth();
      qreal x1  = _chord1->stemPosBeam().x();
      if (!_chord1->up() && stem1)
            x1 += stem1->lineWidth();

      x = (x1 + x2) * .5 - _chord1->pagePos().x();

      QTransform xScaleTransform;
      // TODO const qreal H_MULTIPLIER = score()->styleS(Sid::tremoloBeamLengthMultiplier).val();
      const qreal H_MULTIPLIER = 0.62;
      // TODO const qreal MAX_H_LENGTH = _spatium * score()->styleS(Sid::tremoloBeamLengthMultiplier).val();
      const qreal MAX_H_LENGTH = _spatium * 12.0;

      qreal xScaleFactor = qMin(H_MULTIPLIER * (x2 - x1), MAX_H_LENGTH);
      xScaleFactor /= (2.0 * w2);

      xScaleTransform.scale(xScaleFactor, 1.0);
      path = xScaleTransform.map(path);

      qreal beamYOffset = 0.0;

      if (_chord1->beams() == _chord2->beams() && _chord1->beam()) {
            int beams = _chord1->beams();
            qreal beamHalfLineWidth = point(score()->styleS(Sid::beamWidth)) * .5 * mag();
            beamYOffset = beams * _chord1->beam()->beamDist() - beamHalfLineWidth;
            if (_chord1->up() != _chord2->up()) {  // cross-staff
                  beamYOffset = 2 * beamYOffset + beamHalfLineWidth;
                  }
            else if (!_chord1->up() && !_chord2->up()) {
                  beamYOffset = -beamYOffset;
                  }
            }
      QTransform shearTransform;
      qreal dy = y2 - y1;
      qreal dx = x2 - x1;
      qreal ds = dy / dx;
      if (_chord1->beams() == 0 && _chord2->beams() == 0) {
            if (_chord1->up() && !_chord2->up())
                  ds = (dy - path.boundingRect().height()) / dx;
            else if (!_chord1->up() && _chord2->up())
                  ds = (dy + path.boundingRect().height()) / dx;
            }
      shearTransform.shear(0.0, ds);
      path = shearTransform.map(path);

      setbbox(path.boundingRect());
      setPos(x, y + beamYOffset);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tremolo::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
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
      return type2name(tremoloType());
      }

//---------------------------------------------------------
//   setTremoloType
//---------------------------------------------------------

void Tremolo::setTremoloType(const QString& s)
      {
      setTremoloType(name2Type(s));
      }


//---------------------------------------------------------
//   type2Name
//---------------------------------------------------------

QString Tremolo::type2name(TremoloType t)
      {
      switch(t) {
            case TremoloType::R8:  return QString("r8");
            case TremoloType::R16: return QString("r16");
            case TremoloType::R32: return QString("r32");
            case TremoloType::R64: return QString("r64");
            case TremoloType::C8:  return QString("c8");
            case TremoloType::C16: return QString("c16");
            case TremoloType::C32: return QString("c32");
            case TremoloType::C64: return QString("c64");
            case TremoloType::BUZZ_ROLL: return QString("buzzroll");
            default:
                  break;
            }
      return QString("??");
      }


//---------------------------------------------------------
//   nameToType
//---------------------------------------------------------

TremoloType Tremolo::name2Type(const QString& s)
      {
      TremoloType t = TremoloType::INVALID_TREMOLO;
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
      else if (s == "buzzroll")
            t = TremoloType::BUZZ_ROLL;
      return t;
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
      return qApp->translate("Tremolo", tremoloName[subtype()]);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Tremolo::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(subtypeName());
      }

}
