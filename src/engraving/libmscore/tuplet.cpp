/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "tuplet.h"

#include "draw/types/pen.h"

#include "style/textstyle.h"
#include "types/typesconv.h"

#include "beam.h"
#include "chord.h"
#include "engravingitem.h"
#include "factory.h"
#include "measure.h"
#include "note.h"
#include "rest.h"
#include "score.h"
#include "stafftype.h"
#include "stem.h"
#include "system.h"
#include "text.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   tupletStyle
//---------------------------------------------------------

static const ElementStyle tupletStyle {
    { Sid::tupletDirection,                    Pid::DIRECTION },
    { Sid::tupletNumberType,                   Pid::NUMBER_TYPE },
    { Sid::tupletBracketType,                  Pid::BRACKET_TYPE },
    { Sid::tupletBracketWidth,                 Pid::LINE_WIDTH },
    { Sid::tupletFontFace,                     Pid::FONT_FACE },
    { Sid::tupletFontSize,                     Pid::FONT_SIZE },
    { Sid::tupletFontStyle,                    Pid::FONT_STYLE },
    { Sid::tupletAlign,                        Pid::ALIGN },
    { Sid::tupletMinDistance,                  Pid::MIN_DISTANCE },
    { Sid::tupletFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
};

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

Tuplet::Tuplet(Measure* parent)
    : DurationElement(ElementType::TUPLET, parent)
{
    _direction    = DirectionV::AUTO;
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
    for (DurationElement* de : _elements) {
        de->setTuplet(nullptr);
    }
    delete _number;
}

void Tuplet::setParent(Measure* parent)
{
    EngravingItem::setParent(parent);
}

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Tuplet::setSelected(bool f)
{
    EngravingItem::setSelected(f);
    if (_number) {
        _number->setSelected(f);
    }
}

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Tuplet::setVisible(bool f)
{
    EngravingItem::setVisible(f);
    if (_number) {
        _number->setVisible(f);
    }
}

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
    resetNumberProperty(_number);
}

