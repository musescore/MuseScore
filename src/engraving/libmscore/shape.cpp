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

#include "shape.h"
#include "segment.h"
#include "chord.h"
#include "score.h"
#include "system.h"

#include "infrastructure/draw/painter.h"

using namespace mu;
using namespace mu::draw;

namespace Ms {
//---------------------------------------------------------
//   addHorizontalSpacing
//    This methods creates "walls". They are represented by
//    rectangles of zero height, and it is assumed that rectangles
//    of zero height vertically collide with everything. Use this
//    method ONLY when you want to crate space that cannot tuck
//    above/below other elements of the staff.
//---------------------------------------------------------

void Shape::addHorizontalSpacing(EngravingItem* item, qreal leftEdge, qreal rightEdge)
{
    constexpr qreal eps = 100 * std::numeric_limits<qreal>::epsilon();
    if (leftEdge == rightEdge) { // HACK zero-width shapes collide with everything currently.
        rightEdge += eps;
    }
    add(RectF(leftEdge, 0, rightEdge - leftEdge, 0), item);
}

//---------------------------------------------------------
//   translate
//---------------------------------------------------------

void Shape::translate(const PointF& pt)
{
    for (RectF& r : *this) {
        r.translate(pt);
    }
}

void Shape::translateX(qreal xo)
{
    for (RectF& r : *this) {
        r.setLeft(r.left() + xo);
        r.setRight(r.right() + xo);
    }
}

void Shape::translateY(qreal yo)
{
    for (RectF& r : *this) {
        r.setTop(r.top() + yo);
        r.setBottom(r.bottom() + yo);
    }
}

//---------------------------------------------------------
//   translated
//---------------------------------------------------------

Shape Shape::translated(const PointF& pt) const
{
    Shape s;
    for (const ShapeElement& r : *this) {
        s.add(r.translated(pt), r.toItem);
    }
    return s;
}

bool Shape::sameVoiceExceptions(const EngravingItem* item1, const EngravingItem* item2) const
{
    if (item1->track() != item2->track()) {
        return false;
    }
    if ((item1->isNote() || item1->isRest() || item1->isBreath())
        && (item2->isNote() || item2->isRest() || item2->isStem() || item2->isBreath())) {
        return true;
    }
    return false;
}

bool Shape::limitedKerningExceptions(const EngravingItem* item1, const EngravingItem* item2) const
{
    if ((item1->isClef() || item2->isClef())
        && !(item1->isLyrics() || item2->isLyrics())) {
        return true;
    }
    return false;
}

bool Shape::nonKerningExceptions(const ShapeElement& r1, const ShapeElement& r2) const
{
    const EngravingItem* item1 = r1.toItem;
    const EngravingItem* item2 = r2.toItem;
    if (item1 && !item1->isKernable()) { // Prepared for future user option, for now always false
        return true;
    }
    if (r1.width() == 0 || r2.width() == 0) { // Shapes of zero width are assumed to collide with everything
        return true;
    }
    if (item1 && item2 // this is needed for lyrics-to-lyrics and harmony-to-harmony spacing
        && ((item1->isLyrics() && item2->isLyrics()) || (item1->isHarmony() && item2->isHarmony()))) {
        return true;
    }
    if (item1 && item2
        && (item1->isTimeSig() || item2->isTimeSig()
            || item1->isKeySig() || item2->isKeySig())     // these items can never kern...
        && !(item1->isLyrics() || item2->isLyrics() // except with lyrics and harmony
             || item1->isHarmony() || item2->isHarmony())) {
        return true;
    }
    if (item1 && item2
        && (item1->isBarLine() || (item2->isBarLine())) // barlines can never kern...
        && !(item1->isHarmony() || item2->isHarmony())) { // except with harmony
        return true;
    }
    if (!item1 && item2 && item2->isLyrics()) { // Temporary hack: avoid lyrics overlapping the melisma line
        return true;
    }
    return false;
}

//-------------------------------------------------------------------
//   minHorizontalDistance
//    a is located right of this shape.
//    Calculates the minimum horizontal distance between the two shapes
//    so they don’t touch.
//-------------------------------------------------------------------

qreal Shape::minHorizontalDistance(const Shape& a, Score* score) const
{
    qreal dist = -1000000.0;        // min real
    const PaddingTable& paddingTable = score->paddingTable();
    double padding = 0;
    double verticalClearance = 0.2 * score->spatium();
    bool sameVoiceCases = false;
    bool nonKerning = false; // These items behave as is their padding has infinite height
    bool limitedKerning = false; // These items can get close to each other when they vertically clear but not overlap
    for (const ShapeElement& r2 : a) {
        const EngravingItem* item2 = r2.toItem;
        qreal by1 = r2.top();
        qreal by2 = r2.bottom();
        for (const ShapeElement& r1 : *this) {
            const EngravingItem* item1 = r1.toItem;
            qreal ay1 = r1.top();
            qreal ay2 = r1.bottom();
            padding = 0;
            sameVoiceCases = false;
            limitedKerning = false;
            nonKerning = nonKerningExceptions(r1, r2);
            if (item1 && item2) {
                padding = paddingTable.at(item1->type()).at(item2->type());
                padding *= (item1->mag() + item2->mag()) / 2; // scales with items magnification
                verticalClearance *= (item1->mag() + item2->mag()) / 2;
                sameVoiceCases = sameVoiceExceptions(item1, item2);
                limitedKerning = limitedKerningExceptions(item1, item2);
            }
            if (sameVoiceCases // padding for note-note and note-stem needs needs this exception
                && Ms::intersects(ay1, ay2, by1, by2, verticalClearance)
                && (item2->isNote() || item2->isStem())) {
                padding = std::max(padding, double(score->styleMM(Sid::minNoteDistance)));
            }
            if (limitedKerning && !Ms::intersects(ay1, ay2, by1, by2, verticalClearance)) {
                padding = score->minimumPaddingUnit();
            }
            if (Ms::intersects(ay1, ay2, by1, by2, verticalClearance)
                || sameVoiceCases
                || limitedKerning
                || nonKerning) {
                dist = qMax(dist, r1.right() - r2.left() + padding);
            }
            if (item1 && item2
                && item1->track() == item2->track()
                && item1->isKernableUntilOrigin()) { //prepared for future user option, for now always false
                qreal origin = r1.left();
                dist = qMax(dist, origin - r2.left());
            }
        }
    }
    return dist;
}

//-------------------------------------------------------------------
//   minVerticalDistance
//    a is located below this shape.
//    Calculates the minimum distance between two shapes.
//-------------------------------------------------------------------

qreal Shape::minVerticalDistance(const Shape& a) const
{
    if (empty() || a.empty()) {
        return 0.0;
    }

    qreal dist = -1000000.0; // min real
    for (const RectF& r2 : a) {
        if (r2.height() <= 0.0) {
            continue;
        }
        qreal bx1 = r2.left();
        qreal bx2 = r2.right();
        for (const RectF& r1 : *this) {
            if (r1.height() <= 0.0) {
                continue;
            }
            qreal ax1 = r1.left();
            qreal ax2 = r1.right();
            if (Ms::intersects(ax1, ax2, bx1, bx2, 0.0)) {
                dist = qMax(dist, r1.bottom() - r2.top());
            }
        }
    }
    return dist;
}

//---------------------------------------------------------
//   left
//    compute left border
//---------------------------------------------------------

qreal Shape::left() const
{
    qreal dist = 0.0;
    for (const RectF& r : *this) {
        if (r.height() != 0.0 && r.left() < dist) {
            // if (r.left() < dist)
            dist = r.left();
        }
    }
    return -dist;
}

//---------------------------------------------------------
//   right
//    compute right border
//---------------------------------------------------------

qreal Shape::right() const
{
    qreal dist = 0.0;
    for (const RectF& r : *this) {
        if (r.right() > dist) {
            dist = r.right();
        }
    }
    return dist;
}

/* NOTE: these top() and bottom() methods look very weird to me, as they
 * seem to return the opposite of what they say. Or it seems like the
 * rectangles are defined upside down, for some reason. Needs some
 * more understanding. [M.S.] */

//---------------------------------------------------------
//   top
//---------------------------------------------------------

qreal Shape::top() const
{
    qreal dist = 1000000.0;
    for (const RectF& r : *this) {
        if (r.top() < dist) {
            dist = r.top();
        }
    }
    return dist;
}

//---------------------------------------------------------
//   bottom
//---------------------------------------------------------

qreal Shape::bottom() const
{
    qreal dist = -1000000.0;
    for (const RectF& r : *this) {
        if (r.bottom() > dist) {
            dist = r.bottom();
        }
    }
    return dist;
}

//---------------------------------------------------------
//   topDistance
//    p is on top of shape
//    returns negative values if there is an overlap
//---------------------------------------------------------

qreal Shape::topDistance(const PointF& p) const
{
    qreal dist = 1000000.0;
    for (const RectF& r : *this) {
        if (p.x() >= r.left() && p.x() < r.right()) {
            dist = qMin(dist, r.top() - p.y());
        }
    }
    return dist;
}

//---------------------------------------------------------
//   bottomDistance
//    p is below the shape
//    returns negative values if there is an overlap
//---------------------------------------------------------

qreal Shape::bottomDistance(const PointF& p) const
{
    qreal dist = 1000000.0;
    for (const RectF& r : *this) {
        if (p.x() >= r.left() && p.x() < r.right()) {
            dist = qMin(dist, p.y() - r.bottom());
        }
    }
    return dist;
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Shape::remove(const RectF& r)
{
    for (auto i = begin(); i != end(); ++i) {
        if (*i == r) {
            erase(i);
            return;
        }
    }
    // qWarning("Shape::remove: RectF not found in Shape");
    qFatal("Shape::remove: RectF not found in Shape");
}

void Shape::remove(const Shape& s)
{
    for (const RectF& r : s) {
        remove(r);
    }
}

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

bool Shape::contains(const PointF& p) const
{
    for (const RectF& r : *this) {
        if (r.contains(p)) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   intersects
//---------------------------------------------------------

bool Shape::intersects(const RectF& rr) const
{
    for (const RectF& r : *this) {
        if (r.intersects(rr)) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   intersects
//---------------------------------------------------------

bool Shape::intersects(const Shape& other) const
{
    for (const RectF& r : other) {
        if (intersects(r)) {
            return true;
        }
    }
    return false;
}

void Shape::paint(Painter& painter) const
{
    for (const RectF& r : *this) {
        painter.drawRect(r);
    }
}

#ifndef NDEBUG
//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Shape::dump(const char* p) const
{
    qDebug("Shape dump: %p %s size %zu", this, p, size());
    for (const ShapeElement& r : *this) {
        r.dump();
    }
}

void ShapeElement::dump() const
{
    qDebug("   %s: %f %f %f %f", toItem ? toItem->typeName() : "", x(), y(), width(), height());
}

#endif
} // namespace Ms
