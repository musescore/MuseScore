/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include <cfloat>

#include "horizontalspacing.h"

#include "dom/barline.h"
#include "dom/beam.h"
#include "dom/chord.h"
#include "dom/engravingitem.h"
#include "dom/glissando.h"
#include "dom/lyrics.h"
#include "dom/note.h"
#include "dom/rest.h"
#include "dom/score.h"
#include "dom/stemslash.h"
#include "dom/staff.h"
#include "dom/stem.h"
#include "dom/system.h"
#include "dom/tie.h"
#include "dom/timesig.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

double HorizontalSpacing::computeSpacingForFullSystem(System* system, double stretchReduction, double squeezeFactor,
                                                      bool overrideMinMeasureWidth)
{
    HorizontalSpacingContext ctx;
    ctx.system = system;
    ctx.spatium = system->spatium();

    ctx.stretchReduction = stretchReduction;
    ctx.squeezeFactor = squeezeFactor;
    ctx.overrideMinMeasureWidth = overrideMinMeasureWidth;

    ctx.xCur = system->leftMargin();
    ctx.xLeftBarrier = system->leftMargin();
    ctx.systemIsFull = true;

    std::vector<Measure*> measureGroup;
    measureGroup.reserve(system->measures().size());

    for (MeasureBase* mb : system->measures()) {
        if (mb->isHBox()) {
            spaceMeasureGroup(measureGroup, ctx);
            measureGroup.clear();

            mb->mutldata()->setPosX(ctx.xCur);
            mb->computeMinWidth();
            ctx.xCur += mb->width();
        } else {
            measureGroup.push_back(toMeasure(mb));
        }
    }

    spaceMeasureGroup(measureGroup, ctx);

    return ctx.xCur;
}

double HorizontalSpacing::updateSpacingForLastAddedMeasure(System* system, bool startOfContinuousLayoutRegion)
{
    HorizontalSpacingContext ctx;
    ctx.system = system;
    ctx.spatium = system->spatium();
    ctx.xLeftBarrier = system->leftMargin();

    size_t measureCount = system->measures().size();
    IF_ASSERT_FAILED(measureCount > 0) {
        return 0.0;
    }

    MeasureBase* secondToLast = measureCount >= 2 ? system->measures()[measureCount - 2] : nullptr;
    MeasureBase* last = system->measures()[measureCount - 1];

    if (secondToLast) {
        ctx.xCur = secondToLast->x();
        if (secondToLast->isHBox() || last->isHBox() || startOfContinuousLayoutRegion) {
            ctx.xCur += secondToLast->width();
        }
    } else {
        ctx.xCur = system->leftMargin();
    }

    if (last->isHBox()) {
        last->mutldata()->setPosX(ctx.xCur);
        last->computeMinWidth();
        ctx.xCur += last->width();
    } else if (!secondToLast || secondToLast->isHBox() || startOfContinuousLayoutRegion) {
        spaceMeasureGroup({ toMeasure(last) }, ctx);
    } else {
        ctx.startMeas = toMeasure(last);
        spaceMeasureGroup({ toMeasure(secondToLast), toMeasure(last) }, ctx);
    }

    return ctx.xCur;
}

void HorizontalSpacing::squeezeSystemToFit(System* system, double& curSysWidth, double targetSysWidth)
{
    Measure* firstMeasure = system->firstMeasure();
    if (!firstMeasure) {
        return;
    }

    static constexpr double STRETCH_REDUCTION_STEP = 0.33;
    static constexpr double SQUEEZE_STEP = 0.2;

    double squeezeFactor = 1 - SQUEEZE_STEP; // Reduces shape paddings
    double stretchReduction = 1 - STRETCH_REDUCTION_STEP; // Reduces duration stretch

    while (curSysWidth > targetSysWidth && muse::RealIsEqualOrMore(squeezeFactor, 0.0)) {
        curSysWidth = HorizontalSpacing::computeSpacingForFullSystem(system, stretchReduction, squeezeFactor, true);
        squeezeFactor -= SQUEEZE_STEP;
        stretchReduction -= STRETCH_REDUCTION_STEP;
    }

    if (curSysWidth < targetSysWidth) {
        return;
    }

    // Things don't fit without collisions, so give up and allow collisions
    double smallerStep = 0.25 * SQUEEZE_STEP;
    double widthReduction = 1 - smallerStep;
    while (curSysWidth > targetSysWidth && muse::RealIsEqualOrMore(widthReduction, 0.0)) {
        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }

            Measure* m = toMeasure(mb);
            double prevWidth = m->width();
            for (Segment& segment : m->segments()) {
                if (!segment.isChordRestType()) {
                    continue;
                }
                double curSegmentWidth = segment.width();
                segment.setWidth(curSegmentWidth * widthReduction);
            }
            m->respaceSegments();
            curSysWidth += m->width() - prevWidth;
        }
        widthReduction -= smallerStep;
    }
}

void HorizontalSpacing::justifySystem(System* system, double curSysWidth, double targetSystemWidth)
{
    double rest = targetSystemWidth - curSysWidth;
    if (muse::RealIsNull(rest)) {
        return;
    }

    IF_ASSERT_FAILED(rest > 0) {
        return;
    }

    std::vector<Spring> springs;

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment& s : toMeasure(mb)->segments()) {
            if (s.isChordRestType() && s.ticks() > Fraction(0, 1) && s.visible() && s.enabled() && !s.allElementsInvisible()) {
                double springConst = 1 / s.stretch();
                double width = s.width() - s.widthOffset();
                double preTension = width * springConst;
                springs.emplace_back(springConst, width, preTension, &s);
            }
        }
    }

    stretchSegmentsToWidth(springs, rest);

    double x = system->leftMargin();
    for (MeasureBase* mb : system->measures()) {
        if (mb->isMeasure()) {
            toMeasure(mb)->respaceSegments();
        }
        mb->mutldata()->setPosX(x);
        x += mb->width();
    }
}

void HorizontalSpacing::spaceMeasureGroup(const std::vector<Measure*>& measureGroup, HorizontalSpacingContext& ctx)
{
    if (measureGroup.empty()) {
        return;
    }

    measureGroup.front()->mutldata()->setPosX(ctx.xCur);

    std::vector<Segment*> segList;
    segList.reserve(2 * measureGroup.size());

    int startSegIdx = -1;
    for (Measure* measure : measureGroup) {
        if (measure == ctx.startMeas) {
            startSegIdx = int(segList.size());
        }
        for (Segment& segment : measure->segments()) {
            segList.push_back(&segment);
        }
    }

    IF_ASSERT_FAILED(!segList.empty()) {
        return;
    }

    std::vector<SegmentPosition> segmentPositions = spaceSegments(segList, startSegIdx, ctx);
    moveRightAlignedSegments(segmentPositions, ctx);

    setPositionsAndWidths(segmentPositions);

    if (!ctx.overrideMinMeasureWidth) {
        enforceMinimumMeasureWidths(measureGroup);
    }

    Measure* lastMeas = measureGroup.back();
    ctx.xCur = lastMeas->x() + lastMeas->width();
}