void Tuplet::resetNumberProperty(Text* number)
{
    for (auto p : { Pid::FONT_FACE, Pid::FONT_STYLE, Pid::FONT_SIZE, Pid::ALIGN, Pid::SIZE_SPATIUM_DEPENDENT }) {
        number->resetProperty(p);
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tuplet::layout()
{
    if (_elements.empty()) {
        LOGD("Tuplet::layout(): tuplet is empty");
        return;
    }
    // is in a TAB without stems, skip any format: tuplets are not shown
    const StaffType* stt = staffType();
    if (stt && stt->isTabStaff() && stt->stemless()) {
        return;
    }

    //
    // create tuplet number if necessary
    //
    double _spatium = spatium();
    if (_numberType != TupletNumberType::NO_TEXT) {
        if (_number == 0) {
            _number = Factory::createText(this, TextStyleType::TUPLET);
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
        if (_numberType == TupletNumberType::SHOW_NUMBER) {
            _number->setXmlText(String(u"%1").arg(_ratio.numerator()));
        } else {
            _number->setXmlText(String(u"%1:%2").arg(_ratio.numerator(), _ratio.denominator()));
        }

        _isSmall = true;
        for (const DurationElement* e : _elements) {
            if ((e->isChordRest() && !toChordRest(e)->isSmall()) || (e->isTuplet() && !toTuplet(e)->isSmall())) {
                _isSmall = false;
                break;
            }
        }
        _number->setMag(_isSmall ? score()->styleD(Sid::smallNoteMag) : 1.0);
    } else {
        if (_number) {
            if (_number->selected()) {
                score()->deselect(_number);
            }
            delete _number;
            _number = 0;
        }
    }
    //
    // find out main direction
    //
    if (_direction == DirectionV::AUTO) {
        int up = 1;
        for (const DurationElement* e : _elements) {
            if (e->isChord()) {
                const Chord* c = toChord(e);
                if (c->stemDirection() != DirectionV::AUTO) {
                    up += c->stemDirection() == DirectionV::UP ? 1000 : -1000;
                } else {
                    up += c->up() ? 1 : -1;
                }
            } else if (e->isTuplet()) {
                // TODO
            }
        }
        _isUp = up > 0;
    } else {
        _isUp = _direction == DirectionV::UP;
    }

    //
    // find first and last chord of tuplet
    // (tuplets can be nested)
    //
    const DurationElement* cr1 = _elements.front();
    while (cr1->isTuplet()) {
        const Tuplet* t = toTuplet(cr1);
        if (t->elements().empty()) {
            break;
        }
        cr1 = t->elements().front();
    }
    const DurationElement* cr2 = _elements.back();
    while (cr2->isTuplet()) {
        const Tuplet* t = toTuplet(cr2);
        if (t->elements().empty()) {
            break;
        }
        cr2 = t->elements().back();
    }

    _hasBracket = calcHasBracket(cr1, cr2);
    setMag((cr1->mag() + cr2->mag()) / 2);

    //
    //    calculate bracket start and end point p1 p2
    //
    double maxSlope      = score()->styleD(Sid::tupletMaxSlope);
    bool outOfStaff     = score()->styleB(Sid::tupletOufOfStaff);
    double vHeadDistance = score()->styleMM(Sid::tupletVHeadDistance) * mag();
    double vStemDistance = score()->styleMM(Sid::tupletVStemDistance) * mag();
    double stemLeft      = (score()->styleMM(Sid::tupletStemLeftDistance) - score()->styleMM(Sid::tupletBracketWidth) / 2) * cr1->mag();
    double stemRight     = (score()->styleMM(Sid::tupletStemRightDistance) - score()->styleMM(Sid::tupletBracketWidth) / 2) * cr2->mag();
    double noteLeft      = (score()->styleMM(Sid::tupletNoteLeftDistance) - score()->styleMM(Sid::tupletBracketWidth) / 2) * cr1->mag();
    double noteRight     = (score()->styleMM(Sid::tupletNoteRightDistance) - score()->styleMM(Sid::tupletBracketWidth) / 2) * cr2->mag();

    int move = 0;
    setStaffIdx(cr1->vStaffIdx());
    if (outOfStaff && cr1->isChordRest() && cr2->isChordRest()) {
        // account for staff move when adjusting bracket to avoid staff
        // but don't attempt adjustment unless both endpoints are in same staff
        if (toChordRest(cr1)->staffMove() == toChordRest(cr2)->staffMove()) {
            move = toChordRest(cr1)->staffMove();
            if (move == 1) {
                setStaffIdx(cr1->vStaffIdx());
            }
        } else {
            outOfStaff = false;
        }
    }

    double l1  =  score()->styleMM(Sid::tupletBracketHookHeight) * mag();
    double l2l = vHeadDistance;      // left bracket vertical distance
    double l2r = vHeadDistance;      // right bracket vertical distance right

    if (_isUp) {
        vHeadDistance = -vHeadDistance;
    }

    p1      = cr1->pagePos();
    p2      = cr2->pagePos();

    p1.rx() -= noteLeft;
    p2.rx() += score()->noteHeadWidth() + noteRight;
    p1.ry() += vHeadDistance;          // TODO: Direction ?
    p2.ry() += vHeadDistance;

    double xx1 = p1.x();   // use to center the number on the beam

    double leftNoteEdge = 0.0; // page coordinates
    double rightNoteEdge = 0.0;
    if (cr1->isChord()) {
        const Chord* chord1 = toChord(cr1);
        leftNoteEdge = chord1->up() ? chord1->downNote()->abbox().left() : chord1->upNote()->abbox().left();
    }
    if (cr2->isChord()) {
        const Chord* chord2 = toChord(cr2);
        rightNoteEdge = chord2->up() ? chord2->downNote()->abbox().right() : chord2->upNote()->abbox().right();
    }

    if (_isUp) {
        if (cr1->isChord()) {
            const Chord* chord1 = toChord(cr1);
            Stem* stem = chord1->stem();
            if (stem) {
                xx1 = stem->abbox().x();
            }
            if (chord1->up() && stem) {
                p1.ry() = stem->abbox().y();
                l2l = vStemDistance;
                p1.rx() = stem->abbox().left() - stemLeft;
            } else {
                p1.ry() = chord1->upNote()->abbox().top();
                p1.rx() = leftNoteEdge - noteLeft;
            }
        }

        if (cr2->isChord()) {
            const Chord* chord2 = toChord(cr2);
            Stem* stem = chord2->stem();
            if (stem && chord2->up()) {
                p2.ry() = stem->abbox().top();
                l2r = vStemDistance;
                p2.rx() = stem->abbox().right() + stemRight;
            } else {
                p2.ry() = chord2->upNote()->abbox().top();
                p2.rx() = rightNoteEdge + noteRight;
            }
        }
        //
        // special case: one of the bracket endpoints is
        // a rest
        //
        if (!cr1->isChord() && cr2->isChord()) {
            if (p2.y() < p1.y()) {
                p1.setY(p2.y());
            } else {
                p2.setY(p1.y());
            }
        } else if (cr1->isChord() && !cr2->isChord()) {
            if (p1.y() < p2.y()) {
                p2.setY(p1.y());
            } else {
                p1.setY(p2.y());
            }
        }

        // outOfStaff
        if (outOfStaff) {
            double min = cr1->measure()->staffabbox(cr1->staffIdx() + move).y();
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
        double d = (p2.y() - p1.y()) / (p2.x() - p1.x());
        if (d < -maxSlope) {
            // move p1 y up
            p1.ry() = p2.y() + maxSlope * (p2.x() - p1.x());
        } else if (d > maxSlope) {
            // move p2 y up
            p2.ry() = p1.ry() + maxSlope * (p2.x() - p1.x());
        }

        // check for collisions
        size_t n = _elements.size();
        if (n >= 3) {
            d = (p2.y() - p1.y()) / (p2.x() - p1.x());
            for (size_t i = 1; i < (n - 1); ++i) {
                EngravingItem* e = _elements[i];
                if (e->isChord()) {
                    const Chord* chord = toChord(e);
                    const Stem* stem = chord->stem();
                    if (stem) {
                        RectF r(chord->up() ? stem->abbox() : chord->upNote()->abbox());
                        double y3 = r.top();
                        double x3 = r.x() + r.width() * .5;
                        double y0 = p1.y() + (x3 - p1.x()) * d;
                        double c  = y0 - y3;
                        if (c > 0) {
                            p1.ry() -= c;
                            p2.ry() -= c;
                        }
                    }
                }
            }
        }
    } else {
        if (cr1->isChord()) {
            const Chord* chord1 = toChord(cr1);
            Stem* stem = chord1->stem();
            if (stem) {
                xx1 = stem->abbox().x();
            }
            if (!chord1->up() && stem) {
                p1.ry() = stem->abbox().bottom();
                l2l = vStemDistance;
                p1.rx() = stem->abbox().left() - stemLeft;
            } else {
                p1.ry() = chord1->downNote()->abbox().bottom();
                p1.rx() = leftNoteEdge - noteLeft;
            }
        }

        if (cr2->isChord()) {
            const Chord* chord2 = toChord(cr2);
            Stem* stem = chord2->stem();
            if (stem && !chord2->up()) {
                p2.ry() = stem->abbox().bottom();
                l2r = vStemDistance;
                p2.rx() = stem->abbox().right() + stemRight;
            } else {
                p2.ry() = chord2->downNote()->abbox().bottom();
                p2.rx() = rightNoteEdge + noteRight;
            }
        }
        //
        // special case: one of the bracket endpoints is
        // a rest
        //
        if (!cr1->isChord() && cr2->isChord()) {
            if (p2.y() > p1.y()) {
                p1.setY(p2.y());
            } else {
                p2.setY(p1.y());
            }
        } else if (cr1->isChord() && !cr2->isChord()) {
            if (p1.y() > p2.y()) {
                p2.setY(p1.y());
            } else {
                p1.setY(p2.y());
            }
        }
        // outOfStaff
        if (outOfStaff) {
            double max = cr1->measure()->staffabbox(cr1->staffIdx() + move).bottom();
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
        double d = (p2.y() - p1.y()) / (p2.x() - p1.x());
        if (d < -maxSlope) {
            // move p1 y up
            p2.ry() = p1.y() - maxSlope * (p2.x() - p1.x());
        } else if (d > maxSlope) {
            // move p2 y up
            p1.ry() = p2.ry() - maxSlope * (p2.x() - p1.x());
        }

        // check for collisions
        size_t n = _elements.size();
        if (n >= 3) {
            d  = (p2.y() - p1.y()) / (p2.x() - p1.x());
            for (size_t i = 1; i < (n - 1); ++i) {
                EngravingItem* e = _elements[i];
                if (e->isChord()) {
                    const Chord* chord = toChord(e);
                    const Stem* stem = chord->stem();
                    if (stem) {
                        RectF r(chord->up() ? chord->downNote()->abbox() : stem->abbox());
                        double y3 = r.bottom();
                        double x3 = r.x() + r.width() * .5;
                        double y0 = p1.y() + (x3 - p1.x()) * d;
                        double c  = y0 - y3;
                        if (c < 0) {
                            p1.ry() -= c;
                            p2.ry() -= c;
                        }
                    }
                }
            }
        }
    }

    if (!cr1->isChord()) {
        p1.rx() = cr1->abbox().left() - noteLeft;
    }
    if (!cr2->isChord()) {
        p2.rx() = cr2->abbox().right() + noteRight;
    }

    setPos(0.0, 0.0);
    PointF mp(parentItem()->pagePos());
    if (explicitParent()->isMeasure()) {
        System* s = toMeasure(explicitParent())->system();
        if (s) {
            mp.ry() += s->staff(staffIdx())->y();
        }
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
    double x3 = 0.0;
    double numberWidth = 0.0;
    if (_number) {
        _number->layout();
        numberWidth = _number->bbox().width();

        double y3 = p1.y() + (p2.y() - p1.y()) * .5 - l1 * (_isUp ? 1.0 : -1.0);
        // for beamed tuplets, center number on beam - if they don't have a bracket
        if (cr1->beam() && cr2->beam() && cr1->beam() == cr2->beam() && !_hasBracket) {
            const ChordRest* crr = toChordRest(cr1);
            if (_isUp == crr->up()) {
                double deltax = cr2->pagePos().x() - cr1->pagePos().x();
                x3 = xx1 + deltax * .5;
            } else {
                double deltax = p2.x() - p1.x();
                x3 = p1.x() + deltax * .5;
            }
        } else {
            // otherwise center on the bracket (TODO: make centering rules customizable?)
            double deltax = p2.x() - p1.x();
            x3 = p1.x() + deltax * .5;
        }

        _number->setPos(PointF(x3, y3) - ipos());
    }

    if (_hasBracket) {
        double slope = (p2.y() - p1.y()) / (p2.x() - p1.x());

        if (_isUp) {
            if (_number) {
                //set width of bracket hole
                double x     = x3 - numberWidth * .5 - _spatium * .5;
                p1.rx() = std::min(p1.rx(), x - 0.5 * l1); // ensure enough space for the number
                double y     = p1.y() + (x - p1.x()) * slope;
                bracketL[0] = PointF(p1.x(), p1.y());
                bracketL[1] = PointF(p1.x(), p1.y() - l1);
                bracketL[2] = PointF(x,   y - l1);

                //set width of bracket hole
                x           = x3 + numberWidth * .5 + _spatium * .5;
                p2.rx() = std::max(p2.rx(), x + 0.5 * l1); // ensure enough space for the number
                y           = p1.y() + (x - p1.x()) * slope;
                bracketR[0] = PointF(x,   y - l1);
                bracketR[1] = PointF(p2.x(), p2.y() - l1);
                bracketR[2] = PointF(p2.x(), p2.y());
            } else {
                bracketL[0] = PointF(p1.x(), p1.y());
                bracketL[1] = PointF(p1.x(), p1.y() - l1);
                bracketL[2] = PointF(p2.x(), p2.y() - l1);
                bracketL[3] = PointF(p2.x(), p2.y());
            }
        } else {
            if (_number) {
                //set width of bracket hole
                double x     = x3 - numberWidth * .5 - _spatium * .5;
                p1.rx() = std::min(p1.rx(), x - 0.5 * l1); // ensure enough space for the number
                double y     = p1.y() + (x - p1.x()) * slope;
                bracketL[0] = PointF(p1.x(), p1.y());
                bracketL[1] = PointF(p1.x(), p1.y() + l1);
                bracketL[2] = PointF(x,   y + l1);

                //set width of bracket hole
                x           = x3 + numberWidth * .5 + _spatium * .5;
                p2.rx() = std::max(p2.rx(), x + 0.5 * l1);
                y           = p1.y() + (x - p1.x()) * slope;
                bracketR[0] = PointF(x,   y + l1);
                bracketR[1] = PointF(p2.x(), p2.y() + l1);
                bracketR[2] = PointF(p2.x(), p2.y());
            } else {
                bracketL[0] = PointF(p1.x(), p1.y());
                bracketL[1] = PointF(p1.x(), p1.y() + l1);
                bracketL[2] = PointF(p2.x(), p2.y() + l1);
                bracketL[3] = PointF(p2.x(), p2.y());
            }
        }
    }

    // collect bounding box
    RectF r;
    if (_number) {
        r |= _number->bbox().translated(_number->pos());
        if (_hasBracket) {
            RectF b;
            b.setCoords(bracketL[1].x(), bracketL[1].y(), bracketR[2].x(), bracketR[2].y());
            r |= b;
        }
    } else if (_hasBracket) {
        RectF b;
        b.setCoords(bracketL[1].x(), bracketL[1].y(), bracketL[3].x(), bracketL[3].y());
        r |= b;
    }
    setbbox(r);

    if (outOfStaff && !cross()) {
        autoplaceMeasureElement(_isUp, /* add to skyline */ true);
    }
}

//---------------------------------------------------------
//   calcHasBracket
//---------------------------------------------------------

bool Tuplet::calcHasBracket(const DurationElement* cr1, const DurationElement* cr2) const
{
    if (_bracketType != TupletBracketType::AUTO_BRACKET) {
        return _bracketType != TupletBracketType::SHOW_NO_BRACKET;
    }
    if (cr1 == cr2) { // Degenerate tuplet
        return false;
    }
    if (!cr1->isChord() || !cr2->isChord()) {
        return true;
    }
    const Chord* c1 = toChord(cr1);
    const Chord* c2 = toChord(cr2);

    Beam const* beamStart = c1->beam();
    Beam const* beamEnd = c2->beam();
    if (!beamStart || !beamEnd || beamStart != beamEnd) {
        return true;
    }
    bool tupletStartsBeam = beamStart->elements().front() == c1;
    bool tupletEndsBeam = beamEnd->elements().back() == c2;
    if (tupletStartsBeam && tupletEndsBeam) {
        return false;
    }

    int beamCount = -1;
    for (DurationElement* e : _elements) {
        if (e->isTuplet() || e->isRest()) {
            return true;
        } else if (e->isChordRest()) {
            ChordRest* cr = toChordRest(e);
            if (cr->beam() == 0) {
                return true;
            }
            if (beamCount == -1) {
                beamCount = cr->beams();
            } else if (beamCount != cr->beams()) {
                return true;
            }
        }
    }
    if (beamCount < 1) {
        return true;
    }

    bool startChordBreaks32 = false;
    bool startChordBreaks64 = false;
    Chord* prevStartChord = c1->prev();
    if (prevStartChord) {
        beamStart->calcBeamBreaks(c1, prevStartChord, beamCount - 1, startChordBreaks32, startChordBreaks64);
    }
    bool startChordDefinesTuplet = startChordBreaks32 || startChordBreaks64 || tupletStartsBeam;
    if (prevStartChord) {
        startChordDefinesTuplet = startChordDefinesTuplet || prevStartChord->beams() < beamCount;
    }

    bool endChordBreaks32 = false;
    bool endChordBreaks64 = false;
    Chord* nextEndChord = c2->next();
    if (nextEndChord) {
        beamEnd->calcBeamBreaks(nextEndChord, c2, beamCount - 1, endChordBreaks32, endChordBreaks64);
    }
    bool endChordDefinesTuplet = endChordBreaks32 || endChordBreaks64 || tupletEndsBeam;
    if (nextEndChord) {
        endChordDefinesTuplet = endChordDefinesTuplet || nextEndChord->beams() < beamCount;
    }

    if (startChordDefinesTuplet && endChordDefinesTuplet) {
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Tuplet::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    // if in a TAB without stems, tuplets are not shown
    const StaffType* stt = staffType();
    if (stt && stt->isTabStaff() && stt->stemless()) {
        return;
    }

    Color color(curColor());
    if (_number) {
        painter->setPen(color);
        PointF pos(_number->pos());
        painter->translate(pos);
        _number->draw(painter);
        painter->translate(-pos);
    }
    if (_hasBracket) {
        painter->setPen(Pen(color, _bracketWidth.val() * mag()));
        if (!_number) {
            painter->drawPolyline(bracketL, 4);
        } else {
            painter->drawPolyline(bracketL, 3);
            painter->drawPolyline(bracketR, 3);
        }
    }
}

//---------------------------------------------------------
//   Rect
//    helper class
//---------------------------------------------------------

class TupletRect : public RectF
{
    OBJECT_ALLOCATOR(engraving, TupletRect)
public:
    TupletRect(const PointF& p1, const PointF& p2, double w)
    {
        double w2 = w * .5;
        setCoords(std::min(p1.x(), p2.x()) - w2, std::min(p1.y(), p2.y()) - w2,  std::max(p1.x(), p2.x()) + w2, std::max(p1.y(),
                                                                                                                         p2.y()) + w2);
    }
};

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape Tuplet::shape() const
{
    Shape s;
    if (_hasBracket) {
        double w = _bracketWidth.val() * mag();
        s.add(TupletRect(bracketL[0], bracketL[1], w));
        s.add(TupletRect(bracketL[1], bracketL[2], w));
        if (_number) {
            s.add(TupletRect(bracketR[0], bracketR[1], w));
            s.add(TupletRect(bracketR[1], bracketR[2], w));
        } else {
            s.add(TupletRect(bracketL[2], bracketL[3], w));
        }
    }
    if (_number) {
        s.add(_number->bbox().translated(_number->pos()));
    }
    return s;
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Tuplet::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    for (EngravingObject* child : scanChildren()) {
        if (child == _number && !all) {
            continue; // don't scan number unless all is true
        }
        child->scanElements(data, func, all);
    }
    if (all || visible() || score()->showInvisible()) {
        func(data, this);
    }
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Tuplet::add(EngravingItem* e)
{
#ifndef NDEBUG
    for (DurationElement* el : _elements) {
        if (el == e) {
            LOGD("%p: %p %s already there", this, e, e->typeName());
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
        if (tick != Fraction(-1, 1)) {
            for (unsigned int i = 0; i < _elements.size(); ++i) {
                if (_elements[i]->rtick() > tick) {
                    _elements.insert(_elements.begin() + i, de);
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            _elements.push_back(de);
        }
        de->setTuplet(this);
    }
    break;

    default:
        LOGD("Tuplet::add() unknown element");
        return;
    }

    e->added();
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Tuplet::remove(EngravingItem* e)
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
            LOGD("Tuplet::remove: cannot find element <%s>", e->typeName());
            LOGD("  elements %zu", _elements.size());
        } else {
            _elements.erase(i);
            e->removed();
        }
    }
    break;
    default:
        LOGD("Tuplet::remove: unknown element");
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
    ElementEditDataPtr eed = ed.getData(this);

    eed->pushProperty(Pid::P1);
    eed->pushProperty(Pid::P2);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Tuplet::editDrag(EditData& ed)
{
    if (ed.curGrip == Grip::START) {
        _p1 += ed.delta;
    } else {
        _p2 += ed.delta;
    }
    setGenerated(false);
    //layout();
    //score()->setUpdateAll();
    triggerLayout();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Tuplet::gripsPositions(const EditData&) const
{
    const PointF pp(pagePos());
    return { pp + p1, pp + p2 };
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Tuplet::reset()
{
    undoChangeProperty(Pid::P1, PointF());
    undoChangeProperty(Pid::P2, PointF());
    EngravingItem::reset();
}

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Tuplet::dump() const
{
    EngravingItem::dump();
    LOGD() << "ratio: " << _ratio.toString();
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Tuplet::setTrack(track_idx_t val)
{
    if (tuplet()) {
        tuplet()->setTrack(val);
    }
    if (_number) {
        _number->setTrack(val);
    }
    EngravingItem::setTrack(val);
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
        } else if (de->isChordRest()) {
            if (toChordRest(de)->staffMove()) {
                return true;
            }
        } else if (de->isTuplet()) {
            if (toTuplet(de)->cross()) {
                return true;
            }
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
    for (DurationElement* el : _elements) {
        f += el->ticks();
    }
    return f;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Tuplet::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::DIRECTION:
        return PropertyValue::fromValue<DirectionV>(_direction);
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
        return _number ? _number->getProperty(propertyId) : PropertyValue();
    default:
        break;
    }
    return DurationElement::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Tuplet::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::DIRECTION:
        setDirection(v.value<DirectionV>());
        break;
    case Pid::NUMBER_TYPE:
        setNumberType(TupletNumberType(v.toInt()));
        break;
    case Pid::BRACKET_TYPE:
        setBracketType(TupletBracketType(v.toInt()));
        break;
    case Pid::LINE_WIDTH:
        setBracketWidth(v.value<Millimetre>());
        break;
    case Pid::NORMAL_NOTES:
        _ratio.setDenominator(v.toInt());
        break;
    case Pid::ACTUAL_NOTES:
        _ratio.setNumerator(v.toInt());
        break;
    case Pid::P1:
        _p1 = v.value<PointF>();
        break;
    case Pid::P2:
        _p2 = v.value<PointF>();
        break;
    case Pid::FONT_SIZE:
    case Pid::FONT_FACE:
    case Pid::FONT_STYLE:
    case Pid::ALIGN:
    case Pid::SIZE_SPATIUM_DEPENDENT:
        if (_number) {
            _number->setProperty(propertyId, v);
        }
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

PropertyValue Tuplet::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::TUPLET;
    case Pid::SYSTEM_FLAG:
        return false;
    case Pid::TEXT:
        return String(u"");
    case Pid::NORMAL_NOTES:
    case Pid::ACTUAL_NOTES:
        return 0;
    case Pid::P1:
    case Pid::P2:
        return PointF();
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
        break;
    }

    // TODO: what does this do? Why TextStyleType::DEFAULT?
    for (const auto& spp : *textStyle(TextStyleType::DEFAULT)) {
        if (spp.pid == id) {
            return styleValue(id, spp.sid);
        }
    }

    return DurationElement::propertyDefault(id);
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
    if (ratio().numerator() == ratio().reduced().numerator()) { // return if the ratio is an irreducible fraction
        return;
    }
    Fraction baseLenDuration = (Fraction(ratio().denominator(), 1) * baseLen().fraction()).reduced();

    // Due to a bug present in 2.1 (and before), a tuplet with non-reduced ratio could be
    // in a corrupted state (mismatch between duration and base length).
    // A tentative will now be made to retrieve the correct duration by summing up all the
    // durations of the elements constituting the tuplet. This does not work for
    // not-completely filled tuplets, such as tuplets in voices > 0 with
    // gaps (for example, a tuplet in second voice with a deleted chordrest element)

    Fraction testDuration(0, 1);
    for (DurationElement* de : elements()) {
        if (de == 0) {
            continue;
        }
        Fraction elementDuration(0, 1);
        if (de->isTuplet()) {
            Tuplet* t = toTuplet(de);
            t->sanitizeTuplet();
            elementDuration = t->ticks();
        } else {
            elementDuration = de->ticks();
        }
        testDuration += elementDuration;
    }
    testDuration = testDuration / ratio();
    testDuration.reduce();
    if (elements().back()->tick() + elements().back()->actualTicks() - elements().front()->tick() > testDuration) {
        return;         // this tuplet has missing elements; do not sanitize
    }
    if (!(testDuration == baseLenDuration && baseLenDuration == ticks())) {
        Fraction f = testDuration * Fraction(1, ratio().denominator());
        f.reduce();
        Fraction fbl(1, f.denominator());
        if (TDuration::isValid(fbl)) {
            setTicks(testDuration);
            setBaseLen(fbl);
            LOGD("Tuplet %p sanitized duration %d/%d   baseLen %d/%d", this,
                 testDuration.numerator(), testDuration.denominator(),
                 1, fbl.denominator());
        } else {
            LOGD("Impossible to sanitize the tuplet");
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
        LOGD("Tuplet::addMissingElement(): invalid duration: %d/%d", f.numerator(), f.denominator());
        return Fraction::fromTicks(0);
    }
    f = d.fraction();
    Segment* segment = measure()->getSegment(SegmentType::ChordRest, startTick);
    Rest* rest = Factory::createRest(segment);
    rest->setDurationType(d);
    rest->setTicks(f);
    rest->setTrack(track());
    rest->setVisible(false);

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
    if (tuplet()) {
        return;         // do not correct nested tuplets
    }
    if (voice() == 0) {
        return;         // nothing to do for tuplets in voice 1
    }
    Fraction missingElementsDuration = ticks() * ratio() - elementsDuration();
    if (missingElementsDuration.isZero()) {
        return;
    }
    // first, fill in any holes in the middle of the tuplet
    Fraction expectedTick = elements().front()->tick();
    for (DurationElement* de : elements()) {
        if (de->tick() != expectedTick) {
            missingElementsDuration -= addMissingElement(expectedTick, de->tick());
            if (missingElementsDuration.isZero()) {
                return;
            }
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
        if (prevChordRest && prevChordRest->measure() == measure()) {
            firstAvailableTick = prevChordRest->tick() + prevChordRest->actualTicks();
        }
        if (firstAvailableTick != elements().front()->tick()) {
            Fraction f = missingElementsDuration / ratio();
            Fraction ticksRequired = f;
            Fraction endTick = elements().front()->tick();
            Fraction startTick = std::max(firstAvailableTick, endTick - ticksRequired);
            if (expectedTick > startTick) {
                startTick = expectedTick;
            }
            missingElementsDuration -= addMissingElement(startTick, endTick);
            if (missingElementsDuration.isZero()) {
                return;
            }
        }
    }
    // now fill a hole at the end of the tuplet
    Fraction startTick = elements().back()->tick() + elements().back()->actualTicks();
    Fraction endTick = elements().front()->tick() + ticks();
    // just to be safe, find the next ChordRest in the track, and adjust endTick if necessary
    Segment* segment = measure()->findSegment(SegmentType::ChordRest, elements().back()->tick());
    ChordRest* nextChordRest = segment && segment->next() ? segment->next()->nextChordRest(track(), false) : nullptr;
    if (nextChordRest && nextChordRest->tick() < endTick) {
        endTick = nextChordRest->tick();
    }
    missingElementsDuration -= addMissingElement(startTick, endTick);
    if (!missingElementsDuration.isZero()) {
        LOGD("Tuplet::addMissingElements(): still missing duration of %d/%d",
             missingElementsDuration.numerator(), missingElementsDuration.denominator());
    }
}

int Tuplet::computeTupletDenominator(int numerator, Fraction totalDuration)
{
    if (numerator <= 0) {
        return 0;
    }
    Fraction noteDuration = totalDuration / numerator;
    Fraction baseLen(1, 1);
    Fraction ratio = (totalDuration / baseLen).reduced();
    while (ratio.denominator() != 1 || baseLen / 2 >= noteDuration) {
        baseLen /= 2;
        ratio = (totalDuration / baseLen).reduced();
    }
    return ratio.numerator();
}
} // namespace mu::engraving
