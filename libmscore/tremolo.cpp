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
//   tremoloStyle
//---------------------------------------------------------

static const ElementStyle tremoloStyle {
      { Sid::tremoloPlacement, Pid::TREMOLO_PLACEMENT },
      };

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
      initElementStyle(&tremoloStyle);
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
      _durationType = t._durationType;
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
            const qreal _spatium = spatium();
            if (_tremoloType == TremoloType::BUZZ_ROLL)
                  painter->drawLine(QLineF(x, -_spatium, x, bbox().bottom() + _spatium));
            else
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
      computeShape();
      }

//---------------------------------------------------------
//   placeMidStem
///   For one-note tremolo, whether the tremolo should be
///   placed at stem middle.
//---------------------------------------------------------

bool Tremolo::placeMidStem() const
      {
      return _tremoloPlacement == TremoloPlacement::STEM_CENTER;
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Tremolo::spatiumChanged(qreal oldValue, qreal newValue)
      {
      Element::spatiumChanged(oldValue, newValue);
      computeShape();
      }

//---------------------------------------------------------
//   localSpatiumChanged
//    the scale of a staff changed
//---------------------------------------------------------

void Tremolo::localSpatiumChanged(qreal oldValue, qreal newValue)
      {
      Element::localSpatiumChanged(oldValue, newValue);
      computeShape();
      }

//---------------------------------------------------------
//   styleChanged
//    the scale of a staff changed
//---------------------------------------------------------

void Tremolo::styleChanged()
      {
      Element::styleChanged();
      computeShape();
      }

//---------------------------------------------------------
//   basePath
//---------------------------------------------------------

QPainterPath Tremolo::basePath() const
      {
      if (_tremoloType == TremoloType::BUZZ_ROLL)
            return QPainterPath();

      const qreal _spatium  = spatium();

      qreal w2  = _spatium * score()->styleS(Sid::tremoloWidth).val() * .5;
      qreal lw  = _spatium * score()->styleS(Sid::tremoloStrokeWidth).val();
      qreal td  = _spatium * score()->styleS(Sid::tremoloDistance).val();

      QPainterPath ppath;
      qreal ty  = 0.0;

      for (int i = 0; i < _lines; i++) {
            ppath.addRect(-w2, ty, 2.0 * w2, lw);
            ty += td;
            }

      if (!parent()) {
            // just for the palette
            QTransform shearTransform;
            shearTransform.shear(0.0, -(lw / 2.0) / w2);
            ppath = shearTransform.map(ppath);
            }
      else if (!twoNotes()) {
            QTransform shearTransform;
            shearTransform.shear(0.0, -(lw / 2.0) / w2);
            ppath = shearTransform.map(ppath);
            }

      return ppath;
      }

//---------------------------------------------------------
//   computeShape
//---------------------------------------------------------

void Tremolo::computeShape()
      {
      if (parent() && twoNotes())
            return; // cannot compute shape here, should be done at layout stage

      if (_tremoloType == TremoloType::BUZZ_ROLL)
            setbbox(symBbox(SymId::buzzRoll));
      else {
            path = basePath();
            setbbox(path.boundingRect());
            }
      }

//---------------------------------------------------------
//   layoutOneNoteTremolo
//---------------------------------------------------------

void Tremolo::layoutOneNoteTremolo(qreal x, qreal y, qreal _spatium)
      {
      Q_ASSERT(!twoNotes());

      bool up = _chord1->up();
      int line = up ? _chord1->upLine() : _chord1->downLine();

      if (!placeMidStem()) {
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
            y = (line + t[idx][up][_lines-1][line & 1]) * .5 * _spatium;
            }
      else {
            const Note* n = up ? chord()->downNote() : chord()->upNote();
            const qreal noteBorder = n->y() + (up ? n->bbox().top() : n->bbox().bottom());

            const Stem* stem = chord()->stem();
            const qreal stemLen = stem ? stem->height() : (3 * _spatium);
            const qreal stemY = stem ? (stem->y() + (up ? stem->bbox().bottom() : stem->bbox().top())) : noteBorder;
            const qreal stemNoteOverlap = std::max(0.0, (up ? 1.0 : -1.0) * (stemY - noteBorder));

            y = stemY
                  + (up ? -1 : 1) * (
                     stemNoteOverlap // calculate offset from note top or bottom rather than stem anchor point
                     + 0.5 * (stemLen - stemNoteOverlap) // divide stem by 2, excluding the area overlapping with the note
                     )
                  - 0.5 * height() - bbox().top(); // center the tremolo at the given position

            if (const Beam* b = chord()->beam()) {
                  // apply a correction for beam overlapping with the stem
                  const qreal beamHalfLineWidth = point(score()->styleS(Sid::beamWidth)) * .5 * mag();
                  const qreal beamSpace = b->beamDist() - 2 * beamHalfLineWidth;

                  int beamLvl = 1;
                  for (const ChordRest* cr : chord()->beam()->elements()) {
                        if (cr->isChord()) {
                              const int crBeamLvl = toChord(cr)->beams();
                              if (crBeamLvl > beamLvl)
                                    beamLvl = crBeamLvl;
                              }
                        }

                  const qreal stemBeamOverlap = beamLvl * b->beamDist() // initial guess
                                                   - beamHalfLineWidth // exclude the part of the beam line that does not overlap with the stem
                                                   - beamSpace; // exclude an extra spacing between beams that was included in the initial guess

                  y += (up ? 1 : -1) * stemBeamOverlap / 2;
                  }
            else if (chord()->hook()) {
                  const qreal hookLvlHeight = 0.5 * _spatium; // TODO: avoid hardcoding this (how?)
                  y += (up ? 1 : -1) * (chord()->beams() + 0.5) * hookLvlHeight;
                  }
            }

      setPos(x, y);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tremolo::layout()
      {
      const qreal _spatium  = spatium();

      path = basePath();

      _chord1 = toChord(parent());
      if (!_chord1) {
            // palette
            if (_tremoloType != TremoloType::BUZZ_ROLL) {
                  const QRectF box = path.boundingRect();
                  addbbox(QRectF(box.x(), box.bottom(), box.width(), _spatium));
                  }
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

      if (twoNotes())
            layoutTwoNotesTremolo(x, y, h, _spatium);
      else
            layoutOneNoteTremolo(x, y, _spatium);
      }

//---------------------------------------------------------
//   layoutTwoNotesTremolo
//---------------------------------------------------------

void Tremolo::layoutTwoNotesTremolo(qreal x, qreal y, qreal h, qreal _spatium)
      {
      y += (h - bbox().height()) * .5;
      //
      // two chord tremolo
      //

#if 0 // Needs to be done earlier, see connectTremolo in layout.cpp
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
#endif

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
      const qreal w2  = _spatium * score()->styleS(Sid::tremoloWidth).val() * .5;
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
      writeProperty(xml, Pid::TREMOLO_TYPE);
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

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Tremolo::getProperty(Pid propertyId) const
      {
      switch(propertyId) {
            case Pid::TREMOLO_TYPE:
                  return int(_tremoloType);
            case Pid::TREMOLO_PLACEMENT:
                  return int (_tremoloPlacement);
            default:
                  break;
            }
      return Element::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Tremolo::setProperty(Pid propertyId, const QVariant& val)
      {
      switch(propertyId) {
            case Pid::TREMOLO_TYPE:
                  setTremoloType(TremoloType(val.toInt()));
                  break;
            case Pid::TREMOLO_PLACEMENT:
                  _tremoloPlacement = TremoloPlacement(val.toInt());
                  break;
            default:
                  return Element::setProperty(propertyId, val);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid Tremolo::propertyId(const QStringRef& name) const
      {
      if (name == "subtype")
            return Pid::TREMOLO_TYPE;
      return Element::propertyId(name);
      }

//---------------------------------------------------------
//   propertyUserValue
//---------------------------------------------------------

QString Tremolo::propertyUserValue(Pid pid) const
      {
      switch(pid) {
            case Pid::TREMOLO_TYPE:
                  return subtypeName();
            default:
                  break;
            }
      return Element::propertyUserValue(pid);
      }
}