std::vector<HorizontalSpacing::SegmentPosition> HorizontalSpacing::spaceSegments(const std::vector<Segment*>& segList, int startSegIdx,
                                                                                 HorizontalSpacingContext& ctx)
{
    std::vector<SegmentPosition> placedSegments;
    placedSegments.reserve(segList.size());

    for (size_t i = 0; i < segList.size(); ++i) {
        Segment* curSeg = segList[i];
        if (ignoreSegmentForSpacing(curSeg)) {
            placedSegments.emplace_back(curSeg, ctx.xCur);
            continue;
        }

        if (static_cast<int>(i) < startSegIdx) {
            placedSegments.emplace_back(curSeg, curSeg->xPosInSystemCoords());
            ctx.xCur = curSeg->xPosInSystemCoords() + curSeg->width();
            continue;
        }

        curSeg->clearWidthOffset();

        if (placedSegments.empty() || ignoreAllSegmentsForSpacing(placedSegments)) {
            double xFirst = getFirstSegmentXPos(curSeg, ctx);
            ctx.xCur += xFirst;
            placedSegments.emplace_back(curSeg, ctx.xCur);
        } else {
            spaceAgainstPreviousSegments(curSeg, placedSegments, ctx);
        }

        double leadingSpace = curSeg->extraLeadingSpace().toMM(ctx.spatium);
        placedSegments.back().xPosInSystemCoords += leadingSpace;

        if (curSeg->isChordRestType()) {
            bool isFirstCROfSystem = curSeg->rtick().isZero() && curSeg->measure()->isFirstInSystem();
            if (isFirstCROfSystem) {
                double xMinSystemHeaderDist = ctx.system->leftMargin() + curSeg->style().styleMM(Sid::systemHeaderMinStartOfSystemDistance);
                placedSegments.back().xPosInSystemCoords = std::max(placedSegments.back().xPosInSystemCoords, xMinSystemHeaderDist);
                ctx.xCur = std::max(ctx.xCur, xMinSystemHeaderDist);
            }

            double chordRestSegWidth = chordRestSegmentNaturalWidth(curSeg, ctx);

            Segment* nextSeg = i < segList.size() - 1 ? segList[i + 1] : nullptr;
            if (nextSeg) {
                double nextSegLeadingSpace = nextSeg->extraLeadingSpace().toMM(ctx.spatium);
                if (!muse::RealIsNull(nextSegLeadingSpace)) {
                    nextSegLeadingSpace = std::max(nextSegLeadingSpace, -chordRestSegWidth);
                    curSeg->addWidthOffset(nextSegLeadingSpace);
                }
                if (nextSeg->isChordRestType()) {
                    applyCrossBeamSpacingCorrection(curSeg, nextSeg, chordRestSegWidth);
                }
            }

            ctx.xCur += chordRestSegWidth;
        }
    }

    if (ctx.systemIsFull) {
        checkLyricsAgainstRightMargin(placedSegments);
        checkLargeTimeSigAgainstRightMargin(placedSegments);
    }

    return placedSegments;
}

bool HorizontalSpacing::ignoreSegmentForSpacing(const Segment* segment)
{
    return !segment->enabled() || !segment->isActive() || segment->allElementsInvisible();
}

bool HorizontalSpacing::ignoreAllSegmentsForSpacing(const std::vector<SegmentPosition>& segmentPositions)
{
    for (const SegmentPosition& segPos : segmentPositions) {
        if (!ignoreSegmentForSpacing(segPos.segment)) {
            return false;
        }
    }

    return true;
}

void HorizontalSpacing::spaceAgainstPreviousSegments(Segment* segment, std::vector<SegmentPosition>& prevSegPositions,
                                                     HorizontalSpacingContext& ctx)
{
    double x = -DBL_MAX;
    int prevCRSegmentsCount = 0;

    if (segment->measure()->isFirstInSystem()) {
        checkLyricsAgainstLeftMargin(segment, x, ctx);
    }

    bool segmentHasTimeSigAboveStaff = segment->hasTimeSigAboveStaves();

    for (size_t i = prevSegPositions.size(); i > 0; --i) {
        const SegmentPosition& prevSegPos = prevSegPositions[i - 1];
        Segment* prevSeg = prevSegPos.segment;
        double xPrevSeg = prevSegPos.xPosInSystemCoords;

        if (ignoreSegmentForSpacing(prevSeg)) {
            continue;
        }

        if (prevSeg->isChordRestType()) {
            ++prevCRSegmentsCount;
        }

        bool timeSigAboveBarlineCase = segmentHasTimeSigAboveStaff && prevSeg->isEndBarLineType() && prevSeg->tick() == segment->tick();
        bool timeSigAboveKeySigCase = segmentHasTimeSigAboveStaff && prevSeg->isType(SegmentType::KeySig)
                                      && prevSeg->tick() == segment->tick();
        bool timeSigAboveRepeatKeySigCase = segmentHasTimeSigAboveStaff && prevSeg->isType(SegmentType::KeySigRepeatAnnounce)
                                            && prevSeg->tick() == segment->tick();

        if (timeSigAboveBarlineCase) {
            x = xPrevSeg + prevSeg->minRight() - 0.5 * prevSeg->style().styleMM(Sid::barWidth); // align to the preceding barline
        } else if (timeSigAboveKeySigCase) {
            x = xPrevSeg + segment->minLeft(); // align to the preceding keySig
        } else if (timeSigAboveRepeatKeySigCase) {
            x = xPrevSeg + prevSeg->minRight();
        } else {
            double minHorDist = minHorizontalDistance(prevSeg, segment, ctx.squeezeFactor);
            minHorDist = std::max(minHorDist, spaceLyricsAgainstBarlines(prevSeg, segment, ctx));
            double xNonCollision = xPrevSeg + minHorDist;
            x = std::max(x, xNonCollision);
        }

        if (x > ctx.xCur || timeSigAboveBarlineCase) {
            if (prevCRSegmentsCount > 0) {
                double spaceIncrease = (x - ctx.xCur) / prevCRSegmentsCount;
                double xMovement = spaceIncrease;
                for (size_t j = i; j < prevSegPositions.size(); ++j) {
                    SegmentPosition& segPos = prevSegPositions[j];
                    if (ignoreSegmentForSpacing(segPos.segment)) {
                        continue;
                    }
                    segPos.xPosInSystemCoords += xMovement;
                    if (segPos.segment->isChordRestType()) {
                        xMovement += spaceIncrease;
                    }
                }
            }

            ctx.xCur = x;
        }

        if (stopCheckingPreviousSegments(prevSegPos, SegmentPosition(segment, ctx.xCur))) {
            break;
        }
    }

    double xSeg = segment->isRightAligned() ? x : ctx.xCur;

    prevSegPositions.emplace_back(segment, xSeg);
}

