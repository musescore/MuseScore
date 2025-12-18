/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "../editing/editdata.h"
#include "../editing/elementeditdata.h"
#include "../editing/transpose.h"

#include "accidental.h"
#include "actionicon.h"
#include "chord.h"
#include "guitarbend.h"
#include "navigate.h"
#include "note.h"
#include "part.h"
#include "rest.h"
#include "score.h"
#include "staff.h"
#include "tie.h"
#include "utils.h"
#include "whammybar.h"

namespace mu::engraving {
/****************************************
 *              GuitarBend
 * **************************************/

GuitarBend::GuitarBend(EngravingItem* parent)
    : SLine(ElementType::GUITAR_BEND, parent, ElementFlag::MOVABLE)
{
}

GuitarBend::GuitarBend(const GuitarBend& g)
    : SLine(g)
{
    m_bendType = g.bendType();
    _bendAmountInQuarterTones = g.bendAmountInQuarterTones();
}

GuitarBend::~GuitarBend()
{
    if (m_holdLine) {
        delete m_holdLine;
    }
}

LineSegment* GuitarBend::createLineSegment(System* parent)
{
    GuitarBendSegment* seg = new GuitarBendSegment(this, parent);
    seg->setTrack(track());
    seg->setColor(lineColor());
    return seg;
}

Note* GuitarBend::startNote() const
{
    EngravingItem* startEl = startElement();
    IF_ASSERT_FAILED(startEl && startEl->isNote()) {
        return nullptr;
    }
    return toNote(startEl);
}

Note* GuitarBend::startNoteOfChain() const
{
    Note* startOfChain = startNote();
    GuitarBend* prevBend = findPrecedingBend();
    while (startOfChain && prevBend) {
        startOfChain = prevBend->startNote();
        prevBend = prevBend->findPrecedingBend();
    }

    return startOfChain;
}

Note* GuitarBend::endNote() const
{
    EngravingItem* endEl = endElement();
    IF_ASSERT_FAILED(endEl && endEl->isNote()) {
        return nullptr;
    }
    return toNote(endEl);
}

void GuitarBend::changeBendAmount(int bendAmount)
{
    if (bendAmount == SLACK_BEND_AMOUNT) {
        undoChangeProperty(Pid::GUITAR_DIVE_IS_SLACK, true);
        if (endNote()) {
            endNote()->undoChangeProperty(Pid::HEAD_GROUP, NoteHeadGroup::HEAD_CROSS);
        }
        return;
    } else if (isSlack()) {
        undoResetProperty(Pid::GUITAR_DIVE_IS_SLACK);
        if (endNote()) {
            endNote()->undoResetProperty(Pid::HEAD_GROUP);
        }
    }

    if (bendType() == GuitarBendType::DIP) {
        // Dips and slack have no end note so set bend amount directly
        undoChangeProperty(Pid::GUITAR_BEND_AMOUNT, bendAmount);
        return;
    }

    // All other bends: set bend amount by transposing end note appropriately
    int pitch = bendAmount / 2 + startNoteOfChain()->pitch();
    QuarterOffset quarterOff = bendAmount % 2 == 1 ? QuarterOffset::QUARTER_SHARP
                               : bendAmount % 2 == -1 ? QuarterOffset::QUARTER_FLAT : QuarterOffset::NONE;
    if (pitch == startNote()->pitch() && quarterOff == QuarterOffset::QUARTER_SHARP) {
        // Because a flat second is more readable than a sharp unison
        pitch += 1;
        quarterOff = QuarterOffset::QUARTER_FLAT;
    }

    setEndNotePitch(pitch, quarterOff);
}

void GuitarBend::setEndNotePitch(int pitch, QuarterOffset quarterOff)
{
    Note* note = endNote();
    IF_ASSERT_FAILED(note) {
        return;
    }

    Fraction tick = note->tick();
    Staff* staff = note->staff();

    Key key = staff->key(tick);
    Interval interval = staff->transpose(tick);
    interval.flip();

    int targetTpc1 = pitch2tpc(pitch, key, Prefer::NEAREST);
    int targetTpc2 = Transpose::transposeTpc(targetTpc1, interval, true);

    score()->undoChangePitch(note, pitch, targetTpc1, targetTpc2);

    Note* tiedNote = note->tieFor() ? note->tieFor()->endNote() : nullptr;
    while (tiedNote) {
        score()->undoChangePitch(tiedNote, pitch, targetTpc1, targetTpc2);
        tiedNote = tiedNote->tieFor() ? tiedNote->tieFor()->endNote() : nullptr;
    }

    AccidentalType accidentalType = Accidental::value2subtype(tpc2alter(targetTpc1));
    if (quarterOff == QuarterOffset::QUARTER_SHARP) {
        switch (accidentalType) {
        case AccidentalType::NONE:
        case AccidentalType::NATURAL:
            accidentalType = AccidentalType::SHARP_ARROW_DOWN;
            break;
        case AccidentalType::FLAT:
            accidentalType = AccidentalType::FLAT_ARROW_UP;
            break;
        case AccidentalType::SHARP:
            accidentalType = AccidentalType::SHARP_ARROW_UP;
            break;
        default:
            break;
        }
    } else if (quarterOff == QuarterOffset::QUARTER_FLAT) {
        switch (accidentalType) {
        case AccidentalType::NONE:
        case AccidentalType::NATURAL:
            accidentalType = AccidentalType::FLAT_ARROW_UP;
            break;
        case AccidentalType::FLAT:
            accidentalType = AccidentalType::FLAT_ARROW_DOWN;
            break;
        case AccidentalType::SHARP:
            accidentalType = AccidentalType::SHARP_ARROW_DOWN;
            break;
        default:
            break;
        }
    }

    if (accidentalType != note->accidentalType()) {
        for (EngravingObject* linked : note->linkList()) {
            toNote(linked)->updateLine();
            score()->changeAccidental(toNote(linked), accidentalType);
        }
    }

    computeBendAmount();

    triggerLayout();
}

bool GuitarBend::isReleaseBend() const
{
    return bendAmountInQuarterTones() < 0;
}

bool GuitarBend::isFullRelease() const
{
    return isReleaseBend() && totBendAmountIncludingPrecedingBends() == 0;
}

bool GuitarBend::angledPreBend() const
{
    Note* endN = endNote();
    Chord* endChord = endNote()->chord();
    return bendType() == GuitarBendType::PRE_BEND && endN->string() > endChord->upString();
}

bool GuitarBend::isDive() const
{
    switch (bendType()) {
    case GuitarBendType::DIVE:
    case GuitarBendType::PRE_DIVE:
    case GuitarBendType::DIP:
    case GuitarBendType::SCOOP:
        return true;
    default:
        return false;
    }
}

bool GuitarBend::isFullReleaseDive() const
{
    return isDive() && totBendAmountIncludingPrecedingBends() == 0;
}

void GuitarBend::fixNotesFrettingForStandardBend(Note* startNote, Note* endNote)
{
    Staff* curStaff =  startNote->staff();
    Part* curPart = startNote->part();
    if (!curStaff || !curPart) {
        return;
    }

    const StringData* stringData = curPart->stringData(startNote->tick(), curStaff->idx());
    int startFret = startNote->fret();
    int startString = startNote->string();
    if (startFret == 0) {
        // Bend can't start from empty string
        startString += 1;
        startFret = stringData->fret(startNote->pitch() + curStaff->pitchOffset(startNote->tick()), startString, curStaff);
        startNote->undoChangeProperty(Pid::STRING, startString);
        startNote->undoChangeProperty(Pid::FRET, startFret);
    }

    if (endNote->string() != startString) {
        endNote->undoChangeProperty(Pid::STRING, startString);
        int endFret = stringData->fret(endNote->pitch() + curStaff->pitchOffset(endNote->tick()), startString, curStaff);
        endNote->undoChangeProperty(Pid::FRET, endFret);
    }
}

Note* GuitarBend::createEndNote(Note* startNote, GuitarBendType bendType)
{
    track_idx_t track = startNote->track();
    Chord* startChord = startNote->chord();
    Segment* startSegment = startChord->segment();

    Segment* endSegment = startSegment->nextCR(track);
    if (!endSegment) {
        return nullptr;
    }

    EngravingItem* item = endSegment->element(track);
    if (!item) {
        return nullptr;
    }

    Score* score = startNote->score();
    NoteVal noteVal = startNote->noteVal();
    Note* endNote = nullptr;

    if (item->isRest()) {
        Rest* rest = toRest(item);
        Fraction duration = std::min(startChord->ticks(), rest->ticks());

        endSegment = score->setNoteRest(endSegment, track, noteVal, duration);
        Chord* endChord = endSegment ? toChord(endSegment->element(track)) : nullptr;
        endNote = endChord ? endChord->upNote() : nullptr;
    } else { // isChord
        Chord* chord = toChord(item);
        endNote = score->addNote(chord, noteVal);
    }

    if (endNote) {
        endNote->transposeDiatonic(bendType == GuitarBendType::BEND ? 1 : -1, true, false);
    }

    return endNote;
}

void GuitarBend::fixNotesFrettingForGraceBend(Note* grace, Note* main)
{
    // The start grace-note of bend must be on the same string as the main note
    int mainString = main->string();
    const StringData* stringData = main->part()->stringData(main->tick(), main->staff()->idx());
    int staffPitchOffset = main->staff()->pitchOffset(main->tick());
    int graceFret = stringData->fret(grace->pitch() + staffPitchOffset, mainString, main->staff());
    if (graceFret > 0) {
        // There is valid fretting
        grace->undoChangeProperty(Pid::STRING, mainString);
        grace->undoChangeProperty(Pid::FRET, graceFret);
    } else {
        // No valid fretting on this string, so it must be played on the lower string
        mainString += 1;
        main->undoChangeProperty(Pid::STRING, mainString);
        int mainFret = stringData->fret(main->pitch() + staffPitchOffset, mainString, main->staff());
        main->undoChangeProperty(Pid::FRET, mainFret);

        grace->undoChangeProperty(Pid::STRING, mainString);
        graceFret = stringData->fret(grace->pitch() + staffPitchOffset, mainString, main->staff());
        grace->undoChangeProperty(Pid::FRET, graceFret);
    }
}

PropertyValue GuitarBend::getProperty(Pid id) const
{
    switch (id) {
    case Pid::DIRECTION:
        return direction();
    case Pid::BEND_SHOW_HOLD_LINE:
        return static_cast<int>(showHoldLine());
    case Pid::BEND_START_TIME_FACTOR:
        return startTimeFactor();
    case Pid::BEND_END_TIME_FACTOR:
        return endTimeFactor();
    case Pid::GUITAR_DIVE_TAB_POS:
        return isDive() ? m_diveTabPos : DirectionV::AUTO;
    case Pid::GUITAR_BEND_AMOUNT:
        return bendAmountInQuarterTones();
    case Pid::VIBRATO_LINE_TYPE:
        return dipVibratoType();
    case Pid::GUITAR_DIVE_IS_SLACK:
        return isSlack();
    default:
        return SLine::getProperty(id);
    }
}

bool GuitarBend::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::DIRECTION:
        setDirection(v.value<DirectionV>());
        break;
    case Pid::BEND_SHOW_HOLD_LINE:
        setShowHoldLine(static_cast<GuitarBendShowHoldLine>(v.toInt()));
        break;
    case Pid::BEND_START_TIME_FACTOR:
        setStartTimeFactor(v.toReal());
        break;
    case Pid::BEND_END_TIME_FACTOR:
        setEndTimeFactor(v.toReal());
        break;
    case Pid::COLOR:
        setColor(v.value<Color>());
        setLineColor(v.value<Color>());
        break;
    case Pid::GUITAR_DIVE_TAB_POS:
    {
        IF_ASSERT_FAILED(isDive()) {
            return false;
        }
        setDiveTabPos(v.value<DirectionV>());
        break;
    }
    case Pid::GUITAR_BEND_AMOUNT:
        setBendAmountInQuarterTones(v.toInt());
        break;
    case Pid::VIBRATO_LINE_TYPE:
    {
        IF_ASSERT_FAILED(bendType() == GuitarBendType::DIP) {
            return false;
        }
        setDipVibratoType(v.value<VibratoType>());
        break;
    }
    case Pid::GUITAR_DIVE_IS_SLACK:
        setIsSlack(v.toBool());
        break;
    default:
        return SLine::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

PropertyValue GuitarBend::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::DIRECTION:
        return DirectionV::AUTO;
    case Pid::BEND_SHOW_HOLD_LINE:
        return static_cast<int>(GuitarBendShowHoldLine::AUTO);
    case Pid::BEND_START_TIME_FACTOR:
        if (m_bendType == GuitarBendType::DIP) {
            return DIP_DEFAULT_START_TIME_FACTOR;
        }
        return 0.f;
    case Pid::BEND_END_TIME_FACTOR:
        if (m_bendType == GuitarBendType::GRACE_NOTE_BEND) {
            return GRACE_NOTE_BEND_DEFAULT_END_TIME_FACTOR;
        }
        if (m_bendType == GuitarBendType::DIP) {
            return DIP_DEFAULT_END_TIME_FACTOR;
        }
        return 1.f;
    case Pid::GUITAR_DIVE_TAB_POS:
        return DirectionV::AUTO;
    case Pid::GUITAR_BEND_AMOUNT:
        return -2;
    case Pid::VIBRATO_LINE_TYPE:
        return VibratoType::NONE;
    case Pid::GUITAR_DIVE_IS_SLACK:
        return false;
    default:
        return SLine::propertyDefault(id);
    }
}

