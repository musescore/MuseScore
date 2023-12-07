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

#include "beambeamlayout.h"

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

void BeamBeamLayout::setupLData(Beam::LayoutData* info, EngravingItem* e)
{
    IF_ASSERT_FAILED(e && e->isBeam()) {
        return;
    }

    Beam* beam = toBeam(e);

    //info->m_beam = toBeam(e);
    info->m_up = beam->up();

    bool isGrace = beam->elements().front()->isGrace();

    info->m_spatium = e->spatium();
    info->m_tick = beam->tick();
    info->m_beamSpacing = e->style().styleB(Sid::useWideBeams) ? 4 : 3;
    info->m_beamDist = (info->m_beamSpacing / 4.0) * info->m_spatium * e->mag() * (isGrace ? e->style().styleD(Sid::graceNoteMag) : 1.);
    info->m_beamWidth = (e->style().styleS(Sid::beamWidth).val() * info->m_spatium) * e->mag();
    const StaffType* staffType = e->staffType();
    info->m_tab = (staffType && staffType->isTabStaff()) ? staffType : nullptr;
    info->m_isBesideTabStaff = info->m_tab && !info->m_tab->stemless() && !info->m_tab->stemThrough();
}

void BeamBeamLayout::offsetBeamToRemoveCollisions(const Beam* beam, const Beam::LayoutData* ldata,
                                                  const std::vector<ChordRest*> chordRests,
                                                  int& dictator,
                                                  int& pointer,
                                                  const double startX, const double endX,
                                                  bool isFlat, bool isStartDictator)
{
    if (endX == startX) {
        return;
    }

    // tolerance eliminates all possibilities of floating point rounding errors
    const double tolerance = ldata->m_beamWidth * 0.25 * (ldata->m_up ? -1 : 1);
    bool isSmall = ldata->m_isGrace || ldata->mag() < 1.;

    double startY = (isStartDictator ? dictator : pointer) * ldata->m_spatium / 4 + tolerance;
    double endY = (isStartDictator ? pointer : dictator) * ldata->m_spatium / 4 + tolerance;

    for (ChordRest* chordRest : chordRests) {
        if (!chordRest->isChord() || chordRest == ldata->m_elements.back() || chordRest == ldata->m_elements.front()) {
            continue;
        }

        PointF anchor = chordBeamAnchor(beam, ldata, chordRest, ChordBeamAnchorType::Middle) - beam->pagePos();

        int slope = abs(dictator - pointer);
        double reduction = 0.0;
        if (!isFlat) {
            if (slope <= 3) {
                reduction = 0.25 * ldata->m_spatium;
            } else if (slope <= 6) {
                reduction = 0.5 * ldata->m_spatium;
            } else { // slope > 6
                reduction = 0.75 * ldata->m_spatium;
            }
        }

        if (endX != startX) {
            // avoid division by zero for zero-length beams (can exist as a pre-layout state used
            // for horizontal spacing computations)
            double proportionAlongX = (anchor.x() - startX) / (endX - startX);

            while (true) {
                double desiredY = proportionAlongX * (endY - startY) + startY;
                bool beamClearsAnchor = (ldata->m_up && RealIsEqualOrLess(desiredY, anchor.y() + reduction))
                                        || (!ldata->m_up && RealIsEqualOrMore(desiredY, anchor.y() - reduction));
                if (beamClearsAnchor) {
                    break;
                }

                if (isFlat || (isSmall && dictator == pointer)) {
                    dictator += ldata->m_up ? -1 : 1;
                    pointer += ldata->m_up ? -1 : 1;
                } else if (std::abs(dictator - pointer) == 1) {
                    dictator += ldata->m_up ? -1 : 1;
                } else {
                    pointer += ldata->m_up ? -1 : 1;
                }

                startY = (isStartDictator ? dictator : pointer) * ldata->m_spatium / 4 + tolerance;
                endY = (isStartDictator ? pointer : dictator) * ldata->m_spatium / 4 + tolerance;
            }
        }
    }
}