bool HorizontalSpacing::stopCheckingPreviousSegments(const SegmentPosition& prevSegPos, const SegmentPosition& curSegPos)
{
    Segment* prevSeg = prevSegPos.segment;
    Segment* curSeg = curSegPos.segment;

    Fraction prevSegEndTick = prevSeg->tick() + prevSeg->ticks();
    Fraction curSegTick = curSeg->tick();

    if (prevSegEndTick >= curSegTick || curSeg->hasTimeSigAboveStaves()) {
        return false;
    }

    if (!curSeg->isChordRestType()) {
        return true;
    }

    bool curHasLyricsOrHarmony = false;
    double curX = curSegPos.xPosInSystemCoords;
    double curMinLeft = DBL_MAX;
    for (const Shape& shape : curSeg->shapes()) {
        for (const ShapeElement& shapeEl : shape.elements()) {
            curMinLeft = std::min(curMinLeft, shapeEl.left());
            if (const EngravingItem* item = shapeEl.item()) {
                if (item->isLyrics() || item->isHarmony()) {
                    curHasLyricsOrHarmony = true;
                }
            }
        }
    }
    curMinLeft += curX;

    bool prevHasLyricsOrHarmony = false;
    double prevX = prevSegPos.xPosInSystemCoords;
    double prevMinRight = -DBL_MAX;
    for (const Shape& shape : prevSeg->shapes()) {
        for (const ShapeElement& shapeEl : shape.elements()) {
            prevMinRight = std::max(prevMinRight, shapeEl.right());
            if (const EngravingItem* item = shapeEl.item()) {
                if (item->isLyrics() || item->isHarmony()) {
                    prevHasLyricsOrHarmony = true;
                }
            }
        }
    }
    prevMinRight += prevX;

    if (curHasLyricsOrHarmony == prevHasLyricsOrHarmony && curMinLeft > prevMinRight) {
        return true;
    }

    if (prevSeg->measure() != curSeg->measure() && prevSeg->measure() != curSeg->measure()->prevMM()) {
        return true;
    }

    return false;
}

void HorizontalSpacing::checkLyricsAgainstLeftMargin(Segment* segment, double& x, HorizontalSpacingContext& ctx)
{
    for (const Shape& staffShape : segment->shapes()) {
        for (const ShapeElement& shapeEl : staffShape.elements()) {
            if (shapeEl.item() && shapeEl.item()->isLyrics()) {
                x = std::max(x, ctx.xLeftBarrier - shapeEl.left());
            }
        }
    }
}

void HorizontalSpacing::checkLyricsAgainstRightMargin(std::vector<SegmentPosition>& segPositions)
{
    int chordRestSegmentsCount = 0;

    for (size_t i = segPositions.size(); i > 1; --i) {
        double systemEdge = segPositions.back().xPosInSystemCoords + segPositions.back().segment->minRight();

        SegmentPosition& segPos = segPositions[i - 1];
        double x = segPos.xPosInSystemCoords;
        Segment* seg = segPos.segment;
        if (!seg->measure()->isLastInSystem()) {
            break;
        }
        if (seg->isChordRestType()) {
            ++chordRestSegmentsCount;
        } else {
            continue;
        }
        double xMaxLyrics = x;
        for (const Shape& shape : seg->shapes()) {
            for (const ShapeElement& shapeEl : shape.elements()) {
                if (shapeEl.item() && shapeEl.item()->isLyrics()) {
                    xMaxLyrics = std::max(xMaxLyrics, x + shapeEl.right());
                }
            }
        }
        if (xMaxLyrics > systemEdge) {
            double lyricsOvershoot = xMaxLyrics - systemEdge;
            double spaceIncrease = lyricsOvershoot / chordRestSegmentsCount;
            double xMovement = spaceIncrease;
            for (size_t j = i; j < segPositions.size(); ++j) {
                SegmentPosition& sp = segPositions[j];
                sp.xPosInSystemCoords += xMovement;
                if (sp.segment->isChordRestType()) {
                    xMovement += spaceIncrease;
                }
            }
        }
    }
}

double HorizontalSpacing::spaceLyricsAgainstBarlines(Segment* firstSeg, Segment* secondSeg, const HorizontalSpacingContext& ctx)
{
    if (!(firstSeg->isType(SegmentType::ChordRest) && secondSeg->isType(SegmentType::BarLineType))
        && !(firstSeg->isType(SegmentType::BarLineType) && secondSeg->isType(SegmentType::ChordRest))) {
        return 0.0;
    }

    double w = 0.0;

    bool crSegIsBefore = firstSeg->isChordRestType();
    Segment* crSegment = crSegIsBefore ? firstSeg : secondSeg;
    Segment* barlineSegment = crSegIsBefore ? secondSeg : firstSeg;

    staff_idx_t nstaves = crSegment->score()->nstaves();

    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        if (!ctx.system->staff(staffIdx)->show()) {
            continue;
        }

        staff_idx_t nextStaff = ctx.system->nextVisibleStaff(staffIdx);
        if (nextStaff == muse::nidx) {
            continue;
        }

        BarLine* barline = toBarLine(barlineSegment->element(staff2track(staffIdx)));
        if (!barline || barline->spanStaff() == 0) {
            continue;
        }

        Shape barlineShape = barline->shape().translate(barline->pos());

        track_idx_t startTrack = staff2track(staffIdx);
        track_idx_t endTrack = staff2track(nextStaff) + VOICES;
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            ChordRest* chordRest = toChordRest(crSegment->element(track));
            if (!chordRest) {
                continue;
            }

            for (Lyrics* lyrics : chordRest->lyrics()) {
                if (!(lyrics->avoidBarlines() && lyrics->visible() && lyrics->autoplace())) {
                    continue;
                }
                staff_idx_t lyricsStaffIdx = lyrics->staffIdx();
                if ((lyricsStaffIdx == staffIdx && lyrics->placeBelow()) || (lyricsStaffIdx == nextStaff && lyrics->placeAbove())) {
                    Shape lyricsShape = lyrics->shape().translate(lyrics->pos());
                    double minDist = crSegIsBefore ? lyricsShape.right() + barlineShape.left() : lyricsShape.left() + barlineShape.right();
                    const double padding = 0.3 * lyrics->fontMetrics().xHeight();
                    minDist += padding;
                    w = std::max(w, minDist);
                }
            }
        }
    }

    return w;
}

void HorizontalSpacing::checkLargeTimeSigAgainstRightMargin(std::vector<SegmentPosition>& segPositions)
{
    SegmentPosition& cautionaryTSSegPos = segPositions.back();
    if (!cautionaryTSSegPos.segment->hasTimeSigAboveStaves()) {
        return;
    }

    const MStyle& style = cautionaryTSSegPos.segment->style();
    TimeSigVSMargin timeSigVSMargin = style.styleB(Sid::timeSigCenterOnBarline)
                                      ? style.styleV(Sid::timeSigVSMarginCentered).value<TimeSigVSMargin>()
                                      : style.styleV(Sid::timeSigVSMarginNonCentered).value<TimeSigVSMargin>();
    if (timeSigVSMargin != TimeSigVSMargin::RIGHT_ALIGN_TO_BARLINE) {
        return;
    }

    double xMax = 0.0;
    for (auto iter = segPositions.rbegin(); iter != segPositions.rend(); ++iter) {
        if (iter->segment->isType(SegmentType::EndBarLine | SegmentType::KeySigAnnounce)) {
            xMax = std::max(xMax, iter->xPosInSystemCoords + iter->segment->minRight());
        }
    }

    if (muse::RealIsNull(xMax)) {
        return;
    }

    double curTSPos = cautionaryTSSegPos.xPosInSystemCoords;
    double xMaxPos = xMax - cautionaryTSSegPos.segment->minRight();
    if (xMaxPos < curTSPos) {
        cautionaryTSSegPos.xPosInSystemCoords = xMaxPos;
    }
}

