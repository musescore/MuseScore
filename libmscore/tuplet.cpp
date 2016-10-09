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

namespace Ms {

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

Tuplet::Tuplet(Score* s)
  : DurationElement(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);

      _direction    = score()->style(StyleIdx::tupletDirection).value<Direction>();
      _numberType   = NumberType(score()->style(StyleIdx::tupletNumberType).toInt());
      _bracketType  = BracketType(score()->style(StyleIdx::tupletBracketType).toInt());

      _ratio        = Fraction(1, 1);
      _number       = 0;
      _hasBracket   = false;
      _isUp         = true;
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

      directionStyle = t.directionStyle;
      numberStyle    = t.numberStyle;
      bracketStyle   = t.bracketStyle;

      _isUp          = t._isUp;

      p1             = t.p1;
      p2             = t.p2;
      _p1            = t._p1;
      _p2            = t._p2;

      if (t._number)
            _number = new Text(*t._number);
      else
            _number = 0;
      }

//---------------------------------------------------------
//   ~Tuplet
//---------------------------------------------------------

Tuplet::~Tuplet()
      {
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
      if (staff() && staff()->isTabStaff() && staff()->staffType()->slashStyle())
            return;

      qreal _spatium = spatium();
      if (_numberType != NumberType::NO_TEXT) {
            if (_number == 0) {
                  _number = new Text(score());
                  _number->setTextStyleType(TextStyleType::TUPLET);
                  _number->setTrack(track());
                  _number->setParent(this);
                  _number->setVisible(visible());
                  }
            if (_numberType == NumberType::SHOW_NUMBER)
                  _number->setXmlText(QString("%1").arg(_ratio.numerator()));
            else
                  _number->setXmlText(QString("%1:%2").arg(_ratio.numerator()).arg(_ratio.denominator()));
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
            foreach (const DurationElement* e, _elements) {
                  if (e->isChord()) {
                        const Chord* c = toChord(e);
                        if (c->stemDirection() != Direction::AUTO)
                              up += c->stemDirection() == Direction::UP ? 1000 : -1000;
                        else
                              up += c->up() ? 1 : -1;
                        }
                  else if (e->isTuplet()) {
                        // TODO
                        }
                  }
            _isUp = up > 0;
            }
      else
            _isUp = _direction == Direction::UP;

      const DurationElement* cr1 = _elements.front();
      while (cr1->isTuplet()) {
            const Tuplet* t = toTuplet(cr1);
            if (t->elements().empty())
                  break;
            cr1 = t->elements().front();
            }
      const DurationElement* cr2 = _elements.back();
      while (cr2->isTuplet()) {
            const Tuplet* t = toTuplet(cr2);
            if (t->elements().empty())
                  break;
            cr2 = t->elements().back();
            }

      //
      //   shall we draw a bracket?
      //
      if (_bracketType == BracketType::AUTO_BRACKET) {
            _hasBracket = false;
            foreach (DurationElement* e, _elements) {
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
            _hasBracket = _bracketType != BracketType::SHOW_NO_BRACKET;


      //
      //    calculate bracket start and end point p1 p2
      //
      qreal maxSlope = score()->styleD(StyleIdx::tupletMaxSlope);
      bool outOfStaff = score()->styleB(StyleIdx::tupletOufOfStaff);
      qreal vHeadDistance = score()->styleP(StyleIdx::tupletVHeadDistance);
      qreal vStemDistance = score()->styleP(StyleIdx::tupletVStemDistance);
      qreal stemLeft = score()->styleP(StyleIdx::tupletStemLeftDistance);
      qreal stemRight = score()->styleP(StyleIdx::tupletStemRightDistance);
      qreal noteLeft = score()->styleP(StyleIdx::tupletNoteLeftDistance);
      qreal noteRight = score()->styleP(StyleIdx::tupletNoteRightDistance);

      int move = 0;
      if (outOfStaff && cr1->isChordRest() && cr2->isChordRest()) {
            // account for staff move when adjusting bracket to avoid staff
            // but don't attempt adjustment unless both endpoints are in same staff
            if (toChordRest(cr1)->staffMove() == toChordRest(cr2)->staffMove())
                  move = toChordRest(cr1)->staffMove();
            else
                  outOfStaff = false;
            }

      qreal l1 = _spatium;          // bracket tip height
      qreal l2l = vHeadDistance;    // left bracket vertical distance
      qreal l2r = vHeadDistance;    // right bracket vertical distance right

      if (_isUp)
            vHeadDistance = -vHeadDistance;

      p1      = cr1->pagePos();
      p2      = cr2->pagePos();
      p1.rx() -= noteLeft;
      p2.rx() += score()->noteHeadWidth() + noteRight;
      p1.ry() += vHeadDistance;
      p2.ry() += vHeadDistance;

      qreal xx1 = p1.x(); // use to center the number on the beam

      // follow beam angle if one beam extends over entire tuplet
      bool followBeam = false;
      qreal beamAdjust = 0.0;
      if (cr1->beam() && cr1->beam() == cr2->beam()) {
            followBeam = true;
            beamAdjust = score()->styleP(StyleIdx::beamWidth) * 0.5 * mag();
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
                        else if (chord2->beam())
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
            int n = _elements.size();
            if (n >= 3) {
                  d = (p2.y() - p1.y())/(p2.x() - p1.x());
                  for (int i = 1; i < (n-1); ++i) {
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
                        if (followBeam)
                              p2.ry() = stem->abbox().bottom() + beamAdjust;
                        if (chord2->beam())
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
            int n = _elements.size();
            if (n >= 3) {
                  qreal d  = (p2.y() - p1.y())/(p2.x() - p1.x());
                  for (int i = 1; i < (n-1); ++i) {
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
      p1 -= mp;
      p2 -= mp;

      p1 += _p1;
      p2 += _p2;
      xx1 -= mp.x();

      p1.ry() -= l2l * (_isUp ? 1.0 : -1.0);
      p2.ry() -= l2r * (_isUp ? 1.0 : -1.0);

      // center number
      qreal x3 = 0.0;
      qreal numberWidth = 0.0;
      if (_number) {
            _number->layout();
            numberWidth = _number->bbox().width();
            //
            // for beamed tuplets, center number on beam
            //
            if (cr1->beam() && cr2->beam() && cr1->beam() == cr2->beam()) {
                  const ChordRest* crr = toChordRest(cr1);
                  if(_isUp == crr->up()) {
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

            qreal y3 = p1.y() + (p2.y() - p1.y()) * .5 - l1 * (_isUp ? 1.0 : -1.0);
            _number->setPos(QPointF(x3, y3) - ipos());
            }

      if (_hasBracket) {
            qreal slope = (p2.y() - p1.y()) / (p2.x() - p1.x());

            if (_isUp) {
                  if (_number) {
                        bracketL[0] = QPointF(p1.x(), p1.y());
                        bracketL[1] = QPointF(p1.x(), p1.y() - l1);
                        qreal x     = x3 - numberWidth * .5 - _spatium * .5;
                        qreal y     = p1.y() + (x - p1.x()) * slope;
                        bracketL[2] = QPointF(x,   y - l1);

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
                        qreal x     = x3 - numberWidth * .5 - _spatium * .5;
                        qreal y     = p1.y() + (x - p1.x()) * slope;
                        bracketL[2] = QPointF(x,   y + l1);

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
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Tuplet::draw(QPainter* painter) const
      {
      // if in a TAB without stems, tuplets are not shown
      if (staff() && staff()->isTabStaff() && staff()->staffType()->slashStyle())
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
            painter->setPen(QPen(color, mag() * score()->styleP(StyleIdx::tupletBracketWidth)));
            if (!_number)
                  painter->drawPolyline(bracketL, 4);
            else {
                  painter->drawPolyline(bracketL, 3);
                  painter->drawPolyline(bracketR, 3);
                  }
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Tuplet::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      func(data, this);
      if (_number && all)
            func(data, _number);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tuplet::write(Xml& xml) const
      {
      xml.stag(QString("Tuplet id=\"%1\"").arg(_id));
      if (tuplet())
            xml.tag("Tuplet", tuplet()->id());
      Element::writeProperties(xml);
      writeProperty(xml, P_ID::DIRECTION);
      writeProperty(xml, P_ID::NUMBER_TYPE);
      writeProperty(xml, P_ID::BRACKET_TYPE);
      writeProperty(xml, P_ID::NORMAL_NOTES);
      writeProperty(xml, P_ID::ACTUAL_NOTES);
      writeProperty(xml, P_ID::P1);
      writeProperty(xml, P_ID::P2);

      xml.tag("baseNote", _baseLen.name());

      if (_number) {
            xml.stag("Number");
            _number->writeProperties(xml);
            xml.etag();
            }
      if (!userOff().isNull())
            xml.tag("offset", userOff() / spatium());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tuplet::read(XmlReader& e)
      {
      int bl = -1;
      _id    = e.intAttribute("id", 0);

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "direction")
                  readProperty(e, P_ID::DIRECTION);
            else if (tag == "numberType")
                  _numberType = NumberType(e.readInt());
            else if (tag == "bracketType")
                  _bracketType = BracketType(e.readInt());
            else if (tag == "normalNotes")
                  _ratio.setDenominator(e.readInt());
            else if (tag == "actualNotes")
                  _ratio.setNumerator(e.readInt());
            else if (tag == "p1")
                  _p1 = e.readPoint();
            else if (tag == "p2")
                  _p2 = e.readPoint();
            else if (tag == "baseNote")
                  _baseLen = TDuration(e.readElementText());
            else if (tag == "Number") {
                  _number = new Text(score());
                  _number->setParent(this);
                  _number->read(e);
                  _number->setTextStyleType(TextStyleType::TUPLET);
                  _number->setVisible(visible());     //?? override saved property
                  _number->setTrack(track());
                  }
            else if (tag == "subtype")    // obsolete
                  e.skipCurrentElement();
            else if (tag == "hasNumber")             // obsolete
                  _numberType = e.readInt() ? NumberType::SHOW_NUMBER : NumberType::NO_TEXT;
            else if (tag == "hasLine") {          // obsolete
                  _hasBracket = e.readInt();
                  _bracketType = BracketType::AUTO_BRACKET;
                  }
            else if (tag == "baseLen")            // obsolete
                  bl = e.readInt();
            else if (!DurationElement::readProperties(e))
                  e.unknown();
            }
      Fraction f(_ratio.reduced().denominator(), _baseLen.fraction().denominator());
      setDuration(f);
      if (bl != -1) {         // obsolete
            TDuration d;
            d.setVal(bl);
            _baseLen = d;
// qDebug("Tuplet base len %d/%d", d.fraction().numerator(), d.fraction().denominator());
// qDebug("   %s  dots %d, %d/%d", qPrintable(d.name()), d.dots(), _ratio.numerator(), _ratio.denominator());
            d.setVal(bl * _ratio.denominator());
            setDuration(d.fraction());
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Tuplet::add(Element* e)
      {
#ifndef NDEBUG
      for(DurationElement* el : _elements) {
            if (el == e) {
                  qDebug("%p Tuplet::add: %p %s already there", this, e, e->name());
                  return;
                  }
            }
#endif

      switch (e->type()) {
            case Element::Type::TEXT:
                  _number = static_cast<Text*>(e);
                  break;
            case Element::Type::CHORD:
            case Element::Type::REST:
            case Element::Type::TUPLET: {
                  bool found = false;
                  DurationElement* de = static_cast<DurationElement*>(e);
                  int tick = de->tick();
                  if (tick != -1) {
                        for (unsigned int i = 0; i < _elements.size(); ++i) {
                              if (_elements[i]->tick() > tick) {
                                    _elements.insert(_elements.begin() + i, de);
                                    found = true;
                                    break;
                                    }
                              }
                        }
                  if (!found)
                        _elements.push_back(de);
                  de->setTuplet(this);

                  // the tick position of a tuplet is the tick position of its
                  // first element:
                  setTick(_elements.front()->tick());
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
            case Element::Type::TEXT:
                  if (e == _number)
                        _number = 0;
                  break;
            case Element::Type::CHORD:
            case Element::Type::REST:
            case Element::Type::TUPLET: {
                  auto i = std::find(_elements.begin(), _elements.end(), static_cast<DurationElement*>(e));
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
//   editDrag
//---------------------------------------------------------

void Tuplet::editDrag(const EditData& ed)
      {
      if (ed.curGrip == Grip::START)
            _p1 += ed.delta;
      else
            _p2 += ed.delta;
      setGenerated(false);
      layout();
      score()->setUpdateAll();
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Tuplet::updateGrips(Grip* defaultGrip, QVector<QRectF>& grip) const
      {
      *defaultGrip = Grip::END;
      grip[0].translate(pagePos() + p1);
      grip[1].translate(pagePos() + p2);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Tuplet::reset()
      {
      if (directionStyle == PropertyStyle::UNSTYLED)
            undoChangeProperty(P_ID::DIRECTION, propertyDefault(P_ID::DIRECTION), PropertyStyle::STYLED);
      if (numberStyle == PropertyStyle::UNSTYLED)
            undoChangeProperty(P_ID::NUMBER_TYPE, propertyDefault(P_ID::NUMBER_TYPE), PropertyStyle::STYLED);
      if (bracketStyle == PropertyStyle::UNSTYLED)
            undoChangeProperty(P_ID::BRACKET_TYPE, propertyDefault(P_ID::BRACKET_TYPE), PropertyStyle::STYLED);

      score()->addRefresh(canvasBoundingRect());

      undoChangeProperty(P_ID::P1,        QPointF());
      undoChangeProperty(P_ID::P2,        QPointF());
      undoChangeProperty(P_ID::DIRECTION, propertyDefault(P_ID::DIRECTION));

      Element::reset();
      layout();
      score()->addRefresh(canvasBoundingRect());
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
      qSort(_elements.begin(), _elements.end(), tickGreater);
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
            f += el->duration();
      return f;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Tuplet::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::DIRECTION:
                  return _direction;
            case P_ID::NUMBER_TYPE:
                  return int(_numberType);
            case P_ID::BRACKET_TYPE:
                  return int(_bracketType);
            case P_ID::NORMAL_NOTES:
                  return _ratio.denominator();
            case P_ID::ACTUAL_NOTES:
                  return _ratio.numerator();
            case P_ID::P1:
                  return _p1;
            case P_ID::P2:
                  return _p2;
            default:
                  break;
            }
      return DurationElement::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Tuplet::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::DIRECTION:
                  directionStyle = PropertyStyle::UNSTYLED;
                  setDirection(v.value<Direction>());
                  break;
            case P_ID::NUMBER_TYPE:
                  numberStyle = PropertyStyle::UNSTYLED;
                  setNumberType(NumberType(v.toInt()));
                  break;
            case P_ID::BRACKET_TYPE:
                  bracketStyle = PropertyStyle::UNSTYLED;
                  setBracketType(BracketType(v.toInt()));
                  break;
            case P_ID::NORMAL_NOTES:
                  _ratio.setDenominator(v.toInt());
                  break;
            case P_ID::ACTUAL_NOTES:
                  _ratio.setNumerator(v.toInt());
                  break;
            case P_ID::P1:
                  _p1 = v.toPointF();
                  break;
            case P_ID::P2:
                  _p2 = v.toPointF();
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

QVariant Tuplet::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_ID::DIRECTION:
                  return score()->style(StyleIdx::tupletDirection);
            case P_ID::NUMBER_TYPE:
                  return score()->style(StyleIdx::tupletNumberType);
            case P_ID::BRACKET_TYPE:
                  return score()->style(StyleIdx::tupletBracketType);
            case P_ID::NORMAL_NOTES:
            case P_ID::ACTUAL_NOTES:
                  return 0;
            case P_ID::P1:
            case P_ID::P2:
                  return QPointF();
            default:
                  return DurationElement::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle Tuplet::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::DIRECTION:
                  return directionStyle;
            case P_ID::NUMBER_TYPE:
                  return numberStyle;
            case P_ID::BRACKET_TYPE:
                  return bracketStyle;
            case P_ID::NORMAL_NOTES:
            case P_ID::ACTUAL_NOTES:
            case P_ID::P1:
            case P_ID::P2:
                  return PropertyStyle::NOSTYLE;
            default:
                  return DurationElement::propertyStyle(id);
            }
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Tuplet::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::DIRECTION:
                  return StyleIdx::tupletDirection;
            case P_ID::NUMBER_TYPE:
                  return StyleIdx::tupletNumberType;
            case P_ID::BRACKET_TYPE:
                  return StyleIdx::tupletBracketType;
            default:
                  break;
            }
      return StyleIdx::NOSTYLE;
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Tuplet::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::DIRECTION:
                  setProperty(id, propertyDefault(id));
                  directionStyle = PropertyStyle::STYLED;
                  break;
            case P_ID::NUMBER_TYPE:
                  setProperty(id, propertyDefault(id));
                  numberStyle = PropertyStyle::STYLED;
                  break;
            case P_ID::BRACKET_TYPE:
                  setProperty(id, propertyDefault(id));
                  bracketStyle = PropertyStyle::STYLED;
                  break;
            case P_ID::NORMAL_NOTES:
            case P_ID::ACTUAL_NOTES:
                  break;
            case P_ID::P1:
            case P_ID::P2:
                  setProperty(id, propertyDefault(id));
                  break;
            default:
                  return DurationElement::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Tuplet::styleChanged()
      {
      if (directionStyle == PropertyStyle::STYLED)
            setDirection(score()->style(StyleIdx::tupletDirection).value<Direction>());
      if (numberStyle == PropertyStyle::STYLED)
            setNumberType(NumberType(score()->styleI(StyleIdx::tupletNumberType)));
      if (bracketStyle == PropertyStyle::STYLED)
            setBracketType(BracketType(score()->styleI(StyleIdx::tupletBracketType)));
      if (!_elements.empty()) {
            _elements.front()->triggerLayout();
            _elements.back()->triggerLayout();
            }
      }

}  // namespace Ms