void GuitarBend::computeBendAmount()
{
    if (isSlack()) {
        int prevAmount = 0;
        if (GuitarBend* prevBend = findPrecedingBend()) {
            prevAmount = prevBend->totBendAmountIncludingPrecedingBends();
        }
        setBendAmountInQuarterTones(SLACK_BEND_AMOUNT - prevAmount);
        computeBendText();
        return;
    }

    if (bendType() == GuitarBendType::DIP) {
        computeBendText();
        return;
    }

    if (bendType() == GuitarBendType::SCOOP) {
        setBendAmountInQuarterTones(-2);
        computeBendText();
        return;
    }

    if (bendType() == GuitarBendType::SLIGHT_BEND) {
        setBendAmountInQuarterTones(1);
        computeBendText();
        return;
    }

    GuitarBend* prevBend = findPrecedingBend();
    GuitarBend* prevSlack = prevBend && prevBend->isSlack() ? prevBend : nullptr;

    Note* startN = prevSlack ? prevSlack->startNote() : startNote();
    Note* endN = endNote();

    int startPitch = startN->pitch();
    int endPitch = endN->pitch();

    int pitchDiffInQuarterTones = 2 * (endPitch - startPitch);

    Accidental* startAcc = startN->accidental();
    Accidental* endAcc = endN->accidental();
    int startCentsOff = startAcc ? Accidental::subtype2centOffset(startAcc->accidentalType()) : 0;
    int endCentsOff = endAcc ? Accidental::subtype2centOffset(endAcc->accidentalType()) : 0;

    int startOffInQuarterTones = round(double(startCentsOff) / 50);
    int endOffInQuarterTones = round(double(endCentsOff) / 50);

    pitchDiffInQuarterTones += endOffInQuarterTones - startOffInQuarterTones;

    if (prevSlack) {
        pitchDiffInQuarterTones -= SLACK_BEND_AMOUNT;
    }

    setBendAmountInQuarterTones(pitchDiffInQuarterTones);

    computeBendText();

    computeIsInvalidOrNeedsWarning();
}

