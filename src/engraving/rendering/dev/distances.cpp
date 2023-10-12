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
#include "distances.h"

#include "style/style.h"

#include "dom/engravingitem.h"
#include "dom/segment.h"
#include "dom/measure.h"
#include "dom/score.h"
#include "dom/chord.h"
#include "dom/note.h"
#include "dom/glissando.h"

//-------------------------------------------------------------------
//   minHorizontalDistance
//    a is located right of this shape.
//    Calculates the minimum horizontal distance between the two shapes
//    so they don’t touch.
//-------------------------------------------------------------------

double mu::engraving::rendering::dev::distances::minHorizontalDistance(const Shape& f, const Shape& s, double spatium, double squeezeFactor)
{
    if (f.elements().size()) {
        DO_ASSERT(RealIsEqual(f.squeezeFactor(), squeezeFactor));
        DO_ASSERT(RealIsEqual(f.spatium(), spatium));
    }

    double dist = -1000000.0;        // min real
    double absoluteMinPadding = 0.1 * spatium * squeezeFactor;
    double verticalClearance = 0.2 * spatium * squeezeFactor;
    for (const ShapeElement& r2 : s.elements()) {
        if (r2.isNull()) {
            continue;
        }
        const EngravingItem* item2 = r2.item();
        double by1 = r2.top();
        double by2 = r2.bottom();
        for (const ShapeElement& r1 : f.elements()) {
            if (r1.isNull()) {
                continue;
            }
            const EngravingItem* item1 = r1.item();
            double ay1 = r1.top();
            double ay2 = r1.bottom();
            bool intersection = mu::engraving::intersects(ay1, ay2, by1, by2, verticalClearance);
            double padding = 0;
            KerningType kerningType = KerningType::NON_KERNING;
            if (item1 && item2) {
                padding = EngravingItem::renderer()->computePadding(item1, item2);
                padding *= squeezeFactor;
                padding = std::max(padding, absoluteMinPadding);
                kerningType = EngravingItem::renderer()->computeKerning(item1, item2);
            }
            if ((intersection && kerningType != KerningType::ALLOW_COLLISION)
                || (r1.width() == 0 || r2.width() == 0)  // Temporary hack: shapes of zero-width are assumed to collide with everyghin
                || (!item1 && item2 && item2->isLyrics())  // Temporary hack: avoids collision with melisma line
                || kerningType == KerningType::NON_KERNING) {
                dist = std::max(dist, r1.right() - r2.left() + padding);
            }
            if (kerningType == KerningType::KERNING_UNTIL_ORIGIN) { //prepared for future user option, for now always false
                double origin = r1.left();
                dist = std::max(dist, origin - r2.left());
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

double mu::engraving::rendering::dev::distances::minVerticalDistance(const Shape& f, const Shape& a)
{
    if (f.empty() || a.empty()) {
        return 0.0;
    }

    double dist = -1000000.0; // min real
    for (const RectF& r2 : a.elements()) {
        if (r2.height() <= 0.0) {
            continue;
        }
        double bx1 = r2.left();
        double bx2 = r2.right();
        for (const RectF& r1 : f.elements()) {
            if (r1.height() <= 0.0) {
                continue;
            }
            double ax1 = r1.left();
            double ax2 = r1.right();
            if (mu::engraving::intersects(ax1, ax2, bx1, bx2, 0.0)) {
                dist = std::max(dist, r1.bottom() - r2.top());
            }
        }
    }
    return dist;
}

//-------------------------------------------------------------------
//   verticalClearance
//    a is located below this shape.
//    Claculates the amount of clearance between the two shapes.
//    If there is an overlap, returns a negative value corresponging
//    to the amount of overlap.
//-------------------------------------------------------------------

double mu::engraving::rendering::dev::distances::verticalClearance(const Shape& f, const Shape& a)
{
    if (f.empty() || a.empty()) {
        return 0.0;
    }

    double dist = 1000000.0; // max real
    for (const RectF& r2 : a.elements()) {
        if (r2.height() <= 0.0) {
            continue;
        }
        double bx1 = r2.left();
        double bx2 = r2.right();
        for (const RectF& r1 : f.elements()) {
            if (r1.height() <= 0.0) {
                continue;
            }
            double ax1 = r1.left();
            double ax2 = r1.right();
            if (mu::engraving::intersects(ax1, ax2, bx1, bx2, 0.0)) {
                dist = std::min(dist, r2.top() - r1.bottom());
            }
        }
    }
    return dist;
}

// Logic moved from Shape
double mu::engraving::rendering::dev::distances::shapeSpatium(const Shape& s)
{
    for (auto it = s.elements().begin(); it != s.elements().end(); ++it) {
        if (it->item()) {
            return it->item()->spatium();
        }
    }
    return 0.0;
}

//---------------------------------------------------------
//   minHorizontalDistance
//    calculate the minimum layout distance to Segment ns
//---------------------------------------------------------

double mu::engraving::rendering::dev::distances::minHorizontalDistance(const Segment* f, const Segment* ns, bool systemHeaderGap,
                                                                       double squeezeFactor)
{
    if (f->isBeginBarLineType() && ns->isStartRepeatBarLineType()) {
        return 0.0;
    }

    double ww = -1000000.0;          // can remain negative
    double d = 0.0;
    for (unsigned staffIdx = 0; staffIdx < f->shapes().size(); ++staffIdx) {
        const Shape& fshape = f->staffShape(staffIdx);
        double sp = shapeSpatium(fshape);
        d = ns ? minHorizontalDistance(fshape, ns->staffShape(staffIdx), sp, squeezeFactor) : 0.0;
        // first chordrest of a staff should clear the widest header for any staff
        // so make sure segment is as wide as it needs to be
        if (systemHeaderGap) {
            d = std::max(d, f->staffShape(staffIdx).right());
        }
        ww = std::max(ww, d);
    }
    double w = std::max(ww, 0.0);        // non-negative

    // Header exceptions that need additional space (more than the padding)
    double absoluteMinHeaderDist = 1.5 * f->spatium();
    if (systemHeaderGap) {
        if (f->isTimeSigType()) {
            w = std::max(w, f->minRight() + f->style().styleMM(Sid::systemHeaderTimeSigDistance));
        } else {
            w = std::max(w, f->minRight() + f->style().styleMM(Sid::systemHeaderDistance));
        }
        if (ns && ns->isStartRepeatBarLineType()) {
            // Align the thin barline of the start repeat to the header
            w -= f->style().styleMM(Sid::endBarWidth) + f->style().styleMM(Sid::endBarDistance);
        }
        double diff = w - f->minRight() - ns->minLeft();
        if (diff < absoluteMinHeaderDist) {
            w += absoluteMinHeaderDist - diff;
        }
    }

    // Multimeasure rest exceptions that need special handling
    if (f->measure() && f->measure()->isMMRest()) {
        if (ns->isChordRestType()) {
            double minDist = f->minRight();
            if (f->isClefType()) {
                minDist += f->score()->paddingTable().at(ElementType::CLEF).at(ElementType::REST);
            } else if (f->isKeySigType()) {
                minDist += f->score()->paddingTable().at(ElementType::KEYSIG).at(ElementType::REST);
            } else if (f->isTimeSigType()) {
                minDist += f->score()->paddingTable().at(ElementType::TIMESIG).at(ElementType::REST);
            }
            w = std::max(w, minDist);
        } else if (f->isChordRestType()) {
            double minWidth = f->style().styleMM(Sid::minMMRestWidth).val();
            if (!f->style().styleB(Sid::oldStyleMultiMeasureRests)) {
                minWidth += f->style().styleMM(Sid::multiMeasureRestMargin).val();
            }
            w = std::max(w, minWidth);
        }
    }

    // Allocate space to ensure minimum length of "dangling" ties or gliss at start of system
    if (systemHeaderGap && ns && ns->isChordRestType()) {
        for (EngravingItem* e : ns->elist()) {
            if (!e || !e->isChord()) {
                continue;
            }
            double headerTieMargin = f->style().styleMM(Sid::HeaderToLineStartDistance);
            for (Note* note : toChord(e)->notes()) {
                bool tieOrGlissBack = note->spannerBack().size() || note->tieBack();
                if (!tieOrGlissBack || note->lineAttachPoints().empty()) {
                    continue;
                }
                const EngravingItem* attachedLine = note->lineAttachPoints().front().line();
                double minLength = 0.0;
                if (attachedLine->isTie()) {
                    minLength = f->style().styleMM(Sid::MinTieLength);
                } else if (attachedLine->isGlissando()) {
                    bool straight = toGlissando(attachedLine)->glissandoType() == GlissandoType::STRAIGHT;
                    minLength = straight ? f->style().styleMM(Sid::MinStraightGlissandoLength)
                                : f->style().styleMM(Sid::MinWigglyGlissandoLength);
                }
                double tieStartPointX = f->minRight() + headerTieMargin;
                double notePosX = w + note->pos().x() + toChord(e)->pos().x() + note->headWidth() / 2;
                double tieEndPointX = notePosX + note->lineAttachPoints().at(0).pos().x();
                double tieLength = tieEndPointX - tieStartPointX;
                if (tieLength < minLength) {
                    w += minLength - tieLength;
                }
            }
        }
    }

    return w;
}

double mu::engraving::rendering::dev::distances::minHorizontalCollidingDistance(const Segment* f, const Segment* ns, double squeezeFactor)
{
    if (f->isBeginBarLineType() && ns->isStartRepeatBarLineType()) {
        return 0.0;
    }

    double w = -100000.0; // This can remain negative in some cases (for instance, mid-system clefs)
    for (unsigned staffIdx = 0; staffIdx < f->shapes().size(); ++staffIdx) {
        const Shape& fshape = f->staffShape(staffIdx);
        double sp = shapeSpatium(fshape);
        double d = minHorizontalDistance(fshape, ns->staffShape(staffIdx), sp, squeezeFactor);
        w = std::max(w, d);
    }
    return w;
}