void HorizontalSpacing::moveRightAlignedSegments(std::vector<SegmentPosition>& placedSegments, const HorizontalSpacingContext& ctx)
{
    for (size_t i = 0; i < placedSegments.size(); ++i) {
        Segment* segment = placedSegments[i].segment;
        if (!segment->isRightAligned()) {
            continue;
        }

        double x = DBL_MAX;

        for (size_t j = i + 1; j < placedSegments.size(); ++j) {
            Segment* followingSeg = placedSegments[j].segment;
            if (followingSeg->isRightAligned() || ignoreSegmentForSpacing(followingSeg) || followingSeg->hasTimeSigAboveStaves()) {
                continue;
            }
            if (followingSeg->measure() != segment->measure()) {
                break;
            }
            double followingSegX = placedSegments[j].xPosInSystemCoords;
            double minDist = minHorizontalDistance(segment, followingSeg, ctx.squeezeFactor);
            x = std::min(x, followingSegX - minDist);
        }

        if (x != DBL_MAX) {
            placedSegments[i].xPosInSystemCoords = x;
            double nextSegX = placedSegments[i + 1].xPosInSystemCoords;
            Segment* prevCRSeg = segment->prev(SegmentType::ChordRest);
            if (prevCRSeg) {
                prevCRSeg->addWidthOffset(-nextSegX + x);
            }
        }
    }
}

void HorizontalSpacing::checkCollisionsWithCrossStaffStems(const Segment* thisSeg, const Segment* nextSeg, staff_idx_t staffIdx,
                                                           double& curMinDist)
{
    std::vector<ChordRest*> itemsToCheck;
    itemsToCheck.reserve(VOICES);

    for (EngravingItem* el : nextSeg->elist()) {
        if (el && el->vStaffIdx() == staffIdx) {
            ChordRest* cr = toChordRest(el);
            if (cr->beam() && cr->beam()->cross()) {
                itemsToCheck.push_back(toChordRest(el));
            }
        }
    }

    for (ChordRest* cr : itemsToCheck) {
        Chord* potentialCollidingChord = nullptr;
        for (ChordRest* beamElement : cr->beam()->elements()) {
            if (beamElement->isChord() && beamElement->parent() == thisSeg) {
                potentialCollidingChord = toChord(beamElement);
                break;
            }
        }

        if (!potentialCollidingChord || potentialCollidingChord->up() != cr->up() || !potentialCollidingChord->stem()) {
            return;
        }

        bool checkStemCollision = (cr->up() && potentialCollidingChord->vStaffIdx() > cr->vStaffIdx())
                                  || (!cr->up() && potentialCollidingChord->vStaffIdx() < cr->vStaffIdx());
        if (!checkStemCollision) {
            return;
        }

        Stem* stem = potentialCollidingChord->stem();
        Shape prevStemShape = stem->shape().translated(stem->pos() + potentialCollidingChord->pos());
        // We don't know where this stem is vertically, but we know from the beam arrangement that
        // it's going to be crossed anyway, so we can space to it as if it had infinite height.
        prevStemShape.adjust(0.0, -100000, 0.0, 100000);

        const Shape& staffShape = nextSeg->staffShape(staffIdx);

        double minDist = minHorizontalDistance(prevStemShape, staffShape, stem->spatium());

        curMinDist = std::max(curMinDist, minDist);
    }
}

double HorizontalSpacing::chordRestSegmentNaturalWidth(Segment* segment, HorizontalSpacingContext& ctx)
{
    double durationStretch = computeSegmentDurationStretch(segment, segment->prev(SegmentType::ChordRest));

    Measure* measure = segment->measure();
    double userStretch = std::clamp(measure->userStretch(), 0.1, 10.0); // TODO: enforce via UI, not here

    double segTotalStretch = durationStretch * userStretch;
    segment->setStretch(segTotalStretch);

    double stdNoteHeadWidth = measure->score()->noteHeadWidth();
    double minNoteDist = measure->score()->style().styleMM(Sid::minNoteDistance);
    double minNoteSpace = stdNoteHeadWidth + minNoteDist;

    static constexpr double QUARTER_NOTE_SPACING = 1.5;

    double naturalWidth = minNoteSpace * segTotalStretch * ctx.stretchReduction * QUARTER_NOTE_SPACING;

    return naturalWidth;
}

double HorizontalSpacing::computeSegmentDurationStretch(const Segment* curSeg, const Segment* prevSeg)
{
    if (curSeg->isMMRestSegment()) {
        return durationStretchForMMRests(curSeg);
    }

    Fraction segTicks = curSeg->ticks();
    Fraction shortestCR = curSeg->shortestChordRest();
    Fraction prevShortestCR = prevSeg ? prevSeg->shortestChordRest() : Fraction(0, 1);

    bool hasAdjacent = curSeg->isChordRestType() && shortestCR == segTicks;
    bool prevHasAdjacent = prevSeg && (prevSeg->isChordRestType() && prevShortestCR == prevSeg->ticks());

    double durStretch;
    const MStyle& style = curSeg->style();
    double slope = style.styleD(Sid::measureSpacing);

    if (hasAdjacent || curSeg->measure()->isMMRest()) {
        durStretch = durationStretchForTicks(slope, segTicks);
    } else {
        // Polyrythms
        if (prevSeg && !prevHasAdjacent && prevShortestCR < shortestCR) {
            durStretch = durationStretchForTicks(slope, prevShortestCR) * (segTicks / prevShortestCR).toDouble();
        } else {
            durStretch = durationStretchForTicks(slope, shortestCR) * (segTicks / shortestCR).toDouble();
        }
    }

    if (style.styleB(Sid::scaleRythmicSpacingForSmallNotes) && needsCueSizeSpacing(curSeg)) {
        durStretch *= curSeg->style().styleD(Sid::smallNoteMag);
    }

    return durStretch;
}

double HorizontalSpacing::durationStretchForMMRests(const Segment* segment)
{
    static constexpr Fraction QUARTER = Fraction(1, 4);
    static constexpr int MIN_MMREST_COUNT  = 2;

    const MStyle& style = segment->style();
    const Measure* measure = segment->measure();

    Fraction timeSig = measure->timesig();
    bool constantWidth = style.styleB(Sid::mmRestConstantWidth);
    int mmRestWidthIncrementCap = style.styleI(Sid::mmRestMaxWidthIncrease);

    Fraction baseDuration = constantWidth ? style.styleI(Sid::mmRestReferenceWidth) * QUARTER : timeSig;
    Fraction durationIncrement = constantWidth ? QUARTER : Fraction(1, timeSig.denominator());
    int incrementCount = std::max(measure->mmRestCount() - MIN_MMREST_COUNT, 0);
    incrementCount = std::min(incrementCount, mmRestWidthIncrementCap);

    Fraction resultDuration = baseDuration + incrementCount * durationIncrement;

    return durationStretchForTicks(style.styleD(Sid::measureSpacing), resultDuration);
}

double HorizontalSpacing::durationStretchForTicks(double slope, const Fraction& ticks)
{
    static constexpr Fraction REFERENCE_DURATION = Fraction(1, 4);

    Fraction durationRatio = ticks / REFERENCE_DURATION;

    return pow(slope, log2(durationRatio.toDouble()));
}

bool HorizontalSpacing::needsCueSizeSpacing(const Segment* segment)
{
    IF_ASSERT_FAILED(segment->isChordRestType()) {
        return false;
    }

    bool hasCueSizedCR = false;
    for (EngravingItem* item : segment->elist()) {
        if (!item) {
            continue;
        }
        const ChordRest* cr = toChordRest(item);
        if (cr->isSmall()) {
            hasCueSizedCR = true;
        } else if (cr->actualTicks() == segment->ticks()) {
            return false;
        }
    }

    return hasCueSizedCR;
}