int GuitarBend::totBendAmountIncludingPrecedingBends() const
{
    int bendAmount = bendAmountInQuarterTones();
    if (bendType() == GuitarBendType::PRE_DIVE) {
        return bendAmount;
    }

    GuitarBend* prevBend = findPrecedingBend();
    while (prevBend) {
        bendAmount += prevBend->bendAmountInQuarterTones();
        if (prevBend->bendType() == GuitarBendType::PRE_DIVE) {
            return bendAmount;
        }
        prevBend = prevBend->findPrecedingBend();
    }

    return bendAmount;
}

void GuitarBend::computeBendText()
{
    if (isSlack()) {
        mutldata()->setBendDigit(String(u"slack"));
        return;
    }

    if (bendType() == GuitarBendType::PRE_DIVE && findPrecedingBend()) {
        mutldata()->setBendDigit(u"");
        return;
    }

    int quarters = totBendAmountIncludingPrecedingBends();

    int fulls = quarters / 4;
    int quarts = quarters % 4;
    if (fulls == 0 && quarters == 0) {
        mutldata()->setBendDigit(String());
        return;
    }

    String string = bendAmountToString(fulls, quarts, style().styleB(Sid::useFractionCharacters));

    if (isDive()) {
        if (quarters > 0) {
            string.insert(0, u"+");
        } else if (quarters < 0 && fulls == 0) {
            string.insert(0, u"-");
        }
    }

    if (string == u"1" && style().styleB(Sid::guitarBendUseFull)) {
        string = String(u"full");
    }

    mutldata()->setBendDigit(string);
}

