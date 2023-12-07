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

#include "tremolobeamlayout.h"

#include <array>

#include "../dom/beam.h"
#include "../dom/chordrest.h"
#include "../dom/note.h"
#include "../dom/score.h"
#include "../dom/staff.h"
#include "../dom/stem.h"
#include "../dom/stemslash.h"
#include "../dom/tremolo.h"

#include "tlayout.h"
#include "chordlayout.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

constexpr std::array _maxSlopes = { 0, 1, 2, 3, 4, 5, 6, 7 };

void TremoloBeamLayout::setupLData(Tremolo::LayoutData* info, EngravingItem* e)
{
    IF_ASSERT_FAILED(e && e->isTremolo()) {
        return;
    }

    info->trem = toTremolo(e);
    // check to see if there is a beam happening during this trem
    // if so, it needs to be taken into account in trem placement
    if (info->trem->chord1()->beam() && info->trem->chord1()->beam() == info->trem->chord2()->beam()) {
        info->beam = info->trem->chord1()->beam();
    } else {
        info->beam = nullptr;
    }
    info->up = computeTremoloUp(info);
    info->trem->setUp(info->up);
    bool isGrace = info->trem->chord1()->isGrace();

    info->spatium = e->spatium();
    info->tick = info->trem->tick();
    info->beamSpacing = e->style().styleB(Sid::useWideBeams) ? 4 : 3;
    info->beamDist = (info->beamSpacing / 4.0) * info->spatium * e->mag() * (isGrace ? e->style().styleD(Sid::graceNoteMag) : 1.);
    info->beamWidth = (e->style().styleS(Sid::beamWidth).val() * info->spatium) * e->mag();
    const StaffType* staffType = e->staffType();
    info->tab = (staffType && staffType->isTabStaff()) ? staffType : nullptr;
    info->isBesideTabStaff = info->tab && !info->tab->stemless() && !info->tab->stemThrough();
}

void TremoloBeamLayout::offsetBeamToRemoveCollisions(const Tremolo::LayoutData* info, const std::vector<ChordRest*> chordRests,
                                                     int& dictator,
                                                     int& pointer,
                                                     const double startX, const double endX,
                                                     bool isFlat, bool isStartDictator)
{
    if (endX == startX) {
        return;
    }

    // tolerance eliminates all possibilities of floating point rounding errors
    const double tolerance = info->beamWidth * 0.25 * (info->up ? -1 : 1);
    bool isSmall = info->isGrace || info->trem->mag() < 1.;

    double startY = (isStartDictator ? dictator : pointer) * info->spatium / 4 + tolerance;
    double endY = (isStartDictator ? pointer : dictator) * info->spatium / 4 + tolerance;

    for (ChordRest* chordRest : chordRests) {
        if (!chordRest->isChord() || chordRest == info->elements.back() || chordRest == info->elements.front()) {
            continue;
        }

        PointF anchor = chordBeamAnchor(info, chordRest, ChordBeamAnchorType::Middle) - info->trem->pagePos();

        int slope = abs(dictator - pointer);
        double reduction = 0.0;
        if (!isFlat) {
            if (slope <= 3) {
                reduction = 0.25 * info->spatium;
            } else if (slope <= 6) {
                reduction = 0.5 * info->spatium;
            } else { // slope > 6
                reduction = 0.75 * info->spatium;
            }
        }

        if (endX != startX) {
            // avoid division by zero for zero-length beams (can exist as a pre-layout state used
            // for horizontal spacing computations)
            double proportionAlongX = (anchor.x() - startX) / (endX - startX);

            while (true) {
                double desiredY = proportionAlongX * (endY - startY) + startY;
                bool beamClearsAnchor = (info->up && RealIsEqualOrLess(desiredY, anchor.y() + reduction))
                                        || (!info->up && RealIsEqualOrMore(desiredY, anchor.y() - reduction));
                if (beamClearsAnchor) {
                    break;
                }

                if (isFlat || (isSmall && dictator == pointer)) {
                    dictator += info->up ? -1 : 1;
                    pointer += info->up ? -1 : 1;
                } else if (std::abs(dictator - pointer) == 1) {
                    dictator += info->up ? -1 : 1;
                } else {
                    pointer += info->up ? -1 : 1;
                }

                startY = (isStartDictator ? dictator : pointer) * info->spatium / 4 + tolerance;
                endY = (isStartDictator ? pointer : dictator) * info->spatium / 4 + tolerance;
            }
        }
    }
}