void HorizontalSpacing::applyCrossBeamSpacingCorrection(Segment* thisSeg, Segment* nextSeg, double& width)
{
    CrossBeamSpacing crossBeamSpacing = computeCrossBeamSpacing(thisSeg, nextSeg);

    const Score* score = thisSeg->score();
    const PaddingTable& paddingTable = score->paddingTable();
    const MStyle& style = score->style();

    double displacement = score->noteHeadWidth() - style.styleMM(Sid::stemWidth);

    if (crossBeamSpacing.upDown && crossBeamSpacing.canBeAdjusted) {
        thisSeg->addWidthOffset(displacement);
        width += displacement;
    } else if (crossBeamSpacing.downUp && crossBeamSpacing.canBeAdjusted) {
        thisSeg->addWidthOffset(-displacement);
        width -= displacement;
    }

    if (crossBeamSpacing.upDown) {
        if (crossBeamSpacing.hasOpposingBeamlets) {
            double minBeamletClearance = style.styleMM(Sid::beamMinLen) * 2.0 + paddingTable.at(ElementType::BEAM).at(ElementType::BEAM);
            width = std::max(width, displacement + minBeamletClearance);
        } else {
            width = std::max(width, 2 * displacement);
        }
    }

    if (crossBeamSpacing.preventCrossStaffKerning) {
        double padding = crossBeamSpacing.ensureMinStemDistance ? paddingTable.at(ElementType::STEM).at(ElementType::STEM)
                         : style.styleMM(Sid::minNoteDistance);
        width = std::max(width, score->noteHeadWidth() + padding);
    } else if (crossBeamSpacing.ensureMinStemDistance) {
        width = std::max(width, score->paddingTable().at(ElementType::STEM).at(ElementType::STEM));
    }
}

HorizontalSpacing::CrossBeamSpacing HorizontalSpacing::computeCrossBeamSpacing(Segment* thisSeg, Segment* nextSeg)
{
    CrossBeamSpacing crossBeamType;

    if (!thisSeg->isChordRestType() || !nextSeg || !nextSeg->isChordRestType()) {
        return crossBeamType;
    }

    bool preventCrossStaffKerning = false;
    bool ensureMinStemDistance = false;
    for (EngravingItem* e : thisSeg->elist()) {
        if (!e || !e->isChord() || !e->visible() || !e->staff()->visible()) {
            continue;
        }

        Chord* thisChord = toChord(e);
        ChordRest* nextCR = toChordRest(nextSeg->element(thisChord->track()));
        Chord* nextChord = nextCR && nextCR->isChord() ? toChord(nextCR) : nullptr;
        if (!nextChord) {
            continue;
        }

        int thisStaffMove = thisChord->staffMove();
        int nextStaffMove = nextChord->staffMove();
        if (thisStaffMove == nextStaffMove) {
            continue;
        }

        preventCrossStaffKerning = thisStaffMove > nextStaffMove;
        ensureMinStemDistance = (thisStaffMove > nextStaffMove && thisChord->up() && !nextChord->up())
                                || (thisStaffMove < nextStaffMove && thisChord->up() == nextChord->up());
    }

    crossBeamType.preventCrossStaffKerning = preventCrossStaffKerning;
    crossBeamType.ensureMinStemDistance = ensureMinStemDistance;

    bool upDown = false;
    bool downUp = false;
    bool canBeAdjusted = true;
    bool hasOpposingBeamlets = false;

    // Spacing can be adjusted for cross-beam cases only if there aren't
    // chords in other voices in this or next segment.
    for (EngravingItem* e : thisSeg->elist()) {
        if (!e || !e->isChordRest() || !e->staff()->visible()) {
            continue;
        }
        ChordRest* thisCR = toChordRest(e);
        if (!thisCR->visible() || thisCR->isFullMeasureRest() || (thisCR->isRest() && toRest(thisCR)->isGap())) {
            continue;
        }
        if (!thisCR->beam()) {
            canBeAdjusted = false;
            continue;
        }
        Beam* beam = thisCR->beam();
        for (EngravingItem* ee : nextSeg->elist()) {
            if (!ee || !ee->isChordRest() || !ee->staff()->visible()) {
                continue;
            }
            ChordRest* nextCR = toChordRest(ee);
            if (!nextCR->visible() || nextCR->isFullMeasureRest() || (thisCR->isRest() && toRest(thisCR)->isGap())) {
                continue;
            }
            if (!nextCR->beam()) {
                canBeAdjusted = false;
                continue;
            }
            if (nextCR->beam() != beam) {
                continue;
            }
            if (thisCR->up() == nextCR->up()) {
                return crossBeamType;
            }
            if (thisCR->up() && !nextCR->up()) {
                if (thisCR->beamlet() && nextCR->beamlet()) {
                    hasOpposingBeamlets = true;
                }
                upDown = true;
            }
            if (!thisCR->up() && nextCR->up()) {
                downUp = true;
            }
            if (upDown && downUp) {
                return crossBeamType;
            }
        }
    }

    crossBeamType.upDown = upDown;
    crossBeamType.downUp = downUp;
    crossBeamType.canBeAdjusted = canBeAdjusted;
    crossBeamType.hasOpposingBeamlets = hasOpposingBeamlets;

    return crossBeamType;
}

double HorizontalSpacing::computeMinMeasureWidth(Measure* m)
{
    const MStyle& style = m->style();
    double minWidth = style.styleMM(Sid::minMeasureWidth);
    double maxSysWidth = m->system()->width() - m->system()->leftMargin();

    if (maxSysWidth <= 0) {
        maxSysWidth = minWidth;
    }
    minWidth = std::min(minWidth, maxSysWidth);

    if (m->ticks() < m->timesig()) {
        minWidth *= (m->ticks() / m->timesig()).toDouble();
    }

    Segment* firstCRSegment = m->findFirstR(SegmentType::ChordRest, Fraction(0, 1));
    if (!firstCRSegment) {
        return minWidth;
    }
    if (firstCRSegment == m->firstEnabled()) {
        return minWidth;
    }

    double startPosition = firstCRSegment->x() - firstCRSegment->minLeft();
    if (firstCRSegment->hasAccidentals()) {
        startPosition -= style.styleMM(Sid::barAccidentalDistance);
    } else {
        startPosition -= style.styleMM(Sid::barNoteDistance);
    }

    minWidth += startPosition;
    minWidth = std::min(minWidth, maxSysWidth);

    return minWidth;
}

void HorizontalSpacing::enforceMinimumMeasureWidths(const std::vector<Measure*> measureGroup)
{
    for (size_t i = 0; i < measureGroup.size(); ++i) {
        Measure* measure = measureGroup[i];
        if (measure->isMMRest()) {
            continue; // minimum mmRest width is already enforced during spacing
        }
        double minWidth = computeMinMeasureWidth(measure);
        double diff = minWidth - measure->width();
        if (diff > 0) {
            stretchMeasureToTargetWidth(measure, minWidth);
            for (size_t j = i + 1; j < measureGroup.size(); ++j) {
                measureGroup[j]->mutldata()->moveX(diff);
            }
        }
    }
}

void HorizontalSpacing::stretchMeasureToTargetWidth(Measure* m, double targetWidth)
{
    IF_ASSERT_FAILED(targetWidth > m->width()) {
        return;
    }

    std::vector<Spring> springs;
    for (Segment& s : m->segments()) {
        if (s.isChordRestType() && s.visible() && s.enabled() && !s.allElementsInvisible()) {
            double springConst = 1 / s.stretch();
            double width = s.width() - s.widthOffset();
            double preTension = width * springConst;
            springs.emplace_back(springConst, width, preTension, &s);
        }
    }

    stretchSegmentsToWidth(springs, targetWidth - m->width());

    m->respaceSegments();
}

