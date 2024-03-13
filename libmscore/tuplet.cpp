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

#include "tuplet.h"
#include "score.h"
#include "chord.h"
#include "note.h"
#include "xml.h"
#include "staff.h"
#include "style.h"
#include "text.h"
#include "element.h"
#include "undo.h"
#include "stem.h"
#include "beam.h"
#include "measure.h"
#include "system.h"

namespace Ms {

//---------------------------------------------------------
//   tupletStyle
//---------------------------------------------------------

static const ElementStyle tupletStyle {
      { Sid::tupletDirection,                    Pid::DIRECTION               },
      { Sid::tupletNumberType,                   Pid::NUMBER_TYPE             },
      { Sid::tupletBracketType,                  Pid::BRACKET_TYPE            },
      { Sid::tupletBracketWidth,                 Pid::LINE_WIDTH              },
      { Sid::tupletFontFace,                     Pid::FONT_FACE               },
      { Sid::tupletFontSize,                     Pid::FONT_SIZE               },
      { Sid::tupletFontStyle,                    Pid::FONT_STYLE              },
      { Sid::tupletAlign,                        Pid::ALIGN                   },
      { Sid::tupletMinDistance,                  Pid::MIN_DISTANCE            },
      { Sid::tupletFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT  },
      };

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

Tuplet::Tuplet(Score* s)
  : DurationElement(s)
      {
      _direction    = Direction::AUTO;
      _numberType   = TupletNumberType::SHOW_NUMBER;
      _bracketType  = TupletBracketType::AUTO_BRACKET;
      _ratio        = Fraction(1, 1);
      _number       = 0;
      _hasBracket   = false;
      _isUp         = true;
      _id           = 0;
      initElementStyle(&tupletStyle);
      }

Tuplet::Tuplet(const Tuplet& t)
   : DurationElement(t)
      {
      _tick         = t._tick;
      _hasBracket   = t._hasBracket;
      _ratio        = t._ratio;
      _baseLen      = t._baseLen;
      _direction    = t._direction;
      _numberType   = t._numberType;
      _bracketType  = t._bracketType;
      _bracketWidth = t._bracketWidth;

      _isUp          = t._isUp;

      p1             = t.p1;
      p2             = t.p2;
      _p1            = t._p1;
      _p2            = t._p2;

      _id            = t._id;
      // recreated on layout
      _number = 0;
      }

//---------------------------------------------------------
//   ~Tuplet
//---------------------------------------------------------

Tuplet::~Tuplet()
      {
      for (DurationElement* de : _elements)
            de->setTuplet(nullptr);
      delete _number;
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Tuplet::setSelected(bool f)
      {
      Element::setSelected(f);
      if (_number)
            _number->setSelected(f);
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Tuplet::setVisible(bool f)
      {
      Element::setVisible(f);
      if (_number)
            _number->setVisible(f);
      }

#if 0
//---------------------------------------------------------
//   tick
//---------------------------------------------------------

Fraction Tuplet::tick() const
      {
      std::vector<DurationElement*> _elements;

      const DurationElement* de = this;
      while (de->isTuplet()) {
            const Tuplet* t = toTuplet(de);
            if (t->_elements.empty())
                  return Fraction(0, 1);
            de = t->_elements.front();
            }
      return toChordRest(de)->tick();
      }
#endif

//---------------------------------------------------------
//   rtick
//---------------------------------------------------------

Fraction Tuplet::rtick() const
      {
      return tick() - measure()->tick();
      }

//---------------------------------------------------------
//   resetNumberProperty
//   reset number properties to default values
//   Set FONT_ITALIC to true, because for tuplets number should be italic
//---------------------------------------------------------

void Tuplet::resetNumberProperty()
      {
      for (auto p : { Pid::FONT_FACE, Pid::FONT_STYLE, Pid::FONT_SIZE, Pid::ALIGN, Pid::SIZE_SPATIUM_DEPENDENT })
            _number->resetProperty(p);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tuplet::layout()
      {
      if (_elements.empty()) {
            qDebug("Tuplet::layout(): tuplet is empty");
            return;
            }
      // is in a TAB without stems, skip any format: tuplets are not shown
      const StaffType* stt = staffType();
      if (stt && stt->isTabStaff() && stt->stemless())
            return;

      //
      // create tuplet number if necessary
      //
      qreal _spatium = spatium();
      if (_numberType != TupletNumberType::NO_TEXT) {
            if (_number == 0) {
                  _number = new Text(score(), Tid::TUPLET);
                  _number->setComposition(true);
                  _number->setTrack(track());
                  _number->setParent(this);
                  _number->setVisible(visible());
                  resetNumberProperty();
                  }
            // tuplet properties are propagated to number automatically by setProperty()
            // but we need to make sure flags are as well
            _number->setPropertyFlags(Pid::FONT_FACE, propertyFlags(Pid::FONT_FACE));
            _number->setPropertyFlags(Pid::FONT_SIZE, propertyFlags(Pid::FONT_SIZE));
            _number->setPropertyFlags(Pid::FONT_STYLE, propertyFlags(Pid::FONT_STYLE));
            _number->setPropertyFlags(Pid::ALIGN, propertyFlags(Pid::ALIGN));
            if (_numberType == TupletNumberType::SHOW_NUMBER)
                  _number->setXmlText(QString("%1").arg(_ratio.numerator()));
            else
                  _number->setXmlText(QString("%1:%2").arg(_ratio.numerator()).arg(_ratio.denominator()));

            _isSmall = true;
            for (const DurationElement* e : _elements) {
                  if ((e->isChordRest() && !toChordRest(e)->isSmall()) || (e->isTuplet() && !toTuplet(e)->isSmall())) {
                        _isSmall = false;
                        break;
                        }
                  }
            _number->setMag(_isSmall ? score()->styleD(Sid::smallNoteMag) : 1.0);

            }
      else {
            if (_number) {
                  if (_number->selected())
                        score()->deselect(_number);
                  delete _number;
                  _number = 0;
                  }
            }
      //
      // find out main direction
      //
      if (_direction == Direction::AUTO) {
            int up = 1;
            for (const DurationElement* e : _elements) {
                  if (e->isChord()) {
                        const Chord* c = toChord(e);
                        if (c->stemDirection() != Direction::AUTO)
                              up += c->stemDirection() == Direction::UP ? 1000 : -1000;
                        else {
                              up += c->up() ? 1 : -1;
                              }
                        }
                  else if (e->isTuplet()) {
                        // TODO
                        }
                  }
            _isUp = up > 0;
            }
      else
            _isUp = _direction == Direction::UP;

      //
      // find first and last chord of tuplet
      // (tuplets can be nested)
      //
      bool nested = false;
      const DurationElement* cr1 = _elements.front();
      while (cr1->isTuplet()) {
            const Tuplet* t = toTuplet(cr1);
            if (t->elements().empty())
                  break;
            nested = true;
            cr1 = t->elements().front();
            }
      const DurationElement* cr2 = _elements.back();
      while (cr2->isTuplet()) {
            const Tuplet* t = toTuplet(cr2);
            if (t->elements().empty())
                  break;
            nested = true;
            cr2 = t->elements().back();
            }

      //
      //   shall we draw a bracket?
      //
      if (_bracketType == TupletBracketType::AUTO_BRACKET) {
            _hasBracket = false;
            for (DurationElement* e : _elements) {
                  if (e->isTuplet() || e->isRest()) {
                        _hasBracket = true;
                        break;
                        }
                  else if (e->isChordRest()) {
                        ChordRest* cr = toChordRest(e);
                        //
                        // maybe we should check for more than one beam
                        //
                        if (cr->beam() == 0) {
                              _hasBracket = true;
                              break;
                              }
                        }
                  }
            }
      else
            _hasBracket = _bracketType != TupletBracketType::SHOW_NO_BRACKET;


      //
      //    calculate bracket start and end point p1 p2
      //
      qreal maxSlope      = score()->styleD(Sid::tupletMaxSlope);
      bool outOfStaff     = score()->styleB(Sid::tupletOufOfStaff);
      qreal vHeadDistance = score()->styleP(Sid::tupletVHeadDistance);
      qreal vStemDistance = score()->styleP(Sid::tupletVStemDistance);
      qreal stemLeft      = score()->styleP(Sid::tupletStemLeftDistance);
      qreal stemRight     = score()->styleP(Sid::tupletStemRightDistance);
      qreal noteLeft      = score()->styleP(Sid::tupletNoteLeftDistance);
      qreal noteRight     = score()->styleP(Sid::tupletNoteRightDistance);

      int move = 0;
      setTrack(cr1->staffIdx() * VOICES + voice());
      if (outOfStaff && cr1->isChordRest() && cr2->isChordRest()) {
            // account for staff move when adjusting bracket to avoid staff
            // but don't attempt adjustment unless both endpoints are in same staff
            // and not a nested tuplet
            if (toChordRest(cr1)->staffMove() == toChordRest(cr2)->staffMove() && !tuplet() && !nested) {
                  move = toChordRest(cr1)->staffMove();
                  if (move == 1)
                        setTrack(cr1->vStaffIdx() * VOICES + voice());
                  }
            else
                  outOfStaff = false;
            }

      qreal l1  =  score()->styleP(Sid::tupletBracketHookHeight);
      qreal l2l = vHeadDistance;    // left bracket vertical distance
      qreal l2r = vHeadDistance;    // right bracket vertical distance right

      if (_isUp)
            vHeadDistance = -vHeadDistance;

      p1      = cr1->pagePos();
      p2      = cr2->pagePos();

      p1.rx() -= noteLeft;
      p2.rx() += score()->noteHeadWidth() + noteRight;
      p1.ry() += vHeadDistance;        // TODO: Direction ?
      p2.ry() += vHeadDistance;

      qreal xx1 = p1.x(); // use to center the number on the beam

      // follow beam angle if one beam extends over entire tuplet
      bool followBeam = false;
      qreal beamAdjust = 0.0;
      if (cr1->beam() && cr1->beam() == cr2->beam()) {
            followBeam = true;
            beamAdjust = point(score()->styleS(Sid::beamWidth)) * 0.5 * mag();
            }

      if (_isUp) {
            if (cr1->isChord()) {
                  const Chord* chord1 = toChord(cr1);
                  Stem* stem = chord1->stem();
                  if (stem)
                        xx1 = stem->abbox().x();
                  if (chord1->up()) {
                        if (stem) {
                              if (followBeam)
                                    p1.ry() = stem->abbox().y() - beamAdjust;
                              else if (chord1->beam())
                                    p1.ry() = chord1->beam()->abbox().y();
                              else
                                    p1.ry() = stem->abbox().y();
                              l2l = vStemDistance;
                              }
                        else {
                              p1.ry() = chord1->upNote()->abbox().top(); // whole note
                              }
                        }
                  else if (!chord1->up()) {
                        p1.ry() = chord1->upNote()->abbox().top();
                        if (stem)
                              p1.rx() = cr1->pagePos().x() - stemLeft;
                        }
                  }

            if (cr2->isChord()) {
                  const Chord* chord2 = toChord(cr2);
                  Stem* stem = chord2->stem();
                  if (stem && chord2->up()) {
                        if (followBeam)
                              p2.ry() = stem->abbox().top() - beamAdjust;
                        else if (chord2->beam() && !chord2->staffMove() && !chord2->beam()->cross())
                              p2.ry() = chord2->beam()->abbox().top();
                        else
                              p2.ry() = stem->abbox().top();
                        l2r = vStemDistance;
                        p2.rx() = chord2->pagePos().x() + chord2->maxHeadWidth() + stemRight;
                        }
                  else {
                        p2.ry() = chord2->upNote()->abbox().top();
                        }
                  }
            //
            // special case: one of the bracket endpoints is
            // a rest
            //
            if (cr1->isChord() && cr2->isChord()) {
                  if (p2.y() < p1.y())
                        p1.setY(p2.y());
                  else
                        p2.setY(p1.y());
                  }
            else if (cr1->isChord() && !cr2->isChord()) {
                  if (p1.y() < p2.y())
                        p2.setY(p1.y());
                  else
                        p1.setY(p2.y());
                  }

            // outOfStaff
            if (outOfStaff) {
                  qreal min = cr1->measure()->staffabbox(cr1->staffIdx() + move).y();
                  if (min < p1.y()) {
                        p1.ry() = min;
                        l2l = vStemDistance;
                        }
                  min = cr2->measure()->staffabbox(cr2->staffIdx() + move).y();
                  if (min < p2.y()) {
                        p2.ry() = min;
                        l2r = vStemDistance;
                        }
                  }

            // check that slope is no more than max
            qreal d = (p2.y() - p1.y())/(p2.x() - p1.x());
            if (d  < -maxSlope) {
                  // move p1 y up
                  p1.ry() = p2.y() + maxSlope * (p2.x() - p1.x());
                  }
            else if (d  > maxSlope) {
                  // move p2 y up
                  p2.ry() = p1.ry() + maxSlope * (p2.x() - p1.x());
                  }

            // check for collisions
            size_t n = _elements.size();
            if (n >= 3) {
                  d = (p2.y() - p1.y())/(p2.x() - p1.x());
                  for (size_t i = 1; i < (n-1); ++i) {
                        Element* e = _elements[i];
                        if (e->isChord()) {
                              const Chord* chord = toChord(e);
                              const Stem* stem = chord->stem();
                              if (stem) {
                                    QRectF r(chord->up() ? stem->abbox() : chord->upNote()->abbox());
                                    qreal y3 = r.top();
                                    qreal x3 = r.x() + r.width() * .5;
                                    qreal y0 = p1.y() + (x3 - p1.x()) * d;
                                    qreal c  = y0 - y3;
                                    if (c > 0) {
                                          p1.ry() -= c;
                                          p2.ry() -= c;
                                          }
                                    }
                              }
                        }
                  }
            }
      else {
            if (cr1->isChord()) {
                  const Chord* chord1 = toChord(cr1);
                  Stem* stem = chord1->stem();
                  if (stem)
                        xx1 = stem->abbox().x();
                  if (!chord1->up()) {
                        if (stem) {
                              if (followBeam)
                                    p1.ry() = stem->abbox().bottom() + beamAdjust;
                              else if (chord1->beam())
                                    p1.ry() = chord1->beam()->abbox().bottom();
                              else
                                    p1.ry() = stem->abbox().bottom();
                              l2l = vStemDistance;
                              p1.rx() = cr1->pagePos().x() - stemLeft;
                              }
                        else {
                              p1.ry() = chord1->downNote()->abbox().bottom(); // whole note
                              }
                        }
                  else if (chord1->up()) {
                        p1.ry() = chord1->downNote()->abbox().bottom();
                        }
                  }

            if (cr2->isChord()) {
                  const Chord* chord2 = toChord(cr2);
                  Stem* stem = chord2->stem();
                  if (stem && !chord2->up()) {
                        // if (chord2->beam())
                        //      p2.setX(stem->abbox().x());
                        if (followBeam)                                          //??
                              p2.ry() = stem->abbox().bottom() + beamAdjust;     //??
                        if (chord2->beam() && !chord2->staffMove() && !chord2->beam()->cross())
                              p2.ry() = chord2->beam()->abbox().bottom();
                        else
                              p2.ry() = stem->abbox().bottom();
                        l2r = vStemDistance;
                        }
                  else {
                        p2.ry() = chord2->downNote()->abbox().bottom();
                        if (stem)
                              p2.rx() = chord2->pagePos().x() + chord2->maxHeadWidth() + stemRight;
                        }
                  }
            //
            // special case: one of the bracket endpoints is
            // a rest
            //
            if (!cr1->isChord() && cr2->isChord()) {
                  if (p2.y() > p1.y())
                        p1.setY(p2.y());
                  else
                        p2.setY(p1.y());
                  }
            else if (cr1->isChord() && !cr2->isChord()) {
                  if (p1.y() > p2.y())
                        p2.setY(p1.y());
                  else
                        p1.setY(p2.y());
                  }
            // outOfStaff
            if (outOfStaff) {
                  qreal max = cr1->measure()->staffabbox(cr1->staffIdx() + move).bottom();
                  if (max > p1.y()) {
                        p1.ry() = max;
                        l2l = vStemDistance;
                        }
                  max = cr2->measure()->staffabbox(cr2->staffIdx() + move).bottom();
                  if (max > p2.y()) {
                        p2.ry() = max;
                        l2r = vStemDistance;
                        }
                  }
            // check that slope is no more than max
            qreal d = (p2.y() - p1.y())/(p2.x() - p1.x());
            if (d  < -maxSlope) {
                  // move p1 y up
                  p2.ry() = p1.y() - maxSlope * (p2.x() - p1.x());
                  }
            else if (d  > maxSlope) {
                  // move p2 y up
                  p1.ry() = p2.ry() - maxSlope * (p2.x() - p1.x());
                  }

            // check for collisions
            size_t n = _elements.size();
            if (n >= 3) {
                  d  = (p2.y() - p1.y())/(p2.x() - p1.x());
                  for (size_t i = 1; i < (n-1); ++i) {
                        Element* e = _elements[i];
                        if (e->isChord()) {
                              const Chord* chord = toChord(e);
                              const Stem* stem = chord->stem();
                              if (stem) {
                                    QRectF r(chord->up() ? chord->downNote()->abbox() : stem->abbox());
                                    qreal y3 = r.bottom();
                                    qreal x3 = r.x() + r.width() * .5;
                                    qreal y0 = p1.y() + (x3 - p1.x()) * d;
                                    qreal c  = y0 - y3;
                                    if (c < 0) {
                                          p1.ry() -= c;
                                          p2.ry() -= c;
                                          }
                                    }
                              }
                        }
                  }
            }

      setPos(0.0, 0.0);
      QPointF mp(parent()->pagePos());
      if (parent()->isMeasure()) {
            System* s = toMeasure(parent())->system();
            if (s)
                  mp.ry() += s->staff(staffIdx())->y();
            }
      p1 -= mp;
      p2 -= mp;

      p1 += _p1;
      p2 += _p2;
      xx1 -= mp.x();

      p1.ry() -= l2l * (_isUp ? 1.0 : -1.0);
      p2.ry() -= l2r * (_isUp ? 1.0 : -1.0);

      // l2l l2r, mp, _p1, _p2 const

      // center number
      qreal x3 = 0.0;
      qreal numberWidth = 0.0;
      if (_number) {
            _number->layout();
            numberWidth = _number->bbox().width();

            qreal y3 = p1.y() + (p2.y() - p1.y()) * .5 - l1 * (_isUp ? 1.0 : -1.0);
            //
            // for beamed tuplets, center number on beam
            //
            if (cr1->beam() && cr2->beam() && cr1->beam() == cr2->beam()) {
                  const ChordRest* crr = toChordRest(cr1);
                  if (_isUp == crr->up()) {
                        qreal deltax = cr2->pagePos().x() - cr1->pagePos().x();
                        x3 = xx1 + deltax * .5;
                        }
                  else {
                        qreal deltax = p2.x() - p1.x();
                        x3 = p1.x() + deltax * .5;
                        }
                  }
            else {
                  qreal deltax = p2.x() - p1.x();
                  x3 = p1.x() + deltax * .5;
                  }

            _number->setPos(QPointF(x3, y3) - ipos());
            }

      if (_hasBracket) {
            qreal slope = (p2.y() - p1.y()) / (p2.x() - p1.x());

            if (_isUp) {
                  if (_number) {
                        bracketL[0] = QPointF(p1.x(), p1.y());
                        bracketL[1] = QPointF(p1.x(), p1.y() - l1);
                        //set width of bracket hole
                        qreal x     = x3 - numberWidth * .5 - _spatium * .5;

                        qreal y     = p1.y() + (x - p1.x()) * slope;
                        bracketL[2] = QPointF(x,   y - l1);

                        //set width of bracket hole
                        x           = x3 + numberWidth * .5 + _spatium * .5;
                        y           = p1.y() + (x - p1.x()) * slope;
                        bracketR[0] = QPointF(x,   y - l1);
                        bracketR[1] = QPointF(p2.x(), p2.y() - l1);
                        bracketR[2] = QPointF(p2.x(), p2.y());
                        }
                  else {
                        bracketL[0] = QPointF(p1.x(), p1.y());
                        bracketL[1] = QPointF(p1.x(), p1.y() - l1);
                        bracketL[2] = QPointF(p2.x(), p2.y() - l1);
                        bracketL[3] = QPointF(p2.x(), p2.y());
                        }
                  }
            else {
                  if (_number) {
                        bracketL[0] = QPointF(p1.x(), p1.y());
                        bracketL[1] = QPointF(p1.x(), p1.y() + l1);
                        //set width of bracket hole
                        qreal x     = x3 - numberWidth * .5 - _spatium * .5;
                        qreal y     = p1.y() + (x - p1.x()) * slope;
                        bracketL[2] = QPointF(x,   y + l1);

                        //set width of bracket hole
                        x           = x3 + numberWidth * .5 + _spatium * .5;
                        y           = p1.y() + (x - p1.x()) * slope;
                        bracketR[0] = QPointF(x,   y + l1);
                        bracketR[1] = QPointF(p2.x(), p2.y() + l1);
                        bracketR[2] = QPointF(p2.x(), p2.y());
                        }
                  else {
                        bracketL[0] = QPointF(p1.x(), p1.y());
                        bracketL[1] = QPointF(p1.x(), p1.y() + l1);
                        bracketL[2] = QPointF(p2.x(), p2.y() + l1);
                        bracketL[3] = QPointF(p2.x(), p2.y());
                        }
                  }
            }

      // collect bounding box
      QRectF r;
      if (_number) {
            r |= _number->bbox().translated(_number->pos());
            if (_hasBracket) {
                  QRectF b;
                  b.setCoords(bracketL[1].x(), bracketL[1].y(), bracketR[2].x(), bracketR[2].y());
                  r |= b;
                  }
            }
      else if (_hasBracket) {
            QRectF b;
            b.setCoords(bracketL[1].x(), bracketL[1].y(), bracketL[3].x(), bracketL[3].y());
            r |= b;
            }
      setbbox(r);

      if (outOfStaff && !cross())
            autoplaceMeasureElement(_isUp, /* add to skyline */ true);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Tuplet::draw(QPainter* painter) const
      {
      // if in a TAB without stems, tuplets are not shown
      const StaffType* stt = staffType();
      if (stt && stt->isTabStaff() && stt->stemless())
            return;

      QColor color(curColor());
      if (_number) {
            painter->setPen(color);
            QPointF pos(_number->pos());
            painter->translate(pos);
            _number->draw(painter);
            painter->translate(-pos);
            }
      if (_hasBracket) {
            QPen pen(color, _bracketWidth.val());
            pen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
            pen.setCapStyle(Qt::PenCapStyle::FlatCap);
            painter->setPen(pen);
            if (!_number)
                  painter->drawPolyline(bracketL, 4);
            else {
                  painter->drawPolyline(bracketL, 3);
                  painter->drawPolyline(bracketR, 3);
                  }
            }
      }

//---------------------------------------------------------
//   Rect
//    helper class
//---------------------------------------------------------

class Rect : public QRectF {
   public:
      Rect(const QPointF& p1, const QPointF& p2, qreal w);
      };

//---------------------------------------------------------
//   Rect
//    construct a rectangle out of a line with width w
//---------------------------------------------------------

Rect::Rect(const QPointF& p1, const QPointF& p2, qreal w)
      {
      qreal w2 = w * .5;
      setCoords(qMin(p1.x(), p2.x()) - w2, qMin(p1.y(), p2.y()) - w2,  qMax(p1.x(), p2.x()) + w2, qMax(p1.y(), p2.y()) + w2);
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape Tuplet::shape() const
      {
      Shape s;
      if (_hasBracket) {
            qreal w = _bracketWidth.val();
            s.add(Rect(bracketL[0], bracketL[1], w));
            s.add(Rect(bracketL[1], bracketL[2], w));
            if (_number) {
                  s.add(Rect(bracketR[0], bracketR[1], w));
                  s.add(Rect(bracketR[1], bracketR[2], w));
                  }
            else {
                  s.add(Rect(bracketL[2], bracketL[3], w));
                  }
            }
      if (_number)
            s.add(_number->bbox().translated(_number->pos()));
      return s;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Tuplet::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (_number && all)
            func(data, _number);
      func(data, this);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tuplet::write(XmlWriter& xml) const
      {
      xml.stag(this);
      Element::writeProperties(xml);

      writeProperty(xml, Pid::NORMAL_NOTES);
      writeProperty(xml, Pid::ACTUAL_NOTES);
      writeProperty(xml, Pid::P1);
      writeProperty(xml, Pid::P2);

      xml.tag("baseNote", _baseLen.name());
      if (int dots = _baseLen.dots())
            xml.tag("baseDots", dots);

      if (_number) {
            xml.stag("Number", _number);
            _number->writeProperty(xml, Pid::SUB_STYLE);
            _number->writeProperty(xml, Pid::TEXT);
            xml.etag();
            }

      writeStyledProperties(xml);

      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tuplet::read(XmlReader& e)
      {
      _id = e.intAttribute("id", 0);
      while (e.readNextStartElement()) {
            if (readProperties(e))
                  ;
            else
                  e.unknown();
            }
      Fraction f = _baseLen.fraction() * _ratio.denominator();
      setTicks(f.reduced());
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Tuplet::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (readStyledProperty(e, tag))
            ;
      else if (tag == "bold") { //important that these properties are read after number is created
            bool val = e.readInt();
            if (_number)
                  _number->setBold(val);
            if (isStyled(Pid::FONT_STYLE))
                  setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
      else if (tag == "italic") {
            bool val = e.readInt();
            if (_number)
                  _number->setItalic(val);
            if (isStyled(Pid::FONT_STYLE))
                  setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
      else if (tag == "underline") {
            bool val = e.readInt();
            if (_number)
                  _number->setUnderline(val);
            if (isStyled(Pid::FONT_STYLE))
                  setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
      else if (tag == "strike") {
            bool val = e.readInt();
            if (_number)
                  _number->setStrike(val);
            if (isStyled(Pid::FONT_STYLE))
                  setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
      else if (tag == "normalNotes")
            _ratio.setDenominator(e.readInt());
      else if (tag == "actualNotes")
            _ratio.setNumerator(e.readInt());
      else if (tag == "p1")
            _p1 = e.readPoint() * score()->spatium();
      else if (tag == "p2")
            _p2 = e.readPoint() * score()->spatium();
      else if (tag == "baseNote")
            _baseLen = TDuration(e.readElementText());
      else if (tag == "baseDots")
            _baseLen.setDots(e.readInt());
      else if (tag == "Number") {
            _number = new Text(score(), Tid::TUPLET);
            _number->setComposition(true);
            _number->setParent(this);
            resetNumberProperty();
            _number->read(e);
            _number->setVisible(visible());     //?? override saved property
            _number->setTrack(track());
            // move property flags from _number back to tuplet
            for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_STYLE, Pid::ALIGN })
                  setPropertyFlags(p, _number->propertyFlags(p));
            }
      else if (!DurationElement::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Tuplet::add(Element* e)
      {
#ifndef NDEBUG
      for(DurationElement* el : _elements) {
            if (el == e) {
                  qDebug("%p: %p %s already there", this, e, e->name());
                  return;
                  }
            }
#endif

      switch (e->type()) {
            case ElementType::CHORD:
            case ElementType::REST:
            case ElementType::TUPLET: {
                  bool found = false;
                  DurationElement* de = toDurationElement(e);
                  Fraction tick = de->rtick();
                  if (tick != Fraction(-1,1)) {
                        for (unsigned int i = 0; i < _elements.size(); ++i) {
                              if (_elements[i]->rtick() > tick) {
                                    _elements.insert(_elements.begin() + i, de);
                                    found = true;
                                    break;
                                    }
                              }
                        }
                  if (!found)
                        _elements.push_back(de);
                  de->setTuplet(this);
                  }
                  break;

            default:
                  qDebug("Tuplet::add() unknown element");
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Tuplet::remove(Element* e)
      {
      switch (e->type()) {
//            case ElementType::TEXT:
//                  if (e == _number)
//                        _number = 0;
//                  break;
            case ElementType::CHORD:
            case ElementType::REST:
            case ElementType::TUPLET: {
                  auto i = std::find(_elements.begin(), _elements.end(), toDurationElement(e));
                  if (i == _elements.end()) {
                        qDebug("Tuplet::remove: cannot find element <%s>", e->name());
                        qDebug("  elements %zu", _elements.size());
                        }
                  else
                        _elements.erase(i);
                  }
                  break;
            default:
                  qDebug("Tuplet::remove: unknown element");
                  break;
            }
      }

//---------------------------------------------------------
//   isEditable
//---------------------------------------------------------

bool Tuplet::isEditable() const
      {
      return _hasBracket;
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void Tuplet::startEditDrag(EditData& ed)
      {
      DurationElement::startEditDrag(ed);
      ElementEditData* eed = ed.getData(this);

      eed->pushProperty(Pid::P1);
      eed->pushProperty(Pid::P2);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Tuplet::editDrag(EditData& ed)
      {
      if (ed.curGrip == Grip::START)
            _p1 += ed.delta;
      else
            _p2 += ed.delta;
      setGenerated(false);
      //layout();
      //score()->setUpdateAll();
      triggerLayout();
      }

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<QPointF> Tuplet::gripsPositions(const EditData&) const
      {
      const QPointF pp(pagePos());
      return { pp + p1, pp + p2 };
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Tuplet::reset()
      {
      undoChangeProperty(Pid::P1, QPointF());
      undoChangeProperty(Pid::P2, QPointF());
      Element::reset();
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Tuplet::dump() const
      {
      Element::dump();
      qDebug("ratio %s", qPrintable(_ratio.print()));
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Tuplet::setTrack(int val)
      {
      if (tuplet())
            tuplet()->setTrack(val);
      if (_number)
            _number->setTrack(val);
      Element::setTrack(val);
      }

//---------------------------------------------------------
//   tickGreater
//---------------------------------------------------------

static bool tickGreater(const DurationElement* a, const DurationElement* b)
      {
      return a->tick() < b->tick();
      }

//---------------------------------------------------------
//   sortElements
//---------------------------------------------------------

void Tuplet::sortElements()
      {
      std::sort(_elements.begin(), _elements.end(), tickGreater);
      }

//---------------------------------------------------------
//   cross
//---------------------------------------------------------

bool Tuplet::cross() const
      {
      for (DurationElement* de : _elements) {
            if (!de) {
                  continue;
                  }
            else if (de->isChordRest()) {
                  if (toChordRest(de)->staffMove())
                        return true;
                  }
            else if (de->isTuplet()) {
                  if (toTuplet(de)->cross())
                        return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   elementsDuration
///  Get the sum of the element fraction in the tuplet,
///  even if the tuplet is not complete yet
//---------------------------------------------------------

Fraction Tuplet::elementsDuration()
      {
      Fraction f;
      for (DurationElement* el : _elements)
            f += el->ticks();
      return f;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Tuplet::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::DIRECTION:
                  return QVariant::fromValue<Direction>(_direction);
            case Pid::NUMBER_TYPE:
                  return int(_numberType);
            case Pid::BRACKET_TYPE:
                  return int(_bracketType);
            case Pid::LINE_WIDTH:
                  return _bracketWidth;
            case Pid::NORMAL_NOTES:
                  return _ratio.denominator();
            case Pid::ACTUAL_NOTES:
                  return _ratio.numerator();
            case Pid::P1:
                  return _p1;
            case Pid::P2:
                  return _p2;
            case Pid::FONT_SIZE:
            case Pid::FONT_FACE:
            case Pid::FONT_STYLE:
            case Pid::ALIGN:
            case Pid::SIZE_SPATIUM_DEPENDENT:
                  return _number ? _number->getProperty(propertyId) : QVariant();
            default:
                  break;
            }
      return DurationElement::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Tuplet::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::DIRECTION:
                  setDirection(v.value<Direction>());
                  break;
            case Pid::NUMBER_TYPE:
                  setNumberType(TupletNumberType(v.toInt()));
                  break;
            case Pid::BRACKET_TYPE:
                  setBracketType(TupletBracketType(v.toInt()));
                  break;
            case Pid::LINE_WIDTH:
                  setBracketWidth(v.value<Spatium>());
                  break;
            case Pid::NORMAL_NOTES:
                  _ratio.setDenominator(v.toInt());
                  break;
            case Pid::ACTUAL_NOTES:
                  _ratio.setNumerator(v.toInt());
                  break;
            case Pid::P1:
                  _p1 = v.toPointF();
                  break;
            case Pid::P2:
                  _p2 = v.toPointF();
                  break;
            case Pid::FONT_SIZE:
            case Pid::FONT_FACE:
            case Pid::FONT_STYLE:
            case Pid::ALIGN:
            case Pid::SIZE_SPATIUM_DEPENDENT:
                  if (_number)
                        _number->setProperty(propertyId, v);
                  break;
            default:
                  return DurationElement::setProperty(propertyId, v);
            }
      if (!_elements.empty()) {
            _elements.front()->triggerLayout();
            _elements.back()->triggerLayout();
            }
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Tuplet::propertyDefault(Pid id) const
      {
      switch(id) {
            case Pid::SUB_STYLE:
                  return int(Tid::TUPLET);
            case Pid::SYSTEM_FLAG:
                  return false;
            case Pid::TEXT:
                  return QString("");
            case Pid::NORMAL_NOTES:
            case Pid::ACTUAL_NOTES:
                  return 0;
            case Pid::P1:
            case Pid::P2:
                  return QPointF();
            case Pid::ALIGN:
                  return score()->styleV(Sid::tupletAlign);
            case Pid::FONT_FACE:
                  return score()->styleV(Sid::tupletFontFace);
            case Pid::FONT_SIZE:
                  return score()->styleV(Sid::tupletFontSize);
            case Pid::FONT_STYLE:
                  return score()->styleV(Sid::tupletFontStyle);
            case Pid::SIZE_SPATIUM_DEPENDENT:
                  return score()->styleV(Sid::tupletFontSpatiumDependent);
            default:
                  {
                  QVariant v = ScoreElement::propertyDefault(id, Tid::DEFAULT);
                  if (v.isValid())
                        return v;
                  }
                  return DurationElement::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   sanitizeTuplet
///    Check validity of tuplets and coherence between duration
///    and baselength. Needed for importing old files due to a bug
///    in the released version for corner-case tuplets.
///    See issue #136406 and Pull request #2881
//---------------------------------------------------------

void Tuplet::sanitizeTuplet()
      {
      if (ratio().numerator() == ratio().reduced().numerator()) // return if the ratio is an irreducible fraction
            return;
      Fraction baseLenDuration = (Fraction(ratio().denominator(),1) * baseLen().fraction()).reduced();

      // Due to a bug present in 2.1 (and before), a tuplet with non-reduced ratio could be
      // in a corrupted state (mismatch between duration and base length).
      // A tentative will now be made to retrieve the correct duration by summing up all the
      // durations of the elements constituting the tuplet. This does not work for
      // not-completely filled tuplets, such as tuplets in voices > 0 with
      // gaps (for example, a tuplet in second voice with a deleted chordrest element)

      Fraction testDuration(0,1);
      for (DurationElement* de : elements()) {
            if (!de)
                  continue;
            Fraction elementDuration(0,1);
            if (de->isTuplet()){
                  Tuplet* t = toTuplet(de);
                  t->sanitizeTuplet();
                  elementDuration = t->ticks();
                  }
            else {
                  elementDuration = de->ticks();
                  }
            testDuration += elementDuration;
            }
      testDuration = testDuration / ratio();
      testDuration.reduce();
      if (elements().back()->tick() + elements().back()->actualTicks() - elements().front()->tick() > testDuration)
            return;     // this tuplet has missing elements; do not sanitize
      if (!(testDuration == baseLenDuration && baseLenDuration == ticks())) {
            Fraction f = testDuration * Fraction(1, ratio().denominator());
            f.reduce();
            Fraction fbl(1, f.denominator());
            if (TDuration::isValid(fbl)) {
                  setTicks(testDuration);
                  setBaseLen(fbl);
                  qDebug("Tuplet %p sanitized duration %d/%d   baseLen %d/%d",this,
                        testDuration.numerator(), testDuration.denominator(),
                        1, fbl.denominator());
                  }
            else {
                  qDebug("Impossible to sanitize the tuplet");
                  }
            }
      }

//---------------------------------------------------------
//   addMissingElement
//     Add a rest with the given start and end ticks.
//     Should only be called from Tuplet::addMissingElements().
//     Needed for importing files that saved incomplete tuplets.
//---------------------------------------------------------

Fraction Tuplet::addMissingElement(const Fraction& startTick, const Fraction& endTick)
      {
      Fraction f = (endTick - startTick) * ratio();
      TDuration d = TDuration(f, true);
      if (!d.isValid()) {
            qDebug("Tuplet::addMissingElement(): invalid duration: %d/%d", f.numerator(), f.denominator());
            return Fraction::fromTicks(0);
            }
      f = d.fraction();
      Rest* rest = new Rest(score());
      rest->setDurationType(d);
      rest->setTicks(f);
      rest->setTrack(track());
      rest->setVisible(false);
      Segment* segment = measure()->getSegment(SegmentType::ChordRest, startTick);
      segment->add(rest);
      add(rest);
      return f;
      }

//---------------------------------------------------------
//   addMissingElements
//     Make this tuplet complete by filling in holes where
//     there ought to be rests. Needed for importing files
//     that saved incomplete tuplets.
//---------------------------------------------------------

void Tuplet::addMissingElements()
      {
      if (tuplet())
            return;     // do not correct nested tuplets
      if (voice() == 0)
            return;     // nothing to do for tuplets in voice 1
      Fraction missingElementsDuration = ticks() * ratio() - elementsDuration();
      if (missingElementsDuration.isZero())
            return;
      // first, fill in any holes in the middle of the tuplet
      Fraction expectedTick = elements().front()->tick();
      for (DurationElement* de : elements()) {
            if (!de)
                  continue;
            if (de->tick() != expectedTick) {
                  missingElementsDuration -= addMissingElement(expectedTick, de->tick());
                  if (missingElementsDuration.isZero())
                        return;
                  }
            expectedTick += de->actualTicks();
            }
      // calculate the tick where we would expect a tuplet of this duration to start
      // TODO: check:
      expectedTick = elements().front()->tick() - Fraction::fromTicks(elements().front()->tick().ticks() % ticks().ticks());
      if (expectedTick != elements().front()->tick()) {
            // try to fill a hole at the beginning of the tuplet
            Fraction firstAvailableTick = measure()->tick();
            Segment* segment = measure()->findSegment(SegmentType::ChordRest, elements().front()->tick());
            ChordRest* prevChordRest = segment && segment->prev() ? segment->prev()->nextChordRest(track(), true) : nullptr;
            if (prevChordRest && prevChordRest->measure() == measure())
                  firstAvailableTick = prevChordRest->tick() + prevChordRest->actualTicks();
            if (firstAvailableTick != elements().front()->tick()) {
                  Fraction f = missingElementsDuration / ratio();
                  Fraction ticksRequired = f;
                  Fraction endTick = elements().front()->tick();
                  Fraction startTick = std::max(firstAvailableTick, endTick - ticksRequired);
                  if (expectedTick > startTick)
                        startTick = expectedTick;
                  missingElementsDuration -= addMissingElement(startTick, endTick);
                  if (missingElementsDuration.isZero())
                        return;
                  }
            }
      // now fill a hole at the end of the tuplet
      Fraction startTick = elements().back()->tick() + elements().back()->actualTicks();
      Fraction endTick = elements().front()->tick() + ticks();
      // just to be safe, find the next ChordRest in the track, and adjust endTick if necessary
      Segment* segment = measure()->findSegment(SegmentType::ChordRest, elements().back()->tick());
      ChordRest* nextChordRest = segment && segment->next() ? segment->next()->nextChordRest(track(), false) : nullptr;
      if (nextChordRest && nextChordRest->tick() < endTick)
            endTick = nextChordRest->tick();
      missingElementsDuration -= addMissingElement(startTick, endTick);
      if (!missingElementsDuration.isZero())
            qDebug("Tuplet::addMissingElements(): still missing duration of %d/%d", missingElementsDuration.numerator(), missingElementsDuration.denominator());
      }
}  // namespace Ms

