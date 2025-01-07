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

#include "beamtremololayout.h"

#include <array>

#include "../dom/beam.h"
#include "../dom/chordrest.h"
#include "../dom/chord.h"
#include "../dom/note.h"
#include "../dom/score.h"
#include "../dom/staff.h"
#include "../dom/stem.h"
#include "../dom/stemslash.h"
#include "../dom/system.h"
#include "../dom/tremolotwochord.h"

#include "tlayout.h"
#include "chordlayout.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

constexpr std::array _maxSlopes = { 0, 1, 2, 3, 4, 5, 6, 7 };

void BeamTremoloLayout::setupLData(const BeamBase* item, BeamBase::LayoutData* ldata, const LayoutContext& ctx)
{
    bool isGrace = false;
    IF_ASSERT_FAILED(item && (item->isBeam() || item->isType(ElementType::TREMOLO_TWOCHORD))) {
        // right now only beams and trems are supported
        return;
    } else if (item->isBeam()) {
        ldata->beamType = BeamType::BEAM;
        ldata->beam = item_cast<const Beam*>(item);
        ldata->up = ldata->beam->up();
        ldata->trem = nullptr; // there can be many different trems in a beam, they will all be checked
        isGrace = ldata->beam->elements().front()->isGrace();
    } else {
        ldata->trem = item_cast<const TremoloTwoChord*>(item);
        ldata->beamType = BeamType::TREMOLO;
        // check to see if there is a beam happening during this trem
        // if so, it needs to be taken into account in trem placement
        if (ldata->trem->chord1()->beam() && ldata->trem->chord1()->beam() == ldata->trem->chord2()->beam()) {
            ldata->beam = ldata->trem->chord1()->beam();
        } else {
            ldata->beam = nullptr;
        }
        ldata->up = computeTremoloUp(ldata);
        //! HACK Temporary
        const_cast<TremoloTwoChord*>(ldata->trem)->setUp(ldata->up);
        isGrace = ldata->trem->chord1()->isGrace();
    }

    ldata->spatium = item->spatium();
    ldata->tick = item->tick();
    ldata->beamSpacing = ctx.conf().styleB(Sid::useWideBeams) ? 4 : 3;
    ldata->beamDist = (ldata->beamSpacing / 4.0) * ldata->spatium * ldata->mag() * (isGrace ? ctx.conf().styleD(Sid::graceNoteMag) : 1.);
    ldata->beamWidth = (ctx.conf().styleS(Sid::beamWidth).val() * ldata->spatium) * ldata->mag();
    const StaffType* staffType = item->staffType();
    ldata->tab = (staffType && staffType->isTabStaff()) ? staffType : nullptr;
    ldata->isBesideTabStaff = ldata->tab && !ldata->tab->stemless() && !ldata->tab->stemThrough();
}

int BeamTremoloLayout::minStemLength(const ChordRest* cr, const BeamBase::LayoutData* ldata)
{
    // min stem lengths in quarter spaces according to how many beams there are (starting with 1)
    static constexpr int BEAMS_COUNT = 8;
    static constexpr int minStemLengths[BEAMS_COUNT] = { 11, 13, 15, 18, 21, 24, 27, 30 };
    int beams = strokeCount(ldata, cr);
    if (beams > BEAMS_COUNT) {
        LOGE() << "Beam count " << beams << " out of range (" << BEAMS_COUNT - 1 << ")";
        return minStemLengths[BEAMS_COUNT - 1];
    }

    return minStemLengths[std::max(beams - 1, 0)];
}

void BeamTremoloLayout::offsetBeamToRemoveCollisions(const BeamBase* item, const BeamBase::LayoutData* ldata,
                                                     const std::vector<ChordRest*>& chordRests,
                                                     int& dictator,
                                                     int& pointer,
                                                     const double startX, const double endX,
                                                     bool isFlat, bool isStartDictator)
{
    if (endX == startX) {
        return;
    }

    IF_ASSERT_FAILED(!isFlat || (isFlat == (dictator == pointer))) {
        // This will produce false positives if the pointer & dictator are not flat when they should be
        LOGE() << "Beam is flat, dictator and pointer should be equal";
        pointer = dictator;
    }

    // tolerance eliminates all possibilities of floating point rounding errors
    const double tolerance = ldata->beamWidth * 0.25 * (ldata->up ? -1 : 1);
    const bool isSmall = ldata->isGrace || ldata->mag() < 1.;

    double startY = (isStartDictator ? dictator : pointer) * ldata->spatium / 4 + tolerance;
    double endY = (isStartDictator ? pointer : dictator) * ldata->spatium / 4 + tolerance;

    const ChordRest* firstChordRest = chordRests.front();
    const ChordRest* lastChordRest = chordRests.back();

    size_t curChordRest = 0;
    for (ChordRest* chordRest : chordRests) {
        if (!chordRest->isChord() || chordRest == ldata->elements.back() || chordRest == ldata->elements.front()) {
            curChordRest++;
            continue;
        }

        int sameLineException = 0;

        if (chordRest->isChord()
            && ((curChordRest == 1 && firstChordRest->isChord()) || (curChordRest == chordRests.size() - 2 && lastChordRest->isChord()))) {
            const Chord* innerChord = toChord(chordRest);
            const Chord* outerChord = curChordRest == 1 && firstChordRest->isChord() ? toChord(firstChordRest) : toChord(lastChordRest);
            const int innerLine = ldata->up ? innerChord->upNote()->line() : innerChord->downNote()->line();
            const int outerLine = ldata->up ? outerChord->upNote()->line() : outerChord->downNote()->line();

            if (innerLine == outerLine) {
                sameLineException = 1;
            } else if (chordRests.size() == 3) {
                const ChordRest* otherOuterCR = outerChord == firstChordRest ? lastChordRest : firstChordRest;
                const Chord* otherOuterChord = otherOuterCR->isChord() ? toChord(otherOuterCR) : nullptr;
                if (!otherOuterChord) {
                    continue;
                }

                const int otherOuterLine = ldata->up ? otherOuterChord->upNote()->line() : otherOuterChord->downNote()->line();
                if (innerLine == otherOuterLine) {
                    sameLineException = 1;
                }
            }
        }

        PointF anchor = chordBeamAnchor(ldata, chordRest, ChordBeamAnchorType::Middle) - item->pagePos();

        const int minLen = minStemLength(chordRest, ldata) - sameLineException;

        // avoid division by zero for zero-length beams (can exist as a pre-layout state used
        // for horizontal spacing computations)
        if (endX != startX) {
            const double proportionAlongX = (anchor.x() - startX) / (endX - startX);
            while (true) {
                const int slope = std::abs(dictator - pointer);
                double reduction = 0.0;
                if (!isFlat) {
                    if (slope <= 3) {
                        reduction = 0.25 * ldata->spatium;
                    } else if (slope <= 6) {
                        reduction = 0.5 * ldata->spatium;
                    } else { // slope > 6
                        reduction = 0.75 * ldata->spatium;
                    }
                }
                // Ensure the beam clears the anchor's height and inner note's stem minimum stem length is enforced
                const double desiredY = proportionAlongX * (endY - startY) + startY;    // start note stem len + extra for slope
                const bool beamClearsAnchor = (ldata->up && muse::RealIsEqualOrLess(desiredY, anchor.y() + reduction))
                                              || (!ldata->up && muse::RealIsEqualOrMore(desiredY, anchor.y() - reduction));

                const Note* note = ldata->up ? toChord(chordRest)->downNote() : toChord(chordRest)->upNote();
                const double noteAnchor = (ldata->up ? note->stemUpSE().y() : note->stemDownNW().y()) + note->pagePos().y()
                                          - item->pagePos().y();
                // Resultant length in quarter spaces
                const int desiredLen = std::abs(round((desiredY - noteAnchor) / ldata->spatium * 4)) + 1;

                if (beamClearsAnchor && desiredLen >= round(minLen * chordRest->mag())) {
                    break;
                }

                if (isFlat || (isSmall && dictator == pointer)) {
                    dictator += ldata->up ? -1 : 1;
                    pointer += ldata->up ? -1 : 1;
                } else if (std::abs(dictator - pointer) == 1) {
                    dictator += ldata->up ? -1 : 1;
                } else {
                    pointer += ldata->up ? -1 : 1;
                }

                startY = (isStartDictator ? dictator : pointer) * ldata->spatium / 4 + tolerance;
                endY = (isStartDictator ? pointer : dictator) * ldata->spatium / 4 + tolerance;
            }
        }
        curChordRest++;
    }
}