void GuitarBend::computeIsInvalidOrNeedsWarning()
{
    int totBendAmount = totBendAmountIncludingPrecedingBends();
    static constexpr int UNPLAYABLE_THRESHOLD = 14;
    static constexpr int WARNING_THRESHOLD = 8;
    static constexpr int DIVE_MAX = 16;
    static constexpr int DIVE_MIN = -32;

    if (isDive()) {
        m_isInvalid = bendAmountInQuarterTones() == 0 || totBendAmount < DIVE_MIN || totBendAmount > DIVE_MAX;
        m_isBorderlineUnplayable = false;
    } else {
        m_isInvalid = totBendAmount < 0 || totBendAmount > UNPLAYABLE_THRESHOLD || bendAmountInQuarterTones() == 0;
        m_isBorderlineUnplayable =  totBendAmount > WARNING_THRESHOLD && totBendAmount <= UNPLAYABLE_THRESHOLD;
    }
}

GuitarBend* GuitarBend::findPrecedingBend() const
{
    Note* startN = startNote();
    while (startN->tieBack() && startN->tieBack()->startNote()) {
        startN = startN->tieBack()->startNote();
    }

    GuitarBend* precedingBend = startN->bendBack();
    if (precedingBend && precedingBend->isDive() == isDive()
        && precedingBend->bendType() != GuitarBendType::SLIGHT_BEND
        && precedingBend->bendType() != GuitarBendType::DIP
        && precedingBend->bendType() != GuitarBendType::SCOOP) {
        return precedingBend;
    }

    if (bendType() == GuitarBendType::PRE_DIVE) {
        ChordRest* prevCR = prevChordRest(startN->chord());
        if (prevCR && prevCR->isRest()) {
            WhammyBar* whammyBar = findOverlappingWhammyBar(prevCR->tick(), tick2());
            if (whammyBar) {
                while (prevCR && prevCR->isRest() && prevCR->tick() > whammyBar->tick()) {
                    prevCR = prevChordRest(prevCR);
                }
            }
        }
        if (!prevCR || !prevCR->isChord()) {
            return nullptr;
        }
        Chord* prevChord = toChord(prevCR);
        for (Note* note : prevChord->notes()) {
            while (note->tieBack() && note->tieBack()->startNote()) {
                note = note->tieBack()->startNote();
            }
            GuitarBend* prevBend = note->bendBack();
            if (prevBend
                && (prevBend->bendType() == GuitarBendType::DIVE || prevBend->bendType() == GuitarBendType::PRE_DIVE)
                && prevBend->totBendAmountIncludingPrecedingBends() == bendAmountInQuarterTones()) {
                return prevBend;
            }
        }
    }

    return nullptr;
}

