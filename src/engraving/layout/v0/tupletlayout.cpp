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
using namespace mu::engraving::layout::v0;

void TupletLayout::layout(Tuplet* item, LayoutContext& ctx)
{
    if (item->m_currentElements.empty()) {
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
    if (item->m_numberType != TupletNumberType::NO_TEXT) {
        if (item->m_number == 0) {
            item->m_number = Factory::createText(item, TextStyleType::TUPLET);
            item->m_number->setComposition(true);
            item->m_number->setTrack(item->track());
            item->m_number->setParent(item);
            item->m_number->setVisible(item->visible());
            item->resetNumberProperty();
        }
        // tuplet properties are propagated to number automatically by setProperty()
        // but we need to make sure flags are as well
        item->m_number->setPropertyFlags(Pid::FONT_FACE, item->propertyFlags(Pid::FONT_FACE));
        item->m_number->setPropertyFlags(Pid::FONT_SIZE, item->propertyFlags(Pid::FONT_SIZE));
        item->m_number->setPropertyFlags(Pid::FONT_STYLE, item->propertyFlags(Pid::FONT_STYLE));
        item->m_number->setPropertyFlags(Pid::ALIGN, item->propertyFlags(Pid::ALIGN));
        if (item->m_numberType == TupletNumberType::SHOW_NUMBER) {
            item->m_number->setXmlText(String(u"%1").arg(item->m_ratio.numerator()));
        } else {
            item->m_number->setXmlText(String(u"%1:%2").arg(item->m_ratio.numerator(), item->m_ratio.denominator()));
        }

        item->m_isSmall = true;
        for (const DurationElement* e : item->m_currentElements) {
            if ((e->isChordRest() && !toChordRest(e)->isSmall()) || (e->isTuplet() && !toTuplet(e)->isSmall())) {
                item->m_isSmall = false;
                break;
            }
        }
        item->m_number->setMag(item->m_isSmall ? ctx.conf().styleD(Sid::smallNoteMag) : 1.0);
    } else {
        if (item->m_number) {
            if (item->m_number->selected()) {
                ctx.deselect(item->m_number);
            }
            delete item->m_number;
            item->m_number = 0;
        }
    }
    //
    // find out main direction
    //
    if (item->m_direction == DirectionV::AUTO) {
        int up = 0;
        for (const DurationElement* e : item->m_currentElements) {
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
        item->m_isUp = up > 0;
    } else {
        item->m_isUp = item->m_direction == DirectionV::UP;
    }

    //
    // find first and last chord of tuplet
    // (tuplets can be nested)
    //
    const DurationElement* cr1 = item->m_currentElements.front();
    while (cr1->isTuplet()) {
        const Tuplet* t = toTuplet(cr1);
        if (t->elements().empty()) {
            break;
        }
        cr1 = t->elements().front();
    }
    const DurationElement* cr2 = item->m_currentElements.back();
    while (cr2->isTuplet()) {
        const Tuplet* t = toTuplet(cr2);
        if (t->elements().empty()) {
            break;
        }
        cr2 = t->elements().back();
    }

    item->m_hasBracket = item->calcHasBracket(cr1, cr2);
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

    if (item->m_isUp) {
        vHeadDistance = -vHeadDistance;
    }

    item->m_p1      = cr1->pagePos();
    item->m_p2      = cr2->pagePos();

    item->m_p1.rx() -= noteLeft;
    item->m_p2.rx() += ctx.conf().noteHeadWidth() + noteRight;
    item->m_p1.ry() += vHeadDistance;          // TODO: Direction ?
    item->m_p2.ry() += vHeadDistance;

    double xx1 = item->m_p1.x();   // use to center the number on the beam

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

    if (item->m_isUp) {
        if (cr1->isChord()) {
            const Chord* chord1 = toChord(cr1);
            Stem* stem = chord1->stem();
            if (stem) {
                xx1 = stem->abbox().x();
            }
            if (chord1->up() && stem) {
                item->m_p1.ry() = stem->abbox().y();
                l2l = vStemDistance;
                item->m_p1.rx() = stem->abbox().left() - stemLeft;
            } else {
                item->m_p1.ry() = chord1->upNote()->abbox().top();
                item->m_p1.rx() = leftNoteEdge - noteLeft;
            }
        }

        if (cr2->isChord()) {
            const Chord* chord2 = toChord(cr2);
            Stem* stem = chord2->stem();
            if (stem && chord2->up()) {
                item->m_p2.ry() = stem->abbox().top();
                l2r = vStemDistance;
                item->m_p2.rx() = stem->abbox().right() + stemRight;
            } else {
                item->m_p2.ry() = chord2->upNote()->abbox().top();
                item->m_p2.rx() = rightNoteEdge + noteRight;
            }
        }
        //
        // special case: one of the bracket endpoints is
        // a rest
        //
        if (!cr1->isChord() && cr2->isChord()) {
            if (item->m_p2.y() < item->m_p1.y()) {
                item->m_p1.setY(item->m_p2.y());
            } else {
                item->m_p2.setY(item->m_p1.y());
            }
        } else if (cr1->isChord() && !cr2->isChord()) {
            if (item->m_p1.y() < item->m_p2.y()) {
                item->m_p2.setY(item->m_p1.y());
            } else {
                item->m_p1.setY(item->m_p2.y());
            }
        }

        // outOfStaff
        if (outOfStaff) {
            double min = cr1->measure()->staffabbox(cr1->staffIdx() + move).y();
            if (min < item->m_p1.y()) {
                item->m_p1.ry() = min;
                l2l = vStemDistance;
            }
            min = cr2->measure()->staffabbox(cr2->staffIdx() + move).y();
            if (min < item->m_p2.y()) {
                item->m_p2.ry() = min;
                l2r = vStemDistance;
            }
        }

        // check that slope is no more than max
        double d = (item->m_p2.y() - item->m_p1.y()) / (item->m_p2.x() - item->m_p1.x());
        if (d < -maxSlope) {
            // move p1 y up
            item->m_p1.ry() = item->m_p2.y() + maxSlope * (item->m_p2.x() - item->m_p1.x());
        } else if (d > maxSlope) {
            // move p2 y up
            item->m_p2.ry() = item->m_p1.ry() + maxSlope * (item->m_p2.x() - item->m_p1.x());
        }

        // check for collisions
        size_t n = item->m_currentElements.size();
        if (n >= 3) {
            d = (item->m_p2.y() - item->m_p1.y()) / (item->m_p2.x() - item->m_p1.x());
            for (size_t i = 1; i < (n - 1); ++i) {
                EngravingItem* e = item->m_currentElements[i];
                if (e->isChord()) {
                    const Chord* chord = toChord(e);
                    const Stem* stem = chord->stem();
                    if (stem) {
                        RectF r(chord->up() ? stem->abbox() : chord->upNote()->abbox());
                        double y3 = r.top();
                        double x3 = r.x() + r.width() * .5;
                        double y0 = item->m_p1.y() + (x3 - item->m_p1.x()) * d;
                        double c  = y0 - y3;
                        if (c > 0) {
                            item->m_p1.ry() -= c;
                            item->m_p2.ry() -= c;
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
                item->m_p1.ry() = stem->abbox().bottom();
                l2l = vStemDistance;
                item->m_p1.rx() = stem->abbox().left() - stemLeft;
            } else {
                item->m_p1.ry() = chord1->downNote()->abbox().bottom();
                item->m_p1.rx() = leftNoteEdge - noteLeft;
            }
        }

        if (cr2->isChord()) {
            const Chord* chord2 = toChord(cr2);
            Stem* stem = chord2->stem();
            if (stem && !chord2->up()) {
                item->m_p2.ry() = stem->abbox().bottom();
                l2r = vStemDistance;
                item->m_p2.rx() = stem->abbox().right() + stemRight;
            } else {
                item->m_p2.ry() = chord2->downNote()->abbox().bottom();
                item->m_p2.rx() = rightNoteEdge + noteRight;
            }
        }
        //
        // special case: one of the bracket endpoints is
        // a rest
        //
        if (!cr1->isChord() && cr2->isChord()) {
            if (item->m_p2.y() > item->m_p1.y()) {
                item->m_p1.setY(item->m_p2.y());
            } else {
                item->m_p2.setY(item->m_p1.y());
            }
        } else if (cr1->isChord() && !cr2->isChord()) {
            if (item->m_p1.y() > item->m_p2.y()) {
                item->m_p2.setY(item->m_p1.y());
            } else {
                item->m_p1.setY(item->m_p2.y());
            }
        }
        // outOfStaff
        if (outOfStaff) {
            double max = cr1->measure()->staffabbox(cr1->staffIdx() + move).bottom();
            if (max > item->m_p1.y()) {
                item->m_p1.ry() = max;
                l2l = vStemDistance;
            }
            max = cr2->measure()->staffabbox(cr2->staffIdx() + move).bottom();
            if (max > item->m_p2.y()) {
                item->m_p2.ry() = max;
                l2r = vStemDistance;
            }
        }
        // check that slope is no more than max
        double d = (item->m_p2.y() - item->m_p1.y()) / (item->m_p2.x() - item->m_p1.x());
        if (d < -maxSlope) {
            // move p1 y up
            item->m_p2.ry() = item->m_p1.y() - maxSlope * (item->m_p2.x() - item->m_p1.x());
        } else if (d > maxSlope) {
            // move p2 y up
            item->m_p1.ry() = item->m_p2.ry() - maxSlope * (item->m_p2.x() - item->m_p1.x());
        }

        // check for collisions
        size_t n = item->m_currentElements.size();
        if (n >= 3) {
            d  = (item->m_p2.y() - item->m_p1.y()) / (item->m_p2.x() - item->m_p1.x());
            for (size_t i = 1; i < (n - 1); ++i) {
                EngravingItem* e = item->m_currentElements[i];
                if (e->isChord()) {
                    const Chord* chord = toChord(e);
                    const Stem* stem = chord->stem();
                    if (stem) {
                        RectF r(chord->up() ? chord->downNote()->abbox() : stem->abbox());
                        double y3 = r.bottom();
                        double x3 = r.x() + r.width() * .5;
                        double y0 = item->m_p1.y() + (x3 - item->m_p1.x()) * d;
                        double c  = y0 - y3;
                        if (c < 0) {
                            item->m_p1.ry() -= c;
                            item->m_p2.ry() -= c;
                        }
                    }
                }
            }
        }
    }

    if (!cr1->isChord()) {
        item->m_p1.rx() = cr1->abbox().left() - noteLeft;
    }
    if (!cr2->isChord()) {
        item->m_p2.rx() = cr2->abbox().right() + noteRight;
    }

    item->setPos(0.0, 0.0);
    PointF mp(item->parentItem()->pagePos());
    if (item->explicitParent()->isMeasure()) {
        System* s = toMeasure(item->explicitParent())->system();
        if (s) {
            mp.ry() += s->staff(item->staffIdx())->y();
        }
    }
    item->m_p1 -= mp;
    item->m_p2 -= mp;

    item->m_p1 += item->m_userP1;
    item->m_p2 += item->m_userP2;
    xx1 -= mp.x();

    item->m_p1.ry() -= l2l * (item->m_isUp ? 1.0 : -1.0);
    item->m_p2.ry() -= l2r * (item->m_isUp ? 1.0 : -1.0);

    // l2l l2r, mp, _p1, _p2 const

    // center number
    double x3 = 0.0;
    double numberWidth = 0.0;
    if (item->m_number) {
        TLayout::layout(item->m_number, ctx);
        numberWidth = item->m_number->bbox().width();

        double y3 = item->m_p1.y() + (item->m_p2.y() - item->m_p1.y()) * .5 - l1 * (item->m_isUp ? 1.0 : -1.0);
        // for beamed tuplets, center number on beam - if they don't have a bracket
        if (cr1->beam() && cr2->beam() && cr1->beam() == cr2->beam() && !item->m_hasBracket) {
            const ChordRest* crr = toChordRest(cr1);
            if (item->m_isUp == crr->up()) {
                double deltax = cr2->pagePos().x() - cr1->pagePos().x();
                x3 = xx1 + deltax * .5;
            } else {
                double deltax = item->m_p2.x() - item->m_p1.x();
                x3 = item->m_p1.x() + deltax * .5;
            }
        } else {
            // otherwise center on the bracket (TODO: make centering rules customizable?)
            double deltax = item->m_p2.x() - item->m_p1.x();
            x3 = item->m_p1.x() + deltax * .5;
        }

        item->m_number->setPos(PointF(x3, y3) - item->ipos());
    }

    if (item->m_hasBracket) {
        double slope = (item->m_p2.y() - item->m_p1.y()) / (item->m_p2.x() - item->m_p1.x());

        if (item->m_isUp) {
            if (item->m_number) {
                //set width of bracket hole
                double x     = x3 - numberWidth * .5 - _spatium * .5;
                item->m_p1.rx() = std::min(item->m_p1.rx(), x - 0.5 * l1); // ensure enough space for the number
                double y     = item->m_p1.y() + (x - item->m_p1.x()) * slope;
                item->m_bracketL[0] = PointF(item->m_p1.x(), item->m_p1.y());
                item->m_bracketL[1] = PointF(item->m_p1.x(), item->m_p1.y() - l1);
                item->m_bracketL[2] = PointF(x,   y - l1);

                //set width of bracket hole
                x           = x3 + numberWidth * .5 + _spatium * .5;
                item->m_p2.rx() = std::max(item->m_p2.rx(), x + 0.5 * l1); // ensure enough space for the number
                y           = item->m_p1.y() + (x - item->m_p1.x()) * slope;
                item->m_bracketR[0] = PointF(x,   y - l1);
                item->m_bracketR[1] = PointF(item->m_p2.x(), item->m_p2.y() - l1);
                item->m_bracketR[2] = PointF(item->m_p2.x(), item->m_p2.y());
            } else {
                item->m_bracketL[0] = PointF(item->m_p1.x(), item->m_p1.y());
                item->m_bracketL[1] = PointF(item->m_p1.x(), item->m_p1.y() - l1);
                item->m_bracketL[2] = PointF(item->m_p2.x(), item->m_p2.y() - l1);
                item->m_bracketL[3] = PointF(item->m_p2.x(), item->m_p2.y());
            }
        } else {
            if (item->m_number) {
                //set width of bracket hole
                double x     = x3 - numberWidth * .5 - _spatium * .5;
                item->m_p1.rx() = std::min(item->m_p1.rx(), x - 0.5 * l1); // ensure enough space for the number
                double y     = item->m_p1.y() + (x - item->m_p1.x()) * slope;
                item->m_bracketL[0] = PointF(item->m_p1.x(), item->m_p1.y());
                item->m_bracketL[1] = PointF(item->m_p1.x(), item->m_p1.y() + l1);
                item->m_bracketL[2] = PointF(x,   y + l1);

                //set width of bracket hole
                x           = x3 + numberWidth * .5 + _spatium * .5;
                item->m_p2.rx() = std::max(item->m_p2.rx(), x + 0.5 * l1);
                y           = item->m_p1.y() + (x - item->m_p1.x()) * slope;
                item->m_bracketR[0] = PointF(x,   y + l1);
                item->m_bracketR[1] = PointF(item->m_p2.x(), item->m_p2.y() + l1);
                item->m_bracketR[2] = PointF(item->m_p2.x(), item->m_p2.y());
            } else {
                item->m_bracketL[0] = PointF(item->m_p1.x(), item->m_p1.y());
                item->m_bracketL[1] = PointF(item->m_p1.x(), item->m_p1.y() + l1);
                item->m_bracketL[2] = PointF(item->m_p2.x(), item->m_p2.y() + l1);
                item->m_bracketL[3] = PointF(item->m_p2.x(), item->m_p2.y());
            }
        }
    }

    // collect bounding box
    RectF r;
    if (item->m_number) {
        r |= item->m_number->bbox().translated(item->m_number->pos());
        if (item->m_hasBracket) {
            RectF b;
            b.setCoords(item->m_bracketL[1].x(), item->m_bracketL[1].y(), item->m_bracketR[2].x(), item->m_bracketR[2].y());
            r |= b;
        }
    } else if (item->m_hasBracket) {
        RectF b;
        b.setCoords(item->m_bracketL[1].x(), item->m_bracketL[1].y(), item->m_bracketL[3].x(), item->m_bracketL[3].y());
        r |= b;
    }
    item->setbbox(r);

    if (outOfStaff && !item->cross()) {
        item->autoplaceMeasureElement(item->m_isUp, /* add to skyline */ true);
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