void TremoloBeamLayout::offsetBeamWithAnchorShortening(const Tremolo::LayoutData* info, std::vector<ChordRest*> chordRests, int& dictator,
                                                       int& pointer, int staffLines,
                                                       bool isStartDictator, int stemLengthDictator)
{
    Chord* startChord = nullptr;
    Chord* endChord = nullptr;
    for (ChordRest* cr : chordRests) {
        if (cr->isChord()) {
            endChord = toChord(cr);
            if (!startChord) {
                startChord = toChord(cr);
            }
        }
    }
    if (!startChord) {
        // beam full of only rests, don't adjust this
        return;
    }
    // min stem lengths according to how many beams there are (starting with 1)
    static const int minStemLengths[] = { 11, 13, 15, 18, 21, 24, 27, 30 };
    const int middleLine = getMiddleStaffLine(info, startChord, endChord, staffLines);
    int dictatorBeams = strokeCount(info, isStartDictator ? startChord : endChord);
    int pointerBeams = strokeCount(info, isStartDictator ? endChord : startChord);
    int maxDictatorReduce = stemLengthDictator - minStemLengths[std::max(dictatorBeams - 1, 0)];
    maxDictatorReduce = std::min(abs(dictator - middleLine), maxDictatorReduce);

    bool isFlat = dictator == pointer;
    bool isAscending = startChord->line() > endChord->line();
    int towardBeam = info->up ? -1 : 1;
    int newDictator = dictator;
    int newPointer = pointer;
    int reduce = 0;
    auto fourBeamException = [](int beams, int yPos) {
        yPos += 400; // because there is some weirdness with modular division around zero, add
                     // a large multiple of 4 so that we can guarantee that yPos%4 will be correct
        return beams >= 4 && (yPos % 4 == 2);
    };
    while (!fourBeamException(dictatorBeams, newDictator)
           && !isValidBeamPosition(info->up, newDictator, isStartDictator, isAscending, isFlat, staffLines, true)) {
        if (++reduce > maxDictatorReduce) {
            // we can't shorten this stem at all. bring it back to default and start extending
            newDictator = dictator;
            newPointer = pointer;
            while (!isValidBeamPosition(info->up, newDictator, isStartDictator, isAscending, isFlat, staffLines, true)) {
                newDictator += towardBeam;
                newPointer += towardBeam;
            }
            break;
        }
        newDictator += -towardBeam;
        newPointer += -towardBeam;
    }

    // newDictator is guaranteed either valid, or ==dictator
    // first, constrain pointer to valid position
    newPointer = info->up ? std::min(newPointer, middleLine) : std::max(newPointer, middleLine);
    // walk it back beamwards until we get a position that satisfies both pointer and dictator
    while (!fourBeamException(dictatorBeams, newDictator) && !fourBeamException(pointerBeams, newPointer)
           && (!isValidBeamPosition(info->up, newDictator, isStartDictator, isAscending, isFlat, staffLines, true)
               || !isValidBeamPosition(info->up, newPointer, !isStartDictator, isAscending, isFlat, staffLines, true))) {
        if (isFlat) {
            newDictator += towardBeam;
            newPointer += towardBeam;
        } else if (std::abs(newDictator - newPointer) == 1) {
            newDictator += towardBeam;
        } else {
            newPointer += towardBeam;
        }
    }
    dictator = newDictator;
    pointer = newPointer;
}

void TremoloBeamLayout::extendStem(const Tremolo::LayoutData* info, Chord* chord, double addition)
{
    PointF anchor = chordBeamAnchor(info, chord, ChordBeamAnchorType::Middle);
    double desiredY;
    if (info->endAnchor.x() > info->startAnchor.x()) {
        double proportionAlongX = (anchor.x() - info->startAnchor.x()) / (info->endAnchor.x() - info->startAnchor.x());
        desiredY = proportionAlongX * (info->endAnchor.y() - info->startAnchor.y()) + info->startAnchor.y();
    } else {
        desiredY = std::max(info->endAnchor.y(), info->startAnchor.y());
    }

    if (chord->up()) {
        chord->setBeamExtension(anchor.y() - desiredY + addition);
    } else {
        chord->setBeamExtension(desiredY - anchor.y() + addition);
    }
    if (chord->stemSlash()) {
        LayoutContext ctx(chord->stemSlash()->score());
        TLayout::layoutStemSlash(chord->stemSlash(), chord->stemSlash()->mutldata(), ctx.conf());
    }
}

bool TremoloBeamLayout::isBeamInsideStaff(int yPos, int staffLines, bool allowFloater)
{
    int aboveStaff = allowFloater ? -2 : -3;
    int belowStaff = (staffLines - 1) * 4 + (allowFloater ? 2 : 3);
    return yPos > aboveStaff && yPos < belowStaff;
}

int TremoloBeamLayout::getOuterBeamPosOffset(const Tremolo::LayoutData* info, int innerBeam, int beamCount, int staffLines)
{
    int spacing = (info->up ? -info->beamSpacing : info->beamSpacing);
    int offset = (beamCount - 1) * spacing;
    bool isInner = false;
    while (offset != 0 && !isBeamInsideStaff(innerBeam + offset, staffLines, isInner)) {
        offset -= spacing;
        isInner = true;
    }
    return offset;
}

bool TremoloBeamLayout::isValidBeamPosition(const bool isUp, int yPos, bool isStart, bool isAscending, bool isFlat, int staffLines,
                                            bool isOuter)
{
    // outside the staff
    bool slantsAway = (isUp && isAscending == isStart) || (!isUp && isAscending != isStart);
    if (!isBeamInsideStaff(yPos, staffLines, isOuter && (slantsAway || isFlat))) {
        return true;
    }

    // removes modulo weirdness with negative numbers (i.e., right above staff)
    yPos += 8;
    // is floater
    if (yPos % 4 == 2) {
        return false;
    }
    if (isFlat) {
        return true;
    }
    // is on line
    if (yPos % 4 == 0) {
        return true;
    }
    // is sitting
    if (yPos % 4 == 3) {
        // return true only if we're starting here and descending, or ascending and ending here
        return isAscending != isStart;
    }
    // is hanging
    // return true only if we're starting here and ascending, or decending and ending here
    return isAscending == isStart;
}