GuitarBend* GuitarBend::findFollowingPreDive() const
{
    if (!isDive()) {
        return nullptr;
    }

    Note* endN = endNote();
    while (endN->tieFor() && endN->tieFor()->endNote()) {
        endN = endN->tieFor()->endNote();
    }

    ChordRest* nextCR = nextChordRest(endN->chord());
    if (nextCR->isRest()) {
        WhammyBar* whammyBar = findOverlappingWhammyBar(tick(), nextCR->endTick());
        if (whammyBar) {
            while (nextCR && nextCR->isRest() && nextCR->tick() < whammyBar->tick2()) {
                nextCR = nextChordRest(nextCR);
            }
        }
    }

    if (!nextCR || !nextCR->isChord()) {
        return nullptr;
    }

    Chord* nextChord = toChord(nextCR);
    for (Note* note : nextChord->notes()) {
        while (note->tieFor() && note->tieFor()->endNote()) {
            note = note->tieFor()->endNote();
        }
        GuitarBend* bend = note->bendFor();
        if (bend && bend->bendType() == GuitarBendType::PRE_DIVE
            && bend->bendAmountInQuarterTones() == totBendAmountIncludingPrecedingBends()) {
            return bend;
        }
    }

    return nullptr;
}

WhammyBar* GuitarBend::findOverlappingWhammyBar(Fraction startTick, Fraction endTick) const
{
    const auto& spanners = score()->spannerMap().findOverlapping(startTick.ticks(), endTick.ticks());
    for (const auto& item : spanners) {
        Spanner* spanner = item.value;
        if (spanner->isWhammyBar() && spanner->tick() < startTick && spanner->tick2() >= endTick) {
            Staff* spannerStaff = spanner->staff();
            Staff* thisStaff = staff();
            if (spannerStaff == thisStaff || spannerStaff->isLinked(thisStaff)) {
                return toWhammyBar(spanner);
            }
        }
    }

    return nullptr;
}

