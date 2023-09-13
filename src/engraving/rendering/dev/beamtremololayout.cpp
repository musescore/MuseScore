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

namespace mu::engraving {
constexpr std::array _maxSlopes = { 0, 1, 2, 3, 4, 5, 6, 7 };

BeamTremoloLayout::BeamTremoloLayout(EngravingItem* e)
{
    bool isGrace = false;
    IF_ASSERT_FAILED(e && (e->isBeam() || e->isTremolo())) {
        // right now only beams and trems are supported
        return;
    } else if (e->isBeam()) {
        m_beamType = BeamType::BEAM;
        m_beam = toBeam(e);
        m_up = toBeam(e)->up();
        m_trem = nullptr; // there can be many different trems in a beam, they will all be checked
        isGrace = m_beam->elements().front()->isGrace();
    } else { // e->isTremolo()
        m_trem = toTremolo(e);
        m_beamType = BeamType::TREMOLO;
        // check to see if there is a beam happening during this trem
        // if so, it needs to be taken into account in trem placement
        if (m_trem->chord1()->beam() && m_trem->chord1()->beam() == m_trem->chord2()->beam()) {
            m_beam = m_trem->chord1()->beam();
        } else {
            m_beam = nullptr;
        }
        m_up = computeTremoloUp();
        m_trem->setUp(m_up);
        isGrace = m_trem->chord1()->isGrace();
    }
    m_element = e;
    m_spatium = e->spatium();
    m_tick = m_element->tick();
    m_beamSpacing = e->style().styleB(Sid::useWideBeams) ? 4 : 3;
    m_beamDist = (m_beamSpacing / 4.0) * m_spatium * e->mag()
                 * (isGrace ? e->style().styleD(Sid::graceNoteMag) : 1.);
    m_beamWidth = (e->style().styleS(Sid::beamWidth).val() * m_spatium) * e->mag();
    const StaffType* staffType = e->staffType();
    m_tab = (staffType && staffType->isTabStaff()) ? staffType : nullptr;
    m_isBesideTabStaff = m_tab && !m_tab->stemless() && !m_tab->stemThrough();
}

void BeamTremoloLayout::offsetBeamToRemoveCollisions(const std::vector<ChordRest*> chordRests, int& dictator, int& pointer,
                                                     const double startX, const double endX,
                                                     bool isFlat, bool isStartDictator) const
{
    if (endX == startX) {
        return;
    }

    // tolerance eliminates all possibilities of floating point rounding errors
    const double tolerance = m_beamWidth * 0.25 * (m_up ? -1 : 1);
    bool isSmall = m_isGrace || m_element->mag() < 1.;

    double startY = (isStartDictator ? dictator : pointer) * m_spatium / 4 + tolerance;
    double endY = (isStartDictator ? pointer : dictator) * m_spatium / 4 + tolerance;

    for (ChordRest* chordRest : chordRests) {
        if (!chordRest->isChord() || chordRest == m_elements.back() || chordRest == m_elements.front()) {
            continue;
        }

        PointF anchor = chordBeamAnchor(chordRest, ChordBeamAnchorType::Middle) - m_element->pagePos();

        int slope = abs(dictator - pointer);
        double reduction = 0.0;
        if (!isFlat) {
            if (slope <= 3) {
                reduction = 0.25 * m_spatium;
            } else if (slope <= 6) {
                reduction = 0.5 * m_spatium;
            } else { // slope > 6
                reduction = 0.75 * m_spatium;
            }
        }

        if (endX != startX) {
            // avoid division by zero for zero-length beams (can exist as a pre-layout state used
            // for horizontal spacing computations)
            double proportionAlongX = (anchor.x() - startX) / (endX - startX);

            while (true) {
                double desiredY = proportionAlongX * (endY - startY) + startY;
                bool beamClearsAnchor = (m_up && RealIsEqualOrLess(desiredY, anchor.y() + reduction))
                                        || (!m_up && RealIsEqualOrMore(desiredY, anchor.y() - reduction));
                if (beamClearsAnchor) {
                    break;
                }

                if (isFlat || (isSmall && dictator == pointer)) {
                    dictator += m_up ? -1 : 1;
                    pointer += m_up ? -1 : 1;
                } else if (std::abs(dictator - pointer) == 1) {
                    dictator += m_up ? -1 : 1;
                } else {
                    pointer += m_up ? -1 : 1;
                }

                startY = (isStartDictator ? dictator : pointer) * m_spatium / 4 + tolerance;
                endY = (isStartDictator ? pointer : dictator) * m_spatium / 4 + tolerance;
            }
        }
    }
}

void BeamTremoloLayout::offsetBeamWithAnchorShortening(std::vector<ChordRest*> chordRests, int& dictator, int& pointer, int staffLines,
                                                       bool isStartDictator, int stemLengthDictator) const
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
    const int middleLine = getMiddleStaffLine(startChord, endChord, staffLines);
    int dictatorBeams = strokeCount(isStartDictator ? startChord : endChord);
    int pointerBeams = strokeCount(isStartDictator ? endChord : startChord);
    int maxDictatorReduce = stemLengthDictator - minStemLengths[std::max(dictatorBeams - 1, 0)];
    maxDictatorReduce = std::min(abs(dictator - middleLine), maxDictatorReduce);

