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
#include "../dom/tremolotwochord.h"

#include "tlayout.h"
#include "chordlayout.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

constexpr std::array _maxSlopes = { 0, 1, 2, 3, 4, 5, 6, 7 };

void BeamTremoloLayout::setupLData(BeamBase::LayoutData* ldata, EngravingItem* e)
{
    bool isGrace = false;
    IF_ASSERT_FAILED(e && (e->isBeam() || e->isType(ElementType::TREMOLO_TWOCHORD))) {
        // right now only beams and trems are supported
        return;
    } else if (e->isBeam()) {
        ldata->m_beamType = BeamType::BEAM;
        ldata->m_beam = toBeam(e);
        ldata->m_up = toBeam(e)->up();
        ldata->m_trem = nullptr; // there can be many different trems in a beam, they will all be checked
        isGrace = ldata->m_beam->elements().front()->isGrace();
    } else {
        ldata->m_trem = item_cast<TremoloTwoChord*>(e);
        ldata->m_beamType = BeamType::TREMOLO;
        // check to see if there is a beam happening during this trem
        // if so, it needs to be taken into account in trem placement
        if (ldata->m_trem->chord1()->beam() && ldata->m_trem->chord1()->beam() == ldata->m_trem->chord2()->beam()) {
            ldata->m_beam = ldata->m_trem->chord1()->beam();
        } else {
            ldata->m_beam = nullptr;
        }
        ldata->m_up = computeTremoloUp(ldata);
        ldata->m_trem->setUp(ldata->m_up);
        isGrace = ldata->m_trem->chord1()->isGrace();
    }
    ldata->m_element = e;
    ldata->m_spatium = e->spatium();
    ldata->m_tick = ldata->m_element->tick();
    ldata->m_beamSpacing = e->style().styleB(Sid::useWideBeams) ? 4 : 3;
    ldata->m_beamDist = (ldata->m_beamSpacing / 4.0) * ldata->m_spatium * e->mag() * (isGrace ? e->style().styleD(Sid::graceNoteMag) : 1.);
    ldata->m_beamWidth = (e->style().styleS(Sid::beamWidth).val() * ldata->m_spatium) * e->mag();
    const StaffType* staffType = e->staffType();
    ldata->m_tab = (staffType && staffType->isTabStaff()) ? staffType : nullptr;
    ldata->m_isBesideTabStaff = ldata->m_tab && !ldata->m_tab->stemless() && !ldata->m_tab->stemThrough();
}