void GuitarBend::updateHoldLine()
{
    Note* startOfHold = nullptr;
    Note* endOfHold = nullptr;

    bool isDipWithVibrato = bendType() == GuitarBendType::DIP && dipVibratoType() != VibratoType::NONE;
    bool canHaveHoldLine = isDipWithVibrato || (staffType()->isTabStaff() && !isFullRelease() && bendType() != GuitarBendType::SCOOP
                                                && bendType() != GuitarBendType::DIP);
    if (isDive()) {
        canHaveHoldLine &= startNote() && startNote() == startNote()->chord()->upNote();
    }

    bool needsHoldLine = false;
    if (canHaveHoldLine) {
        startOfHold = endNote();
        endOfHold = startOfHold;

        GuitarBend* followingPreDive = findFollowingPreDive();
        endOfHold = followingPreDive ? followingPreDive->startNote() : startOfHold;

        if (endOfHold == startOfHold) {
            while (endOfHold->tieFor() && endOfHold->tieFor()->endNote()) {
                endOfHold = endOfHold->tieFor()->endNote();
            }
        }

        if (showHoldLine() == GuitarBendShowHoldLine::AUTO) {
            needsHoldLine = endOfHold != startOfHold;
        } else {
            needsHoldLine = showHoldLine() == GuitarBendShowHoldLine::SHOW || isDipWithVibrato;
        }
    }

    if (!needsHoldLine) {
        if (m_holdLine) {
            if (!m_holdLine->segmentsEmpty()) {
                m_holdLine->eraseSpannerSegments();
            }
            delete m_holdLine;
            m_holdLine = nullptr;
        }
        return;
    }

    if (endOfHold == startOfHold) {
        Chord* guessedEndChord = startOfHold->chord()->next();
        if (guessedEndChord) {
            for (Note* note : guessedEndChord->notes()) {
                if (note->isPreBendStart()) {
                    endOfHold = note;
                    break;
                }
            }
        }
    }

    if (!m_holdLine) {
        m_holdLine = new GuitarBendHold(this);
    }

    m_holdLine->setAnchor(Spanner::Anchor::NOTE);
    m_holdLine->setStartElement(startOfHold);
    m_holdLine->setEndElement(endOfHold);
    m_holdLine->setTick(startOfHold->tick());
    m_holdLine->setTick2(endOfHold->tick());
    m_holdLine->setTrack(track());
    m_holdLine->setTrack2(track());
    m_holdLine->setGenerated(true);
    m_holdLine->setParent(this);
}

double GuitarBend::lineWidth() const
{
    if (isDive()) {
        return (staffType() && staffType()->isTabStaff())
               ? style().styleMM(Sid::guitarDiveLineWidthTab)
               : style().styleMM(Sid::guitarDiveLineWidth);
    }

    return (staffType() && staffType()->isTabStaff())
           ? style().styleMM(Sid::guitarBendLineWidthTab)
           : style().styleMM(Sid::guitarBendLineWidth);
}

/****************************************
 *              GuitarBendSegment
 * **************************************/

GuitarBendSegment::GuitarBendSegment(GuitarBend* sp, System* parent)
    : LineSegment(ElementType::GUITAR_BEND_SEGMENT, sp, parent, ElementFlag::MOVABLE)
{
    m_text = new GuitarBendText(this);
    m_text->setParent(this);
    setFlag(ElementFlag::ON_STAFF, true);
}

GuitarBendSegment::GuitarBendSegment(const GuitarBendSegment& s)
    : LineSegment(s)
{
    m_vertexPointOff = s.m_vertexPointOff;
    m_text = new GuitarBendText(this);
    m_text->setParent(this);
}