    bool isFlat = dictator == pointer;
    bool isAscending = startChord->line() > endChord->line();
    int towardBeam = m_up ? -1 : 1;
    int newDictator = dictator;
    int newPointer = pointer;
    int reduce = 0;
    auto fourBeamException = [](int beams, int yPos) {
        yPos += 400; // because there is some weirdness with modular division around zero, add
                     // a large multiple of 4 so that we can guarantee that yPos%4 will be correct
        return beams >= 4 && (yPos % 4 == 2);
    };
    while (!fourBeamException(dictatorBeams, newDictator)
           && !isValidBeamPosition(newDictator, isStartDictator, isAscending, isFlat, staffLines, true)) {
        if (++reduce > maxDictatorReduce) {
            // we can't shorten this stem at all. bring it back to default and start extending
            newDictator = dictator;
            newPointer = pointer;
            while (!isValidBeamPosition(newDictator, isStartDictator, isAscending, isFlat, staffLines, true)) {
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
    newPointer = m_up ? std::min(newPointer, middleLine) : std::max(newPointer, middleLine);
    // walk it back beamwards until we get a position that satisfies both pointer and dictator
    while (!fourBeamException(dictatorBeams, newDictator) && !fourBeamException(pointerBeams, newPointer)
           && (!isValidBeamPosition(newDictator, isStartDictator, isAscending, isFlat, staffLines, true)
               || !isValidBeamPosition(newPointer, !isStartDictator, isAscending, isFlat, staffLines, true))) {
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

void BeamTremoloLayout::extendStem(Chord* chord, double addition)
{
    if ((chord->staff() && chord->staff()->clefType(Fraction()) == ClefType::JIANPU) && (chord->staffType() && chord->staffType()->lines() == 0)) {
        return;
    }

    PointF anchor = chordBeamAnchor(chord, ChordBeamAnchorType::Middle);
    double desiredY;
    if (m_endAnchor.x() > m_startAnchor.x()) {
        double proportionAlongX = (anchor.x() - m_startAnchor.x()) / (m_endAnchor.x() - m_startAnchor.x());
        desiredY = proportionAlongX * (m_endAnchor.y() - m_startAnchor.y()) + m_startAnchor.y();
    } else {
        desiredY = std::max(m_endAnchor.y(), m_startAnchor.y());
    }

    if (chord->up()) {
        chord->setBeamExtension(anchor.y() - desiredY + addition);
    } else {
        chord->setBeamExtension(desiredY - anchor.y() + addition);
    }
    if (chord->stemSlash()) {
        LayoutContext ctx(chord->stemSlash()->score());
        TLayout::layout(chord->stemSlash(), ctx);
    }
}

bool BeamTremoloLayout::isBeamInsideStaff(int yPos, int staffLines, bool allowFloater) const
{
    int aboveStaff = allowFloater ? -2 : -3;
    int belowStaff = (staffLines - 1) * 4 + (allowFloater ? 2 : 3);
    return yPos > aboveStaff && yPos < belowStaff;
}

int BeamTremoloLayout::getOuterBeamPosOffset(int innerBeam, int beamCount, int staffLines) const
{
    int spacing = (m_up ? -m_beamSpacing : m_beamSpacing);
    int offset = (beamCount - 1) * spacing;
    bool isInner = false;
    while (offset != 0 && !isBeamInsideStaff(innerBeam + offset, staffLines, isInner)) {
        offset -= spacing;
        isInner = true;
    }
    return offset;
}

bool BeamTremoloLayout::isValidBeamPosition(int yPos, bool isStart, bool isAscending, bool isFlat, int staffLines, bool isOuter) const
{
    // outside the staff
    bool slantsAway = (m_up && isAscending == isStart) || (!m_up && isAscending != isStart);
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

bool BeamTremoloLayout::is64thBeamPositionException(int& yPos, int staffLines) const
{
    if (m_beamSpacing == 4) {
        return false;
    }
    return yPos == 2 || yPos == staffLines * 4 - 2 || yPos == staffLines * 4 - 6 || yPos == -2;
}

int BeamTremoloLayout::findValidBeamOffset(int outer, int beamCount, int staffLines, bool isStart, bool isAscending,
                                           bool isFlat) const
{
    bool isBeamValid = false;
    int offset = 0;
    int innerBeam = outer + (beamCount - 1) * (m_up ? m_beamSpacing : -m_beamSpacing);
    while (!isBeamValid) {
        while (!isValidBeamPosition(innerBeam + offset, isStart, isAscending, isFlat, staffLines, beamCount < 2)) {
            offset += m_up ? -1 : 1;
        }
        int outerMostBeam = innerBeam + offset + getOuterBeamPosOffset(innerBeam + offset, beamCount, staffLines);
        if (isValidBeamPosition(outerMostBeam, isStart, isAscending, isFlat,
                                staffLines, true)
            || (beamCount == 4 && is64thBeamPositionException(outerMostBeam, staffLines))) {
            isBeamValid = true;
        } else {
            offset += m_up ? -1 : 1;
        }
    }
    return offset;
}

void BeamTremoloLayout::setValidBeamPositions(int& dictator, int& pointer, int beamCountD, int beamCountP, int staffLines,
                                              bool isStartDictator,
                                              bool isFlat, bool isAscending)
{
    bool areBeamsValid = false;
    bool has3BeamsInsideStaff = beamCountD >= 3 || beamCountP >= 3;
    while (!areBeamsValid && has3BeamsInsideStaff && m_beamSpacing != 4) {
        int dictatorInner = dictator + (beamCountD - 1) * (m_up ? m_beamSpacing : -m_beamSpacing);
        // use dictatorInner for both to simulate flat beams
        int outerDictatorOffset = getOuterBeamPosOffset(dictatorInner, beamCountD, staffLines);
        if (std::abs(outerDictatorOffset) <= m_beamSpacing) {
            has3BeamsInsideStaff = false;
            break;
        }
        int offsetD = findValidBeamOffset(dictator, beamCountD, staffLines, isStartDictator, false, true);
        int offsetP = findValidBeamOffset(pointer, beamCountP, staffLines, isStartDictator, false, true);
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
        int dictatorOffset = findValidBeamOffset(dictator, beamCountD, staffLines, isStartDictator, isAscending, isFlat);
        dictator += dictatorOffset;
        pointer += dictatorOffset;
        if (isFlat) {
            pointer = dictator;
            int currOffset = 0;
            for (ChordRest* cr : m_elements) {
                if (!cr->isChord() && (cr != m_elements.front() && cr != m_elements.back())) {
                    continue;
                }
                // we can use dictator beam position because all of the notes have the same beam position
                int beamCount = strokeCount(cr);
                currOffset = findValidBeamOffset(dictator, beamCount, staffLines, isStartDictator, isAscending, isFlat);
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
            pointer += findValidBeamOffset(pointer, beamCountP, staffLines, !isStartDictator, isAscending, isFlat);
            if ((m_up && pointer <= dictator) || (!m_up && pointer >= dictator)) {
                dictator = pointer + (m_up ? -1 : 1);
            } else {
                areBeamsValid = true;
            }
        }
    }
}

void BeamTremoloLayout::addMiddleLineSlant(int& dictator, int& pointer, int beamCount, int middleLine, int interval, int desiredSlant)
{
    bool isSmall = m_element->mag() < 1. || m_isGrace;
    if (interval == 0 || (!isSmall && beamCount > 2 && m_beamSpacing != 4) || noSlope()) {
        return;
    }
    bool isOnMiddleLine = pointer == middleLine && (std::abs(pointer - dictator) < 2);
    if (isOnMiddleLine) {
        if (abs(desiredSlant) == 1 || interval == 1 || (beamCount == 2 && m_beamSpacing != 4 && !isSmall)) {
            dictator = middleLine + (m_up ? -1 : 1);
        } else {
            dictator = middleLine + (m_up ? -2 : 2);
        }
    }
}

void BeamTremoloLayout::add8thSpaceSlant(PointF& dictatorAnchor, int dictator, int pointer, int beamCount,
                                         int interval, int middleLine, bool isFlat)
{
    if (beamCount != 3 || noSlope() || m_beamSpacing != 3) {
        return;
    }
    if ((isFlat && dictator != middleLine) || (dictator != pointer) || interval == 0) {
        return;
    }
    if ((m_up && (dictator + 4) % 4 == 3) || (!m_up && (dictator + 4) % 4 == 1)) {
        return;
    }
    dictatorAnchor.setY(dictatorAnchor.y() + (m_up ? -0.125 * m_spatium : 0.125 * m_spatium));
    m_beamDist += 0.0625 * m_spatium;
}

bool BeamTremoloLayout::computeTremoloUp()
{
    if (!m_beam || !m_beam->cross()) {
        return m_trem->up();
    }
    Chord* c1 = m_trem->chord1();
    Chord* c2 = m_trem->chord2();
    int staffMove = 0;
    for (ChordRest* cr : m_beam->elements()) {
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
        return m_trem->up();
    }
}

int BeamTremoloLayout::strokeCount(ChordRest* cr) const
{
    if (cr->isRest()) {
        return cr->beams();
    }
    int strokes = 0;
    Chord* c = toChord(cr);
    if (m_beamType == BeamType::TREMOLO) {
        strokes = m_trem->lines();
    } else if (m_beamType == BeamType::BEAM && c->tremolo()) {
        Tremolo* t = c->tremolo();
        if (t->twoNotes()) {
            strokes = t->lines();
        }
    }
    strokes += m_beam ? cr->beams() : 0;
    return strokes;
}

bool BeamTremoloLayout::calculateAnchors(const std::vector<ChordRest*>& chordRests, const std::vector<int>& notes)
{
    m_startAnchor = PointF();
    m_endAnchor = PointF();
    if (m_beamType == BeamType::TREMOLO && m_beam) {
        // this is a trem inside a beam, and we are currently calculating the anchors of the tremolo.
        // we can do this as long as the beam has been layed out first: it saves trem anchor positions
        ChordRest* cr1 = m_trem->chord1();
        ChordRest* cr2 = m_trem->chord2();
        double anchorX1 = chordBeamAnchorX(cr1, ChordBeamAnchorType::Middle);
        double anchorX2 = chordBeamAnchorX(cr2, ChordBeamAnchorType::Middle);
        for (const TremAnchor& t : m_beam->tremAnchors()) {
            if (t.chord1 == cr1) {
                m_startAnchor = PointF(anchorX1, t.y1);
                m_endAnchor = PointF(anchorX2, t.y2);
                m_slope = (t.y2 - t.y1) / (anchorX2 - anchorX1);
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
    m_elements = chordRests;
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
    m_isGrace = startChord->isGrace();
    m_notes = notes;
    if (calculateAnchorsCross()) {
        return true;
    }

    m_startAnchor = chordBeamAnchor(startChord, ChordBeamAnchorType::Start);
    m_endAnchor = chordBeamAnchor(endChord, ChordBeamAnchorType::End);

    double startLength = startChord->defaultStemLength();
    double endLength = endChord->defaultStemLength();
    double startAnchorBase = m_startAnchor.y() + (m_up ? startLength : -startLength);
    double endAnchorBase = m_endAnchor.y() + (m_up ? endLength : -endLength);
    int startNote = m_up ? startChord->upNote()->line() : startChord->downNote()->line();
    int endNote = m_up ? endChord->upNote()->line() : endChord->downNote()->line();
    if (m_tab) {
        startNote = m_up ? startChord->upString() : startChord->downString();
        endNote = m_up ? endChord->upString() : endChord->downString();
    }
    const int interval = std::abs(startNote - endNote);
    const bool isStartDictator = m_up ? startNote < endNote : startNote > endNote;
    const double quarterSpace = m_spatium / 4;
    PointF startAnchor = m_startAnchor - m_element->pagePos();
    PointF endAnchor = m_endAnchor - m_element->pagePos();
    int dictator = round((isStartDictator ? startAnchor.y() : endAnchor.y()) / quarterSpace);
    int pointer = round((isStartDictator ? endAnchor.y() : startAnchor.y()) / quarterSpace);

    const int staffLines = startChord->staff()->lines(m_tick);
    const int middleLine = getMiddleStaffLine(startChord, endChord, staffLines);

    int slant = computeDesiredSlant(startNote, endNote, middleLine, dictator, pointer);
    bool isFlat = slant == 0;
    SlopeConstraint specialSlant = isFlat ? getSlopeConstraint(startNote, endNote) : SlopeConstraint::NO_CONSTRAINT;
    bool forceFlat = specialSlant == SlopeConstraint::FLAT;
    bool smallSlant = specialSlant == SlopeConstraint::SMALL_SLOPE;
    if (isFlat) {
        dictator = m_up ? std::min(pointer, dictator) : std::max(pointer, dictator);
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

    int stemLengthStart = abs(round((startAnchorBase - m_startAnchor.y()) / m_spatium * 4));
    int stemLengthEnd = abs(round((endAnchorBase - m_endAnchor.y()) / m_spatium * 4));
    int stemLengthDictator = isStartDictator ? stemLengthStart : stemLengthEnd;
    bool isSmall = m_element->mag() < 1. || m_isGrace;
    if (endAnchor.x() > startAnchor.x()) {
        /* When beam layout is called before horizontal spacing (see LayoutMeasure::getNextMeasure() to
         * know why) the x positions aren't yet determined and may be all zero, which would cause the
         * following function to get stuck in a loop. The if() condition avoids that case. */
        if (!isSmall) {
            // Adjust anchor stems
            offsetBeamWithAnchorShortening(chordRests, dictator, pointer, staffLines, isStartDictator, stemLengthDictator);
        }
        // Adjust inner stems
        offsetBeamToRemoveCollisions(chordRests, dictator, pointer, startAnchor.x(), endAnchor.x(), isFlat, isStartDictator);
    }
    int beamCount = std::max(beamCountD, beamCountP);
    if (!m_tab) {
        if (!m_isGrace) {
            setValidBeamPositions(dictator, pointer, beamCountD, beamCountP, staffLines, isStartDictator, isFlat, isAscending);
        }
        if (!forceFlat) {
            addMiddleLineSlant(dictator, pointer, beamCount, middleLine, interval, smallSlant ? 1 : slant);
        }
    }

    m_startAnchor.setY(quarterSpace * (isStartDictator ? dictator : pointer) + m_element->pagePos().y());
    m_endAnchor.setY(quarterSpace * (isStartDictator ? pointer : dictator) + m_element->pagePos().y());

    bool add8th = true;
    if (m_beamType == BeamType::BEAM && toBeam(m_element)->userModified()) {
        add8th = false;
    }
    if (!m_tab && add8th) {
        add8thSpaceSlant(isStartDictator ? m_startAnchor : m_endAnchor, dictator, pointer, beamCount, interval, middleLine, isFlat);
    }
    m_startAnchor.setX(chordBeamAnchorX(startCr, ChordBeamAnchorType::Start));
    m_endAnchor.setX(chordBeamAnchorX(endCr, ChordBeamAnchorType::End));
    return true;
}

bool BeamTremoloLayout::calculateAnchorsCross()
{
    double spatium = m_element->style().spatium();
    //int fragmentIndex = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    ChordRest* startCr = m_elements.front();
    ChordRest* endCr = m_elements.back();

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
    for (ChordRest* c : m_elements) {
        if (!c) {
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
    for (ChordRest* cr : m_elements) {
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
            maxY = std::min(maxY, chordBeamAnchorY(toChord(c)));
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
            minY = std::max(minY, chordBeamAnchorY(toChord(c)));
        }
    }

    m_startAnchor.ry() = (maxY + minY) / 2;
    m_endAnchor.ry() = (maxY + minY) / 2;
    m_startAnchor.setX(chordBeamAnchorX(startCr, ChordBeamAnchorType::Start));
    m_endAnchor.setX(chordBeamAnchorX(endCr, ChordBeamAnchorType::End));

    m_slope = 0;

    if (!noSlope()) {
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
            if (m_elements[0] == topFirst) {
                yFirst = topFirst->stemPos().y();
                yLast = bottomFirst->stemPos().y();
            } else {
                yFirst = bottomFirst->stemPos().y();
                yLast = topFirst->stemPos().y();
            }
            int desiredSlant = round((yFirst - yLast) / spatium);
            int slant = std::min(std::abs(desiredSlant), getMaxSlope());
            slant *= (desiredSlant < 0) ? -quarterSpace : quarterSpace;
            m_startAnchor.ry() += (slant / 2);
            m_endAnchor.ry() -= (slant / 2);
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
                slant = std::min(std::abs(slant), getMaxSlope());
                if ((!bottomLast && constrainTopToQuarter) || (!topLast && constrainBottomToQuarter)) {
                    slant = 1;
                }
                double slope = slant * (startNote > endNote ? quarterSpace : -quarterSpace);
                m_startAnchor.ry() += (slope / 2);
                m_endAnchor.ry() -= (slope / 2);
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
                slant = std::min(std::abs(slant), getMaxSlope());
                double slope = slant * ((topSlant < 0) ? -quarterSpace : quarterSpace);
                m_startAnchor.ry() += (slope / 2);
                m_endAnchor.ry() -= (slope / 2);
            } else {
                // if the two slopes are in opposite directions, flat!
                // nothing needs to be done, the beam is already horizontal and placed nicely
            }
        }
        m_slope = (m_endAnchor.y() - m_startAnchor.y()) / (m_endAnchor.x() - m_startAnchor.x());
    }
    return true;
}

bool BeamTremoloLayout::noSlope()
{
    return m_beam && m_beam->noSlope();
}

int BeamTremoloLayout::getMiddleStaffLine(ChordRest* startChord, ChordRest* endChord, int staffLines) const
{
    bool isFullSize = RealIsEqual(m_element->mag(), 1.0) && !m_isGrace;
    bool useWideBeams = m_element->score()->style().styleB(Sid::useWideBeams);
    int startBeams = strokeCount(startChord);
    int endBeams = strokeCount(endChord);
    int startMiddleLine = Chord::minStaffOverlap(m_up, staffLines, startBeams, false, m_beamSpacing / 4.0, useWideBeams, isFullSize);
    int endMiddleLine = Chord::minStaffOverlap(m_up, staffLines, endBeams, false, m_beamSpacing / 4.0, useWideBeams, !m_isGrace);

    // offset middle line by 1 or -1 since the anchor is at the middle of the beam,
    // not at the tip of the stem
    if (m_up) {
        return std::min(startMiddleLine, endMiddleLine) + 1;
    }
    return std::max(startMiddleLine, endMiddleLine) - 1;
}

int BeamTremoloLayout::computeDesiredSlant(int startNote, int endNote, int middleLine, int dictator, int pointer) const
{
    Chord* startChord = toChord(m_elements.front());
    if ((startChord->staff() && startChord->staff()->clefType(Fraction()) == ClefType::JIANPU)
        && (startChord->staffType() && startChord->staffType()->lines() == 0)) {
        return 0;
    }

    if (m_beamType == BeamType::BEAM && toBeam(m_element)->noSlope()) {
        return 0;
    }
    int dictatorExtension = middleLine - dictator; // we need to make sure that beams extended to the middle line
    int pointerExtension = middleLine - pointer;  // are properly treated as flat.
    if (m_up) {
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
    SlopeConstraint slopeConstrained = getSlopeConstraint(startNote, endNote);
    if (slopeConstrained == SlopeConstraint::FLAT) {
        return 0;
    } else if (slopeConstrained == SlopeConstraint::SMALL_SLOPE) {
        return dictator > pointer ? -1 : 1;
    }

    // calculate max slope based on distance between first and last chords
    int maxSlope = getMaxSlope();

    // calculate max slope based on note interval
    int interval = std::min(std::abs(endNote - startNote), (int)_maxSlopes.size() - 1);
    return std::min(maxSlope, _maxSlopes[interval]) * (m_up ? 1 : -1);
}

SlopeConstraint BeamTremoloLayout::getSlopeConstraint(int startNote, int endNote) const
{
    if (m_notes.empty()) {
        return SlopeConstraint::NO_CONSTRAINT;
    }

    // 0 to constrain to flat, 1 to constrain to 0.25, <0 for no constraint
    if (startNote == endNote) {
        return SlopeConstraint::FLAT;
    } else if (m_beamType == BeamType::TREMOLO) {
        // tremolos don't need the small slope constraint since they only have two notes
        return SlopeConstraint::NO_CONSTRAINT;
    }
    // if a note is more extreme than the endpoints, slope is 0
    // p.s. _notes is a sorted vector
    if (m_elements.size() > 2) {
        if (m_up) {
            int higherEnd = std::min(startNote, endNote);
            if (higherEnd > m_notes[0]) {
                return SlopeConstraint::FLAT; // a note is higher in the staff than the highest end
            }
            if (higherEnd == m_notes[0] && higherEnd >= m_notes[1]) {
                if (higherEnd > m_notes[1]) {
                    return SlopeConstraint::FLAT; // a note is higher in the staff than the highest end
                }
                size_t chordCount = m_elements.size();
                if (chordCount >= 3 && m_notes.size() >= 3) {
                    bool middleNoteHigherThanHigherEnd = higherEnd >= m_notes[2];
                    if (middleNoteHigherThanHigherEnd) {
                        return SlopeConstraint::FLAT; // two notes are the same as the highest end (notes [0] [1] and [2] higher than or same as higherEnd)
                    }
                    bool secondNoteSameHeightAsHigherEnd = startNote < endNote && m_elements[1]->isChord()
                                                           && toChord(m_elements[1])->upLine() == higherEnd;
                    bool secondToLastNoteSameHeightAsHigherEnd = endNote < startNote && m_elements[chordCount - 2]->isChord() && toChord(
                        m_elements[chordCount - 2])->upLine() == higherEnd;
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
            if (lowerEnd < m_notes[m_notes.size() - 1]) {
                return SlopeConstraint::FLAT;
            }
            if (lowerEnd == m_notes[m_notes.size() - 1] && lowerEnd <= m_notes[m_notes.size() - 2]) {
                if (lowerEnd < m_notes[m_notes.size() - 2]) {
                    return SlopeConstraint::FLAT;
                }
                size_t chordCount = m_elements.size();
                if (chordCount >= 3 && m_notes.size() >= 3) {
                    bool middleNoteLowerThanLowerEnd = lowerEnd <= m_notes[m_notes.size() - 3];
                    if (middleNoteLowerThanLowerEnd) {
                        return SlopeConstraint::FLAT;
                    }
                    bool secondNoteSameHeightAsLowerEnd = startNote > endNote && m_elements[1]->isChord()
                                                          && toChord(m_elements[1])->downLine() == lowerEnd;
                    bool secondToLastNoteSameHeightAsLowerEnd = endNote > startNote && m_elements[chordCount - 2]->isChord() && toChord(
                        m_elements[chordCount - 2])->downLine() == lowerEnd;
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

int BeamTremoloLayout::getMaxSlope() const
{
    // for 2-indexed interval i (seconds, thirds, etc.)
    // maxSlopes[i] = max slope of beam for notes with interval i

    // calculate max slope based on distance between first and last chords
    double endX = chordBeamAnchorX(m_elements[m_elements.size() - 1], ChordBeamAnchorType::Start);
    double startX = chordBeamAnchorX(m_elements[0], ChordBeamAnchorType::End);
    double beamWidth = endX - startX;
    beamWidth /= m_spatium;
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

int BeamTremoloLayout::getBeamCount(const std::vector<ChordRest*> chordRests) const
{
    int maxBeams = 0;
    for (ChordRest* chordRest : chordRests) {
        if (chordRest->isChord() && strokeCount(chordRest) > maxBeams) {
            maxBeams = strokeCount(chordRest);
        }
    }
    return maxBeams;
}

double BeamTremoloLayout::chordBeamAnchorX(const ChordRest* cr, ChordBeamAnchorType anchorType) const
{
    double pagePosX = m_trem ? m_trem->pagePos().x() : m_beam->pagePos().x();
    double stemPosX = cr->stemPosX() + cr->pagePos().x() - pagePosX;

    if ((cr->staff() && cr->staff()->clefType(Fraction()) == ClefType::JIANPU) && (cr->staffType() && cr->staffType()->lines() == 0)) {
        double symWidth = cr->symWidth(SymId::keysig_1_Jianpu);
        return cr->pagePos().x() - pagePosX + symWidth / 5.0 * 2.15;
    }
    if (!cr->isChord() || !toChord(cr)->stem()) {
        if (!m_up) {
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
        if (m_tab) {
            return stemPosX - 0.5 * stemWidth;
        }

        if (chord->up()) {
            return stemPosX - stemWidth;
        }

        break;
    case ChordBeamAnchorType::Middle:
        if (m_tab) {
            return stemPosX;
        }

        return chord->up() ? stemPosX - 0.5 * stemWidth : stemPosX + 0.5 * stemWidth;

    case ChordBeamAnchorType::End:
        if (m_tab) {
            return stemPosX + 0.5 * stemWidth;
        }

        if (!chord->up()) {
            return stemPosX + stemWidth;
        }

        break;
    }

    return stemPosX;
}

double BeamTremoloLayout::chordBeamAnchorY(const ChordRest* cr) const
{
    const Chord* chord = toChord(cr);

    if ((chord->staff() && chord->staff()->clefType(Fraction()) == ClefType::JIANPU) && (chord->staffType() && chord->staffType()->lines() == 0)) {
        Note* note = chord->downNote();
        PointF position = note->pagePos();
        return position.y() - 0.000001;
    }

    if (!cr->isChord()) {
        return cr->pagePos().y();
    }

    Note* note = cr->up() ? chord->downNote() : chord->upNote();
    PointF position = note->pagePos();

    int upValue = chord->up() ? -1 : 1;
    double beamOffset = m_beamWidth / 2 * upValue;

    if (m_isBesideTabStaff) {
        double stemLength = m_tab->chordStemLength(chord) * (m_up ? -1 : 1);
        double y = m_tab->chordRestStemPosY(chord) + stemLength;
        y *= m_spatium;
        y -= beamOffset;
        return y + chord->pagePos().y();
    }

    return position.y() + (chord->defaultStemLength() * upValue) - beamOffset;
}

PointF BeamTremoloLayout::chordBeamAnchor(const ChordRest* cr, ChordBeamAnchorType anchorType) const
{
    return PointF(chordBeamAnchorX(cr, anchorType), chordBeamAnchorY(cr));
}
}