void BeamBeamLayout::offsetBeamWithAnchorShortening(const Beam* beam, const Beam::LayoutData* ldata,
                                                    std::vector<ChordRest*> chordRests,
                                                    int& dictator, int& pointer, int staffLines,
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
    const int middleLine = getMiddleStaffLine(beam, ldata, startChord, endChord, staffLines);
    int dictatorBeams = strokeCount(isStartDictator ? startChord : endChord);
    int pointerBeams = strokeCount(isStartDictator ? endChord : startChord);
    int maxDictatorReduce = stemLengthDictator - minStemLengths[std::max(dictatorBeams - 1, 0)];
    maxDictatorReduce = std::min(abs(dictator - middleLine), maxDictatorReduce);

    bool isFlat = dictator == pointer;
    bool isAscending = startChord->line() > endChord->line();
    int towardBeam = ldata->m_up ? -1 : 1;
    int newDictator = dictator;
    int newPointer = pointer;
    int reduce = 0;
    auto fourBeamException = [](int beams, int yPos) {
        yPos += 400; // because there is some weirdness with modular division around zero, add
                     // a large multiple of 4 so that we can guarantee that yPos%4 will be correct
        return beams >= 4 && (yPos % 4 == 2);
    };
    while (!fourBeamException(dictatorBeams, newDictator)
           && !isValidBeamPosition(ldata->m_up, newDictator, isStartDictator, isAscending, isFlat, staffLines, true)) {
        if (++reduce > maxDictatorReduce) {
            // we can't shorten this stem at all. bring it back to default and start extending
            newDictator = dictator;
            newPointer = pointer;
            while (!isValidBeamPosition(ldata->m_up, newDictator, isStartDictator, isAscending, isFlat, staffLines, true)) {
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
    newPointer = ldata->m_up ? std::min(newPointer, middleLine) : std::max(newPointer, middleLine);
    // walk it back beamwards until we get a position that satisfies both pointer and dictator
    while (!fourBeamException(dictatorBeams, newDictator) && !fourBeamException(pointerBeams, newPointer)
           && (!isValidBeamPosition(ldata->m_up, newDictator, isStartDictator, isAscending, isFlat, staffLines, true)
               || !isValidBeamPosition(ldata->m_up, newPointer, !isStartDictator, isAscending, isFlat, staffLines, true))) {
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

void BeamBeamLayout::extendStem(const Beam* beam, const Beam::LayoutData* ldata, Chord* chord, double addition)
{
    PointF anchor = chordBeamAnchor(beam, ldata, chord, ChordBeamAnchorType::Middle);
    double desiredY;
    if (ldata->m_endAnchor.x() > ldata->m_startAnchor.x()) {
        double proportionAlongX = (anchor.x() - ldata->m_startAnchor.x()) / (ldata->m_endAnchor.x() - ldata->m_startAnchor.x());
        desiredY = proportionAlongX * (ldata->m_endAnchor.y() - ldata->m_startAnchor.y()) + ldata->m_startAnchor.y();
    } else {
        desiredY = std::max(ldata->m_endAnchor.y(), ldata->m_startAnchor.y());
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

bool BeamBeamLayout::isBeamInsideStaff(int yPos, int staffLines, bool allowFloater)
{
    int aboveStaff = allowFloater ? -2 : -3;
    int belowStaff = (staffLines - 1) * 4 + (allowFloater ? 2 : 3);
    return yPos > aboveStaff && yPos < belowStaff;
}

int BeamBeamLayout::getOuterBeamPosOffset(const Beam::LayoutData* info, int innerBeam, int beamCount, int staffLines)
{
    int spacing = (info->m_up ? -info->m_beamSpacing : info->m_beamSpacing);
    int offset = (beamCount - 1) * spacing;
    bool isInner = false;
    while (offset != 0 && !isBeamInsideStaff(innerBeam + offset, staffLines, isInner)) {
        offset -= spacing;
        isInner = true;
    }
    return offset;
}

bool BeamBeamLayout::isValidBeamPosition(const bool isUp, int yPos, bool isStart, bool isAscending, bool isFlat, int staffLines,
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

bool BeamBeamLayout::is64thBeamPositionException(const int beamSpacing, int& yPos, int staffLines)
{
    if (beamSpacing == 4) {
        return false;
    }
    return yPos == 2 || yPos == staffLines * 4 - 2 || yPos == staffLines * 4 - 6 || yPos == -2;
}

int BeamBeamLayout::findValidBeamOffset(const Beam::LayoutData* info, int outer, int beamCount, int staffLines, bool isStart,
                                        bool isAscending, bool isFlat)
{
    bool isBeamValid = false;
    int offset = 0;
    int innerBeam = outer + (beamCount - 1) * (info->m_up ? info->m_beamSpacing : -info->m_beamSpacing);
    while (!isBeamValid) {
        while (!isValidBeamPosition(info->m_up, innerBeam + offset, isStart, isAscending, isFlat, staffLines, beamCount < 2)) {
            offset += info->m_up ? -1 : 1;
        }
        int outerMostBeam = innerBeam + offset + getOuterBeamPosOffset(info, innerBeam + offset, beamCount, staffLines);
        if (isValidBeamPosition(info->m_up, outerMostBeam, isStart, isAscending, isFlat, staffLines, true)
            || (beamCount == 4 && is64thBeamPositionException(info->m_beamSpacing, outerMostBeam, staffLines))) {
            isBeamValid = true;
        } else {
            offset += info->m_up ? -1 : 1;
        }
    }
    return offset;
}

void BeamBeamLayout::setValidBeamPositions(const Beam::LayoutData* info, int& dictator, int& pointer, int beamCountD, int beamCountP,
                                           int staffLines,
                                           bool isStartDictator,
                                           bool isFlat, bool isAscending)
{
    bool areBeamsValid = false;
    bool has3BeamsInsideStaff = beamCountD >= 3 || beamCountP >= 3;
    while (!areBeamsValid && has3BeamsInsideStaff && info->m_beamSpacing != 4) {
        int dictatorInner = dictator + (beamCountD - 1) * (info->m_up ? info->m_beamSpacing : -info->m_beamSpacing);
        // use dictatorInner for both to simulate flat beams
        int outerDictatorOffset = getOuterBeamPosOffset(info, dictatorInner, beamCountD, staffLines);
        if (std::abs(outerDictatorOffset) <= info->m_beamSpacing) {
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
            for (ChordRest* cr : info->m_elements) {
                if (!cr->isChord() && (cr != info->m_elements.front() && cr != info->m_elements.back())) {
                    continue;
                }
                // we can use dictator beam position because all of the notes have the same beam position
                int beamCount = strokeCount(cr);
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
            if ((info->m_up && pointer <= dictator) || (!info->m_up && pointer >= dictator)) {
                dictator = pointer + (info->m_up ? -1 : 1);
            } else {
                areBeamsValid = true;
            }
        }
    }
}

void BeamBeamLayout::addMiddleLineSlant(const Beam* beam, const Beam::LayoutData* ldata,
                                        int& dictator, int& pointer, int beamCount,
                                        int middleLine, int interval, int desiredSlant)
{
    bool isSmall = ldata->mag() < 1. || ldata->m_isGrace;
    if (interval == 0 || (!isSmall && beamCount > 2 && ldata->m_beamSpacing != 4) || beam->noSlope()) {
        return;
    }
    bool isOnMiddleLine = pointer == middleLine && (std::abs(pointer - dictator) < 2);
    if (isOnMiddleLine) {
        if (abs(desiredSlant) == 1 || interval == 1 || (beamCount == 2 && ldata->m_beamSpacing != 4 && !isSmall)) {
            dictator = middleLine + (ldata->m_up ? -1 : 1);
        } else {
            dictator = middleLine + (ldata->m_up ? -2 : 2);
        }
    }
}

void BeamBeamLayout::add8thSpaceSlant(const Beam* beam, Beam::LayoutData* ldata, PointF& dictatorAnchor,
                                      int dictator, int pointer, int beamCount,
                                      int interval, int middleLine, bool isFlat)
{
    if (beamCount != 3 || beam->noSlope() || ldata->m_beamSpacing != 3) {
        return;
    }
    if ((isFlat && dictator != middleLine) || (dictator != pointer) || interval == 0) {
        return;
    }
    if ((ldata->m_up && (dictator + 4) % 4 == 3) || (!ldata->m_up && (dictator + 4) % 4 == 1)) {
        return;
    }
    dictatorAnchor.setY(dictatorAnchor.y() + (ldata->m_up ? -0.125 * ldata->m_spatium : 0.125 * ldata->m_spatium));
    ldata->m_beamDist += 0.0625 * ldata->m_spatium;
}

int BeamBeamLayout::strokeCount(ChordRest* cr)
{
    if (cr->isRest()) {
        return cr->beams();
    }

    int strokes = 0;
    Chord* c = toChord(cr);
    if (c->tremolo()) {
        Tremolo* t = c->tremolo();
        if (t->twoNotes()) {
            strokes = t->lines();
        }
    }
    strokes += cr->beams();
    return strokes;
}

bool BeamBeamLayout::calculateAnchors(const Beam* beam, Beam::LayoutData* ldata,
                                      const std::vector<ChordRest*>& chordRests, const std::vector<int>& notes)
{
    ldata->m_startAnchor = PointF();
    ldata->m_endAnchor = PointF();

    Chord* startChord = nullptr;
    Chord* endChord = nullptr;
    ChordRest* startCr = nullptr;
    ChordRest* endCr = nullptr;
    ldata->m_elements = chordRests;
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
    ldata->m_isGrace = startChord->isGrace();
    ldata->m_notes = notes;
    if (calculateAnchorsCross(beam, ldata)) {
        return true;
    }

    ldata->m_startAnchor = chordBeamAnchor(beam, ldata, startChord, ChordBeamAnchorType::Start);
    ldata->m_endAnchor = chordBeamAnchor(beam, ldata, endChord, ChordBeamAnchorType::End);

    double startLength = startChord->defaultStemLength();
    double endLength = endChord->defaultStemLength();
    double startAnchorBase = ldata->m_startAnchor.y() + (ldata->m_up ? startLength : -startLength);
    double endAnchorBase = ldata->m_endAnchor.y() + (ldata->m_up ? endLength : -endLength);
    int startNote = ldata->m_up ? startChord->upNote()->line() : startChord->downNote()->line();
    int endNote = ldata->m_up ? endChord->upNote()->line() : endChord->downNote()->line();
    if (ldata->m_tab) {
        startNote = ldata->m_up ? startChord->upString() : startChord->downString();
        endNote = ldata->m_up ? endChord->upString() : endChord->downString();
    }
    const int interval = std::abs(startNote - endNote);
    const bool isStartDictator = ldata->m_up ? startNote < endNote : startNote > endNote;
    const double quarterSpace = ldata->m_spatium / 4;
    PointF startAnchor = ldata->m_startAnchor - beam->pagePos();
    PointF endAnchor = ldata->m_endAnchor - beam->pagePos();
    int dictator = round((isStartDictator ? startAnchor.y() : endAnchor.y()) / quarterSpace);
    int pointer = round((isStartDictator ? endAnchor.y() : startAnchor.y()) / quarterSpace);

    const int staffLines = startChord->staff()->lines(ldata->m_tick);
    const int middleLine = getMiddleStaffLine(beam, ldata, startChord, endChord, staffLines);

    int slant = computeDesiredSlant(beam, ldata, startNote, endNote, middleLine, dictator, pointer);
    bool isFlat = slant == 0;
    SlopeConstraint specialSlant = isFlat ? getSlopeConstraint(ldata, startNote, endNote) : SlopeConstraint::NO_CONSTRAINT;
    bool forceFlat = specialSlant == SlopeConstraint::FLAT;
    bool smallSlant = specialSlant == SlopeConstraint::SMALL_SLOPE;
    if (isFlat) {
        dictator = ldata->m_up ? std::min(pointer, dictator) : std::max(pointer, dictator);
        pointer = dictator;
    } else {
        if ((dictator > pointer) != (isStartDictator ? startNote > endNote : endNote > startNote)) {
            dictator = pointer - slant;
        } else {
            pointer = dictator + slant;
        }
    }
    bool isAscending = startNote > endNote;
    int beamCountD = strokeCount(isStartDictator ? startChord : endChord);
    int beamCountP = strokeCount(isStartDictator ? endChord : startChord);

    int stemLengthStart = abs(round((startAnchorBase - ldata->m_startAnchor.y()) / ldata->m_spatium * 4));
    int stemLengthEnd = abs(round((endAnchorBase - ldata->m_endAnchor.y()) / ldata->m_spatium * 4));
    int stemLengthDictator = isStartDictator ? stemLengthStart : stemLengthEnd;
    bool isSmall = ldata->mag() < 1. || ldata->m_isGrace;
    if (endAnchor.x() > startAnchor.x()) {
        /* When beam layout is called before horizontal spacing (see LayoutMeasure::getNextMeasure() to
         * know why) the x positions aren't yet determined and may be all zero, which would cause the
         * following function to get stuck in a loop. The if() condition avoids that case. */
        if (!isSmall) {
            // Adjust anchor stems
            offsetBeamWithAnchorShortening(beam, ldata, chordRests, dictator, pointer, staffLines, isStartDictator, stemLengthDictator);
        }
        // Adjust inner stems
        offsetBeamToRemoveCollisions(beam, ldata, chordRests, dictator, pointer, startAnchor.x(), endAnchor.x(), isFlat, isStartDictator);
    }
    int beamCount = std::max(beamCountD, beamCountP);
    if (!ldata->m_tab) {
        if (!ldata->m_isGrace) {
            setValidBeamPositions(ldata, dictator, pointer, beamCountD, beamCountP, staffLines, isStartDictator, isFlat, isAscending);
        }
        if (!forceFlat) {
            addMiddleLineSlant(beam, ldata, dictator, pointer, beamCount, middleLine, interval, smallSlant ? 1 : slant);
        }
    }

    ldata->m_startAnchor.setY(quarterSpace * (isStartDictator ? dictator : pointer) + beam->pagePos().y());
    ldata->m_endAnchor.setY(quarterSpace * (isStartDictator ? pointer : dictator) + beam->pagePos().y());

    bool add8th = true;
    if (beam->userModified()) {
        add8th = false;
    }
    if (!ldata->m_tab && add8th) {
        add8thSpaceSlant(beam, ldata, isStartDictator ? ldata->m_startAnchor : ldata->m_endAnchor,
                         dictator, pointer, beamCount, interval, middleLine, isFlat);
    }
    ldata->m_startAnchor.setX(chordBeamAnchorX(beam, ldata, startCr, ChordBeamAnchorType::Start));
    ldata->m_endAnchor.setX(chordBeamAnchorX(beam, ldata, endCr, ChordBeamAnchorType::End));
    return true;
}

bool BeamBeamLayout::calculateAnchorsCross(const Beam* beam, Beam::LayoutData* ldata)
{
    double spatium = beam->style().spatium();
    //int fragmentIndex = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    ChordRest* startCr = ldata->m_elements.front();
    ChordRest* endCr = ldata->m_elements.back();

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
    for (ChordRest* c : ldata->m_elements) {
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
    for (ChordRest* cr : ldata->m_elements) {
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
            minY = std::max(minY, chordBeamAnchorY(ldata, toChord(c)));
        }
    }

    ldata->m_startAnchor.setY((maxY + minY) / 2);
    ldata->m_endAnchor.setY((maxY + minY) / 2);
    ldata->m_startAnchor.setX(chordBeamAnchorX(beam, ldata, startCr, ChordBeamAnchorType::Start));
    ldata->m_endAnchor.setX(chordBeamAnchorX(beam, ldata, endCr, ChordBeamAnchorType::End));

    ldata->m_slope = 0;

    if (!beam->noSlope()) {
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
            if (ldata->m_elements[0] == topFirst) {
                yFirst = topFirst->stemPos().y();
                yLast = bottomFirst->stemPos().y();
            } else {
                yFirst = bottomFirst->stemPos().y();
                yLast = topFirst->stemPos().y();
            }
            int desiredSlant = round((yFirst - yLast) / spatium);
            int slant = std::min(std::abs(desiredSlant), getMaxSlope(beam, ldata));
            slant *= (desiredSlant < 0) ? -quarterSpace : quarterSpace;
            ldata->m_startAnchor.ry() += (slant / 2);
            ldata->m_endAnchor.ry() -= (slant / 2);
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
                slant = std::min(std::abs(slant), getMaxSlope(beam, ldata));
                if ((!bottomLast && constrainTopToQuarter) || (!topLast && constrainBottomToQuarter)) {
                    slant = 1;
                }
                double slope = slant * (startNote > endNote ? quarterSpace : -quarterSpace);
                ldata->m_startAnchor.ry() += (slope / 2);
                ldata->m_endAnchor.ry() -= (slope / 2);
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
                slant = std::min(std::abs(slant), getMaxSlope(beam, ldata));
                double slope = slant * ((topSlant < 0) ? -quarterSpace : quarterSpace);
                ldata->m_startAnchor.ry() += (slope / 2);
                ldata->m_endAnchor.ry() -= (slope / 2);
            } else {
                // if the two slopes are in opposite directions, flat!
                // nothing needs to be done, the beam is already horizontal and placed nicely
            }
        }
        ldata->m_slope = (ldata->m_endAnchor.y() - ldata->m_startAnchor.y()) / (ldata->m_endAnchor.x() - ldata->m_startAnchor.x());
    }
    return true;
}

bool BeamBeamLayout::noSlope(const Beam* beam)
{
    return beam && beam->noSlope();
}

int BeamBeamLayout::getMiddleStaffLine(const Beam* beam, const Beam::LayoutData* ldata,
                                       ChordRest* startChord, ChordRest* endChord, int staffLines)
{
    bool isFullSize = RealIsEqual(ldata->mag(), 1.0) && !ldata->m_isGrace;
    bool useWideBeams = beam->score()->style().styleB(Sid::useWideBeams);
    int startBeams = strokeCount(startChord);
    int endBeams = strokeCount(endChord);
    int startMiddleLine = Chord::minStaffOverlap(ldata->m_up, staffLines, startBeams, false,
                                                 ldata->m_beamSpacing / 4.0, useWideBeams, isFullSize);
    int endMiddleLine = Chord::minStaffOverlap(ldata->m_up, staffLines, endBeams, false,
                                               ldata->m_beamSpacing / 4.0, useWideBeams, !ldata->m_isGrace);

    // offset middle line by 1 or -1 since the anchor is at the middle of the beam,
    // not at the tip of the stem
    if (ldata->m_up) {
        return std::min(startMiddleLine, endMiddleLine) + 1;
    }
    return std::max(startMiddleLine, endMiddleLine) - 1;
}

int BeamBeamLayout::computeDesiredSlant(const Beam* beam, const Beam::LayoutData* ldata,
                                        int startNote, int endNote, int middleLine,
                                        int dictator, int pointer)
{
    if (beam->noSlope()) {
        return 0;
    }
    int dictatorExtension = middleLine - dictator; // we need to make sure that beams extended to the middle line
    int pointerExtension = middleLine - pointer;  // are properly treated as flat.
    if (ldata->m_up) {
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
    SlopeConstraint slopeConstrained = getSlopeConstraint(ldata, startNote, endNote);
    if (slopeConstrained == SlopeConstraint::FLAT) {
        return 0;
    } else if (slopeConstrained == SlopeConstraint::SMALL_SLOPE) {
        return dictator > pointer ? -1 : 1;
    }

    // calculate max slope based on distance between first and last chords
    int maxSlope = getMaxSlope(beam, ldata);

    // calculate max slope based on note interval
    int interval = std::min(std::abs(endNote - startNote), (int)_maxSlopes.size() - 1);
    return std::min(maxSlope, _maxSlopes[interval]) * (ldata->m_up ? 1 : -1);
}

BeamBeamLayout::SlopeConstraint BeamBeamLayout::getSlopeConstraint(const Beam::LayoutData* info, int startNote, int endNote)
{
    if (info->m_notes.empty()) {
        return SlopeConstraint::NO_CONSTRAINT;
    }

    // 0 to constrain to flat, 1 to constrain to 0.25, <0 for no constraint
    if (startNote == endNote) {
        return SlopeConstraint::FLAT;
    }

    // if a note is more extreme than the endpoints, slope is 0
    // p.s. _notes is a sorted vector
    if (info->m_elements.size() > 2) {
        if (info->m_up) {
            int higherEnd = std::min(startNote, endNote);
            if (higherEnd > info->m_notes[0]) {
                return SlopeConstraint::FLAT; // a note is higher in the staff than the highest end
            }
            if (higherEnd == info->m_notes[0] && higherEnd >= info->m_notes[1]) {
                if (higherEnd > info->m_notes[1]) {
                    return SlopeConstraint::FLAT; // a note is higher in the staff than the highest end
                }
                size_t chordCount = info->m_elements.size();
                if (chordCount >= 3 && info->m_notes.size() >= 3) {
                    bool middleNoteHigherThanHigherEnd = higherEnd >= info->m_notes[2];
                    if (middleNoteHigherThanHigherEnd) {
                        return SlopeConstraint::FLAT; // two notes are the same as the highest end (notes [0] [1] and [2] higher than or same as higherEnd)
                    }
                    bool secondNoteSameHeightAsHigherEnd = startNote < endNote && info->m_elements[1]->isChord()
                                                           && toChord(info->m_elements[1])->upLine() == higherEnd;
                    bool secondToLastNoteSameHeightAsHigherEnd = endNote < startNote && info->m_elements[chordCount - 2]->isChord()
                                                                 && toChord(
                        info->m_elements[chordCount - 2])->upLine() == higherEnd;
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
            int lowerEnd = std::max(startNote, endNote);
            if (lowerEnd < info->m_notes[info->m_notes.size() - 1]) {
                return SlopeConstraint::FLAT;
            }
            if (lowerEnd == info->m_notes[info->m_notes.size() - 1] && lowerEnd <= info->m_notes[info->m_notes.size() - 2]) {
                if (lowerEnd < info->m_notes[info->m_notes.size() - 2]) {
                    return SlopeConstraint::FLAT;
                }
                size_t chordCount = info->m_elements.size();
                if (chordCount >= 3 && info->m_notes.size() >= 3) {
                    bool middleNoteLowerThanLowerEnd = lowerEnd <= info->m_notes[info->m_notes.size() - 3];
                    if (middleNoteLowerThanLowerEnd) {
                        return SlopeConstraint::FLAT;
                    }
                    bool secondNoteSameHeightAsLowerEnd = startNote > endNote && info->m_elements[1]->isChord()
                                                          && toChord(info->m_elements[1])->downLine() == lowerEnd;
                    bool secondToLastNoteSameHeightAsLowerEnd = endNote > startNote && info->m_elements[chordCount - 2]->isChord()
                                                                && toChord(
                        info->m_elements[chordCount - 2])->downLine() == lowerEnd;
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

int BeamBeamLayout::getMaxSlope(const Beam* beam, const Beam::LayoutData* ldata)
{
    // for 2-indexed interval i (seconds, thirds, etc.)
    // maxSlopes[i] = max slope of beam for notes with interval i

    // calculate max slope based on distance between first and last chords
    double endX = chordBeamAnchorX(beam, ldata, ldata->m_elements[ldata->m_elements.size() - 1], ChordBeamAnchorType::Start);
    double startX = chordBeamAnchorX(beam, ldata, ldata->m_elements[0], ChordBeamAnchorType::End);
    double beamWidth = endX - startX;
    beamWidth /= ldata->m_spatium;
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

int BeamBeamLayout::getBeamCount(const std::vector<ChordRest*>& chordRests)
{
    int maxBeams = 0;
    for (ChordRest* chordRest : chordRests) {
        if (chordRest->isChord() && strokeCount(chordRest) > maxBeams) {
            maxBeams = strokeCount(chordRest);
        }
    }
    return maxBeams;
}

double BeamBeamLayout::chordBeamAnchorX(const Beam* beam, const Beam::LayoutData* ldata, const ChordRest* cr,
                                        ChordBeamAnchorType anchorType)
{
    double pagePosX = beam->pagePos().x();
    double stemPosX = cr->stemPosX() + cr->pagePos().x() - pagePosX;

    if (!cr->isChord() || !toChord(cr)->stem()) {
        if (!ldata->m_up) {
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
        if (ldata->m_tab) {
            return stemPosX - 0.5 * stemWidth;
        }

        if (chord->up()) {
            return stemPosX - stemWidth;
        }

        break;
    case ChordBeamAnchorType::Middle:
        if (ldata->m_tab) {
            return stemPosX;
        }

        return chord->up() ? stemPosX - 0.5 * stemWidth : stemPosX + 0.5 * stemWidth;

    case ChordBeamAnchorType::End:
        if (ldata->m_tab) {
            return stemPosX + 0.5 * stemWidth;
        }

        if (!chord->up()) {
            return stemPosX + stemWidth;
        }

        break;
    }

    return stemPosX;
}

double BeamBeamLayout::chordBeamAnchorY(const Beam::LayoutData* info, const ChordRest* cr)
{
    if (!cr->isChord()) {
        return cr->pagePos().y();
    }

    const Chord* chord = toChord(cr);
    Note* note = cr->up() ? chord->downNote() : chord->upNote();
    PointF position = note->pagePos();

    int upValue = chord->up() ? -1 : 1;
    double beamOffset = info->m_beamWidth / 2 * upValue;

    if (info->m_isBesideTabStaff) {
        double stemLength = info->m_tab->chordStemLength(chord) * (info->m_up ? -1 : 1);
        double y = info->m_tab->chordRestStemPosY(chord) + stemLength;
        y *= info->m_spatium;
        y -= beamOffset;
        return y + chord->pagePos().y();
    }

    return position.y() - note->offset().y() + (chord->defaultStemLength() * upValue) - beamOffset;
}

PointF BeamBeamLayout::chordBeamAnchor(const Beam* beam, const Beam::LayoutData* ldata, const ChordRest* cr, ChordBeamAnchorType anchorType)
{
    return PointF(chordBeamAnchorX(beam, ldata, cr, anchorType), chordBeamAnchorY(ldata, cr));
}