void BeamTremoloLayout::offsetBeamToRemoveCollisions(const BeamBase::LayoutData* ldata, const std::vector<ChordRest*> chordRests,
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
    bool isSmall = ldata->m_isGrace || ldata->m_element->mag() < 1.;

    double startY = (isStartDictator ? dictator : pointer) * ldata->m_spatium / 4 + tolerance;
    double endY = (isStartDictator ? pointer : dictator) * ldata->m_spatium / 4 + tolerance;

    for (ChordRest* chordRest : chordRests) {
        if (!chordRest->isChord() || chordRest == ldata->m_elements.back() || chordRest == ldata->m_elements.front()) {
            continue;
        }

        PointF anchor = chordBeamAnchor(ldata, chordRest, ChordBeamAnchorType::Middle) - ldata->m_element->pagePos();

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

void BeamTremoloLayout::offsetBeamWithAnchorShortening(const BeamBase::LayoutData* ldata, std::vector<ChordRest*> chordRests, int& dictator,
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
    const int middleLine = getMiddleStaffLine(ldata, startChord, endChord, staffLines);
    int dictatorBeams = strokeCount(ldata, isStartDictator ? startChord : endChord);
    int pointerBeams = strokeCount(ldata, isStartDictator ? endChord : startChord);
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

void BeamTremoloLayout::extendStem(const BeamBase::LayoutData* ldata, Chord* chord, double addition)
{
    PointF anchor = chordBeamAnchor(ldata, chord, ChordBeamAnchorType::Middle);
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

bool BeamTremoloLayout::isBeamInsideStaff(int yPos, int staffLines, bool allowFloater)
{
    int aboveStaff = allowFloater ? -2 : -3;
    int belowStaff = (staffLines - 1) * 4 + (allowFloater ? 2 : 3);
    return yPos > aboveStaff && yPos < belowStaff;
}

int BeamTremoloLayout::getOuterBeamPosOffset(const BeamBase::LayoutData* ldata, int innerBeam, int beamCount, int staffLines)
{
    int spacing = (ldata->m_up ? -ldata->m_beamSpacing : ldata->m_beamSpacing);
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
    int innerBeam = outer + (beamCount - 1) * (ldata->m_up ? ldata->m_beamSpacing : -ldata->m_beamSpacing);
    while (!isBeamValid) {
        while (!isValidBeamPosition(ldata->m_up, innerBeam + offset, isStart, isAscending, isFlat, staffLines, beamCount < 2)) {
            offset += ldata->m_up ? -1 : 1;
        }
        int outerMostBeam = innerBeam + offset + getOuterBeamPosOffset(ldata, innerBeam + offset, beamCount, staffLines);
        if (isValidBeamPosition(ldata->m_up, outerMostBeam, isStart, isAscending, isFlat, staffLines, true)
            || (beamCount == 4 && is64thBeamPositionException(ldata->m_beamSpacing, outerMostBeam, staffLines))) {
            isBeamValid = true;
        } else {
            offset += ldata->m_up ? -1 : 1;
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
    while (!areBeamsValid && has3BeamsInsideStaff && ldata->m_beamSpacing != 4) {
        int dictatorInner = dictator + (beamCountD - 1) * (ldata->m_up ? ldata->m_beamSpacing : -ldata->m_beamSpacing);
        // use dictatorInner for both to simulate flat beams
        int outerDictatorOffset = getOuterBeamPosOffset(ldata, dictatorInner, beamCountD, staffLines);
        if (std::abs(outerDictatorOffset) <= ldata->m_beamSpacing) {
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
            for (ChordRest* cr : ldata->m_elements) {
                if (!cr->isChord() && (cr != ldata->m_elements.front() && cr != ldata->m_elements.back())) {
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
            if ((ldata->m_up && pointer <= dictator) || (!ldata->m_up && pointer >= dictator)) {
                dictator = pointer + (ldata->m_up ? -1 : 1);
            } else {
                areBeamsValid = true;
            }
        }
    }
}

void BeamTremoloLayout::addMiddleLineSlant(const BeamBase::LayoutData* ldata, int& dictator, int& pointer, int beamCount, int middleLine,
                                           int interval, int desiredSlant)
{
    bool isSmall = ldata->m_element->mag() < 1. || ldata->m_isGrace;
    if (interval == 0 || (!isSmall && beamCount > 2 && ldata->m_beamSpacing != 4) || noSlope(ldata->m_beam)) {
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

void BeamTremoloLayout::add8thSpaceSlant(BeamBase::LayoutData* ldata, PointF& dictatorAnchor, int dictator, int pointer, int beamCount,
                                         int interval, int middleLine, bool isFlat)
{
    if (beamCount != 3 || noSlope(ldata->m_beam) || ldata->m_beamSpacing != 3) {
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

bool BeamTremoloLayout::computeTremoloUp(const BeamBase::LayoutData* ldata)
{
    if (!ldata->m_beam || !ldata->m_beam->cross()) {
        return ldata->m_trem->up();
    }
    Chord* c1 = ldata->m_trem->chord1();
    Chord* c2 = ldata->m_trem->chord2();
    int staffMove = 0;
    for (ChordRest* cr : ldata->m_beam->elements()) {
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
        return ldata->m_trem->up();
    }
}

int BeamTremoloLayout::strokeCount(const BeamBase::LayoutData* ldata, ChordRest* cr)
{
    if (cr->isRest()) {
        return cr->beams();
    }
    int strokes = 0;
    Chord* c = toChord(cr);
    if (ldata->m_beamType == BeamType::TREMOLO) {
        strokes = ldata->m_trem->lines();
    } else if (ldata->m_beamType == BeamType::BEAM && c->tremoloTwoChord()) {
        strokes = c->tremoloTwoChord()->lines();
    }
    strokes += ldata->m_beam ? cr->beams() : 0;
    return strokes;
}

bool BeamTremoloLayout::calculateAnchors(BeamBase::LayoutData* ldata, const std::vector<ChordRest*>& chordRests,
                                         const std::vector<int>& notes)
{
    ldata->m_startAnchor = PointF();
    ldata->m_endAnchor = PointF();
    if (ldata->m_beamType == BeamType::TREMOLO && ldata->m_beam) {
        // this is a trem inside a beam, and we are currently calculating the anchors of the tremolo.
        // we can do this as long as the beam has been layed out first: it saves trem anchor positions
        ChordRest* cr1 = ldata->m_trem->chord1();
        ChordRest* cr2 = ldata->m_trem->chord2();
        double anchorX1 = chordBeamAnchorX(ldata, cr1, ChordBeamAnchorType::Middle);
        double anchorX2 = chordBeamAnchorX(ldata, cr2, ChordBeamAnchorType::Middle);
        for (const TremAnchor& t : ldata->m_beam->tremAnchors()) {
            if (t.chord1 == cr1) {
                ldata->m_startAnchor = PointF(anchorX1, t.y1);
                ldata->m_endAnchor = PointF(anchorX2, t.y2);
                ldata->m_slope = (t.y2 - t.y1) / (anchorX2 - anchorX1);
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
    if (calculateAnchorsCross(ldata)) {
        return true;
    }

    ldata->m_startAnchor = chordBeamAnchor(ldata, startChord, ChordBeamAnchorType::Start);
    ldata->m_endAnchor = chordBeamAnchor(ldata, endChord, ChordBeamAnchorType::End);

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
    PointF startAnchor = ldata->m_startAnchor - ldata->m_element->pagePos();
    PointF endAnchor = ldata->m_endAnchor - ldata->m_element->pagePos();
    int dictator = round((isStartDictator ? startAnchor.y() : endAnchor.y()) / quarterSpace);
    int pointer = round((isStartDictator ? endAnchor.y() : startAnchor.y()) / quarterSpace);

    const int staffLines = startChord->staff()->lines(ldata->m_tick);
    const int middleLine = getMiddleStaffLine(ldata, startChord, endChord, staffLines);

    int slant = computeDesiredSlant(ldata, startNote, endNote, middleLine, dictator, pointer);
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
    int beamCountD = strokeCount(ldata, isStartDictator ? startChord : endChord);
    int beamCountP = strokeCount(ldata, isStartDictator ? endChord : startChord);

    int stemLengthStart = abs(round((startAnchorBase - ldata->m_startAnchor.y()) / ldata->m_spatium * 4));
    int stemLengthEnd = abs(round((endAnchorBase - ldata->m_endAnchor.y()) / ldata->m_spatium * 4));
    int stemLengthDictator = isStartDictator ? stemLengthStart : stemLengthEnd;
    bool isSmall = ldata->m_element->mag() < 1. || ldata->m_isGrace;
    if (endAnchor.x() > startAnchor.x()) {
        /* When beam layout is called before horizontal spacing (see LayoutMeasure::getNextMeasure() to
         * know why) the x positions aren't yet determined and may be all zero, which would cause the
         * following function to get stuck in a loop. The if() condition avoids that case. */
        if (!isSmall) {
            // Adjust anchor stems
            offsetBeamWithAnchorShortening(ldata, chordRests, dictator, pointer, staffLines, isStartDictator, stemLengthDictator);
        }
        // Adjust inner stems
        offsetBeamToRemoveCollisions(ldata, chordRests, dictator, pointer, startAnchor.x(), endAnchor.x(), isFlat, isStartDictator);
    }
    int beamCount = std::max(beamCountD, beamCountP);
    if (!ldata->m_tab) {
        if (!ldata->m_isGrace) {
            setValidBeamPositions(ldata, dictator, pointer, beamCountD, beamCountP, staffLines, isStartDictator, isFlat, isAscending);
        }
        if (!forceFlat) {
            addMiddleLineSlant(ldata, dictator, pointer, beamCount, middleLine, interval, smallSlant ? 1 : slant);
        }
    }

    ldata->m_startAnchor.setY(quarterSpace * (isStartDictator ? dictator : pointer) + ldata->m_element->pagePos().y());
    ldata->m_endAnchor.setY(quarterSpace * (isStartDictator ? pointer : dictator) + ldata->m_element->pagePos().y());

    bool add8th = true;
    if (ldata->m_beamType == BeamType::BEAM && toBeam(ldata->m_element)->userModified()) {
        add8th = false;
    }
    if (!ldata->m_tab && add8th) {
        add8thSpaceSlant(ldata, isStartDictator ? ldata->m_startAnchor : ldata->m_endAnchor, dictator, pointer, beamCount, interval,
                         middleLine, isFlat);
    }
    ldata->m_startAnchor.setX(chordBeamAnchorX(ldata, startCr, ChordBeamAnchorType::Start));
    ldata->m_endAnchor.setX(chordBeamAnchorX(ldata, endCr, ChordBeamAnchorType::End));
    return true;
}

bool BeamTremoloLayout::calculateAnchorsCross(BeamBase::LayoutData* ldata)
{
    double spatium = ldata->m_element->style().spatium();
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
    ldata->m_startAnchor.setX(chordBeamAnchorX(ldata, startCr, ChordBeamAnchorType::Start));
    ldata->m_endAnchor.setX(chordBeamAnchorX(ldata, endCr, ChordBeamAnchorType::End));

    ldata->m_slope = 0;

    if (!noSlope(ldata->m_beam)) {
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
            int slant = std::min(std::abs(desiredSlant), getMaxSlope(ldata));
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
                slant = std::min(std::abs(slant), getMaxSlope(ldata));
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
                slant = std::min(std::abs(slant), getMaxSlope(ldata));
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

bool BeamTremoloLayout::noSlope(const Beam* beam)
{
    return beam && beam->noSlope();
}

int BeamTremoloLayout::getMiddleStaffLine(const BeamBase::LayoutData* ldata, ChordRest* startChord, ChordRest* endChord, int staffLines)
{
    bool isFullSize = RealIsEqual(ldata->m_element->mag(), 1.0) && !ldata->m_isGrace;
    bool useWideBeams = ldata->m_element->score()->style().styleB(Sid::useWideBeams);
    int startBeams = strokeCount(ldata, startChord);
    int endBeams = strokeCount(ldata, endChord);
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

int BeamTremoloLayout::computeDesiredSlant(const BeamBase::LayoutData* ldata, int startNote, int endNote, int middleLine, int dictator,
                                           int pointer)
{
    if (ldata->m_beamType == BeamType::BEAM && toBeam(ldata->m_element)->noSlope()) {
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
    int maxSlope = getMaxSlope(ldata);

    // calculate max slope based on note interval
    int interval = std::min(std::abs(endNote - startNote), (int)_maxSlopes.size() - 1);
    return std::min(maxSlope, _maxSlopes[interval]) * (ldata->m_up ? 1 : -1);
}

SlopeConstraint BeamTremoloLayout::getSlopeConstraint(const BeamBase::LayoutData* ldata, int startNote, int endNote)
{
    if (ldata->m_notes.empty()) {
        return SlopeConstraint::NO_CONSTRAINT;
    }

    // 0 to constrain to flat, 1 to constrain to 0.25, <0 for no constraint
    if (startNote == endNote) {
        return SlopeConstraint::FLAT;
    } else if (ldata->m_beamType == BeamType::TREMOLO) {
        // tremolos don't need the small slope constraint since they only have two notes
        return SlopeConstraint::NO_CONSTRAINT;
    }
    // if a note is more extreme than the endpoints, slope is 0
    // p.s. _notes is a sorted vector
    if (ldata->m_elements.size() > 2) {
        if (ldata->m_up) {
            int higherEnd = std::min(startNote, endNote);
            if (higherEnd > ldata->m_notes[0]) {
                return SlopeConstraint::FLAT; // a note is higher in the staff than the highest end
            }
            if (higherEnd == ldata->m_notes[0] && higherEnd >= ldata->m_notes[1]) {
                if (higherEnd > ldata->m_notes[1]) {
                    return SlopeConstraint::FLAT; // a note is higher in the staff than the highest end
                }
                size_t chordCount = ldata->m_elements.size();
                if (chordCount >= 3 && ldata->m_notes.size() >= 3) {
                    bool middleNoteHigherThanHigherEnd = higherEnd >= ldata->m_notes[2];
                    if (middleNoteHigherThanHigherEnd) {
                        return SlopeConstraint::FLAT; // two notes are the same as the highest end (notes [0] [1] and [2] higher than or same as higherEnd)
                    }
                    bool secondNoteSameHeightAsHigherEnd = startNote < endNote && ldata->m_elements[1]->isChord()
                                                           && toChord(ldata->m_elements[1])->upLine() == higherEnd;
                    bool secondToLastNoteSameHeightAsHigherEnd = endNote < startNote && ldata->m_elements[chordCount - 2]->isChord()
                                                                 && toChord(
                        ldata->m_elements[chordCount - 2])->upLine() == higherEnd;
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
            if (lowerEnd < ldata->m_notes[ldata->m_notes.size() - 1]) {
                return SlopeConstraint::FLAT;
            }
            if (lowerEnd == ldata->m_notes[ldata->m_notes.size() - 1] && lowerEnd <= ldata->m_notes[ldata->m_notes.size() - 2]) {
                if (lowerEnd < ldata->m_notes[ldata->m_notes.size() - 2]) {
                    return SlopeConstraint::FLAT;
                }
                size_t chordCount = ldata->m_elements.size();
                if (chordCount >= 3 && ldata->m_notes.size() >= 3) {
                    bool middleNoteLowerThanLowerEnd = lowerEnd <= ldata->m_notes[ldata->m_notes.size() - 3];
                    if (middleNoteLowerThanLowerEnd) {
                        return SlopeConstraint::FLAT;
                    }
                    bool secondNoteSameHeightAsLowerEnd = startNote > endNote && ldata->m_elements[1]->isChord()
                                                          && toChord(ldata->m_elements[1])->downLine() == lowerEnd;
                    bool secondToLastNoteSameHeightAsLowerEnd = endNote > startNote && ldata->m_elements[chordCount - 2]->isChord()
                                                                && toChord(
                        ldata->m_elements[chordCount - 2])->downLine() == lowerEnd;
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
    double endX = chordBeamAnchorX(ldata, ldata->m_elements[ldata->m_elements.size() - 1], ChordBeamAnchorType::Start);
    double startX = chordBeamAnchorX(ldata, ldata->m_elements[0], ChordBeamAnchorType::End);
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

int BeamTremoloLayout::getBeamCount(const BeamBase::LayoutData* ldata, const std::vector<ChordRest*> chordRests)
{
    int maxBeams = 0;
    for (ChordRest* chordRest : chordRests) {
        if (chordRest->isChord() && strokeCount(ldata, chordRest) > maxBeams) {
            maxBeams = strokeCount(ldata, chordRest);
        }
    }
    return maxBeams;
}

double BeamTremoloLayout::chordBeamAnchorX(const BeamBase::LayoutData* ldata, const ChordRest* cr, ChordBeamAnchorType anchorType)
{
    double pagePosX = ldata->m_trem ? ldata->m_trem->pagePos().x() : ldata->m_beam->pagePos().x();
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

double BeamTremoloLayout::chordBeamAnchorY(const BeamBase::LayoutData* ldata, const ChordRest* cr)
{
    if (!cr->isChord()) {
        return cr->pagePos().y();
    }

    const Chord* chord = toChord(cr);
    Note* note = cr->up() ? chord->downNote() : chord->upNote();
    PointF position = note->pagePos();

    int upValue = chord->up() ? -1 : 1;
    double beamOffset = ldata->m_beamWidth / 2 * upValue;

    if (ldata->m_isBesideTabStaff) {
        double stemLength = ldata->m_tab->chordStemLength(chord) * (ldata->m_up ? -1 : 1);
        double y = ldata->m_tab->chordRestStemPosY(chord) + stemLength;
        y *= ldata->m_spatium;
        y -= beamOffset;
        return y + chord->pagePos().y();
    }

    return position.y() - note->offset().y() + (chord->defaultStemLength() * upValue) - beamOffset;
}

PointF BeamTremoloLayout::chordBeamAnchor(const BeamBase::LayoutData* ldata, const ChordRest* cr, ChordBeamAnchorType anchorType)
{
    return PointF(chordBeamAnchorX(ldata, cr, anchorType), chordBeamAnchorY(ldata, cr));
}