void HorizontalSpacing::stretchSegmentsToWidth(std::vector<Spring>& springs, double width)
{
    if (springs.empty() || muse::RealIsEqualOrLess(width, 0.0)) {
        return;
    }

    std::sort(springs.begin(), springs.end(), [](const Spring& a, const Spring& b) { return a.preTension < b.preTension; });
    double inverseSpringConst = 0.0;
    double force = 0.0;

    //! NOTE springs.cbegin() != springs.cend() because of the emptiness check at the top
    auto spring = springs.cbegin();
    do {
        inverseSpringConst += 1 / spring->springConst;
        width += spring->width;
        force = width / inverseSpringConst;
        ++spring;
    } while (spring != springs.cend() && !(force < spring->preTension));

    for (const Spring& spring2 : springs) {
        if (force > spring2.preTension) {
            double newWidth = force / spring2.springConst;
            spring2.segment->setWidth(newWidth + spring2.segment->widthOffset());
        }
    }
}

void HorizontalSpacing::setPositionsAndWidths(const std::vector<SegmentPosition>& segmentPositions)
{
    size_t segmentsSize = segmentPositions.size();
    for (size_t i = 0; i < segmentsSize; ++i) {
        Segment* curSeg = segmentPositions[i].segment;
        double curX = segmentPositions[i].xPosInSystemCoords;

        if (ignoreSegmentForSpacing(curSeg)) {
            curSeg->setWidth(0.0);
            continue;
        }

        Segment* nextSeg = curSeg;
        double nextX = curX;
        for (size_t j = i + 1; j < segmentsSize; ++j) {
            if (!ignoreSegmentForSpacing(segmentPositions[j].segment)) {
                nextSeg = segmentPositions[j].segment;
                nextX = segmentPositions[j].xPosInSystemCoords;
                break;
            }
        }

        Measure* curSegMeasure = curSeg->measure();
        Measure* nextSegMeasure = nextSeg->measure();

        bool leadingNonCRException = ((nextSeg->isKeySigType() || nextSeg->isTimeSigType()
                                       || nextSeg->isClefType()) && !curSeg->isType(SegmentType::BarLineType));
        bool computeWidthByDifferenceFromNext = curSegMeasure == nextSegMeasure || nextSeg->isStartRepeatBarLineType()
                                                || leadingNonCRException;

        double segWidth = computeWidthByDifferenceFromNext ? nextX - curX : curSeg->minRight();

        curSeg->setWidth(segWidth);

        if (curSegMeasure != nextSegMeasure) {
            curSegMeasure->setWidth(curX + segWidth - curSegMeasure->x());
            nextSegMeasure->mutldata()->setPosX(curX + segWidth);
        }

        curSeg->setXPosInSystemCoords(curX);

        if (nextSeg == curSeg) {
            double nextSegWidth = 0.0;
            if (nextSeg->hasTimeSigAboveStaves()) {
                assert(i > 0);
                if (nextSeg->makeSpaceForTimeSigAboveStaves()) {
                    nextSegWidth = nextSeg->minRight();
                }
                Segment* prevSeg = segmentPositions[i - 1].segment;
                double xDiff = segmentPositions[i - 1].xPosInSystemCoords - curX;
                double minSpaceForPrevSeg = xDiff + prevSeg->minRight();
                if (prevSeg->trailer()) {
                    minSpaceForPrevSeg += prevSeg->style().styleMM(Sid::systemTrailerRightMargin);
                }
                nextSegWidth = std::max(nextSegWidth, minSpaceForPrevSeg);
            } else if (nextSeg->trailer()) {
                nextSegWidth = nextSeg->minRight() + nextSeg->style().styleMM(Sid::systemTrailerRightMargin);
            } else {
                nextSegWidth = nextSeg->minRight();
            }
            nextSeg->setWidth(nextSegWidth);
            nextSegMeasure->setWidth(nextX + nextSegWidth - nextSegMeasure->x());
            nextSeg->setXPosInSystemCoords(nextX);
        }
    }
}

double HorizontalSpacing::getFirstSegmentXPos(Segment* segment, HorizontalSpacingContext& ctx)
{
    const MStyle& style = segment->style();

    double x = 0.0;
    switch (segment->segmentType()) {
    case SegmentType::BeginBarLine:
        return 0.0;
    case SegmentType::ChordRest:
    {
        Shape leftBarrier(RectF(0.0, -0.5 * DBL_MAX, 0.0, DBL_MAX));
        x = minLeft(segment, leftBarrier);
        x += style.styleMM(segment->hasAccidentals() ? Sid::barAccidentalDistance : Sid::barNoteDistance);
        x = std::max(x, style.styleMM(Sid::systemHeaderMinStartOfSystemDistance).val());
        break;
    }
    case SegmentType::Clef:
    case SegmentType::HeaderClef:
        x = style.styleMM(Sid::clefLeftMargin);
        break;
    case SegmentType::KeySig:
        x = style.styleMM(Sid::keysigLeftMargin);
        break;
    case SegmentType::TimeSig:
        x = style.styleMM(Sid::timesigLeftMargin);
        break;
    default:
        x = 0.0;
    }

    return x * ctx.squeezeFactor;
}