bool TremoloBeamLayout::is64thBeamPositionException(const int beamSpacing, int& yPos, int staffLines)
{
    if (beamSpacing == 4) {
        return false;
    }
    return yPos == 2 || yPos == staffLines * 4 - 2 || yPos == staffLines * 4 - 6 || yPos == -2;
}

int TremoloBeamLayout::findValidBeamOffset(const Tremolo::LayoutData* info, int outer, int beamCount, int staffLines, bool isStart,
                                           bool isAscending, bool isFlat)
{
    bool isBeamValid = false;
    int offset = 0;
    int innerBeam = outer + (beamCount - 1) * (info->up ? info->beamSpacing : -info->beamSpacing);
    while (!isBeamValid) {
        while (!isValidBeamPosition(info->up, innerBeam + offset, isStart, isAscending, isFlat, staffLines, beamCount < 2)) {
            offset += info->up ? -1 : 1;
        }
        int outerMostBeam = innerBeam + offset + getOuterBeamPosOffset(info, innerBeam + offset, beamCount, staffLines);
        if (isValidBeamPosition(info->up, outerMostBeam, isStart, isAscending, isFlat, staffLines, true)
            || (beamCount == 4 && is64thBeamPositionException(info->beamSpacing, outerMostBeam, staffLines))) {
            isBeamValid = true;
        } else {
            offset += info->up ? -1 : 1;
        }
    }
    return offset;
}

void TremoloBeamLayout::setValidBeamPositions(const Tremolo::LayoutData* info, int& dictator, int& pointer, int beamCountD, int beamCountP,
                                              int staffLines,
                                              bool isStartDictator,
                                              bool isFlat, bool isAscending)
{
    bool areBeamsValid = false;
    bool has3BeamsInsideStaff = beamCountD >= 3 || beamCountP >= 3;
    while (!areBeamsValid && has3BeamsInsideStaff && info->beamSpacing != 4) {
        int dictatorInner = dictator + (beamCountD - 1) * (info->up ? info->beamSpacing : -info->beamSpacing);
        // use dictatorInner for both to simulate flat beams
        int outerDictatorOffset = getOuterBeamPosOffset(info, dictatorInner, beamCountD, staffLines);
        if (std::abs(outerDictatorOffset) <= info->beamSpacing) {
            has3BeamsInsideStaff = false;
            break;
        }
        int offsetD = findValidBeamOffset(info, dictator, beamCountD, staffLines, isStartDictator, false, true);
        int offsetP = findValidBeamOffset(info, pointer, beamCountP, staffLines, isStartDictator, false, true);
        int offset = (offsetD == 0 ? offsetP : offsetD);
        if (pointer == dictator) {
            dictator += offset;
        }
        pointer = dictator;
        if (offset == 0) {
            areBeamsValid = true;
        }
    }
    if (isFlat) {
        // flat beams need more checks (non-dictator/pointer notes with floater inner beams)
        areBeamsValid = false;
    }
    while (!areBeamsValid) {
        int dictatorOffset = findValidBeamOffset(info, dictator, beamCountD, staffLines, isStartDictator, isAscending, isFlat);
        dictator += dictatorOffset;
        pointer += dictatorOffset;
        if (isFlat) {
            pointer = dictator;
            int currOffset = 0;
            for (ChordRest* cr : info->elements) {
                if (!cr->isChord() && (cr != info->elements.front() && cr != info->elements.back())) {
                    continue;
                }
                // we can use dictator beam position because all of the notes have the same beam position
                int beamCount = strokeCount(info, cr);
                currOffset = findValidBeamOffset(info, dictator, beamCount, staffLines, isStartDictator, isAscending, isFlat);
                if (currOffset) {
                    break;
                }
            }

            if (currOffset == 0) {
                areBeamsValid = true;
            } else {
                dictator += currOffset;
                pointer += currOffset;
            }
        } else {
            pointer += findValidBeamOffset(info, pointer, beamCountP, staffLines, !isStartDictator, isAscending, isFlat);
            if ((info->up && pointer <= dictator) || (!info->up && pointer >= dictator)) {
                dictator = pointer + (info->up ? -1 : 1);
            } else {
                areBeamsValid = true;
            }
        }
    }
}

void TremoloBeamLayout::addMiddleLineSlant(const Tremolo::LayoutData* info, int& dictator, int& pointer, int beamCount, int middleLine,
                                           int interval, int desiredSlant)
{
    bool isSmall = info->trem->mag() < 1. || info->isGrace;
    if (interval == 0 || (!isSmall && beamCount > 2 && info->beamSpacing != 4) || noSlope(info->beam)) {
        return;
    }
    bool isOnMiddleLine = pointer == middleLine && (std::abs(pointer - dictator) < 2);
    if (isOnMiddleLine) {
        if (abs(desiredSlant) == 1 || interval == 1 || (beamCount == 2 && info->beamSpacing != 4 && !isSmall)) {
            dictator = middleLine + (info->up ? -1 : 1);
        } else {
            dictator = middleLine + (info->up ? -2 : 2);
        }
    }
}