GuitarBendSegment::~GuitarBendSegment()
{
    delete m_text;
}

std::vector<PointF> GuitarBendSegment::gripsPositions(const EditData&) const
{
    std::vector<PointF> grips(gripsCount());
    PointF pp(pagePos());
    grips[int(Grip::START)] = pp;
    grips[int(Grip::END)] = pos2() + pp;
    grips[int(Grip::MIDDLE)] = pos2() * .5 + pp;
    grips[int(Grip::APERTURE)] = ldata()->vertexPoint() + vertexPointOff() + pp;
    return grips;
}

void GuitarBendSegment::startDragGrip(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        return;
    }
    eed->pushProperty(Pid::BEND_VERTEX_OFF);
    LineSegment::startDragGrip(ed);
}

void GuitarBendSegment::dragGrip(EditData& ed)
{
    PointF delta = ed.evtDelta;

    switch (ed.curGrip) {
    case Grip::START:
        setOffset(offset() + delta);
        setUserOff2(userOff2() - delta);
        setVertexPointOff(vertexPointOff() - delta);
        break;
    case Grip::END:
        setUserOff2(userOff2() + delta);
        break;
    case Grip::APERTURE:
        setVertexPointOff(vertexPointOff() + delta);
        break;
    case Grip::MIDDLE:
        setOffset(offset() + delta);
        setOffsetChanged(true);
        break;
    default:
        UNREACHABLE;
        break;
    }

    triggerLayout();
}

PropertyValue GuitarBendSegment::getProperty(Pid id) const
{
    switch (id) {
    case Pid::BEND_VERTEX_OFF:
        return vertexPointOff();
    default:
        return LineSegment::getProperty(id);
    }
}

bool GuitarBendSegment::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::BEND_VERTEX_OFF:
        setVertexPointOff(v.value<PointF>());
        break;
    default:
        return LineSegment::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

PropertyValue GuitarBendSegment::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::BEND_VERTEX_OFF:
        return PointF();
    default:
        return LineSegment::propertyDefault(id);
    }
}

EngravingObject* GuitarBendSegment::propertyDelegate(Pid id) const
{
    switch (id) {
    case Pid::DIRECTION:
    case Pid::BEND_SHOW_HOLD_LINE:
    case Pid::POSITION_LINKED_TO_MASTER:
    case Pid::APPEARANCE_LINKED_TO_MASTER:
    case Pid::GUITAR_DIVE_TAB_POS:
    case Pid::VIBRATO_LINE_TYPE:
        return guitarBend();
    default:
        return LineSegment::propertyDelegate(id);
    }
}

void GuitarBendSegment::reset()
{
    undoResetProperty(Pid::DIRECTION);
    undoResetProperty(Pid::BEND_VERTEX_OFF);
    LineSegment::reset();
}

double GuitarBendSegment::lineWidth() const
{
    return guitarBend()->lineWidth();
}

void GuitarBendSegment::scanElements(std::function<void(EngravingItem*)> func)
{
    func(m_text);
    LineSegment::scanElements(func);
}

bool GuitarBendSegment::isUserModified() const
{
    bool modified = !vertexPointOff().isNull() || (m_text && m_text->isUserModified());
    return modified || LineSegment::isUserModified();
}

Color GuitarBend::curColor(const rendering::PaintOptions& opt) const
{
    if (!opt.isPrinting && MScore::warnGuitarBends) {
        if (m_isInvalid) {
            return selected() ? configuration()->criticalSelectedColor() : configuration()->criticalColor();
        }

        if (m_isBorderlineUnplayable) {
            return selected() ? configuration()->warningSelectedColor() : configuration()->warningColor();
        }
    }

    return EngravingItem::curColor(opt);
}

void GuitarBend::adaptBendsFromTabToStandardStaff(const Staff* staff)
{
    // On tabs, bends force end notes to be invisible. When switching to
    // normal staff we need to turn all the end notes visible again.

    auto processBends = [](Chord* chord) {
        for (Note* note : chord->notes()) {
            GuitarBend* bend = note->bendFor();
            if (!bend) {
                continue;
            }
            bend->endNote()->setVisible(true);
        }
    };

    staff_idx_t staffIdx = staff->idx();
    track_idx_t startTrack = staff2track(staffIdx);
    track_idx_t endTrack = startTrack + VOICES;
    for (Segment* segment = staff->score()->firstSegment(SegmentType::ChordRest); segment; segment = segment->next1()) {
        if (!segment->isChordRestType()) {
            continue;
        }
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            EngravingItem* item = segment->element(track);
            if (!item || !item->isChord()) {
                continue;
            }
            Chord* chord = toChord(item);
            processBends(chord);
            for (Chord* grace : chord->graceNotes()) {
                processBends(grace);
            }
        }
    }
}

