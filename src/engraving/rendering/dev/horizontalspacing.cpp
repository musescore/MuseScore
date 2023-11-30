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
#include "horizontalspacing.h"

#include "dom/chord.h"
#include "dom/engravingitem.h"
#include "dom/glissando.h"
#include "dom/note.h"
#include "dom/rest.h"
#include "dom/score.h"
#include "dom/stemslash.h"
#include "dom/staff.h"
#include "dom/tie.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

//-------------------------------------------------------------------
//   minHorizontalDistance
//    a is located right of this shape.
//    Calculates the minimum horizontal distance between the two shapes
//    so they don’t touch.
//-------------------------------------------------------------------

double HorizontalSpacing::minHorizontalDistance(const Shape& f, const Shape& s, double spatium, double squeezeFactor)
{
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
                padding = computePadding(item1, item2);
                padding *= squeezeFactor;
                padding = std::max(padding, absoluteMinPadding);
                kerningType = computeKerning(item1, item2);
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

// Logic moved from Shape
double HorizontalSpacing::shapeSpatium(const Shape& s)
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

double HorizontalSpacing::minHorizontalDistance(const Segment* f, const Segment* ns, bool systemHeaderGap,
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
                bool tieOrGlissBack = note->spannerBack().size() || (note->tieBack() && !note->tieBack()->segmentsEmpty());
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

double HorizontalSpacing::minHorizontalCollidingDistance(const Segment* f, const Segment* ns, double squeezeFactor)
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

//---------------------------------------------------------
//   minLeft
//    Calculate minimum distance needed to the left shape
//    sl. Sl is the same for all staves.
//---------------------------------------------------------

double HorizontalSpacing::minLeft(const Segment* seg, const Shape& ls)
{
    double distance = 0.0;
    double sp = shapeSpatium(ls);
    for (const Shape& sh : seg->shapes()) {
        double d = minHorizontalDistance(ls, sh, sp, 1.0);
        if (d > distance) {
            distance = d;
        }
    }
    return distance;
}

void HorizontalSpacing::spaceRightAlignedSegments(Measure* m, double segmentShapeSqueezeFactor)
{
    // Collect all the right-aligned segments starting from the back
    std::vector<Segment*> rightAlignedSegments;
    for (Segment* segment = m->segments().last(); segment; segment = segment->prev()) {
        if (segment->enabled() && segment->isRightAligned()) {
            rightAlignedSegments.push_back(segment);
        }
    }
    // Compute spacing
    static constexpr double arbitraryLowReal = -10000.0;
    for (Segment* raSegment : rightAlignedSegments) {
        // 1) right-align the segment against the following ones
        double minDistAfter = arbitraryLowReal;
        for (Segment* seg = raSegment->next(); seg; seg = seg->next()) {
            double xDiff = seg->x() - raSegment->x();
            double minDist = minHorizontalCollidingDistance(raSegment, seg, segmentShapeSqueezeFactor);
            minDistAfter = std::max(minDistAfter, minDist - xDiff);
        }
        if (minDistAfter != arbitraryLowReal && raSegment->prevActive()) {
            Segment* prevSegment = raSegment->prev();
            prevSegment->setWidth(prevSegment->width() - minDistAfter);
            prevSegment->setWidthOffset(prevSegment->widthOffset() - minDistAfter);
            raSegment->mutldata()->moveX(-minDistAfter);
            raSegment->setWidth(raSegment->width() + minDistAfter);
        }
        // 2) Make sure the segment isn't colliding with anything behind
        double minDistBefore = 0.0;
        for (Segment* seg = raSegment->prevActive(); seg; seg = seg->prevActive()) {
            double xDiff = raSegment->x() - seg->x();
            double minDist = minHorizontalCollidingDistance(seg, raSegment, segmentShapeSqueezeFactor);
            minDistBefore = std::max(minDistBefore, minDist - xDiff);
        }
        Segment* prevSegment = raSegment->prevActive();
        if (prevSegment) {
            prevSegment->setWidth(prevSegment->width() + minDistBefore);
        }
        for (Segment* seg = raSegment; seg; seg = seg->next()) {
            seg->mutldata()->moveX(minDistBefore);
        }
        m->setWidth(m->width() + minDistBefore);
    }
}

double HorizontalSpacing::computeFirstSegmentXPosition(const Measure* m, const Segment* segment, double segmentShapeSqueezeFactor)
{
    double x = 0;

    Shape ls(RectF(0.0, 0.0, 0.0, m->spatium() * 4));

    // First, try to compute first segment x-position by padding against end barline of previous measure
    Measure* prevMeas = (m->prev() && m->prev()->isMeasure() && m->prev()->system() == m->system()) ? toMeasure(m->prev()) : nullptr;
    Segment* prevMeasEnd = prevMeas ? prevMeas->lastEnabled() : nullptr;
    bool ignorePrev = !prevMeas || prevMeas->system() != m->system() || !prevMeasEnd
                      || (prevMeasEnd->segmentType() & SegmentType::BarLineType && segment->segmentType() & SegmentType::BarLineType);
    if (!ignorePrev) {
        x = minHorizontalCollidingDistance(prevMeasEnd, segment, segmentShapeSqueezeFactor);
        x -= prevMeas->width() - prevMeasEnd->x();
    }

    // If that doesn't succeed (e.g. first bar) then just use left-margins
    if (x <= 0) {
        x = minLeft(segment, ls);
        if (segment->isChordRestType()) {
            x += m->style().styleMM(segment->hasAccidentals() ? Sid::barAccidentalDistance : Sid::barNoteDistance);
        } else if (segment->isClefType() || segment->isHeaderClefType()) {
            x += m->style().styleMM(Sid::clefLeftMargin);
        } else if (segment->isKeySigType()) {
            x = std::max(x, m->style().styleMM(Sid::keysigLeftMargin).val());
        } else if (segment->isTimeSigType()) {
            x = std::max(x, m->style().styleMM(Sid::timesigLeftMargin).val());
        }
    }

    // Special case: the start-repeat should overlap the end-repeat of the previous measure
    bool prevIsEndRepeat = prevMeas && prevMeas->repeatEnd() && prevMeasEnd && prevMeasEnd->isEndBarLineType();
    if (prevIsEndRepeat && segment->isStartRepeatBarLineType() && (prevMeas->system() == m->system())) {
        x -= m->style().styleMM(Sid::endBarWidth);
    }

    // Do a final check of chord distances (invisible items may in some cases elude the 2 previous steps)
    if (segment->isChordRestType()) {
        double barNoteDist = m->style().styleMM(Sid::barNoteDistance).val();
        for (EngravingItem* e : segment->elist()) {
            if (!e || !e->isChordRest() || (e->staff() && e->staff()->isTabStaff(e->tick()))) {
                continue;
            }
            x = std::max(x, barNoteDist * e->mag() - e->pos().x());
        }
    }
    x += segment->extraLeadingSpace().val() * m->spatium();
    return x;
}

double HorizontalSpacing::computePadding(const EngravingItem* item1, const EngravingItem* item2)
{
    const PaddingTable& paddingTable = item1->score()->paddingTable();
    ElementType type1 = item1->type();
    ElementType type2 = item2->type();

    double padding = paddingTable.at(type1).at(type2);
    double scaling = (item1->mag() + item2->mag()) / 2;

    if (type1 == ElementType::NOTE && isSpecialNotePaddingType(type2)) {
        computeNotePadding(toNote(item1), item2, padding, scaling);
    } else {
        padding *= scaling;
    }

    if (!item1->isLedgerLine() && item2->isRest()) {
        computeLedgerRestPadding(toRest(item2), padding);
    }

    return padding;
}

bool HorizontalSpacing::isSpecialNotePaddingType(ElementType type)
{
    switch (type) {
    case ElementType::NOTE:
    case ElementType::REST:
    case ElementType::STEM:
        return true;
    default:
        return false;
    }
}

void HorizontalSpacing::computeNotePadding(const Note* note, const EngravingItem* item2, double& padding, double scaling)
{
    const MStyle& style = note->style();

    bool sameVoiceNoteOrStem = (item2->isNote() || item2->isStem()) && note->track() == item2->track();
    if (sameVoiceNoteOrStem) {
        bool intersection = note->shape().translate(note->pos()).intersects(item2->shape().translate(item2->pos()));
        if (intersection) {
            padding = std::max(padding, static_cast<double>(style.styleMM(Sid::minNoteDistance)));
        }
    }

    padding *= scaling;

    if (!(item2->isNote() || item2->isRest())) {
        return;
    }

    if (note->isGrace() && item2->isNote() && toNote(item2)->isGrace()) {
        // Grace-to-grace
        padding = std::max(padding, static_cast<double>(style.styleMM(Sid::graceToGraceNoteDist)));
    } else if (note->isGrace() && (item2->isRest() || (item2->isNote() && !toNote(item2)->isGrace()))) {
        // Grace-to-main
        padding = std::max(padding, static_cast<double>(style.styleMM(Sid::graceToMainNoteDist)));
    } else if (!note->isGrace() && item2->isNote() && toNote(item2)->isGrace()) {
        // Main-to-grace
        padding = std::max(padding, static_cast<double>(style.styleMM(Sid::graceToMainNoteDist)));
    }

    if (!item2->isNote()) {
        return;
    }

    const Note* note2 = toNote(item2);
    if (note->lineAttachPoints().empty() || note2->lineAttachPoints().empty()) {
        return;
    }

    // Allocate space for minTieLenght and minGlissandoLength
    for (LineAttachPoint laPoint1 : note->lineAttachPoints()) {
        for (LineAttachPoint laPoint2 : note2->lineAttachPoints()) {
            if (laPoint1.line() != laPoint2.line()) {
                continue;
            }

            double minEndPointsDistance = 0.0;
            if (laPoint1.line()->isTie()) {
                minEndPointsDistance = style.styleMM(Sid::MinTieLength);
            } else if (laPoint1.line()->isGlissando()) {
                bool straight = toGlissando(laPoint1.line())->glissandoType() == GlissandoType::STRAIGHT;
                double minGlissandoLength = straight
                                            ? style.styleMM(Sid::MinStraightGlissandoLength)
                                            : style.styleMM(Sid::MinWigglyGlissandoLength);
                minEndPointsDistance = minGlissandoLength;
            } else if (laPoint1.line()->isGuitarBend()) {
                double minBendLength = 2 * note->spatium(); // TODO: style
                minEndPointsDistance = minBendLength;
            }

            double lapPadding = (laPoint1.pos().x() - note->headWidth()) + minEndPointsDistance - laPoint2.pos().x();
            lapPadding *= scaling;

            padding = std::max(padding, lapPadding);
        }
    }
}

void HorizontalSpacing::computeLedgerRestPadding(const Rest* rest2, double& padding)
{
    SymId restSym = rest2->ldata()->sym();
    switch (restSym) {
    case SymId::restWholeLegerLine:
    case SymId::restDoubleWholeLegerLine:
    case SymId::restHalfLegerLine:
        padding += rest2->ldata()->bbox().left();
        return;
    default:
        return;
    }
}

KerningType HorizontalSpacing::computeKerning(const EngravingItem* item1, const EngravingItem* item2)
{
    if (isSameVoiceKerningLimited(item1) && isSameVoiceKerningLimited(item2) && item1->track() == item2->track()) {
        return KerningType::NON_KERNING;
    }

    if ((isNeverKernable(item1) || isNeverKernable(item2))
        && !(isAlwaysKernable(item1) || isAlwaysKernable(item2))) {
        return KerningType::NON_KERNING;
    }

    return doComputeKerningType(item1, item2);
}

bool HorizontalSpacing::isSameVoiceKerningLimited(const EngravingItem* item)
{
    ElementType type = item->type();

    switch (type) {
    case ElementType::NOTE:
    case ElementType::REST:
    case ElementType::STEM:
    case ElementType::CHORDLINE:
    case ElementType::BREATH:
        return true;
    default:
        return false;
    }
}

bool HorizontalSpacing::isNeverKernable(const EngravingItem* item)
{
    ElementType type = item->type();

    switch (type) {
    case ElementType::CLEF:
    case ElementType::TIMESIG:
    case ElementType::KEYSIG:
    case ElementType::BAR_LINE:
        return true;
    default:
        return false;
    }
}

bool HorizontalSpacing::isAlwaysKernable(const EngravingItem* item)
{
    return item->isTextBase() || item->isChordLine();
}

KerningType HorizontalSpacing::doComputeKerningType(const EngravingItem* item1, const EngravingItem* item2)
{
    ElementType type1 = item1->type();
    switch (type1) {
    case ElementType::BAR_LINE:
        return KerningType::NON_KERNING;
    case ElementType::CHORDLINE:
        return item2->isBarLine() ? KerningType::ALLOW_COLLISION : KerningType::KERNING;
    case ElementType::HARMONY:
        return item2->isHarmony() ? KerningType::NON_KERNING : KerningType::KERNING;
    case ElementType::LYRICS:
        return (item2->isLyrics() || item2->isBarLine()) ? KerningType::NON_KERNING : KerningType::KERNING;
    case ElementType::NOTE:
        return computeNoteKerningType(toNote(item1), item2);
    case ElementType::STEM_SLASH:
        return computeStemSlashKerningType(toStemSlash(item1), item2);
    default:
        return KerningType::KERNING;
    }
}

KerningType HorizontalSpacing::computeNoteKerningType(const Note* note, const EngravingItem* item2)
{
    EngravingItem* nextParent = item2->parentItem(true);
    if (nextParent && nextParent->isNote() && toNote(nextParent)->isTrillCueNote()) {
        return KerningType::NON_KERNING;
    }

    Chord* c = note->chord();
    if (!c || (c->allowKerningAbove() && c->allowKerningBelow())) {
        return KerningType::KERNING;
    }
    bool kerningAbove = item2->canvasPos().y() < note->canvasPos().y();
    if (kerningAbove && !c->allowKerningAbove()) {
        return KerningType::NON_KERNING;
    }
    if (!kerningAbove && !c->allowKerningBelow()) {
        return KerningType::NON_KERNING;
    }

    return KerningType::KERNING;
}

KerningType HorizontalSpacing::computeStemSlashKerningType(const StemSlash* stemSlash, const EngravingItem* item2)
{
    if (!stemSlash->chord() || !stemSlash->chord()->beam() || !item2->parentItem()) {
        return KerningType::KERNING;
    }

    EngravingItem* nextParent = item2->parentItem();
    Chord* nextChord = nullptr;
    if (nextParent->isChord()) {
        nextChord = toChord(nextParent);
    } else if (nextParent->isNote()) {
        nextChord = toChord(nextParent->parentItem());
    }
    if (!nextChord) {
        return KerningType::KERNING;
    }

    if (nextChord->beam() && nextChord->beam() == stemSlash->chord()->beam()) {
        // Stem slash is allowed to collide with items from the same grace notes group
        return KerningType::ALLOW_COLLISION;
    }

    return KerningType::KERNING;
}