void TremoloBeamLayout::add8thSpaceSlant(Tremolo::LayoutData* info, PointF& dictatorAnchor, int dictator, int pointer, int beamCount,
                                         int interval, int middleLine, bool isFlat)
{
    if (beamCount != 3 || noSlope(info->beam) || info->beamSpacing != 3) {
        return;
    }
    if ((isFlat && dictator != middleLine) || (dictator != pointer) || interval == 0) {
        return;
    }
    if ((info->up && (dictator + 4) % 4 == 3) || (!info->up && (dictator + 4) % 4 == 1)) {
        return;
    }
    dictatorAnchor.setY(dictatorAnchor.y() + (info->up ? -0.125 * info->spatium : 0.125 * info->spatium));
    info->beamDist += 0.0625 * info->spatium;
}

bool TremoloBeamLayout::computeTremoloUp(const Tremolo::LayoutData* info)
{
    if (!info->beam || !info->beam->cross()) {
        return info->trem->up();
    }
    Chord* c1 = info->trem->chord1();
    Chord* c2 = info->trem->chord2();
    int staffMove = 0;
    for (ChordRest* cr : info->beam->elements()) {
        if (cr->staffMove() != 0) {
            staffMove = cr->staffMove();
            break;
        }
    }
    // for inset trems in cross-staff beams, there are two potentialities:
    // 1) both trem notes are on the same staff
    if (c1->staffMove() == c2->staffMove()) {
        // (in which case, up is opposite to the move)
        if (staffMove < 0) {
            return c1->staffMove() != staffMove;
        } else {
            return c1->staffMove() == staffMove;
        }
    } else {
        // or 2) the trem notes are between staves. what a pain.
        return info->trem->up();
    }
}

int TremoloBeamLayout::strokeCount(const Tremolo::LayoutData* info, ChordRest* cr)
{
    if (cr->isRest()) {
        return cr->beams();
    }

    IF_ASSERT_FAILED(info->trem) {
        return 0;
    }

    int strokes = info->trem->lines();

    strokes += info->beam ? cr->beams() : 0;
    return strokes;
}