void BeamTremoloLayout::offsetBeamWithAnchorShortening(const BeamBase::LayoutData* ldata, const std::vector<ChordRest*>& chordRests,
                                                       int& dictator, int& pointer, int staffLines, bool isStartDictator,
                                                       int stemLengthDictator, const int targetLine)
{
    const Chord* startChord = nullptr;
    const Chord* endChord = nullptr;
    for (const ChordRest* cr : chordRests) {
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
    int dictatorBeams = strokeCount(ldata, isStartDictator ? startChord : endChord);
    int pointerBeams = strokeCount(ldata, isStartDictator ? endChord : startChord);
    int maxDictatorReduce = stemLengthDictator - minStemLengths[std::max(dictatorBeams - 1, 0)];
    maxDictatorReduce = std::min(std::abs(dictator - targetLine), maxDictatorReduce);

    bool isFlat = dictator == pointer;
    const BeamBase::NotePosition startPos = BeamBase::NotePosition(startChord->line(), startChord->vStaffIdx());
    const BeamBase::NotePosition endPos = BeamBase::NotePosition(endChord->line(), endChord->vStaffIdx());
    bool isAscending = startPos > endPos;
    int towardBeam = ldata->up ? -1 : 1;
    int newDictator = dictator;
    int newPointer = pointer;
    int reduce = 0;
    auto fourBeamException = [](int beams, int yPos) {
        yPos += 400; // because there is some weirdness with modular division around zero, add
                     // a large multiple of 4 so that we can guarantee that yPos%4 will be correct
        return beams >= 4 && (yPos % 4 == 2);
    };
    while (!fourBeamException(dictatorBeams, newDictator)
           && !isValidBeamPosition(ldata->up, newDictator, isStartDictator, isAscending, isFlat, staffLines, true)) {
        if (++reduce > maxDictatorReduce) {
            // we can't shorten this stem at all. bring it back to default and start extending
            newDictator = dictator;
            newPointer = pointer;
            while (!isValidBeamPosition(ldata->up, newDictator, isStartDictator, isAscending, isFlat, staffLines, true)) {
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
    newPointer = ldata->up ? std::min(newPointer, targetLine) : std::max(newPointer, targetLine);
    // walk it back beamwards until we get a position that satisfies both pointer and dictator
    while (!fourBeamException(dictatorBeams, newDictator) && !fourBeamException(pointerBeams, newPointer)
           && (!isValidBeamPosition(ldata->up, newDictator, isStartDictator, isAscending, isFlat, staffLines, true)
               || !isValidBeamPosition(ldata->up, newPointer, !isStartDictator, isAscending, isFlat, staffLines, true))) {
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

    if (isFlat) {
        pointer = dictator;
    }
}

void BeamTremoloLayout::extendStem(const BeamBase::LayoutData* ldata, Chord* chord, double addition)
{
    LayoutContext ctx(chord->score());

    PointF anchor = chordBeamAnchor(ldata, chord, ChordBeamAnchorType::Middle);
    double desiredY;
    if (ldata->endAnchor.x() > ldata->startAnchor.x()) {
        double proportionAlongX = (anchor.x() - ldata->startAnchor.x()) / (ldata->endAnchor.x() - ldata->startAnchor.x());
        desiredY = proportionAlongX * (ldata->endAnchor.y() - ldata->startAnchor.y()) + ldata->startAnchor.y();
    } else {
        desiredY = std::max(ldata->endAnchor.y(), ldata->startAnchor.y());
    }

    if (chord->up()) {
        chord->setBeamExtension(anchor.y() - desiredY + addition);
    } else {
        chord->setBeamExtension(desiredY - anchor.y() + addition);
    }
    TLayout::layoutStem(chord->stem(), chord->stem()->mutldata(), ctx.conf());

    if (chord->stemSlash()) {
        TLayout::layoutStemSlash(chord->stemSlash(), chord->stemSlash()->mutldata(), ctx.conf());
    }
}

bool BeamTremoloLayout::isBeamInsideStaff(int yPos, int staffLines, bool allowFloater)
{
    int aboveStaff = allowFloater ? -2 : -3;
    int belowStaff = (staffLines - 1) * 4 + (allowFloater ? 2 : 3);
    return yPos > aboveStaff && yPos < belowStaff;
}

void BeamTremoloLayout::setSmallInnerBeamPos(const BeamBase::LayoutData* ldata, int& dictator, int& pointer, const int staffLines,
                                             const bool isFlat, const bool isSmall, const LayoutContext& ctx)
{
    // We only want this adjustment if the beam is small and flat
    if (!isSmall || !isFlat) {
        return;
    }
    bool noteOutsideStaff = false;
    int beamCount = 0;
    for (const ChordRest* cr : ldata->elements) {
        if (!cr->isChord()) {
            continue;
        }
        beamCount = std::max(beamCount, strokeCount(ldata, cr));
        noteOutsideStaff |= (cr->downLine() < -2 && !ldata->up) || (cr->upLine() >= 11 && ldata->up);
    }

    // AND stems have been extended to the second stave line when the notes are far enough outside of the stave
    if (!noteOutsideStaff) {
        return;
    }

    // Allow fractions of quarter spaces in inner beam position, as mag can change
    const double mag = ldata->isGrace ? ctx.conf().style().value(Sid::graceNoteMag).toDouble() : ldata->mag();
    const double innerBeam = dictator + (beamCount - 1) * (ldata->up ? ldata->beamSpacing : -ldata->beamSpacing) * mag;
    const int aboveStaff = 1;
    const int belowStaff = (staffLines - 1) * 4 - 1;
    int offset = 0;

    // Make sure the innermost beam is at least 0.5sp within the stave
    if (ldata->up) {
        offset = std::min(offset, int(std::floor(belowStaff - innerBeam)));
    } else {
        offset = std::max(offset, int(std::ceil(aboveStaff - innerBeam)));
    }

    // the beam is flat, so dictator and pointer should match
    dictator += offset;
    pointer = dictator;
}

int BeamTremoloLayout::getOuterBeamPosOffset(const BeamBase::LayoutData* ldata, int innerBeam, int beamCount, int staffLines)
{
    int spacing = (ldata->up ? -ldata->beamSpacing : ldata->beamSpacing);
    int offset = (beamCount - 1) * spacing;
    bool isInner = false;
    while (offset != 0 && !isBeamInsideStaff(innerBeam + offset, staffLines, isInner)) {
        offset -= spacing;
        isInner = true;
    }
    return offset;
}

bool BeamTremoloLayout::isValidBeamPosition(const bool isUp, int yPos, bool isStart, bool isAscending, bool isFlat, int staffLines,
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

bool BeamTremoloLayout::is64thBeamPositionException(const int beamSpacing, int& yPos, int staffLines)
{
    if (beamSpacing == 4) {
        return false;
    }
    return yPos == 2 || yPos == staffLines * 4 - 2 || yPos == staffLines * 4 - 6 || yPos == -2;
}

int BeamTremoloLayout::findValidBeamOffset(const BeamBase::LayoutData* ldata, int outer, int beamCount, int staffLines, bool isStart,
                                           bool isAscending, bool isFlat)
{
    bool isBeamValid = false;
    int offset = 0;
    int innerBeam = outer + (beamCount - 1) * (ldata->up ? ldata->beamSpacing : -ldata->beamSpacing);
    while (!isBeamValid) {
        while (!isValidBeamPosition(ldata->up, innerBeam + offset, isStart, isAscending, isFlat, staffLines, beamCount < 2)) {
            offset += ldata->up ? -1 : 1;
        }
        int outerMostBeam = innerBeam + offset + getOuterBeamPosOffset(ldata, innerBeam + offset, beamCount, staffLines);
        if (isValidBeamPosition(ldata->up, outerMostBeam, isStart, isAscending, isFlat, staffLines, true)
            || (beamCount == 4 && is64thBeamPositionException(ldata->beamSpacing, outerMostBeam, staffLines))) {
            isBeamValid = true;
        } else {
            offset += ldata->up ? -1 : 1;
        }
    }
    return offset;
}

void BeamTremoloLayout::setValidBeamPositions(const BeamBase::LayoutData* ldata, int& dictator, int& pointer, int beamCountD,
                                              int beamCountP,
                                              int staffLines,
                                              bool isStartDictator,
                                              bool isFlat, bool isAscending)
{
    bool areBeamsValid = false;
    bool has3BeamsInsideStaff = beamCountD >= 3 || beamCountP >= 3;
    while (!areBeamsValid && has3BeamsInsideStaff && ldata->beamSpacing != 4) {
        int dictatorInner = dictator + (beamCountD - 1) * (ldata->up ? ldata->beamSpacing : -ldata->beamSpacing);
        // use dictatorInner for both to simulate flat beams
        int outerDictatorOffset = getOuterBeamPosOffset(ldata, dictatorInner, beamCountD, staffLines);
        if (std::abs(outerDictatorOffset) <= ldata->beamSpacing) {
            has3BeamsInsideStaff = false;
            break;
        }
        int offsetD = findValidBeamOffset(ldata, dictator, beamCountD, staffLines, isStartDictator, false, true);
        int offsetP = findValidBeamOffset(ldata, pointer, beamCountP, staffLines, isStartDictator, false, true);
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
        int dictatorOffset = findValidBeamOffset(ldata, dictator, beamCountD, staffLines, isStartDictator, isAscending, isFlat);
        dictator += dictatorOffset;
        pointer += dictatorOffset;
        if (isFlat) {
            pointer = dictator;
            int currOffset = 0;
            for (ChordRest* cr : ldata->elements) {
                if (!cr->isChord() && (cr != ldata->elements.front() && cr != ldata->elements.back())) {
                    continue;
                }
                // we can use dictator beam position because all of the notes have the same beam position
                int beamCount = strokeCount(ldata, cr);
                currOffset = findValidBeamOffset(ldata, dictator, beamCount, staffLines, isStartDictator, isAscending, isFlat);
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
            pointer += findValidBeamOffset(ldata, pointer, beamCountP, staffLines, !isStartDictator, isAscending, isFlat);
            if ((ldata->up && pointer <= dictator) || (!ldata->up && pointer >= dictator)) {
                dictator = pointer + (ldata->up ? -1 : 1);
            } else {
                areBeamsValid = true;
            }
        }
    }
}

void BeamTremoloLayout::addMiddleLineSlant(const BeamBase::LayoutData* ldata, int& dictator, int& pointer, int beamCount, int targetLine,
                                           int interval, int desiredSlant)
{
    bool isSmall = ldata->mag() < 1. || ldata->isGrace;
    if (interval == 0 || (!isSmall && beamCount > 2 && ldata->beamSpacing != 4) || noSlope(ldata->beam)) {
        return;
    }
    bool isOnTargetLine = pointer == targetLine && (std::abs(pointer - dictator) < 2);
    if (isOnTargetLine) {
        if (std::abs(desiredSlant) == 1 || interval == 1 || (beamCount == 2 && ldata->beamSpacing != 4 && !isSmall)) {
            dictator = targetLine + (ldata->up ? -1 : 1);
        } else {
            dictator = targetLine + (ldata->up ? -2 : 2);
        }
    }
}

void BeamTremoloLayout::add8thSpaceSlant(BeamBase::LayoutData* ldata, PointF& dictatorAnchor, int dictator, int pointer, int beamCount,
                                         int interval, int targetLine, bool isFlat)
{
    if (beamCount != 3 || noSlope(ldata->beam) || ldata->beamSpacing != 3) {
        return;
    }
    if ((isFlat && dictator != targetLine) || (dictator != pointer) || interval == 0) {
        return;
    }
    if ((ldata->up && (dictator + 4) % 4 == 3) || (!ldata->up && (dictator + 4) % 4 == 1)) {
        return;
    }
    dictatorAnchor.setY(dictatorAnchor.y() + (ldata->up ? -0.125 * ldata->spatium : 0.125 * ldata->spatium));
    ldata->beamDist += 0.0625 * ldata->spatium;
}

bool BeamTremoloLayout::computeTremoloUp(const BeamBase::LayoutData* ldata)
{
    if (!ldata->beam || !ldata->beam->cross()) {
        return ldata->trem->up();
    }
    Chord* c1 = ldata->trem->chord1();
    Chord* c2 = ldata->trem->chord2();
    int staffMove = 0;
    for (ChordRest* cr : ldata->beam->elements()) {
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
        return ldata->trem->up();
    }
}

int BeamTremoloLayout::strokeCount(const BeamBase::LayoutData* ldata, const ChordRest* cr)
{
    if (cr->isRest()) {
        return cr->beams();
    }
    int strokes = 0;
    const Chord* c = toChord(cr);
    if (ldata->beamType == BeamType::TREMOLO) {
        strokes = ldata->trem->lines();
    } else if (ldata->beamType == BeamType::BEAM && c->tremoloTwoChord()) {
        strokes = c->tremoloTwoChord()->lines();
    }
    strokes += ldata->beam ? cr->beams() : 0;
    return strokes;
}

bool BeamTremoloLayout::calculateAnchors(const BeamBase* item, BeamBase::LayoutData* ldata, const LayoutContext& ctx,
                                         const std::vector<ChordRest*>& chordRests,
                                         const std::vector<BeamBase::NotePosition>& notePositions)
{
    ldata->startAnchor = PointF();
    ldata->endAnchor = PointF();
    if (ldata->beamType == BeamType::TREMOLO && ldata->beam) {
        // this is a trem inside a beam, and we are currently calculating the anchors of the tremolo.
        // we can do this as long as the beam has been layed out first: it saves trem anchor positions
        ChordRest* cr1 = ldata->trem->chord1();
        ChordRest* cr2 = ldata->trem->chord2();
        double anchorX1 = chordBeamAnchorX(ldata, cr1, ChordBeamAnchorType::Middle);
        double anchorX2 = chordBeamAnchorX(ldata, cr2, ChordBeamAnchorType::Middle);
        for (const TremAnchor& t : ldata->beam->tremAnchors()) {
            if (t.chord1 == cr1) {
                ldata->startAnchor = PointF(anchorX1, t.y1);
                ldata->endAnchor = PointF(anchorX2, t.y2);
                ldata->slope = (t.y2 - t.y1) / (anchorX2 - anchorX1);
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
    ldata->elements = chordRests;
    if (chordRests.empty()) {
        return false;
    }

    bool hasChordAboveBeam = false;
    bool hasChordBelowBeam = false;

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
            ChordLayout::layoutStem(toChord(chordRest), ctx);
        }
        if (chordRest->isBelowCrossBeam(item)) {
            hasChordBelowBeam = true;
        } else {
            hasChordAboveBeam = true;
        }
    }
    if (!startChord) {
        // we were passed a vector of only rests. we don't support beams across only rests
        // this beam will be deleted in LayoutBeams
        return false;
    }
    ldata->isGrace = startChord->isGrace();
    ldata->notePositions = notePositions;

    // Set cross staff beam position, above, below or between staves
    ldata->crossStaffBeamPos = BeamBase::CrossStaffBeamPosition::INVALID;
    if (item->minCRMove() != item->maxCRMove()) {
        if (hasChordAboveBeam && !hasChordBelowBeam) {
            ldata->crossStaffBeamPos = BeamBase::CrossStaffBeamPosition::BELOW;
        } else if (hasChordBelowBeam && !hasChordAboveBeam) {
            ldata->crossStaffBeamPos = BeamBase::CrossStaffBeamPosition::ABOVE;
        } else if (hasChordAboveBeam && hasChordBelowBeam) {
            ldata->crossStaffBeamPos = BeamBase::CrossStaffBeamPosition::BETWEEN;
        }
    }

    if (calculateAnchorsCross(item, ldata, ctx.conf())) {
        return true;
    }

    const int closestMove = ldata->crossStaffBeamPos == BeamBase::CrossStaffBeamPosition::ABOVE ? item->minCRMove() : item->maxCRMove();
    const staff_idx_t closestStaffToBeam = ldata->crossStaffBeamPos == BeamBase::CrossStaffBeamPosition::ABOVE
                                           || ldata->crossStaffBeamPos
                                           == BeamBase::CrossStaffBeamPosition::BELOW ? item->staffIdx()
                                           + closestMove : startChord->vStaffIdx();

    std::vector<Chord*> chordsClosestToBeam;

    for (ChordRest* cr : ldata->elements) {
        if (cr->isChord() && cr->vStaffIdx() == closestStaffToBeam) {
            chordsClosestToBeam.push_back(toChord(cr));
        }
    }

    ldata->startAnchor = chordBeamAnchor(ldata, startChord, ChordBeamAnchorType::Start);
    ldata->endAnchor = chordBeamAnchor(ldata, endChord, ChordBeamAnchorType::End);

    double startLength = startChord->defaultStemLength();
    double endLength = endChord->defaultStemLength();
    double startAnchorBase = ldata->startAnchor.y() + (ldata->up ? startLength : -startLength);
    double endAnchorBase = ldata->endAnchor.y() + (ldata->up ? endLength : -endLength);
    int startNote = ldata->up ? startChord->upNote()->line() : startChord->downNote()->line();
    int endNote = ldata->up ? endChord->upNote()->line() : endChord->downNote()->line();
    if (ldata->tab) {
        startNote = ldata->up ? startChord->upString() : startChord->downString();
        endNote = ldata->up ? endChord->upString() : endChord->downString();
    }
    staff_idx_t startStaff = startChord->vStaffIdx();
    staff_idx_t endStaff = endChord->vStaffIdx();
    BeamBase::NotePosition startPos = BeamBase::NotePosition(startNote, startStaff);
    BeamBase::NotePosition endPos = BeamBase::NotePosition(endNote, endStaff);

    const int interval = std::abs(startNote - endNote);
    const bool isStartDictator = ldata->up ? startPos < endPos : startPos > endPos;
    const double quarterSpace = ldata->spatium / 4;
    PointF startAnchor = ldata->startAnchor - item->pagePos();
    PointF endAnchor = ldata->endAnchor - item->pagePos();
    int dictator = round((isStartDictator ? startAnchor.y() : endAnchor.y()) / quarterSpace);
    int pointer = round((isStartDictator ? endAnchor.y() : startAnchor.y()) / quarterSpace);

    const int staffLines = ctx.dom().staff(closestStaffToBeam)->lines(ldata->tick);
    // This is the middle of the stave if the notes are normal size or the second line from the bottom if notes are small
    const int targetLine = getTargetStaffLine(ldata, ctx, startChord, endChord, staffLines, closestStaffToBeam, item->staffIdx());

    int slant
        = computeDesiredSlant(item, ldata, startPos, endPos, chordsClosestToBeam, targetLine, dictator, pointer);
    bool isFlat = slant == 0;
    SlopeConstraint specialSlant
        = isFlat ? getSlopeConstraint(ldata, startPos, endPos) : SlopeConstraint::NO_CONSTRAINT;
    bool forceFlat = specialSlant == SlopeConstraint::FLAT;
    bool smallSlant = specialSlant == SlopeConstraint::SMALL_SLOPE;
    if (isFlat) {
        dictator = ldata->up ? std::min(pointer, dictator) : std::max(pointer, dictator);
        pointer = dictator;
    } else {
        if ((dictator > pointer) != (isStartDictator ? startPos > endPos : endPos > startPos)) {
            dictator = pointer - slant;
        } else {
            pointer = dictator + slant;
        }
    }
    bool isAscending = startPos > endPos;
    int beamCountD = strokeCount(ldata, isStartDictator ? startChord : endChord);
    int beamCountP = strokeCount(ldata, isStartDictator ? endChord : startChord);

    int stemLengthStart = std::abs(round((startAnchorBase - ldata->startAnchor.y()) / ldata->spatium * 4));
    int stemLengthEnd = std::abs(round((endAnchorBase - ldata->endAnchor.y()) / ldata->spatium * 4));
    int stemLengthDictator = isStartDictator ? stemLengthStart : stemLengthEnd;
    bool isSmall = ldata->mag() < 1. || ldata->isGrace;
    if (endAnchor.x() > startAnchor.x()) {
        /* When beam layout is called before horizontal spacing (see LayoutMeasure::getNextMeasure() to
         * know why) the x positions aren't yet determined and may be all zero, which would cause the
         * following function to get stuck in a loop. The if() condition avoids that case. */

        // Make sure grace & small note inner beams are within the stave
        setSmallInnerBeamPos(ldata, dictator, pointer, staffLines, isFlat, isSmall, ctx);

        if (!isSmall) {
            // Adjust anchor stems
            offsetBeamWithAnchorShortening(ldata, chordRests, dictator, pointer, staffLines, isStartDictator, stemLengthDictator,
                                           targetLine);
        }
        // Adjust inner stems
        offsetBeamToRemoveCollisions(item, ldata, chordRests, dictator, pointer, startAnchor.x(), endAnchor.x(), isFlat, isStartDictator);
    }

    int beamCount = std::max(beamCountD, beamCountP);
    if (!ldata->tab) {
        if (!ldata->isGrace) {
            setValidBeamPositions(ldata, dictator, pointer, beamCountD, beamCountP, staffLines, isStartDictator, isFlat, isAscending);
        }
        if (!forceFlat) {
            addMiddleLineSlant(ldata, dictator, pointer, beamCount, targetLine, interval, smallSlant ? 1 : slant);
        }
    }

    ldata->startAnchor.setY(quarterSpace * (isStartDictator ? dictator : pointer) + item->pagePos().y());
    ldata->endAnchor.setY(quarterSpace * (isStartDictator ? pointer : dictator) + item->pagePos().y());

    bool add8th = true;
    if (item->isType(ElementType::BEAM) && item_cast<const Beam*>(item)->userModified()) {
        add8th = false;
    }
    if (!ldata->tab && add8th) {
        add8thSpaceSlant(ldata, isStartDictator ? ldata->startAnchor : ldata->endAnchor, dictator, pointer, beamCount, interval,
                         targetLine, isFlat);
    }
    ldata->startAnchor.setX(chordBeamAnchorX(ldata, startCr, ChordBeamAnchorType::Start));
    ldata->endAnchor.setX(chordBeamAnchorX(ldata, endCr, ChordBeamAnchorType::End));
    return true;
}

bool BeamTremoloLayout::calculateAnchorsCross(const BeamBase* item, BeamBase::LayoutData* ldata,
                                              const LayoutConfiguration& conf)
{
    if (ldata->crossStaffBeamPos != BeamBase::CrossStaffBeamPosition::BETWEEN) {
        // The beam is either not cross or above/below and must be layed out like a non-cross beam
        return false;
    }

    double spatium = conf.spatium();
    ChordRest* startCr = ldata->elements.front();
    ChordRest* endCr = ldata->elements.back();

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
    // Get switches between above & below the beam
    // The direction is neutral if there is more than one switch
    bool prevBelowBeam = ldata->elements.front()->isBelowCrossBeam(item);
    bool hasBeamSideSwitch = false;
    int beamSideSwitchDirection = 0;

    // Get switches between staves on the same side of a beam
    // The direction is neutral is there is more than one switch
    staff_idx_t prevStaff = ldata->elements.front()->vStaffIdx();
    bool hasStaffSwitch = false;
    int staffSwitchDirection = 0;

    // Find the notes on the top and bottom of staves
    //
    bool checkNextTop = false;
    bool checkNextBottom = false;
    for (ChordRest* cr : ldata->elements) {
        if (!cr->isChord()) {
            continue;
        }
        Chord* c = toChord(cr);

        if (c->isBelowCrossBeam(item) == prevBelowBeam) {
            c->vStaffIdx();
            if (prevStaff != c->vStaffIdx()) {
                int diff = static_cast<int>(prevStaff - c->vStaffIdx());
                staffSwitchDirection = hasStaffSwitch ? 0 : (diff < 0 ? -1 : 1);
                hasStaffSwitch = true;
            }
        }
        prevStaff = c->vStaffIdx();

        if (c->isBelowCrossBeam(item)) {
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
            if (!prevBelowBeam) {
                beamSideSwitchDirection = hasBeamSideSwitch ? 0 : -1;
                hasBeamSideSwitch = true;
            }
            prevBelowBeam = true;
            maxY = std::min(maxY, chordBeamAnchorY(ldata, toChord(c)));
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
            if (prevBelowBeam) {
                beamSideSwitchDirection = hasBeamSideSwitch ? 0 : 1;
                hasBeamSideSwitch = true;
            }
            prevBelowBeam = false;
            minY = std::max(minY, chordBeamAnchorY(ldata, toChord(c)));
        }
    }

    double yMidPoint = (maxY + minY) / 2;
    if (item->isBeam()) {
        // Cross-staff beams may have segments both above and below the midline.
        // Compute the y-difference between the top and bottom segment and aim for centering in between.
        const Beam* beam = toBeam(item);
        const std::vector<BeamSegment*>& beamSegments = beam->beamSegments();
        const Chord* limitingChordAbove = beam->ldata()->limitingChordAbove;
        const Chord* limitingChordBelow = beam->ldata()->limitingChordBelow;
        if (!beamSegments.empty() && limitingChordAbove && limitingChordBelow) {
            double yCenterSegment = 0.5 * (beamSegments.front()->line.y1() + beamSegments.front()->line.y2());

            const BeamSegment* topBeamSegmentForChordAbove = beam->topLevelSegmentForElement(limitingChordAbove);
            double yTopSegAbove = 0.5 * (topBeamSegmentForChordAbove->line.y1() + topBeamSegmentForChordAbove->line.y2());
            double distAbove = yTopSegAbove - yCenterSegment;

            const BeamSegment* topBeamSegmentForChordBelow = beam->topLevelSegmentForElement(limitingChordBelow);
            double yTopSegBelow = 0.5 * (topBeamSegmentForChordBelow->line.y1() + topBeamSegmentForChordBelow->line.y2());
            double distBelow = yTopSegBelow - yCenterSegment;

            double averageDist = 0.5 * (distAbove + distBelow);
            yMidPoint -= 0.5 * averageDist;
        }
    }

    ldata->startAnchor.setY(yMidPoint);
    ldata->endAnchor.setY(yMidPoint);
    ldata->startAnchor.setX(chordBeamAnchorX(ldata, startCr, ChordBeamAnchorType::Start));
    ldata->endAnchor.setX(chordBeamAnchorX(ldata, endCr, ChordBeamAnchorType::End));

    ldata->slope = 0;

    // Calculates & sets slope
    if (!noSlope(ldata->beam)) {
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
            if (ldata->elements[0] == topFirst) {
                yFirst = topFirst->stemPos().y();
                yLast = bottomFirst->stemPos().y();
            } else {
                yFirst = bottomFirst->stemPos().y();
                yLast = topFirst->stemPos().y();
            }
            int desiredSlant = round((yFirst - yLast) / spatium);
            int slant = std::min(std::abs(desiredSlant), getMaxSlope(ldata));
            slant *= (desiredSlant < 0) ? -quarterSpace : quarterSpace;
            ldata->startAnchor.ry() += (slant / 2);
            ldata->endAnchor.ry() -= (slant / 2);
        } else {
            // Get the direction of groups of notes on top and bottom staves and the direction of switches between sides of the beam and switches between staves
            // Directions are up, down or neutral
            bool forceHoriz = false;

            int topSlant = topFirstLine - topLastLine;
            if (constrainTopToQuarter && topSlant != 0) {
                topSlant = topFirstLine < topLastLine ? -1 : 1;
            }
            if (!topLast) {
                // If there's only one note, set direction to neutral
                topSlant = 0;
            }
            int bottomSlant = bottomFirstLine - bottomLastLine;
            if (constrainBottomToQuarter && bottomSlant != 0) {
                bottomSlant = bottomFirstLine < bottomLastLine ? -1 : 1;
            }
            if (!bottomLast) {
                // If there's only one note, set direction to neutral
                bottomSlant = 0;
            }
            if ((maxMiddleTopLine >= std::max(topFirstLine, topLastLine)
                 || (minMiddleBottomLine <= std::min(bottomFirstLine, bottomLastLine)))) {
                forceHoriz = true;
            }

            const int topSlantDir = topSlant <= 0 ? (topSlant == 0 ? 0 : -1) : 1;
            const int bottomSlantDir = bottomSlant <= 0 ? (bottomSlant == 0 ? 0 : -1) : 1;
            // Check all directions which aren't neutral (0) match
            // If there is any contradiction between top staff direction, bottom direction, beam side switch direction and staff switch direction, force horizontal
            // If all are neutral, force horizontal
            int overallDirection = 0;
            for (int direction : { topSlantDir, bottomSlantDir, beamSideSwitchDirection, staffSwitchDirection }) {
                if (direction != 0) {
                    overallDirection = direction;
                    break;
                }
            }
            const bool overallSlantFlat = overallDirection == 0;
            const bool topSlantMatchesDirection = (topSlantDir != 0 && topSlantDir != overallDirection);
            const bool bottomSlantMatchesDirection = (bottomSlantDir != 0 && bottomSlantDir != overallDirection);
            const bool beamSideSwitchMatchesDirection = (beamSideSwitchDirection != 0 && beamSideSwitchDirection != overallDirection);
            const bool staffSwitchMatchesDirection = (staffSwitchDirection != 0 && staffSwitchDirection != overallDirection);

            forceHoriz = forceHoriz || overallSlantFlat
                         || (topSlantMatchesDirection
                             || bottomSlantMatchesDirection
                             || beamSideSwitchMatchesDirection
                             || staffSwitchMatchesDirection);

            if (!forceHoriz) {
                int slant = 0;
                if (topSlant == 0) {
                    slant = bottomSlant;
                } else if (bottomSlant == 0) {
                    slant = topSlant;
                } else {
                    slant = (std::abs(topSlant) < std::abs(bottomSlant)) ? topSlant : bottomSlant;
                }

                const int absSlant = std::min(std::abs(slant), getMaxSlope(ldata));
                const double slope = absSlant * ((slant < 0) ? -quarterSpace : quarterSpace);
                ldata->startAnchor.ry() += (slope / 2);
                ldata->endAnchor.ry() -= (slope / 2);
            }
        }
        ldata->slope = (ldata->endAnchor.y() - ldata->startAnchor.y()) / (ldata->endAnchor.x() - ldata->startAnchor.x());
    }
    return true;
}

bool BeamTremoloLayout::noSlope(const Beam* beam)
{
    return beam && beam->noSlope();
}

int BeamTremoloLayout::getTargetStaffLine(const BeamBase::LayoutData* ldata, const LayoutContext& ctx,
                                          const ChordRest* startChord, const ChordRest* endChord,
                                          const int staffLines, const staff_idx_t beamClosestStaffIdx, const staff_idx_t actualBeamStaffIdx)
{
    bool isFullSize = muse::RealIsEqual(ldata->mag(), 1.0) && !ldata->isGrace;
    bool useWideBeams = ctx.conf().styleB(Sid::useWideBeams);
    int startBeams = strokeCount(ldata, startChord);
    int endBeams = strokeCount(ldata, endChord);
    int startTargetLine = Chord::minStaffOverlap(ldata->up, staffLines, startBeams, false,
                                                 ldata->beamSpacing / 4.0, useWideBeams, isFullSize);
    int endTargetLine = Chord::minStaffOverlap(ldata->up, staffLines, endBeams, false,
                                               ldata->beamSpacing / 4.0, useWideBeams, !ldata->isGrace);

    int diff = 0;
    // Get diff between beam closest staff and actual staff, unless its full cross?
    if (ldata->crossStaffBeamPos == BeamBase::CrossStaffBeamPosition::ABOVE
        || ldata->crossStaffBeamPos == BeamBase::CrossStaffBeamPosition::BELOW) {
        // Make sure we calculate the distance to the target line on the stave closest to the beam
        const System* curSys = startChord->measure()->system();
        const int dir = ldata->up ? -1 : 1;
        if (curSys && beamClosestStaffIdx != actualBeamStaffIdx) {
            diff = std::round(std::abs(curSys->staffYpage(beamClosestStaffIdx) - curSys->staffYpage(
                                           actualBeamStaffIdx)) / ldata->spatium * 4 * dir);
        }
    }
    // offset target line by 1 or -1 since the anchor is at the middle of the beam,
    // not at the tip of the stem
    if (ldata->up) {
        return std::min(startTargetLine + diff, endTargetLine + diff) + 1;
    }
    return std::max(startTargetLine + diff, endTargetLine + diff) - 1;
}

int BeamTremoloLayout::computeDesiredSlant(const BeamBase* item, const BeamBase::LayoutData* ldata, const BeamBase::NotePosition& startPos,
                                           const BeamBase::NotePosition& endPos, std::vector<Chord*> closestChordsToBeam,
                                           int targetLine, int dictator, int pointer)
{
    if (item->isType(ElementType::BEAM) && item_cast<const Beam*>(item)->noSlope()) {
        return 0;
    }
    int dictatorExtension = targetLine - dictator; // we need to make sure that beams extended to the middle line
    int pointerExtension = targetLine - pointer;  // are properly treated as flat.
    if (ldata->up) {
        dictatorExtension = std::min(dictatorExtension, 0);
        pointerExtension = std::min(pointerExtension, 0);
    } else {
        dictatorExtension = std::max(dictatorExtension, 0);
        pointerExtension = std::max(pointerExtension, 0);
    }
    if (dictator + dictatorExtension == targetLine && pointer + pointerExtension == targetLine) {
        return 0;
    }
    if (startPos == endPos) {
        return 0;
    }
    SlopeConstraint slopeConstrained = getSlopeConstraint(ldata, startPos, endPos);
    if (slopeConstrained == SlopeConstraint::FLAT) {
        return 0;
    } else if (slopeConstrained == SlopeConstraint::SMALL_SLOPE) {
        return dictator > pointer ? -1 : 1;
    }

    // calculate max slope based on distance between first and last chords
    int maxSlope = getMaxSlope(ldata);

    if (ldata->crossStaffBeamPos == BeamBase::CrossStaffBeamPosition::ABOVE
        || ldata->crossStaffBeamPos == BeamBase::CrossStaffBeamPosition::BELOW) {
        // Calculate slant between first and last notes
        int beamDir = startPos == endPos ? 0 : (startPos > endPos ? 1 : -1);

        // Calculate slant between notes on stave closest to the beam
        size_t closestChordsSize = closestChordsToBeam.size();
        BeamBase::NotePosition closestStartPos(closestChordsSize > 0 ? closestChordsToBeam.front()->line() : 0,
                                               closestChordsSize > 0 ? closestChordsToBeam.front()->vStaffIdx() : 0);
        BeamBase::NotePosition closestEndPos(closestChordsSize > 0 ? closestChordsToBeam.back()->line() : 0,
                                             closestChordsSize > 0 ? closestChordsToBeam.back()->vStaffIdx() : 0);
        int staveClosestToBeamDir = closestStartPos == closestEndPos ? 0 : (closestStartPos > closestEndPos ? 1 : -1);

        // Contradiction, beam must be flat
        if (beamDir != staveClosestToBeamDir && beamDir != 0 && staveClosestToBeamDir != 0) {
            return 0;
        }

        // When the notes on the stave closest to the beam have a neutral direction, set a slant of 1/-1 in the direction of the staff change between first and last chords
        if (staveClosestToBeamDir == 0 && beamDir != 0) {
            if (startPos.staff != endPos.staff) {
                return std::abs(beamDir) * (ldata->up ? 1 : -1);
            }
        }
    }

    // calculate max slope based on note interval
    int interval = std::min(std::abs(endPos.line - startPos.line), (int)_maxSlopes.size() - 1);
    return std::min(maxSlope, _maxSlopes[interval]) * (ldata->up ? 1 : -1);
}

SlopeConstraint BeamTremoloLayout::getSlopeConstraint(const BeamBase::LayoutData* ldata, const BeamBase::NotePosition& startPos,
                                                      const BeamBase::NotePosition& endPos)
{
    if (ldata->notePositions.empty()) {
        return SlopeConstraint::NO_CONSTRAINT;
    }

    // 0 to constrain to flat, 1 to constrain to 0.25, <0 for no constraint
    if (startPos == endPos) {
        return SlopeConstraint::FLAT;
    } else if (ldata->beamType == BeamType::TREMOLO) {
        // tremolos don't need the small slope constraint since they only have two notes
        return SlopeConstraint::NO_CONSTRAINT;
    }

    // if a note is more extreme than the endpoints, slope is 0
    // p.s. _notes is a sorted vector
    const std::vector<ChordRest*>& elements = ldata->elements;
    const std::vector<BeamBase::NotePosition>& notePositions = ldata->notePositions;

    if (elements.size() > 2) {
        if (ldata->up) {
            BeamBase::NotePosition higherEnd = std::min(startPos, endPos);
            if (higherEnd > notePositions[0]) {
                return SlopeConstraint::FLAT; // a note is higher in the staff than the highest end
            }
            if (higherEnd == notePositions[0] && higherEnd >= notePositions[1]) {
                if (higherEnd > notePositions[1]) {
                    return SlopeConstraint::FLAT; // a note is higher in the staff than the highest end
                }
                size_t chordCount = elements.size();
                if (chordCount >= 3 && notePositions.size() >= 3) {
                    bool middleNoteHigherThanHigherEnd = higherEnd >= notePositions[2];
                    if (middleNoteHigherThanHigherEnd) {
                        return SlopeConstraint::FLAT; // two notes are the same as the highest end (notes [0] [1] and [2] higher than or same as higherEnd)
                    }
                    Chord* secondNote = elements[1]->isChord() ? toChord(elements[1]) : nullptr;
                    Chord* secondToLastNote
                        = elements[chordCount - 2]->isChord() ? toChord(elements[chordCount - 2]) : nullptr;
                    bool secondNoteSameHeightAsHigherEnd = startPos < endPos && secondNote && BeamBase::NotePosition(
                        secondNote->upLine(), secondNote->vStaffIdx()) == higherEnd;
                    bool secondToLastNoteSameHeightAsHigherEnd = endPos.line < startPos.line && secondToLastNote && BeamBase::NotePosition(
                        secondToLastNote->upLine(), secondToLastNote->vStaffIdx()) == higherEnd;
                    if (!(secondNoteSameHeightAsHigherEnd || secondToLastNoteSameHeightAsHigherEnd)) {
                        return SlopeConstraint::FLAT; // only one note same as higher end, but it is not a neighbor
                    } else {
                        // there is a single note next to the highest one with equivalent height
                        // and they are neighbors. this is our exception, so
                        // the slope may be a max of 0.25.
                        return SlopeConstraint::SMALL_SLOPE;
                    }
                } else {
                    return SlopeConstraint::FLAT; // only two notes in entire beam, in this case startNote == endNote
                }
            }
        } else {
            BeamBase::NotePosition lowerEnd = std::max(startPos, endPos);
            if (lowerEnd < notePositions[notePositions.size() - 1]) {
                return SlopeConstraint::FLAT;
            }
            if (lowerEnd == notePositions[notePositions.size() - 1]
                && lowerEnd <= notePositions[notePositions.size() - 2]) {
                if (lowerEnd < notePositions[notePositions.size() - 2]) {
                    return SlopeConstraint::FLAT;
                }
                size_t chordCount = elements.size();
                if (chordCount >= 3 && notePositions.size() >= 3) {
                    bool middleNoteLowerThanLowerEnd = lowerEnd <= notePositions[notePositions.size() - 3];
                    if (middleNoteLowerThanLowerEnd) {
                        return SlopeConstraint::FLAT;
                    }
                    const Chord* secondNote = elements[1]->isChord() ? toChord(elements[1]) : nullptr;
                    const Chord* secondToLastNote
                        = elements[chordCount - 2]->isChord() ? toChord(elements[chordCount - 2]) : nullptr;
                    const BeamBase::NotePosition secondNotePos = secondNote ? BeamBase::NotePosition(
                        secondNote->downLine(), secondNote->vStaffIdx()) : BeamBase::NotePosition(0, muse::nidx);
                    const BeamBase::NotePosition secondToLastNotePos = secondToLastNote ? BeamBase::NotePosition(
                        secondToLastNote->downLine(), secondToLastNote->vStaffIdx()) : BeamBase::NotePosition(0, muse::nidx);

                    bool secondNoteSameHeightAsLowerEnd = startPos.line > endPos.line && secondNotePos == lowerEnd;
                    bool secondToLastNoteSameHeightAsLowerEnd = endPos.line > startPos.line && secondToLastNotePos == lowerEnd;
                    if (!(secondNoteSameHeightAsLowerEnd || secondToLastNoteSameHeightAsLowerEnd)) {
                        return SlopeConstraint::FLAT;
                    } else {
                        return SlopeConstraint::SMALL_SLOPE;
                    }
                } else {
                    return SlopeConstraint::FLAT;
                }
            }
        }
    }
    return SlopeConstraint::NO_CONSTRAINT;
}

int BeamTremoloLayout::getMaxSlope(const BeamBase::LayoutData* ldata)
{
    // for 2-indexed interval i (seconds, thirds, etc.)
    // maxSlopes[i] = max slope of beam for notes with interval i

    // calculate max slope based on distance between first and last chords
    double endX = chordBeamAnchorX(ldata, ldata->elements[ldata->elements.size() - 1], ChordBeamAnchorType::Start);
    double startX = chordBeamAnchorX(ldata, ldata->elements[0], ChordBeamAnchorType::End);
    double beamWidth = endX - startX;
    beamWidth /= ldata->spatium;
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

int BeamTremoloLayout::getBeamCount(const BeamBase::LayoutData* ldata, const std::vector<const ChordRest*>& chordRests)
{
    int maxBeams = 0;
    for (const ChordRest* chordRest : chordRests) {
        if (chordRest->isChord() && strokeCount(ldata, chordRest) > maxBeams) {
            maxBeams = strokeCount(ldata, chordRest);
        }
    }
    return maxBeams;
}

double BeamTremoloLayout::chordBeamAnchorX(const BeamBase::LayoutData* ldata, const ChordRest* cr, ChordBeamAnchorType anchorType)
{
    double pagePosX = ldata->trem ? ldata->trem->pagePos().x() : ldata->beam ? ldata->beam->pagePos().x() : 0.0;
    double stemPosX = cr->stemPosX() + cr->pagePos().x() - pagePosX;

    if (!cr->isChord() || !toChord(cr)->stem()) {
        if (!ldata->up) {
            // rests always return the right side of the glyph as their stemPosX
            // so we need to adjust back to the left side if stems are down
            stemPosX -= cr->stemPosX();
        }
        return stemPosX;
    }
    const Chord* chord = toChord(cr);
    const Stem* stem = chord->stem();

    double stemWidth = stem->lineWidthMag();

    switch (anchorType) {
    case ChordBeamAnchorType::Start:
        if (ldata->tab) {
            return stemPosX - 0.5 * stemWidth;
        }

        if (chord->up()) {
            return stemPosX - stemWidth;
        }

        break;
    case ChordBeamAnchorType::Middle:
        if (ldata->tab) {
            return stemPosX;
        }

        return chord->up() ? stemPosX - 0.5 * stemWidth : stemPosX + 0.5 * stemWidth;

    case ChordBeamAnchorType::End:
        if (ldata->tab) {
            return stemPosX + 0.5 * stemWidth;
        }

        if (!chord->up()) {
            return stemPosX + stemWidth;
        }

        break;
    }

    return stemPosX;
}

double BeamTremoloLayout::chordBeamAnchorY(const BeamBase::LayoutData* ldata, const ChordRest* cr)
{
    if (!cr->isChord()) {
        return cr->pagePos().y();
    }

    const Chord* chord = toChord(cr);
    Note* note = cr->up() ? chord->downNote() : chord->upNote();
    PointF position = note->pagePos();

    int upValue = chord->up() ? -1 : 1;
    double beamOffset = ldata->beamWidth / 2 * upValue;

    if (ldata->isBesideTabStaff) {
        double stemLength = ldata->tab->chordStemLength(chord) * (ldata->up ? -1 : 1);
        double y = ldata->tab->chordRestStemPosY(chord) + stemLength;
        y *= ldata->spatium;
        y -= beamOffset;
        return y + chord->pagePos().y();
    }

    return position.y() - note->offset().y() + (chord->defaultStemLength() * upValue) - beamOffset;
}

PointF BeamTremoloLayout::chordBeamAnchor(const BeamBase::LayoutData* ldata, const ChordRest* cr, ChordBeamAnchorType anchorType)
{
    return PointF(chordBeamAnchorX(ldata, cr, anchorType), chordBeamAnchorY(ldata, cr));
}