GuitarBendType GuitarBend::bendTypeFromActionIcon(ActionIconType actionIconType)
{
    switch (actionIconType) {
    case ActionIconType::STANDARD_BEND: return GuitarBendType::BEND;
    case ActionIconType::PRE_BEND: return GuitarBendType::PRE_BEND;
    case ActionIconType::GRACE_NOTE_BEND: return GuitarBendType::GRACE_NOTE_BEND;
    case ActionIconType::SLIGHT_BEND: return GuitarBendType::SLIGHT_BEND;
    case ActionIconType::DIVE: return GuitarBendType::DIVE;
    case ActionIconType::PRE_DIVE: return GuitarBendType::PRE_DIVE;
    case ActionIconType::DIP: return GuitarBendType::DIP;
    case ActionIconType::SCOOP: return GuitarBendType::SCOOP;
    default:
        return GuitarBendType::BEND;
    }
}

void GuitarBend::setBendType(GuitarBendType t)
{
    m_bendType = t;

    resetProperty(Pid::GUITAR_BEND_AMOUNT);
    resetProperty(Pid::BEND_START_TIME_FACTOR);
    resetProperty(Pid::BEND_END_TIME_FACTOR);
}

/****************************************
 *            GuitarBendHold
 * **************************************/

GuitarBendHold::GuitarBendHold(GuitarBend* parent)
    : SLine(ElementType::GUITAR_BEND_HOLD, parent, ElementFlag::MOVABLE)
{
    resetProperty(Pid::LINE_STYLE);
}

GuitarBendHold::GuitarBendHold(const GuitarBendHold& h)
    : SLine(h)
{
}

LineSegment* GuitarBendHold::createLineSegment(System* parent)
{
    GuitarBendHoldSegment* seg = new GuitarBendHoldSegment(this, parent);
    seg->setTrack(track());
    seg->setColor(lineColor());
    return seg;
}

PropertyValue GuitarBendHold::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::LINE_STYLE:
        return LineType::DASHED;
    default:
        return SLine::propertyDefault(id);
    }
}

Note* GuitarBendHold::startNote() const
{
    EngravingItem* startEl = startElement();
    assert(startEl && startEl->isNote());
    return toNote(startEl);
}

Note* GuitarBendHold::endNote() const
{
    EngravingItem* endEl = endElement();
    assert(endEl && endEl->isNote());
    return toNote(endEl);
}

double GuitarBendHold::lineWidth() const
{
    return style().styleMM(parent() && toGuitarBend(parent())->isDive() ? Sid::guitarDiveLineWidthTab : Sid::guitarBendLineWidthTab);
}

/****************************************
 *         GuitarBendHoldSegment
 * **************************************/

GuitarBendHoldSegment::GuitarBendHoldSegment(GuitarBendHold* sp, System* parent)
    : LineSegment(ElementType::GUITAR_BEND_HOLD_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    setFlag(ElementFlag::ON_STAFF, true);
}

void GuitarBendHoldSegment::dragGrip(EditData& ed)
{
    PointF delta = ed.evtDelta;

    switch (ed.curGrip) {
    case Grip::START:
        setOffset(offset() + delta);
        setUserOff2(userOff2() - delta);
        break;
    case Grip::END:
        setUserOff2(userOff2() + delta);
        break;
    case Grip::MIDDLE:
        setOffset(offset() + delta);
        setOffsetChanged(true);
        break;
    default:
        break;
    }

    triggerLayout();
}

/****************************************
 *         GuitarBendText
 * **************************************/

static const ElementStyle guitarBendStyle {};

GuitarBendText::GuitarBendText(GuitarBendSegment* parent)
    : TextBase(ElementType::GUITAR_BEND_TEXT, parent, TextStyleType::BEND)
{
    initElementStyle(&guitarBendStyle);
}
} // namespace mu::engraving