bool TremoloBeamLayout::calculateAnchors(Tremolo::LayoutData* info, const std::vector<ChordRest*>& chordRests,
                                         const std::vector<int>& notes)
{
    IF_ASSERT_FAILED(info->trem) {
        return false;
    }

    info->startAnchor = PointF();
    info->endAnchor = PointF();
    if (info->beam) {
        // this is a trem inside a beam, and we are currently calculating the anchors of the tremolo.
        // we can do this as long as the beam has been layed out first: it saves trem anchor positions
        ChordRest* cr1 = info->trem->chord1();
        ChordRest* cr2 = info->trem->chord2();
        double anchorX1 = chordBeamAnchorX(info, cr1, ChordBeamAnchorType::Middle);
        double anchorX2 = chordBeamAnchorX(info, cr2, ChordBeamAnchorType::Middle);
        for (const TremAnchor& t : info->beam->tremAnchors()) {
            if (t.chord1 == cr1) {
                info->startAnchor = PointF(anchorX1, t.y1);
                info->endAnchor = PointF(anchorX2, t.y2);
                info->slope = (t.y2 - t.y1) / (anchorX2 - anchorX1);
                return true;
            }
        }
        // if we finish the loop before we find the anchor for this tremolo, it could mean that the
        // beam hasn't been laid out yet
        return false;
    }
    Chord* startChord = nullptr;
    Chord* endChord = nullptr;
    ChordRest* startCr = nullptr;
    ChordRest* endCr = nullptr;
    info->elements = chordRests;
    if (chordRests.empty()) {
        return false;
    }

    for (auto chordRest : chordRests) {
        if (!startCr) {
            startCr = chordRest;
        }
        endCr = chordRest;
        if (chordRest->isChord()) {
            if (!startChord) {
                startChord = toChord(chordRest);
                endChord = startChord;
            } else {
                endChord = toChord(chordRest);
            }
            LayoutContext ctx(toChord(chordRest)->score());
            ChordLayout::layoutStem(toChord(chordRest), ctx);
        }
    }
    if (!startChord) {
        // we were passed a vector of only rests. we don't support beams across only rests
        // this beam will be deleted in LayoutBeams
        return false;
    }
    info->isGrace = startChord->isGrace();
    info->notes = notes;
    if (calculateAnchorsCross(info)) {
        return true;
    }

    info->startAnchor = chordBeamAnchor(info, startChord, ChordBeamAnchorType::Start);
    info->endAnchor = chordBeamAnchor(info, endChord, ChordBeamAnchorType::End);

    double startLength = startChord->defaultStemLength();
    double endLength = endChord->defaultStemLength();
    double startAnchorBase = info->startAnchor.y() + (info->up ? startLength : -startLength);
    double endAnchorBase = info->endAnchor.y() + (info->up ? endLength : -endLength);
    int startNote = info->up ? startChord->upNote()->line() : startChord->downNote()->line();
    int endNote = info->up ? endChord->upNote()->line() : endChord->downNote()->line();
    if (info->tab) {
        startNote = info->up ? startChord->upString() : startChord->downString();
        endNote = info->up ? endChord->upString() : endChord->downString();
    }
    const int interval = std::abs(startNote - endNote);
    const bool isStartDictator = info->up ? startNote < endNote : startNote > endNote;
    const double quarterSpace = info->spatium / 4;
    PointF startAnchor = info->startAnchor - info->trem->pagePos();
    PointF endAnchor = info->endAnchor - info->trem->pagePos();
    int dictator = round((isStartDictator ? startAnchor.y() : endAnchor.y()) / quarterSpace);
    int pointer = round((isStartDictator ? endAnchor.y() : startAnchor.y()) / quarterSpace);

    const int staffLines = startChord->staff()->lines(info->tick);
    const int middleLine = getMiddleStaffLine(info, startChord, endChord, staffLines);

    int slant = computeDesiredSlant(info, startNote, endNote, middleLine, dictator, pointer);
    bool isFlat = slant == 0;
    SlopeConstraint specialSlant = isFlat ? getSlopeConstraint(info, startNote, endNote) : SlopeConstraint::NO_CONSTRAINT;
    bool forceFlat = specialSlant == SlopeConstraint::FLAT;
    bool smallSlant = specialSlant == SlopeConstraint::SMALL_SLOPE;
    if (isFlat) {
        dictator = info->up ? std::min(pointer, dictator) : std::max(pointer, dictator);
        pointer = dictator;
    } else {
        if ((dictator > pointer) != (isStartDictator ? startNote > endNote : endNote > startNote)) {
            dictator = pointer - slant;
        } else {
            pointer = dictator + slant;
        }
    }
    bool isAscending = startNote > endNote;
    int beamCountD = strokeCount(info, isStartDictator ? startChord : endChord);
    int beamCountP = strokeCount(info, isStartDictator ? endChord : startChord);

    int stemLengthStart = abs(round((startAnchorBase - info->startAnchor.y()) / info->spatium * 4));
    int stemLengthEnd = abs(round((endAnchorBase - info->endAnchor.y()) / info->spatium * 4));
    int stemLengthDictator = isStartDictator ? stemLengthStart : stemLengthEnd;
    bool isSmall = info->trem->mag() < 1. || info->isGrace;
    if (endAnchor.x() > startAnchor.x()) {
        /* When beam layout is called before horizontal spacing (see LayoutMeasure::getNextMeasure() to
         * know why) the x positions aren't yet determined and may be all zero, which would cause the
         * following function to get stuck in a loop. The if() condition avoids that case. */
        if (!isSmall) {
            // Adjust anchor stems
            offsetBeamWithAnchorShortening(info, chordRests, dictator, pointer, staffLines, isStartDictator, stemLengthDictator);
        }
        // Adjust inner stems
        offsetBeamToRemoveCollisions(info, chordRests, dictator, pointer, startAnchor.x(), endAnchor.x(), isFlat, isStartDictator);
    }
    int beamCount = std::max(beamCountD, beamCountP);
    if (!info->tab) {
        if (!info->isGrace) {
            setValidBeamPositions(info, dictator, pointer, beamCountD, beamCountP, staffLines, isStartDictator, isFlat, isAscending);
        }
        if (!forceFlat) {
            addMiddleLineSlant(info, dictator, pointer, beamCount, middleLine, interval, smallSlant ? 1 : slant);
        }
    }

    info->startAnchor.setY(quarterSpace * (isStartDictator ? dictator : pointer) + info->trem->pagePos().y());
    info->endAnchor.setY(quarterSpace * (isStartDictator ? pointer : dictator) + info->trem->pagePos().y());

    bool add8th = true;

    if (!info->tab && add8th) {
        add8thSpaceSlant(info, isStartDictator ? info->startAnchor : info->endAnchor, dictator, pointer, beamCount, interval,
                         middleLine, isFlat);
    }
    info->startAnchor.setX(chordBeamAnchorX(info, startCr, ChordBeamAnchorType::Start));
    info->endAnchor.setX(chordBeamAnchorX(info, endCr, ChordBeamAnchorType::End));
    return true;
}

