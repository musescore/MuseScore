/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "tupletlayout.h"

#include "libmscore/chordrest.h"
#include "libmscore/tuplet.h"
#include "libmscore/text.h"
#include "libmscore/factory.h"
#include "libmscore/stafftype.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/stem.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"

#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::stable;

void TupletLayout::layout(Tuplet* item, LayoutContext& ctx)
{
    if (item->elements().empty()) {
        LOGD("Tuplet::layout(): tuplet is empty");
        return;
    }
    // is in a TAB without stems, skip any format: tuplets are not shown
    const StaffType* stt = item->staffType();
    if (stt && stt->isTabStaff() && stt->stemless()) {
        return;
    }

    //
    // create tuplet number if necessary
    //
    double _spatium = item->spatium();
    if (item->numberType() != TupletNumberType::NO_TEXT) {
        if (item->number() == nullptr) {
            Text* number = Factory::createText(item, TextStyleType::TUPLET);
            number->setComposition(true);
            number->setTrack(item->track());
            number->setParent(item);
            number->setVisible(item->visible());
            item->setNumber(number);
            item->resetNumberProperty();
        }
        // tuplet properties are propagated to number automatically by setProperty()
        // but we need to make sure flags are as well
        item->number()->setPropertyFlags(Pid::FONT_FACE, item->propertyFlags(Pid::FONT_FACE));
        item->number()->setPropertyFlags(Pid::FONT_SIZE, item->propertyFlags(Pid::FONT_SIZE));
        item->number()->setPropertyFlags(Pid::FONT_STYLE, item->propertyFlags(Pid::FONT_STYLE));
        item->number()->setPropertyFlags(Pid::ALIGN, item->propertyFlags(Pid::ALIGN));
        if (item->numberType() == TupletNumberType::SHOW_NUMBER) {
            item->number()->setXmlText(String(u"%1").arg(item->ratio().numerator()));
        } else {
            item->number()->setXmlText(String(u"%1:%2").arg(item->ratio().numerator(), item->ratio().denominator()));
        }

        item->setIsSmall(true);
        for (const DurationElement* e : item->elements()) {
            if ((e->isChordRest() && !toChordRest(e)->isSmall()) || (e->isTuplet() && !toTuplet(e)->isSmall())) {
                item->setIsSmall(false);
                break;
            }
        }
        item->number()->setMag(item->isSmall() ? ctx.conf().styleD(Sid::smallNoteMag) : 1.0);
    } else {
        if (item->number()) {
            if (item->number()->selected()) {
                ctx.deselect(item->number());
            }
            delete item->number();
            item->setNumber(nullptr);
        }
    }
    //
    // find out main direction
    //
    if (item->direction() == DirectionV::AUTO) {
        int up = 0;
        for (const DurationElement* e : item->elements()) {
            if (e->isChord()) {
                const Chord* c = toChord(e);
                if (c->stemDirection() != DirectionV::AUTO) {
                    up += c->stemDirection() == DirectionV::UP ? 1000 : -1000;
                } else {
                    up += c->up() ? 1 : -1;
                }
            }
        }
        if (up == 0) {
            // this is a tuplet full of rests, default to up but also take voices into consideration
            Measure* m = item->measure();
            if (m && m->hasVoices(item->staffIdx())) {
                up = item->voice() % 2 == 0 ? 1 : -1;
            } else {
                up = 1;         // default up
            }
        }
        item->setIsUp(up > 0);
    } else {
        item->setIsUp(item->direction() == DirectionV::UP);
    }

    //
    // find first and last chord of tuplet
    // (tuplets can be nested)
    //
    const DurationElement* cr1 = item->elements().front();
    while (cr1->isTuplet()) {
        const Tuplet* t = toTuplet(cr1);
        if (t->elements().empty()) {
            break;
        }
        cr1 = t->elements().front();
    }
    const DurationElement* cr2 = item->elements().back();
    while (cr2->isTuplet()) {
        const Tuplet* t = toTuplet(cr2);
        if (t->elements().empty()) {
            break;
        }
        cr2 = t->elements().back();
    }

    item->setHasBracket(item->calcHasBracket(cr1, cr2));
    item->setMag((cr1->mag() + cr2->mag()) / 2);

    //
    //    calculate bracket start and end point p1 p2
    //
    const MStyle& style = ctx.conf().style();
    double maxSlope      = style.styleD(Sid::tupletMaxSlope);
    bool outOfStaff      = style.styleB(Sid::tupletOufOfStaff);
    double vHeadDistance = style.styleMM(Sid::tupletVHeadDistance) * item->mag();
    double vStemDistance = style.styleMM(Sid::tupletVStemDistance) * item->mag();
    double stemLeft      = (style.styleMM(Sid::tupletStemLeftDistance) - style.styleMM(Sid::tupletBracketWidth) / 2) * cr1->mag();
    double stemRight     = (style.styleMM(Sid::tupletStemRightDistance) - style.styleMM(Sid::tupletBracketWidth) / 2) * cr2->mag();
    double noteLeft      = (style.styleMM(Sid::tupletNoteLeftDistance) - style.styleMM(Sid::tupletBracketWidth) / 2) * cr1->mag();
    double noteRight     = (style.styleMM(Sid::tupletNoteRightDistance) - style.styleMM(Sid::tupletBracketWidth) / 2) * cr2->mag();

    int move = 0;
    item->setStaffIdx(cr1->vStaffIdx());
    if (outOfStaff && cr1->isChordRest() && cr2->isChordRest()) {
        // account for staff move when adjusting bracket to avoid staff
        // but don't attempt adjustment unless both endpoints are in same staff
        if (toChordRest(cr1)->staffMove() == toChordRest(cr2)->staffMove()) {
            move = toChordRest(cr1)->staffMove();
            if (move == 1) {
                item->setStaffIdx(cr1->vStaffIdx());
            }
        } else {
            outOfStaff = false;
        }
    }

    double l1  = style.styleMM(Sid::tupletBracketHookHeight) * item->mag();
    double l2l = vHeadDistance;      // left bracket vertical distance
    double l2r = vHeadDistance;      // right bracket vertical distance right

    if (item->isUp()) {
        vHeadDistance = -vHeadDistance;
    }

    item->setP1(cr1->pagePos());
    item->setP2(cr2->pagePos());

    item->p1().rx() -= noteLeft;
    item->p2().rx() += ctx.conf().noteHeadWidth() + noteRight;
    item->p1().ry() += vHeadDistance;          // TODO: Direction ?
    item->p2().ry() += vHeadDistance;

    double xx1 = item->p1().x();   // use to center the number on the beam

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

    if (item->isUp()) {
        if (cr1->isChord()) {
            const Chord* chord1 = toChord(cr1);
            Stem* stem = chord1->stem();
            if (stem) {
                xx1 = stem->abbox().x();
            }
            if (chord1->up() && stem) {
                item->p1().ry() = stem->abbox().y();
                l2l = vStemDistance;
                item->p1().rx() = stem->abbox().left() - stemLeft;
            } else {
                item->p1().ry() = chord1->upNote()->abbox().top();
                item->p1().rx() = leftNoteEdge - noteLeft;
            }
        }

        if (cr2->isChord()) {
            const Chord* chord2 = toChord(cr2);
            Stem* stem = chord2->stem();
            if (stem && chord2->up()) {
                item->p2().ry() = stem->abbox().top();
                l2r = vStemDistance;
                item->p2().rx() = stem->abbox().right() + stemRight;
            } else {
                item->p2().ry() = chord2->upNote()->abbox().top();
                item->p2().rx() = rightNoteEdge + noteRight;
            }
        }
        //
        // special case: one of the bracket endpoints is
        // a rest
        //
        if (!cr1->isChord() && cr2->isChord()) {
            if (item->p2().y() < item->p1().y()) {
                item->p1().setY(item->p2().y());
            } else {
                item->p2().setY(item->p1().y());
            }
        } else if (cr1->isChord() && !cr2->isChord()) {
            if (item->p1().y() < item->p2().y()) {
                item->p2().setY(item->p1().y());
            } else {
                item->p1().setY(item->p2().y());
            }
        }

        // outOfStaff
        if (outOfStaff) {
            double min = cr1->measure()->staffabbox(cr1->staffIdx() + move).y();
            if (min < item->p1().y()) {
                item->p1().ry() = min;
                l2l = vStemDistance;
            }
            min = cr2->measure()->staffabbox(cr2->staffIdx() + move).y();
            if (min < item->p2().y()) {
                item->p2().ry() = min;
                l2r = vStemDistance;
            }
        }

        // check that slope is no more than max
        double d = (item->p2().y() - item->p1().y()) / (item->p2().x() - item->p1().x());
        if (d < -maxSlope) {
            // move p1 y up
            item->p1().ry() = item->p2().y() + maxSlope * (item->p2().x() - item->p1().x());
        } else if (d > maxSlope) {
            // move p2 y up
            item->p2().ry() = item->p1().ry() + maxSlope * (item->p2().x() - item->p1().x());
        }

        // check for collisions
        size_t n = item->elements().size();
        if (n >= 3) {
            d = (item->p2().y() - item->p1().y()) / (item->p2().x() - item->p1().x());
            for (size_t i = 1; i < (n - 1); ++i) {
                const EngravingItem* e = item->elements().at(i);
                if (e->isChord()) {
                    const Chord* chord = toChord(e);
                    const Stem* stem = chord->stem();
                    if (stem) {
                        RectF r(chord->up() ? stem->abbox() : chord->upNote()->abbox());
                        double y3 = r.top();
                        double x3 = r.x() + r.width() * .5;
                        double y0 = item->p1().y() + (x3 - item->p1().x()) * d;
                        double c  = y0 - y3;
                        if (c > 0) {
                            item->p1().ry() -= c;
                            item->p2().ry() -= c;
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
                item->p1().ry() = stem->abbox().bottom();
                l2l = vStemDistance;
                item->p1().rx() = stem->abbox().left() - stemLeft;
            } else {
                item->p1().ry() = chord1->downNote()->abbox().bottom();
                item->p1().rx() = leftNoteEdge - noteLeft;
            }
        }

        if (cr2->isChord()) {
            const Chord* chord2 = toChord(cr2);
            Stem* stem = chord2->stem();
            if (stem && !chord2->up()) {
                item->p2().ry() = stem->abbox().bottom();
                l2r = vStemDistance;
                item->p2().rx() = stem->abbox().right() + stemRight;
            } else {
                item->p2().ry() = chord2->downNote()->abbox().bottom();
                item->p2().rx() = rightNoteEdge + noteRight;
            }
        }
        //
        // special case: one of the bracket endpoints is
        // a rest
        //
        if (!cr1->isChord() && cr2->isChord()) {
            if (item->p2().y() > item->p1().y()) {
                item->p1().setY(item->p2().y());
            } else {
                item->p2().setY(item->p1().y());
            }
        } else if (cr1->isChord() && !cr2->isChord()) {
            if (item->p1().y() > item->p2().y()) {
                item->p2().setY(item->p1().y());
            } else {
                item->p1().setY(item->p2().y());
            }
        }
        // outOfStaff
        if (outOfStaff) {
            double max = cr1->measure()->staffabbox(cr1->staffIdx() + move).bottom();
            if (max > item->p1().y()) {
                item->p1().ry() = max;
                l2l = vStemDistance;
            }
            max = cr2->measure()->staffabbox(cr2->staffIdx() + move).bottom();
            if (max > item->p2().y()) {
                item->p2().ry() = max;
                l2r = vStemDistance;
            }
        }
        // check that slope is no more than max
        double d = (item->p2().y() - item->p1().y()) / (item->p2().x() - item->p1().x());
        if (d < -maxSlope) {
            // move p1 y up
            item->p2().ry() = item->p1().y() - maxSlope * (item->p2().x() - item->p1().x());
        } else if (d > maxSlope) {
            // move p2 y up
            item->p1().ry() = item->p2().ry() - maxSlope * (item->p2().x() - item->p1().x());
        }

        // check for collisions
        size_t n = item->elements().size();
        if (n >= 3) {
            d  = (item->p2().y() - item->p1().y()) / (item->p2().x() - item->p1().x());
            for (size_t i = 1; i < (n - 1); ++i) {
                EngravingItem* e = item->elements()[i];
                if (e->isChord()) {
                    const Chord* chord = toChord(e);
                    const Stem* stem = chord->stem();
                    if (stem) {
                        RectF r(chord->up() ? chord->downNote()->abbox() : stem->abbox());
                        double y3 = r.bottom();
                        double x3 = r.x() + r.width() * .5;
                        double y0 = item->p1().y() + (x3 - item->p1().x()) * d;
                        double c  = y0 - y3;
                        if (c < 0) {
                            item->p1().ry() -= c;
                            item->p2().ry() -= c;
                        }
                    }
                }
            }
        }
    }

    if (!cr1->isChord()) {
        item->p1().rx() = cr1->abbox().left() - noteLeft;
    }
    if (!cr2->isChord()) {
        item->p2().rx() = cr2->abbox().right() + noteRight;
    }

    item->setPos(0.0, 0.0);
    PointF mp(item->parentItem()->pagePos());
    if (item->explicitParent()->isMeasure()) {
        System* s = toMeasure(item->explicitParent())->system();
        if (s) {
            mp.ry() += s->staff(item->staffIdx())->y();
        }
    }
    item->p1() -= mp;
    item->p2() -= mp;

    item->p1() += item->userP1();
    item->p2() += item->userP2();
    xx1 -= mp.x();

    item->p1().ry() -= l2l * (item->isUp() ? 1.0 : -1.0);
    item->p2().ry() -= l2r * (item->isUp() ? 1.0 : -1.0);

    // l2l l2r, mp, _p1, _p2 const

    // center number
    double x3 = 0.0;
    double numberWidth = 0.0;
    if (item->number()) {
        TLayout::layout(item->number(), ctx);
        numberWidth = item->number()->bbox().width();

        double y3 = item->p1().y() + (item->p2().y() - item->p1().y()) * .5 - l1 * (item->isUp() ? 1.0 : -1.0);
        // for beamed tuplets, center number on beam - if they don't have a bracket
        if (cr1->beam() && cr2->beam() && cr1->beam() == cr2->beam() && !item->hasBracket()) {
            const ChordRest* crr = toChordRest(cr1);
            if (item->isUp() == crr->up()) {
                double deltax = cr2->pagePos().x() - cr1->pagePos().x();
                x3 = xx1 + deltax * .5;
            } else {
                double deltax = item->p2().x() - item->p1().x();
                x3 = item->p1().x() + deltax * .5;
            }
        } else {
            // otherwise center on the bracket (TODO: make centering rules customizable?)
            double deltax = item->p2().x() - item->p1().x();
            x3 = item->p1().x() + deltax * .5;
        }

        item->number()->setPos(PointF(x3, y3) - item->ipos());
    }

    if (item->hasBracket()) {
        double slope = (item->p2().y() - item->p1().y()) / (item->p2().x() - item->p1().x());

        if (item->isUp()) {
            if (item->number()) {
                //set width of bracket hole
                double x     = x3 - numberWidth * .5 - _spatium * .5;
                item->p1().rx() = std::min(item->p1().rx(), x - 0.5 * l1); // ensure enough space for the number
                double y     = item->p1().y() + (x - item->p1().x()) * slope;
                item->bracketL[0] = PointF(item->p1().x(), item->p1().y());
                item->bracketL[1] = PointF(item->p1().x(), item->p1().y() - l1);
                item->bracketL[2] = PointF(x,   y - l1);

                //set width of bracket hole
                x           = x3 + numberWidth * .5 + _spatium * .5;
                item->p2().rx() = std::max(item->p2().rx(), x + 0.5 * l1); // ensure enough space for the number
                y           = item->p1().y() + (x - item->p1().x()) * slope;
                item->bracketR[0] = PointF(x,   y - l1);
                item->bracketR[1] = PointF(item->p2().x(), item->p2().y() - l1);
                item->bracketR[2] = PointF(item->p2().x(), item->p2().y());
            } else {
                item->bracketL[0] = PointF(item->p1().x(), item->p1().y());
                item->bracketL[1] = PointF(item->p1().x(), item->p1().y() - l1);
                item->bracketL[2] = PointF(item->p2().x(), item->p2().y() - l1);
                item->bracketL[3] = PointF(item->p2().x(), item->p2().y());
            }
        } else {
            if (item->number()) {
                //set width of bracket hole
                double x     = x3 - numberWidth * .5 - _spatium * .5;
                item->p1().rx() = std::min(item->p1().rx(), x - 0.5 * l1); // ensure enough space for the number
                double y     = item->p1().y() + (x - item->p1().x()) * slope;
                item->bracketL[0] = PointF(item->p1().x(), item->p1().y());
                item->bracketL[1] = PointF(item->p1().x(), item->p1().y() + l1);
                item->bracketL[2] = PointF(x,   y + l1);

                //set width of bracket hole
                x           = x3 + numberWidth * .5 + _spatium * .5;
                item->p2().rx() = std::max(item->p2().rx(), x + 0.5 * l1);
                y           = item->p1().y() + (x - item->p1().x()) * slope;
                item->bracketR[0] = PointF(x,   y + l1);
                item->bracketR[1] = PointF(item->p2().x(), item->p2().y() + l1);
                item->bracketR[2] = PointF(item->p2().x(), item->p2().y());
            } else {
                item->bracketL[0] = PointF(item->p1().x(), item->p1().y());
                item->bracketL[1] = PointF(item->p1().x(), item->p1().y() + l1);
                item->bracketL[2] = PointF(item->p2().x(), item->p2().y() + l1);
                item->bracketL[3] = PointF(item->p2().x(), item->p2().y());
            }
        }
    }

    // collect bounding box
    RectF r;
    if (item->number()) {
        r |= item->number()->bbox().translated(item->number()->pos());
        if (item->hasBracket()) {
            RectF b;
            b.setCoords(item->bracketL[1].x(), item->bracketL[1].y(), item->bracketR[2].x(), item->bracketR[2].y());
            r |= b;
        }
    } else if (item->hasBracket()) {
        RectF b;
        b.setCoords(item->bracketL[1].x(), item->bracketL[1].y(), item->bracketL[3].x(), item->bracketL[3].y());
        r |= b;
    }
    item->setbbox(r);

    if (outOfStaff && !item->cross()) {
        item->autoplaceMeasureElement(item->isUp(), /* add to skyline */ true);
    }
}

/// <summary>
/// Recursively calls layout() on any nested tuplets and then the tuplet itself
/// </summary>
/// <param name="de">Start element of the tuplet</param>
void TupletLayout::layout(DurationElement* de, LayoutContext& ctx)
{
    Tuplet* t = reinterpret_cast<Tuplet*>(de);
    if (!t) {
        return;
    }
    // t is top level tuplet
    // loop through elements of that tuplet
    for (DurationElement* d : t->elements()) {
        if (d == de) {
            continue;
        }
        // if element is tuplet, layoutTuplet(that tuplet)
        if (d->isTuplet()) {
            layout(d, ctx);
        }
    }
    // layout t
    TLayout::layout(t, ctx);
}

bool TupletLayout::isTopTuplet(ChordRest* cr)
{
    Tuplet* t = cr->tuplet();
    if (t && t->elements().front() == cr) {
        // find top level tuplet
        while (t->tuplet()) {
            t = t->tuplet();
        }
        // consider tuplet cross if anything moved within it
        if (t->cross()) {
            return false;
        } else {
            return true;
        }
    }

    // no tuplet or not first element
    return false;
}

bool TupletLayout::notTopTuplet(ChordRest* cr)
{
    Tuplet* t = cr->tuplet();
    if (t && t->elements().front() == cr) {
        // find top level tuplet
        while (t->tuplet()) {
            t = t->tuplet();
        }
        // consider tuplet cross if anything moved within it
        if (t->cross()) {
            return true;
        } else {
            return false;
        }
    }

    // no tuplet or not first element
    return false;
}