double HorizontalSpacing::minHorizontalDistance(const Shape& f, const Shape& s, double spatium, double squeezeFactor)
{
    double dist = -DBL_MAX;        // min real
    double absoluteMinPadding = 0.1 * spatium * squeezeFactor;
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
            double verticalClearance = computeVerticalClearance(item1, item2, spatium) * squeezeFactor;
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

double HorizontalSpacing::minHorizontalDistance(const Segment* f, const Segment* ns, double squeezeFactor)
{
    if (f->segmentType() & SegmentType::BarLineType) {
        if (ns->isStartRepeatBarLineType()) {
            if (f->isBeginBarLineType()) {
                return 0.0;
            }
            double prevBarlineWidth = f->minRight();
            double thickBarlineWidth = f->style().styleMM(Sid::endBarWidth);
            return std::max(prevBarlineWidth - thickBarlineWidth, 0.0);
        }
        if (ns->isMMRestSegment()) {
            return f->minRight() + f->style().styleMM(Sid::barNoteDistance);
        }
    }

    if (f->isHeaderClefType() && ns->hasTimeSigAboveStaves()) {
        Segment* possibleKeySig = f->nextActive();
        if (possibleKeySig && possibleKeySig->isKeySigType() && !ignoreSegmentForSpacing(possibleKeySig)) {
            return 0.0;
        }
        return f->minRight() + ns->minLeft() + f->style().styleMM(Sid::headerToLineStartDistance);
    }

    bool systemHeaderGap = needsHeaderSpacingExceptions(f, ns);

    double ww = -DBL_MAX;          // can remain negative
    double d = 0.0;
    Score* score = f->score();
    for (unsigned staffIdx = 0; staffIdx < f->shapes().size(); ++staffIdx) {
        if (score->staff(staffIdx) && !score->staff(staffIdx)->show()) {
            continue;
        }

        const Shape& fshape = f->staffShape(staffIdx);
        double sp = shapeSpatium(fshape);
        d = minHorizontalDistance(fshape, ns->staffShape(staffIdx), sp, squeezeFactor);
        if (systemHeaderGap) {
            // first chordrest of a staff should clear the widest header for any staff
            // so make sure segment is as wide as it needs to be
            d = std::max(d, f->staffShape(staffIdx).right());
        }

        if (f->isChordRestType() && ns->isChordRestType()) {
            checkCollisionsWithCrossStaffStems(f, ns, staffIdx, d);
        }

        ww = std::max(ww, d);
    }
    double w = std::max(ww, 0.0);        // non-negative

    // Header exceptions that need additional space (more than the padding)
    double absoluteMinHeaderDist = 1.5 * f->spatium() * squeezeFactor;
    if (systemHeaderGap) {
        if (f->isTimeSigType()) {
            double timeSigHeaderDist = squeezeFactor * f->style().styleMM(Sid::systemHeaderTimeSigDistance);
            w = std::max(w, f->minRight() + timeSigHeaderDist);
        } else {
            double headerDist = squeezeFactor * f->style().styleMM(Sid::systemHeaderDistance);
            w = std::max(w, f->minRight() + headerDist);
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

    // Allocate space to ensure minimum length of hanging ties or gliss at start of system
    // These only occur when one segment is a ChordRest and the other isn't
    // Allocate space to ensure minimum length of partial ties
    if (f->isChordRestType() == ns->isChordRestType()) {
        return w;
    }

    const bool repeatSeg = f->isStartRepeatBarLineType() || ns->isStartRepeatBarLineType();
    const bool endOfSystem = f->isChordRestType() && !(ns->isChordRestType() || ns->isStartRepeatBarLineType())
                             && (f->measure()->isLastInSystem() || f->measure()->next()->isHBox());
    const bool systemEnd = repeatSeg || endOfSystem;

    computeHangingLineWidth(f, ns, w, systemHeaderGap, systemEnd);

    return w;
}

bool HorizontalSpacing::needsHeaderSpacingExceptions(const Segment* seg, const Segment* nextSeg)
{
    static const std::unordered_set<SegmentType> HEADER_SEGMENT_TYPES = { SegmentType::HeaderClef,
                                                                          SegmentType::KeySig,
                                                                          SegmentType::KeySigStartRepeatAnnounce,
                                                                          SegmentType::Ambitus,
                                                                          SegmentType::TimeSig,
                                                                          SegmentType::TimeSigStartRepeatAnnounce };

    return muse::contains(HEADER_SEGMENT_TYPES, seg->segmentType())
           && seg->rtick().isZero() && !seg->hasTimeSigAboveStaves()
           && (nextSeg->measure()->isFirstInSystem() || nextSeg->measure()->prev()->isHBox())
           && (nextSeg->isStartRepeatBarLineType() || nextSeg->isChordRestType());
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

double HorizontalSpacing::computePadding(const EngravingItem* item1, const EngravingItem* item2)
{
    const PaddingTable& paddingTable = item1->score()->paddingTable();
    ElementType type1 = item1->type();
    ElementType type2 = item2->type();

    double padding = paddingTable.at(type1).at(type2);
    double scaling = (item1->mag() + item2->mag()) / 2;

    if (type1 == ElementType::NOTE && isSpecialNotePaddingType(type2)) {
        computeNotePadding(toNote(item1), item2, padding, scaling);
    } else if (type1 == ElementType::LYRICS && isSpecialLyricsPaddingType(type2)) {
        computeLyricsPadding(toLyrics(item1), item2, padding);
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

    // Allocate space for minTieLength, minGlissandoLength & minBendLength
    for (LineAttachPoint laPoint1 : note->lineAttachPoints()) {
        if (!laPoint1.line()->addToSkyline()) {
            continue;
        }
        for (LineAttachPoint laPoint2 : note2->lineAttachPoints()) {
            if (laPoint1.line() != laPoint2.line()) {
                continue;
            }

            double minEndPointsDistance = 0.0;
            if (laPoint1.line()->isTie()) {
                minEndPointsDistance = style.styleMM(Sid::minTieLength);
            } else if (laPoint1.line()->isGlissando()) {
                bool straight = toGlissando(laPoint1.line())->glissandoType() == GlissandoType::STRAIGHT;
                double minGlissandoLength = straight
                                            ? style.styleMM(Sid::minStraightGlissandoLength)
                                            : style.styleMM(Sid::minWigglyGlissandoLength);
                minEndPointsDistance = minGlissandoLength;
            } else if (laPoint1.line()->isGuitarBend()) {
                double minBendLength = 2 * note->spatium(); // TODO: style
                minEndPointsDistance = minBendLength;
            } else if (laPoint1.line()->isNoteLine()) {
                minEndPointsDistance = style.styleMM(Sid::minStraightGlissandoLength);
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

bool HorizontalSpacing::isSpecialLyricsPaddingType(ElementType type)
{
    switch (type) {
    case ElementType::NOTE:
    case ElementType::REST:
    case ElementType::LYRICS:
        return true;
    default:
        return false;
    }
}

void HorizontalSpacing::computeLyricsPadding(const Lyrics* lyrics1, const EngravingItem* item2, double& padding)
{
    const MStyle& style = lyrics1->style();

    bool leaveSpaceForMelisma = lyrics1->separator() && lyrics1->separator()->isEndMelisma() && style.styleB(Sid::lyricsMelismaForce);
    if (leaveSpaceForMelisma) {
        double spaceForMelisma = style.styleMM(Sid::lyricsMelismaMinLength).val() + 2 * style.styleMM(Sid::lyricsMelismaPad).val();
        padding = std::max(padding, spaceForMelisma);
        return;
    }

    if (item2->isLyrics()) {
        LyricsSyllabic syllabicType = lyrics1->syllabic();
        bool leaveSpaceForDash = (syllabicType == LyricsSyllabic::BEGIN || syllabicType == LyricsSyllabic::MIDDLE)
                                 && style.styleB(Sid::lyricsDashForce);
        if (leaveSpaceForDash) {
            double spaceForDash = style.styleMM(Sid::lyricsDashMinLength).val() + 2 * style.styleMM(Sid::lyricsDashPad).val();
            padding = std::max(padding, spaceForDash);
        }
    }
}

KerningType HorizontalSpacing::computeKerning(const EngravingItem* item1, const EngravingItem* item2)
{
    if (ignoreItems(item1, item2)) {
        return KerningType::ALLOW_COLLISION;
    }

    if (isSameVoiceKerningLimited(item1) && isSameVoiceKerningLimited(item2) && item1->track() == item2->track()) {
        return KerningType::NON_KERNING;
    }

    if ((isNeverKernable(item1) || isNeverKernable(item2))
        && !(isAlwaysKernable(item1) || isAlwaysKernable(item2))) {
        return KerningType::NON_KERNING;
    }

    return doComputeKerningType(item1, item2);
}

double HorizontalSpacing::computeVerticalClearance(const EngravingItem* item1, const EngravingItem* item2, double spatium)
{
    // To be possibly expanded to more cases
    if (item2 && item2->isAccidental()) {
        return 0.1 * spatium;
    } else if ((item1 && item1->isParenthesis()) || (item2 && item2->isParenthesis())) {
        return 0.1 * spatium;
    }

    return 0.2 * spatium;
}

bool HorizontalSpacing::isSameVoiceKerningLimited(const EngravingItem* item)
{
    ElementType type = item->type();

    switch (type) {
    case ElementType::NOTE:
    case ElementType::NOTEDOT:
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
        if (toClef(item)->isMidMeasureClef()) {
            return false;
        }
    // fall through
    case ElementType::KEYSIG:
    case ElementType::BAR_LINE:
        return true;
    case ElementType::TIMESIG:
        return !toTimeSig(item)->isAboveStaves();
    default:
        return false;
    }
}

bool HorizontalSpacing::isAlwaysKernable(const EngravingItem* item)
{
    return item->isTextBase() || item->isChordLine() || item->isParenthesis() || item->isFretDiagram();
}

bool HorizontalSpacing::ignoreItems(const EngravingItem* item1, const EngravingItem* item2)
{
    if (item1->isRest() && toRest(item1)->isFullMeasureRest()) {
        // Full-measure rest must ignore cross-stave notes
        return item1->staffIdx() != item2->staffIdx();
    }

    return false;
}

KerningType HorizontalSpacing::doComputeKerningType(const EngravingItem* item1, const EngravingItem* item2)
{
    ElementType type1 = item1->type();
    switch (type1) {
    case ElementType::BAR_LINE:
        return item2->isLyrics() || item2->isHarmony() || item2->isFretDiagram() ? KerningType::ALLOW_COLLISION : KerningType::NON_KERNING;
    case ElementType::CHORDLINE:
        return item2->isBarLine() ? KerningType::ALLOW_COLLISION : KerningType::KERNING;
    case ElementType::HARMONY:
    case ElementType::FRET_DIAGRAM:
        return item2->isHarmony() || item2->isFretDiagram() ? KerningType::NON_KERNING : KerningType::KERNING;
    case ElementType::LYRICS
        : return computeLyricsKerningType(toLyrics(item1), item2);
    case ElementType::NOTE:
        return computeNoteKerningType(toNote(item1), item2);
    case ElementType::STEM_SLASH:
        return computeStemSlashKerningType(toStemSlash(item1), item2);
    case ElementType::PARENTHESIS:
        return item2->isBarLine() ? KerningType::NON_KERNING : KerningType::KERNING;
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
    if (!c) {
        return KerningType::KERNING;
    }
    if (item2->isLyrics() && c->isMelismaEnd()) {
        Note* melismaEndNote = c->up() ? c->downNote() : c->upNote();
        return note == melismaEndNote ? KerningType::NON_KERNING : KerningType::KERNING;
    }
    if (c->allowKerningAbove() && c->allowKerningBelow()) {
        return KerningType::KERNING;
    }

    if (c->up() && note->ldata()->pos().x() > 0) {
        // Offset seconds can always be kerned into
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

KerningType HorizontalSpacing::computeLyricsKerningType(const Lyrics* lyrics1, const EngravingItem* item2)
{
    if (item2->isLyrics()) {
        const Lyrics* lyrics2 = toLyrics(item2);
        if (lyrics1->no() == lyrics2->no()) {
            return KerningType::NON_KERNING;
        }
    }

    if ((item2->isNote() || item2->isRest()) && lyrics1->style().styleB(Sid::lyricsMelismaForce)) {
        LyricsLine* melismaLine = lyrics1->separator();
        if (melismaLine && melismaLine->isEndMelisma() && item2->tick() >= melismaLine->tick2()) {
            return KerningType::NON_KERNING;
        }
    }

    return KerningType::ALLOW_COLLISION;
}

void HorizontalSpacing::computeHangingLineWidth(const Segment* firstSeg, const Segment* nextSeg, double& width, bool systemHeaderGap,
                                                bool systemEnd)
{
    const MStyle& style = firstSeg->style();
    const Segment* crSeg = firstSeg->isChordRestType() ? firstSeg : nextSeg;
    const Segment* otherSeg = firstSeg->isChordRestType() ? nextSeg : firstSeg;
    const bool incoming = !firstSeg->isChordRestType();
    const Measure* crMeasure = crSeg->measure();
    const size_t ntracks = crSeg->score()->ntracks();

    std::vector<double> tieLengths;

    for (track_idx_t track = 0; track < ntracks; track++) {
        // Hanging lines only occur at start or end of a measure
        const ChordRest* cr = incoming ? crMeasure->firstChordRest(track) : crMeasure->lastChordRest(track);
        if (!cr || !cr->isChord() || cr->segment() != crSeg) {
            continue;
        }

        const double headerLineMargin = systemHeaderGap ? otherSeg->style().styleMM(Sid::headerToLineStartDistance)
                                        : otherSeg->style().styleMM(Sid::repeatBarlineDotSeparation);
        const double endSystemMargin = style.styleMM(Sid::lineEndToBarlineDistance);

        for (const Note* note : toChord(cr)->notes()) {
            const bool lineBack = note->spannerBack().size()
                                  || (note->tieBack() && !note->tieBack()->segmentsEmpty());
            const bool lineFor = note->spannerFor().size()
                                 || (note->tieFor() && !note->tieFor()->segmentsEmpty());
            if (!(lineBack || lineFor) || note->lineAttachPoints().empty()) {
                continue;
            }

            for (const LineAttachPoint& lap : note->lineAttachPoints()) {
                if (lap.start() == incoming) {
                    continue;
                }
                const Spanner* attachedLine = toSpanner(lap.line());
                if (!attachedLine->addToSkyline()) {
                    continue;
                }

                // Partial ties are adjusted wherever they are in the system, other spanners are only adjusted over system breaks
                if (!(systemEnd || systemHeaderGap) && !attachedLine->isPartialTie()) {
                    continue;
                }

                double minLength = 0.0;
                if (attachedLine->isGlissando()) {
                    bool straight = toGlissando(attachedLine)->glissandoType() == GlissandoType::STRAIGHT;
                    minLength = straight ? style.styleMM(Sid::minStraightGlissandoLength)
                                : style.styleMM(Sid::minWigglyGlissandoLength);
                } else if (attachedLine->isNoteLine()) {
                    minLength = style.styleMM(Sid::minStraightGlissandoLength);
                }

                const double notePosX = note->pos().x() + toChord(cr)->pos().x() + note->headWidth() / 2;
                const double lineNoteEndPos = (incoming ? width : 0.0) + notePosX + lap.pos().x();
                const double lineSegEndPos
                    = (incoming ? otherSeg->minRight() + headerLineMargin : width + otherSeg->minLeft() - endSystemMargin);

                const double lineStartPointX = incoming ? lineSegEndPos : lineNoteEndPos;
                const double lineEndPointX = incoming ? lineNoteEndPos : lineSegEndPos;
                const double lineLength = lineEndPointX - lineStartPointX;

                if (attachedLine->isTie()) {
                    tieLengths.push_back(lineLength);
                    continue;
                }

                if (lineLength < minLength) {
                    width += minLength - lineLength;
                }
            }
        }
    }

    if (tieLengths.empty()) {
        return;
    }

    const double maxLength = *std::max_element(tieLengths.begin(), tieLengths.end());
    const double oldWidth = width;
    for (double& tieLength : tieLengths) {
        // Adjust for new width
        tieLength += width - oldWidth;
        const double minLength = muse::RealIsEqual(tieLength, maxLength) ? style.styleMM(Sid::minHangingTieLength) : style.styleMM(
            Sid::minTieLength);

        if (tieLength < minLength) {
            width += minLength - tieLength;
        }
    }
}