bool TremoloBeamLayout::calculateAnchorsCross(Tremolo::LayoutData* info)
{
    double spatium = info->trem->style().spatium();
    //int fragmentIndex = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    ChordRest* startCr = info->elements.front();
    ChordRest* endCr = info->elements.back();

    const double quarterSpace = spatium / 4;
    // imagine a line of beamed notes all in a row on the same staff. the first and last of those
    // are the 'outside' notes, and the slant of the beam is going to be affected by the 'middle' notes
    // between them.
    // we have to keep track of this for both staves.
    Chord* topFirst = nullptr;
    Chord* topLast = nullptr;
    Chord* bottomFirst = nullptr;
    Chord* bottomLast = nullptr;
    int maxMiddleTopLine = std::numeric_limits<int>::min(); // lowest note in the top staff
    int minMiddleBottomLine = std::numeric_limits<int>::max(); // highest note in the bottom staff
    int prevTopLine = maxMiddleTopLine; // previous note's line position (top)
    int prevBottomLine = minMiddleBottomLine; // previous note's line position (bottom)
    // if the immediate neighbor of one of the two 'outside' notes on either the top or bottom
    // are the same as that outside note, we need to record it so that we can add a 1/4 space slant.
    bool secondTopIsSame = false;
    bool secondBottomIsSame = false;
    bool penultimateTopIsSame = false;
    bool penultimateBottomIsSame = false;
    double maxY = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::min();
    int otherStaff = 0;
    bool isFirstChord = true;
    int firstChordStaff = 0;
    bool allOneStaff = true;
    for (ChordRest* c : info->elements) {
        if (!c || !c->isChord()) {
            continue;
        }
        if (otherStaff == 0) {
            otherStaff = c->staffMove();
        }
        if (isFirstChord) {
            firstChordStaff = otherStaff;
            isFirstChord = false;
        } else {
            if (c->staffMove() != firstChordStaff) {
                allOneStaff = false;
                break; // this beam is cross
            }
        }
    }
    if (otherStaff == 0 || allOneStaff) {
        return false; // not cross
    }
    // Find the notes on the top and bottom of staves
    //
    bool checkNextTop = false;
    bool checkNextBottom = false;
    for (ChordRest* cr : info->elements) {
        if (!cr->isChord()) {
            continue;
        }
        Chord* c = toChord(cr);
        if ((c->staffMove() == otherStaff && otherStaff > 0) || (c->staffMove() != otherStaff && otherStaff < 0)) {
            // this chord is on the bottom staff
            if (penultimateBottomIsSame) {
                // the chord we took as the penultimate bottom note wasn't.
                // so treat it properly as a middle note
                minMiddleBottomLine = std::min(minMiddleBottomLine, prevBottomLine);
                penultimateBottomIsSame = false;
            }
            checkNextTop = false; // we are no longer looking for the second note in the top
                                  // staff being the same as the first--this note is on the bottom.
            if (!bottomFirst) {
                bottomFirst = c;
                checkNextBottom = true; // this was the first bottom note, so check for second next time
            } else {
                penultimateBottomIsSame = prevBottomLine == c->line();
                if (!penultimateBottomIsSame) {
                    minMiddleBottomLine = std::min(minMiddleBottomLine, prevBottomLine);
                }
                if (checkNextBottom) {
                    // this is the second bottom note, so we should see if this one is same line as first
                    secondBottomIsSame = c->line() == bottomFirst->line();
                    checkNextBottom = false;
                } else {
                    prevBottomLine = c->line();
                }
                bottomLast = c;
            }
            maxY = std::min(maxY, chordBeamAnchorY(info, toChord(c)));
        } else {
            // this chord is on the top staff
            if (penultimateTopIsSame) {
                // the chord we took as the penultimate top note wasn't.
                // so treat it properly as a middle note
                maxMiddleTopLine = std::max(maxMiddleTopLine, prevTopLine);
                penultimateTopIsSame = false;
            }
            checkNextBottom = false; // no longer looking for a bottom second note since this is on top
            if (!topFirst) {
                topFirst = c;
                checkNextTop = true;
            } else {
                penultimateTopIsSame = prevTopLine == c->line();
                if (!penultimateTopIsSame) {
                    maxMiddleTopLine = std::max(maxMiddleTopLine, prevTopLine);
                }
                if (checkNextTop) {
                    secondTopIsSame = c->line() == topFirst->line();
                    checkNextTop = false;
                } else {
                    prevTopLine = c->line();
                }
                topLast = c;
            }
            minY = std::max(minY, chordBeamAnchorY(info, toChord(c)));
        }
    }

    info->startAnchor.setY((maxY + minY) / 2);
    info->endAnchor.setY((maxY + minY) / 2);
    info->startAnchor.setX(chordBeamAnchorX(info, startCr, ChordBeamAnchorType::Start));
    info->endAnchor.setX(chordBeamAnchorX(info, endCr, ChordBeamAnchorType::End));

    info->slope = 0;

    if (!noSlope(info->beam)) {
        int topFirstLine = topFirst ? topFirst->downNote()->line() : 0;
        int topLastLine = topLast ? topLast->downNote()->line() : 0;
        int bottomFirstLine = bottomFirst ? bottomFirst->upNote()->line() : 0;
        int bottomLastLine = bottomLast ? bottomLast->upNote()->line() : 0;
        bool constrainTopToQuarter = false;
        bool constrainBottomToQuarter = false;
        if ((topFirstLine > topLastLine && secondTopIsSame)
            || (topFirstLine < topLastLine && penultimateTopIsSame)) {
            constrainTopToQuarter = true;
        }
        if ((bottomFirstLine < bottomLastLine && secondBottomIsSame)
            || (bottomFirstLine > bottomLastLine && penultimateBottomIsSame)) {
            constrainBottomToQuarter = true;
        }
        if (!topLast && !bottomLast && topFirst && bottomFirst) {
            // if there are only two notes, one on each staff, special case
            // take max slope into account
            double yFirst, yLast;
            // we can't rely on comparing topFirst and bottomFirst ->tick() because beamed
            // graces have the same tick
            if (info->elements[0] == topFirst) {
                yFirst = topFirst->stemPos().y();
                yLast = bottomFirst->stemPos().y();
            } else {
                yFirst = bottomFirst->stemPos().y();
                yLast = topFirst->stemPos().y();
            }
            int desiredSlant = round((yFirst - yLast) / spatium);
            int slant = std::min(std::abs(desiredSlant), getMaxSlope(info));
            slant *= (desiredSlant < 0) ? -quarterSpace : quarterSpace;
            info->startAnchor.ry() += (slant / 2);
            info->endAnchor.ry() -= (slant / 2);
        } else if (!topLast || !bottomLast) {
            // otherwise, if there is only one note on one of the staves, use slope from other staff
            int startNote = 0;
            int endNote = 0;
            bool forceHoriz = false;
            if (!topLast) {
                startNote = bottomFirstLine;
                endNote = bottomLastLine;
                if (minMiddleBottomLine <= std::min(startNote, endNote)) {
                    // there is a note closer to the beam than the start and end notes
                    // we force horizontal beam here.
                    forceHoriz = true;
                }
            } else if (!bottomLast) {
                startNote = topFirstLine;
                endNote = topLastLine;
                if (maxMiddleTopLine >= std::max(startNote, endNote)) {
                    // same as above, for the top staff
                    // force horizontal.
                    forceHoriz = true;
                }
            }

            if (!forceHoriz) {
                int slant = startNote - endNote;
                slant = std::min(std::abs(slant), getMaxSlope(info));
                if ((!bottomLast && constrainTopToQuarter) || (!topLast && constrainBottomToQuarter)) {
                    slant = 1;
                }
                double slope = slant * (startNote > endNote ? quarterSpace : -quarterSpace);
                info->startAnchor.ry() += (slope / 2);
                info->endAnchor.ry() -= (slope / 2);
            } // otherwise, do nothing, beam is already horizontal.
        } else {
            // otherwise, there are at least two notes on each staff
            // (that is, topLast and bottomLast are both set)
            bool forceHoriz = false;
            if (topFirstLine == topLastLine || bottomFirstLine == bottomLastLine) {
                // if outside notes on top or bottom staff are on the same staff line, slope = 0
                // no further adjustment needed, the beam is already well-placed and horizontal
                forceHoriz = true;
            }
            // otherwise, we have to compare the slopes from the top staff and bottom staff.
            int topSlant = topFirstLine - topLastLine;
            if (constrainTopToQuarter && topSlant != 0) {
                topSlant = topFirstLine < topLastLine ? -1 : 1;
            }
            int bottomSlant = bottomFirstLine - bottomLastLine;
            if (constrainBottomToQuarter && bottomSlant != 0) {
                bottomSlant = bottomFirstLine < bottomLastLine ? -1 : 1;
            }
            if ((maxMiddleTopLine >= std::max(topFirstLine, topLastLine)
                 || (minMiddleBottomLine <= std::min(bottomFirstLine, bottomLastLine)))) {
                forceHoriz = true;
            }
            if (topSlant == 0 || bottomSlant == 0 || forceHoriz) {
                // if one of the slants is 0, the whole slant is zero
            } else if ((topSlant < 0 && bottomSlant < 0) || (topSlant > 0 && bottomSlant > 0)) {
                int slant = (abs(topSlant) < abs(bottomSlant)) ? topSlant : bottomSlant;
                slant = std::min(std::abs(slant), getMaxSlope(info));
                double slope = slant * ((topSlant < 0) ? -quarterSpace : quarterSpace);
                info->startAnchor.ry() += (slope / 2);
                info->endAnchor.ry() -= (slope / 2);
            } else {
                // if the two slopes are in opposite directions, flat!
                // nothing needs to be done, the beam is already horizontal and placed nicely
            }
        }
        info->slope = (info->endAnchor.y() - info->startAnchor.y()) / (info->endAnchor.x() - info->startAnchor.x());
    }
    return true;
}

bool TremoloBeamLayout::noSlope(const Beam* beam)
{
    return beam && beam->noSlope();
}

int TremoloBeamLayout::getMiddleStaffLine(const Tremolo::LayoutData* info, ChordRest* startChord, ChordRest* endChord, int staffLines)
{
    bool isFullSize = RealIsEqual(info->trem->mag(), 1.0) && !info->isGrace;
    bool useWideBeams = info->trem->score()->style().styleB(Sid::useWideBeams);
    int startBeams = strokeCount(info, startChord);
    int endBeams = strokeCount(info, endChord);
    int startMiddleLine = Chord::minStaffOverlap(info->up, staffLines, startBeams, false,
                                                 info->beamSpacing / 4.0, useWideBeams, isFullSize);
    int endMiddleLine = Chord::minStaffOverlap(info->up, staffLines, endBeams, false,
                                               info->beamSpacing / 4.0, useWideBeams, !info->isGrace);

    // offset middle line by 1 or -1 since the anchor is at the middle of the beam,
    // not at the tip of the stem
    if (info->up) {
        return std::min(startMiddleLine, endMiddleLine) + 1;
    }
    return std::max(startMiddleLine, endMiddleLine) - 1;
}

int TremoloBeamLayout::computeDesiredSlant(const Tremolo::LayoutData* info, int startNote, int endNote, int middleLine, int dictator,
                                           int pointer)
{
    int dictatorExtension = middleLine - dictator; // we need to make sure that beams extended to the middle line
    int pointerExtension = middleLine - pointer;  // are properly treated as flat.
    if (info->up) {
        dictatorExtension = std::min(dictatorExtension, 0);
        pointerExtension = std::min(pointerExtension, 0);
    } else {
        dictatorExtension = std::max(dictatorExtension, 0);
        pointerExtension = std::max(pointerExtension, 0);
    }
    if (dictator + dictatorExtension == middleLine && pointer + pointerExtension == middleLine) {
        return 0;
    }
    if (startNote == endNote) {
        return 0;
    }
    SlopeConstraint slopeConstrained = getSlopeConstraint(info, startNote, endNote);
    if (slopeConstrained == SlopeConstraint::FLAT) {
        return 0;
    } else if (slopeConstrained == SlopeConstraint::SMALL_SLOPE) {
        return dictator > pointer ? -1 : 1;
    }

    // calculate max slope based on distance between first and last chords
    int maxSlope = getMaxSlope(info);

    // calculate max slope based on note interval
    int interval = std::min(std::abs(endNote - startNote), (int)_maxSlopes.size() - 1);
    return std::min(maxSlope, _maxSlopes[interval]) * (info->up ? 1 : -1);
}

TremoloBeamLayout::SlopeConstraint TremoloBeamLayout::getSlopeConstraint(const Tremolo::LayoutData* info, int startNote, int endNote)
{
    if (info->notes.empty()) {
        return SlopeConstraint::NO_CONSTRAINT;
    }

    // 0 to constrain to flat, 1 to constrain to 0.25, <0 for no constraint
    if (startNote == endNote) {
        return SlopeConstraint::FLAT;
    }
    // tremolos don't need the small slope constraint since they only have two notes
    return SlopeConstraint::NO_CONSTRAINT;
}

int TremoloBeamLayout::getMaxSlope(const Tremolo::LayoutData* info)
{
    // for 2-indexed interval i (seconds, thirds, etc.)
    // maxSlopes[i] = max slope of beam for notes with interval i

    // calculate max slope based on distance between first and last chords
    double endX = chordBeamAnchorX(info, info->elements[info->elements.size() - 1], ChordBeamAnchorType::Start);
    double startX = chordBeamAnchorX(info, info->elements[0], ChordBeamAnchorType::End);
    double beamWidth = endX - startX;
    beamWidth /= info->spatium;
    int maxSlope = _maxSlopes.back();
    if (beamWidth < 3.0) {
        maxSlope = _maxSlopes[1];
    } else if (beamWidth < 5.0) {
        maxSlope = _maxSlopes[2];
    } else if (beamWidth < 7.5) {
        maxSlope = _maxSlopes[3];
    } else if (beamWidth < 10.0) {
        maxSlope = _maxSlopes[4];
    } else if (beamWidth < 15.0) {
        maxSlope = _maxSlopes[5];
    } else if (beamWidth < 20.0) {
        maxSlope = _maxSlopes[6];
    } else {
        maxSlope = _maxSlopes[7];
    }

    return maxSlope;
}

int TremoloBeamLayout::getBeamCount(const Tremolo::LayoutData* info, const std::vector<ChordRest*> chordRests)
{
    int maxBeams = 0;
    for (ChordRest* chordRest : chordRests) {
        if (chordRest->isChord() && strokeCount(info, chordRest) > maxBeams) {
            maxBeams = strokeCount(info, chordRest);
        }
    }
    return maxBeams;
}

double TremoloBeamLayout::chordBeamAnchorX(const Tremolo::LayoutData* info, const ChordRest* cr, ChordBeamAnchorType anchorType)
{
    double pagePosX = info->trem ? info->trem->pagePos().x() : info->beam->pagePos().x();
    double stemPosX = cr->stemPosX() + cr->pagePos().x() - pagePosX;

    if (!cr->isChord() || !toChord(cr)->stem()) {
        if (!info->up) {
            // rests always return the right side of the glyph as their stemPosX
            // so we need to adjust back to the left side if stems are down
            stemPosX -= cr->stemPosX();
        }
        return stemPosX;
    }
    const Chord* chord = toChord(cr);

    double stemWidth = chord->stem()->lineWidth().val() * chord->mag();

    switch (anchorType) {
    case ChordBeamAnchorType::Start:
        if (info->tab) {
            return stemPosX - 0.5 * stemWidth;
        }

        if (chord->up()) {
            return stemPosX - stemWidth;
        }

        break;
    case ChordBeamAnchorType::Middle:
        if (info->tab) {
            return stemPosX;
        }

        return chord->up() ? stemPosX - 0.5 * stemWidth : stemPosX + 0.5 * stemWidth;

    case ChordBeamAnchorType::End:
        if (info->tab) {
            return stemPosX + 0.5 * stemWidth;
        }

        if (!chord->up()) {
            return stemPosX + stemWidth;
        }

        break;
    }

    return stemPosX;
}

double TremoloBeamLayout::chordBeamAnchorY(const Tremolo::LayoutData* info, const ChordRest* cr)
{
    if (!cr->isChord()) {
        return cr->pagePos().y();
    }

    const Chord* chord = toChord(cr);
    Note* note = cr->up() ? chord->downNote() : chord->upNote();
    PointF position = note->pagePos();

    int upValue = chord->up() ? -1 : 1;
    double beamOffset = info->beamWidth / 2 * upValue;

    if (info->isBesideTabStaff) {
        double stemLength = info->tab->chordStemLength(chord) * (info->up ? -1 : 1);
        double y = info->tab->chordRestStemPosY(chord) + stemLength;
        y *= info->spatium;
        y -= beamOffset;
        return y + chord->pagePos().y();
    }

    return position.y() - note->offset().y() + (chord->defaultStemLength() * upValue) - beamOffset;
}

PointF TremoloBeamLayout::chordBeamAnchor(const Tremolo::LayoutData* info, const ChordRest* cr, ChordBeamAnchorType anchorType)
{
    return PointF(chordBeamAnchorX(info, cr, anchorType), chordBeamAnchorY(info, cr));
}
