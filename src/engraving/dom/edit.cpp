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

#include <map>
#include <set>

#include "infrastructure/messagebox.h"

#include "accidental.h"
#include "articulation.h"
#include "barline.h"
#include "beam.h"
#include "box.h"
#include "bracket.h"
#include "breath.h"
#include "chord.h"
#include "chordline.h"
#include "clef.h"
#include "excerpt.h"
#include "expression.h"
#include "factory.h"
#include "fingering.h"
#include "glissando.h"
#include "guitarbend.h"
#include "hairpin.h"
#include "harmony.h"
#include "harppedaldiagram.h"
#include "hook.h"
#include "instrchange.h"
#include "instrumentname.h"
#include "key.h"
#include "keylist.h"
#include "keysig.h"
#include "layoutbreak.h"
#include "linkedobjects.h"
#include "lyrics.h"
#include "masterscore.h"
#include "measure.h"
#include "measurerepeat.h"
#include "mscoreview.h"
#include "navigate.h"
#include "note.h"
#include "ornament.h"
#include "ottava.h"
#include "part.h"
#include "range.h"
#include "rehearsalmark.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "sig.h"
#include "slur.h"
#include "staff.h"
#include "stafftext.h"
#include "stem.h"
#include "sticking.h"
#include "system.h"
#include "systemtext.h"
#include "tempotext.h"
#include "textline.h"
#include "tie.h"
#include "tiemap.h"
#include "timesig.h"
#include "tremolo.h"
#include "trill.h"
#include "tuplet.h"
#include "tupletmap.h"
#include "undo.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static ChordRest* chordOrRest(EngravingItem* el)
{
    if (el) {
        if (el->isNote()) {
            return toNote(el)->chord();
        } else if (el->isRestFamily()) {
            return toRest(el);
        } else if (el->isChord()) {
            return toChord(el);
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   getSelectedNote
//---------------------------------------------------------

Note* Score::getSelectedNote()
{
    EngravingItem* el = selection().element();
    if (el && el->isNote()) {
        return toNote(el);
    }
    MScore::setError(MsError::NO_NOTE_SELECTED);
    return 0;
}

//---------------------------------------------------------
//   getSelectedChordRest
//---------------------------------------------------------

ChordRest* Score::getSelectedChordRest() const
{
    ChordRest* cr = chordOrRest(selection().element());
    if (!cr) {
        MScore::setError(MsError::NO_NOTE_REST_SELECTED);
    }

    return cr;
}

//---------------------------------------------------------
//   getSelectedChordRest2
//---------------------------------------------------------

void Score::getSelectedChordRest2(ChordRest** cr1, ChordRest** cr2) const
{
    *cr1 = 0;
    *cr2 = 0;
    for (EngravingItem* e : selection().elements()) {
        if (e->isNote()) {
            e = e->parentItem();
        }
        if (e->isChordRest()) {
            ChordRest* cr = toChordRest(e);
            if (*cr1 == 0 || (*cr1)->tick() > cr->tick()) {
                *cr1 = cr;
            }
            if (*cr2 == 0 || (*cr2)->tick() < cr->tick()) {
                *cr2 = cr;
            }
        }
    }
    if (*cr1 == 0) {
        MScore::setError(MsError::NO_NOTE_REST_SELECTED);
    }
    if (*cr1 == *cr2) {
        *cr2 = 0;
    }
}

//---------------------------------------------------------
//   getSelectedChordRests
//---------------------------------------------------------

std::set<ChordRest*> Score::getSelectedChordRests() const
{
    std::set<ChordRest*> set;
    for (EngravingItem* e : selection().elements()) {
        if (e->isNote()) {
            e = e->parentItem();
        }
        if (e->isChordRest()) {
            set.insert(toChordRest(e));
        }
    }
    return set;
}

//---------------------------------------------------------
//   pos
//---------------------------------------------------------

Fraction Score::pos()
{
    EngravingItem* el = selection().element();
    if (selection().activeCR()) {
        el = selection().activeCR();
    }
    if (el) {
        switch (el->type()) {
        case ElementType::NOTE:
            el = el->parentItem();
        // fall through
        case ElementType::MEASURE_REPEAT:
        case ElementType::REST:
        case ElementType::MMREST:
        case ElementType::CHORD:
            return toChordRest(el)->tick();
        default:
            break;
        }
    }
    return Fraction(0, 1);
}

//---------------------------------------------------------
//   addMeasureRepeat
//    create one MeasureRepeat at tick of subtype numMeasures
//    create segment if necessary
//    does NOT set measureRepeatCount or do anything else with measure(s)!
//---------------------------------------------------------

MeasureRepeat* Score::addMeasureRepeat(const Fraction& tick, track_idx_t track, int numMeasures)
{
    Measure* measure = tick2measure(tick);
    MeasureRepeat* mr = Factory::createMeasureRepeat(this->dummy()->segment());
    mr->setNumMeasures(numMeasures);
    mr->setTicks(measure->stretchedLen(staff(track2staff(track))));
    mr->setTrack(track);
    undoAddCR(mr, measure, tick);
    return mr;
}

Tuplet* Score::addTuplet(ChordRest* destinationChordRest, Fraction ratio, TupletNumberType numberType, TupletBracketType bracketType)
{
    if (destinationChordRest->durationType() < TDuration(DurationType::V_512TH)
        && destinationChordRest->durationType() != TDuration(DurationType::V_MEASURE)) {
        return nullptr;
    }

    Measure* measure = destinationChordRest->measure();
    if (measure && measure->isMMRest()) {
        return nullptr;
    }

    Fraction f(destinationChordRest->ticks());
    Tuplet* ot  = destinationChordRest->tuplet();

    f.reduce();         //measure duration might not be reduced

    Fraction _ratio;
    _ratio.setNumerator(ratio.numerator() != -1 ? ratio.numerator() : f.numerator());
    _ratio.setDenominator(ratio.denominator() != -1 ? ratio.denominator() : f.numerator());

    Fraction fr = f * Fraction(1, _ratio.denominator());
    if (!TDuration::isValid(fr)) {
        MessageBox::warning(mtrc("engraving", "Cannot create tuplet with ratio %1 for duration %2")
                            .arg(_ratio.toString(), f.toString()).toStdString(),
                            std::string(), { MessageBox::Ok });
        return nullptr;
    }

    Tuplet* tuplet = Factory::createTuplet(this->dummy()->measure());
    tuplet->setRatio(_ratio);

    tuplet->setNumberType(numberType);
    if (tuplet->numberType() == TupletNumberType(tuplet->style().styleI(Sid::tupletNumberType))) {
        tuplet->setPropertyFlags(Pid::NUMBER_TYPE, PropertyFlags::STYLED);
    } else {
        tuplet->setPropertyFlags(Pid::NUMBER_TYPE, PropertyFlags::UNSTYLED);
    }

    tuplet->setBracketType(bracketType);
    if (tuplet->bracketType() == TupletBracketType(tuplet->style().styleI(Sid::tupletBracketType))) {
        tuplet->setPropertyFlags(Pid::BRACKET_TYPE, PropertyFlags::STYLED);
    } else {
        tuplet->setPropertyFlags(Pid::BRACKET_TYPE, PropertyFlags::UNSTYLED);
    }

    tuplet->setTicks(f);
    tuplet->setBaseLen(fr);

    tuplet->setTrack(destinationChordRest->track());
    tuplet->setTick(destinationChordRest->tick());
    tuplet->setParent(measure);

    if (ot) {
        tuplet->setTuplet(ot);
    }

    cmdCreateTuplet(destinationChordRest, tuplet);

    const std::vector<DurationElement*>& elements = tuplet->elements();
    DurationElement* elementForSelect = nullptr;
    if (!elements.empty()) {
        DurationElement* firstElement = elements.front();
        if (firstElement->isRest()) {
            elementForSelect = firstElement;
        } else if (elements.size() > 1) {
            elementForSelect = elements[1];
        }
    }

    if (elementForSelect) {
        score()->select(elementForSelect, SelectType::SINGLE, 0);
        score()->inputState().setDuration(tuplet->baseLen());
    }

    return tuplet;
}

//---------------------------------------------------------
//   addRest
//    create one Rest at tick with duration d
//    create segment if necessary
//---------------------------------------------------------

Rest* Score::addRest(const Fraction& tick, track_idx_t track, TDuration d, Tuplet* tuplet)
{
    Measure* measure = tick2measure(tick);
    Rest* rest = Factory::createRest(this->dummy()->segment(), d);
    if (d.type() == DurationType::V_MEASURE) {
        rest->setTicks(measure->stretchedLen(staff(track2staff(track))));
    } else {
        rest->setTicks(d.fraction());
    }
    rest->setTrack(track);
    rest->setTuplet(tuplet);
    undoAddCR(rest, measure, tick);
    return rest;
}

//---------------------------------------------------------
//   addRest
//---------------------------------------------------------

Rest* Score::addRest(Segment* s, track_idx_t track, TDuration d, Tuplet* tuplet)
{
    Rest* rest = Factory::createRest(s, d);
    if (d.type() == DurationType::V_MEASURE) {
        rest->setTicks(s->measure()->stretchedLen(staff(track / VOICES)));
    } else {
        rest->setTicks(d.fraction());
    }
    rest->setTrack(track);
    rest->setParent(s);
    rest->setTuplet(tuplet);
    undoAddCR(rest, tick2measure(s->tick()), s->tick());
    return rest;
}

//---------------------------------------------------------
//   addChord
//    Create one Chord at tick with duration d
//    - create segment if necessary.
//    - Use chord "oc" as prototype;
//    - if "genTie" then tie to chord "oc"
//---------------------------------------------------------

Chord* Score::addChord(const Fraction& tick, TDuration d, Chord* oc, bool genTie, Tuplet* tuplet)
{
    Measure* measure = tick2measure(tick);
    if (measure->endTick() <= tick) {
        LOGD("Score::addChord(): end of score?");
        return 0;
    }

    Chord* chord = Factory::createChord(this->dummy()->segment());
    chord->setTuplet(tuplet);
    chord->setTrack(oc->track());
    chord->setDurationType(d);
    chord->setTicks(d.fraction());

    for (Note* n : oc->notes()) {
        Note* nn = Factory::createNote(chord);
        nn->setPitch(n->pitch());
        nn->setTpc1(n->tpc1());
        nn->setTpc2(n->tpc2());
        chord->add(nn);
    }
    undoAddCR(chord, measure, tick);

    //
    // now as both chords are in place
    // (have segments as parent) we can add ties:
    //
    if (genTie) {
        size_t n = oc->notes().size();
        for (size_t i = 0; i < n; ++i) {
            Note* n1  = oc->notes()[i];
            Note* n2 = chord->notes()[i];
            Tie* tie = Factory::createTie(this->dummy());
            tie->setStartNote(n1);
            tie->setEndNote(n2);
            tie->setTick(tie->startNote()->tick());
            tie->setTick2(tie->endNote()->tick());
            tie->setTrack(n1->track());
            undoAddElement(tie);
        }
    }

    return chord;
}

//---------------------------------------------------------
//   addClone
//---------------------------------------------------------

ChordRest* Score::addClone(ChordRest* cr, const Fraction& tick, const TDuration& d)
{
    ChordRest* newcr;
    // change a MeasureRepeat into an Rest
    if (cr->isMeasureRepeat()) {
        newcr = Factory::copyRest(*toRest(cr));
        toRest(newcr)->hack_toRestType();
    } else {
        newcr = toChordRest(cr->clone());
    }
    newcr->mutldata()->setPosX(0.0);
    newcr->setDurationType(d);
    newcr->setTicks(d.fraction());
    newcr->setTuplet(cr->tuplet());
    newcr->setSelected(false);

    undoAddCR(newcr, cr->measure(), tick);
    return newcr;
}

//---------------------------------------------------------
//   setRest
//    create one or more rests to fill "l"
//---------------------------------------------------------

Rest* Score::setRest(const Fraction& _tick, track_idx_t track, const Fraction& _l, bool useDots, Tuplet* tuplet, bool useFullMeasureRest)
{
    Fraction l       = _l;
    Fraction tick    = _tick;
    Measure* measure = tick2measure(tick);
    Rest* r          = 0;
    Staff* staff     = Score::staff(track / VOICES);

    while (!l.isZero()) {
        //
        // divide into measures
        //
        Fraction f;
        if (tuplet) {
            f = tuplet->baseLen().fraction() * tuplet->ratio().numerator();
            for (DurationElement* de : tuplet->elements()) {
                if (de->tick() >= tick) {
                    break;
                }
                f -= de->ticks();
            }
            //
            // restrict to tuplet len
            //
            if (f < l) {
                l = f;
            }
        } else {
            if (measure->tick() < tick) {
                f = measure->tick() + measure->ticks() - tick;
            } else {
                f = measure->ticks();
            }
            f *= staff->timeStretch(tick);
        }

        if (f > l) {
            f = l;
        }

        if ((track % VOICES) && !measure->hasVoice(track) && (tick == measure->tick())) {
            l -= f;
            measure = measure->nextMeasure();
            if (!measure) {
                break;
            }
            tick = measure->tick();
            continue;
        }

        if ((measure->timesig() == measure->ticks())       // not in pickup measure
            && (measure->tick() == tick)
            && (measure->stretchedLen(staff) == f)
            && !tuplet
            && (useFullMeasureRest)) {
            Rest* rest = addRest(tick, track, TDuration(DurationType::V_MEASURE), tuplet);
            tick += rest->actualTicks();
            if (r == 0) {
                r = rest;
            }
        } else {
            //
            // compute list of durations which will fit l
            //
            std::vector<TDuration> dList;
            if (tuplet || staff->isLocalTimeSignature(tick) || f == Fraction(0, 1)) {
                dList = toDurationList(l, useDots);
                std::reverse(dList.begin(), dList.end());
            } else {
                dList
                    = toRhythmicDurationList(f, true, tick - measure->tick(), sigmap()->timesig(tick).nominal(), measure, useDots ? 1 : 0);
            }
            if (dList.empty()) {
                return 0;
            }

            Rest* rest = 0;
            for (const TDuration& d : dList) {
                rest = addRest(tick, track, d, tuplet);
                if (r == 0) {
                    r = rest;
                }
                tick += rest->actualTicks();
            }
        }
        l -= f;

        measure = measure->nextMeasure();
        if (!measure) {
            break;
        }
        tick = measure->tick();
    }
    return r;
}

//---------------------------------------------------------
//   addNote from NoteVal
//---------------------------------------------------------

Note* Score::addNote(Chord* chord, const NoteVal& noteVal, bool forceAccidental, const std::set<SymId>& articulationIds,
                     InputState* externalInputState)
{
    InputState& is = externalInputState ? (*externalInputState) : m_is;

    Note* note = Factory::createNote(chord);
    note->setParent(chord);
    note->setTrack(chord->track());
    note->setNval(noteVal);
    undoAddElement(note);
    if (forceAccidental) {
        int tpc = style().styleB(Sid::concertPitch) ? noteVal.tpc1 : noteVal.tpc2;
        AccidentalVal alter = tpc2alter(tpc);
        AccidentalType at = Accidental::value2subtype(alter);
        Accidental* a = Factory::createAccidental(note);
        a->setAccidentalType(at);
        a->setRole(AccidentalRole::USER);
        a->setParent(note);
        undoAddElement(a);
    }

    if (!articulationIds.empty()) {
        chord->updateArticulations(articulationIds);
    }

    setPlayNote(true);
    setPlayChord(true);

    if (externalInputState) {
        is.setTrack(note->track());
        is.setLastSegment(is.segment());
        is.setSegment(note->chord()->segment());
    } else {
        select(note, SelectType::SINGLE, 0);
    }

    if (!chord->staff()->isTabStaff(chord->tick())) {
        NoteEntryMethod entryMethod = is.noteEntryMethod();
        if (entryMethod != NoteEntryMethod::REALTIME_AUTO && entryMethod != NoteEntryMethod::REALTIME_MANUAL) {
            is.moveToNextInputPos();
        }
    }
    return note;
}

Note* Score::addNoteToTiedChord(Chord* chord, const NoteVal& noteVal, bool forceAccidental, const std::set<SymId>& articulationIds)
{
    IF_ASSERT_FAILED(!chord->notes().empty()) {
        return nullptr;
    };
    Note* referenceNote = chord->notes().at(0);

    while (referenceNote->tieBack()) {
        referenceNote = referenceNote->tieBack()->startNote();
    }
    // don't add note if it is already exist
    if (referenceNote->chord()->findNote(noteVal.pitch)) {
        return nullptr;
    }

    Tie* tie = nullptr;
    Note* newNote = nullptr;

    while (referenceNote->tieFor()) {
        chord = referenceNote->chord();
        newNote = addNote(chord, noteVal, forceAccidental, articulationIds);
        if (!newNote) {
            return nullptr;
        }
        if (tie) {
            tie->setEndNote(newNote);
            tie->setTick2(newNote->tick());
            newNote->setTieBack(tie);
            undoAddElement(tie);
        }

        tie = Factory::createTie(newNote);
        tie->setStartNote(newNote);
        tie->setTick(newNote->tick());
        tie->setTrack(newNote->track());
        newNote->setTieFor(tie);

        referenceNote = referenceNote->tieFor()->endNote();
    }

    chord = referenceNote->chord();
    newNote = addNote(chord, noteVal, forceAccidental, articulationIds);

    if (!newNote) {
        return nullptr;
    }

    if (tie) {
        tie->setEndNote(newNote);
        tie->setTick2(newNote->tick());
        newNote->setTieBack(tie);
        undoAddElement(tie);
    }

    connectTies();

    return newNote;
}

Slur* Score::addSlur(ChordRest* firstChordRest, ChordRest* secondChordRest, const Slur* slurTemplate)
{
    if (!secondChordRest) {
        secondChordRest = nextChordRest(firstChordRest);
        if (!secondChordRest) {
            secondChordRest = firstChordRest;
        }
    }

    Slur* slur = slurTemplate ? slurTemplate->clone() : Factory::createSlur(firstChordRest->measure()->system());
    slur->setScore(firstChordRest->score());
    slur->setTick(firstChordRest->tick());
    slur->setTick2(secondChordRest->tick());
    slur->setTrack(firstChordRest->track());
    if (secondChordRest->staff()->part() == firstChordRest->staff()->part()
        && !secondChordRest->staff()->isLinked(firstChordRest->staff())) {
        slur->setTrack2(secondChordRest->track());
    } else {
        slur->setTrack2(firstChordRest->track());
    }
    slur->setStartElement(firstChordRest);
    slur->setEndElement(secondChordRest);

    firstChordRest->score()->undoAddElement(slur);
    SlurSegment* ss = new SlurSegment(firstChordRest->score()->dummy()->system());
    ss->setSpannerSegmentType(SpannerSegmentType::SINGLE);
    if (firstChordRest == secondChordRest) {
        ss->setSlurOffset(Grip::END, PointF(3.0 * firstChordRest->style().spatium(), 0.0));
    }
    slur->add(ss);

    return slur;
}

TextBase* Score::addText(TextStyleType type, EngravingItem* destinationElement)
{
    TextBase* textBox = nullptr;

    switch (type) {
    case TextStyleType::TITLE:
    case TextStyleType::SUBTITLE:
    case TextStyleType::COMPOSER:
    case TextStyleType::LYRICIST:
    case TextStyleType::INSTRUMENT_EXCERPT: {
        MeasureBase* frame = nullptr;

        if (destinationElement && destinationElement->isVBox()) {
            frame = toMeasureBase(destinationElement);
        } else {
            MeasureBase* linkedFrame = nullptr;
            for (Score* score : scoreList()) {
                MeasureBase* titleFrame = score->first();
                if (!titleFrame || !titleFrame->isVBox()) {
                    titleFrame = score->insertBox(ElementType::VBOX, titleFrame);
                }
                if (!linkedFrame) {
                    linkedFrame = titleFrame;
                } else {
                    score->undo(new Link(titleFrame, linkedFrame));
                }
                if (score == this->score()) {
                    frame = titleFrame;
                }
            }
        }

        textBox = Factory::createText(frame, type);
        textBox->setParent(frame);
        undoAddElement(textBox);
        break;
    }
    case TextStyleType::FRAME: {
        if (!destinationElement) {
            break;
        }
        textBox = Factory::createText(destinationElement, type);
        textBox->setParent(destinationElement);
        undoAddElement(textBox);
        break;
    }
    case TextStyleType::REHEARSAL_MARK: {
        ChordRest* chordRest = chordOrRest(destinationElement);
        if (!chordRest) {
            break;
        }
        textBox = Factory::createRehearsalMark(dummy()->segment());
        chordRest->undoAddAnnotation(textBox);
        break;
    }
    case TextStyleType::STAFF: {
        ChordRest* chordRest = chordOrRest(destinationElement);
        if (!chordRest) {
            break;
        }
        textBox = Factory::createStaffText(dummy()->segment(), TextStyleType::STAFF);
        chordRest->undoAddAnnotation(textBox);
        break;
    }
    case TextStyleType::SYSTEM: {
        ChordRest* chordRest = chordOrRest(destinationElement);
        if (!chordRest) {
            break;
        }
        textBox = Factory::createSystemText(dummy()->segment(), TextStyleType::SYSTEM);
        chordRest->undoAddAnnotation(textBox);
        break;
    }
    case TextStyleType::EXPRESSION: {
        ChordRest* chordRest = chordOrRest(destinationElement);
        if (!chordRest) {
            break;
        }
        textBox = Factory::createExpression(dummy()->segment());
        chordRest->undoAddAnnotation(textBox);
        break;
    }
    case TextStyleType::INSTRUMENT_CHANGE: {
        ChordRest* chordRest = chordOrRest(destinationElement);
        if (!chordRest) {
            break;
        }
        textBox = Factory::createInstrumentChange(dummy()->segment());
        chordRest->undoAddAnnotation(textBox);
        break;
    }
    case TextStyleType::STICKING: {
        ChordRest* chordRest = chordOrRest(destinationElement);
        if (!chordRest) {
            break;
        }
        textBox = Factory::createSticking(dummy()->segment());
        chordRest->undoAddAnnotation(textBox);
        break;
    }
    case TextStyleType::FINGERING:
    case TextStyleType::LH_GUITAR_FINGERING:
    case TextStyleType::RH_GUITAR_FINGERING:
    case TextStyleType::STRING_NUMBER: {
        if (!destinationElement || !destinationElement->isNote()) {
            break;
        }

        const Staff* staff = destinationElement->staff();
        bool isTablature = staff->isTabStaff(destinationElement->tick());
        bool tabFingering = staff->staffType(destinationElement->tick())->showTabFingering();
        if (isTablature && !tabFingering) {
            break;
        }

        textBox = Factory::createFingering(toNote(destinationElement), type);
        textBox->setTrack(destinationElement->track());
        textBox->setParent(destinationElement);
        undoAddElement(textBox);
        break;
    }
    case TextStyleType::HARMONY_A:
    case TextStyleType::HARMONY_ROMAN:
    case TextStyleType::HARMONY_NASHVILLE: {
        track_idx_t track = mu::nidx;
        Segment* newParent = nullptr;
        if (destinationElement && destinationElement->isFretDiagram()) {
            FretDiagram* fretDiagram = toFretDiagram(destinationElement);
            track = fretDiagram->track();
            newParent = fretDiagram->segment();
        } else {
            ChordRest* chordRest = chordOrRest(destinationElement);
            if (chordRest) {
                track = chordRest->track();
                newParent = chordRest->segment();
            }
        }

        if (track == mu::nidx || !newParent) {
            break;
        }

        Harmony* harmony = Factory::createHarmony(newParent);
        harmony->setTrack(track);
        harmony->setParent(newParent);

        static std::map<TextStyleType, HarmonyType> harmonyTypes = {
            { TextStyleType::HARMONY_A, HarmonyType::STANDARD },
            { TextStyleType::HARMONY_ROMAN, HarmonyType::ROMAN },
            { TextStyleType::HARMONY_NASHVILLE, HarmonyType::NASHVILLE }
        };
        harmony->setHarmonyType(harmonyTypes[type]);

        textBox = harmony;
        undoAddElement(textBox);
        break;
    }
    case TextStyleType::LYRICS_ODD: {
        if (!destinationElement || (!destinationElement->isNote() && !destinationElement->isLyrics() && !destinationElement->isRest())) {
            break;
        }
        ChordRest* chordRest = nullptr;
        if (destinationElement->isNote()) {
            chordRest = toNote(destinationElement)->chord();
            if (chordRest->isGrace()) {
                chordRest = toChordRest(chordRest->explicitParent());
            }
        } else if (destinationElement->isLyrics()) {
            chordRest = toLyrics(destinationElement)->chordRest();
        } else if (destinationElement->isRest()) {
            chordRest = toChordRest(destinationElement);
        } else {
            break;
        }

        int no = static_cast<int>(chordRest->lyrics().size());
        Lyrics* lyrics = Factory::createLyrics(chordRest);
        lyrics->setTrack(chordRest->track());
        lyrics->setParent(chordRest);
        lyrics->setNo(no);

        textBox = lyrics;
        undoAddElement(textBox);
        break;
    }
    case TextStyleType::TEMPO: {
        ChordRest* chordRest = chordOrRest(destinationElement);
        if (!chordRest) {
            break;
        }

        SigEvent event = sigmap()->timesig(chordRest->tick());
        Fraction f = event.nominal();
        String text(u"<sym>metNoteQuarterUp</sym> = 80");
        switch (f.denominator()) {
        case 1:
            text = u"<sym>metNoteWhole</sym> = 80";
            break;
        case 2:
            text = u"<sym>metNoteHalfUp</sym> = 80";
            break;
        case 4:
            text = u"<sym>metNoteQuarterUp</sym> = 80";
            break;
        case 8:
            if (f.numerator() % 3 == 0) {
                text = u"<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80";
            } else {
                text = u"<sym>metNote8thUp</sym> = 80";
            }
            break;
        case 16:
            if (f.numerator() % 3 == 0) {
                text = u"<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80";
            } else {
                text = u"<sym>metNote16thUp</sym> = 80";
            }
            break;
        case 32:
            if (f.numerator() % 3 == 0) {
                text = u"<sym>metNote16thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80";
            } else {
                text = u"<sym>metNote32ndUp</sym> = 80";
            }
            break;
        case 64:
            if (f.numerator() % 3 == 0) {
                text = u"<sym>metNote32ndUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80";
            } else {
                text = u"<sym>metNote64thUp</sym> = 80";
            }
            break;
        case 128:
            if (f.numerator() % 3 == 0) {
                text = u"<sym>metNote64ndUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80";
            } else {
                text = u"<sym>metNote128thUp</sym> = 80";
            }
            break;
        default:
            break;
        }

        TempoText* tempoText = Factory::createTempoText(chordRest->segment());
        tempoText->setParent(chordRest->segment());
        tempoText->setTrack(0);
        tempoText->setXmlText(text);
        tempoText->setFollowText(true);

        textBox = tempoText;
        undoAddElement(textBox);
        break;
    }
    case TextStyleType::HARP_PEDAL_DIAGRAM:
    case TextStyleType::HARP_PEDAL_TEXT_DIAGRAM: {
        ChordRest* chordRest = getSelectedChordRest();
        if (!chordRest) {
            break;
        }
        textBox = Factory::createHarpPedalDiagram(this->dummy()->segment());
        chordRest->undoAddAnnotation(textBox);
        break;
    }
    default:
        break;
    }

    return textBox;
}

//---------------------------------------------------------
//   rewriteMeasures
//    rewrite all measures from fm to lm (including)
//    If staffIdx is valid (>= 0), then rewrite a local
//    timesig change.
//---------------------------------------------------------

bool Score::rewriteMeasures(Measure* fm, Measure* lm, const Fraction& ns, staff_idx_t staffIdx)
{
    if (staffIdx != mu::nidx) {
        // local timesig
        // don't actually rewrite, just update measure rest durations
        // abort if there is anything other than measure rests in range
        track_idx_t strack = staffIdx * VOICES;
        track_idx_t etrack = strack + VOICES;
        for (Measure* m = fm;; m = m->nextMeasure()) {
            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                for (track_idx_t track = strack; track < etrack; ++track) {
                    ChordRest* cr = toChordRest(s->element(track));
                    if (!cr) {
                        continue;
                    }
                    if (cr->isRest() && cr->durationType() == DurationType::V_MEASURE) {
                        cr->undoChangeProperty(Pid::DURATION, ns);
                    } else {
                        return false;
                    }
                }
            }
            if (m == lm) {
                break;
            }
        }
        return true;
    }

    bool fmr = true;

    // Format: chord 1 tick, chord 2 tick, tremolo, track
    std::vector<std::tuple<Fraction, Fraction, Tremolo*, track_idx_t> > tremoloChordTicks;

    track_idx_t strack, etrack;
    if (staffIdx == mu::nidx) {
        strack = 0;
        etrack = ntracks();
    } else {
        strack = staffIdx * VOICES;
        etrack = strack + VOICES;
    }

    for (Measure* m = fm; m; m = m->nextMeasure()) {
        if (!m->isFullMeasureRest()) {
            fmr = false;
        }

        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (track_idx_t track = strack; track < etrack; ++track) {
                ChordRest* cr = toChordRest(s->element(track));
                if (cr && cr->isChord()) {
                    Chord* chord = toChord(cr);
                    if (chord->tremolo() && chord->tremolo()->twoNotes()) {
                        Tremolo* trem = chord->tremolo();

                        // Don't add same chord twice
                        if (trem->chord2() == chord) {
                            continue;
                        }
                        auto newP
                            = std::tuple<Fraction, Fraction, Tremolo*, track_idx_t>(cr->tick(),
                                                                                    trem->chord2()->segment()->tick(), trem, track);
                        tremoloChordTicks.push_back(newP);
                    }
                }
            }
        }

        if (m == lm) {
            break;
        }
    }

    if (!fmr) {
        // check for local time signatures
        for (Measure* m = fm; m; m = m->nextMeasure()) {
            for (size_t si = 0; si < nstaves(); ++si) {
                if (staff(si)->timeStretch(m->tick()) != Fraction(1, 1)) {
                    // we cannot change a staff with a local time signature
                    return false;
                }
                if (m == lm) {
                    break;
                }
            }
        }
    }

    ScoreRange range;
    range.read(fm->first(), lm->last());

    //
    // calculate number of required measures = nm
    //
    Fraction k = range.ticks() / ns;
    int nm     = (k.numerator() + k.denominator() - 1) / k.denominator();

    Fraction nd = ns * Fraction(nm, 1);

    // evtl. we have to fill the last measure
    Fraction fill = nd - range.ticks();
    range.fill(fill);

    for (Score* s : scoreList()) {
        Measure* m1 = s->tick2measure(fm->tick());
        Measure* m2 = s->tick2measure(lm->tick());

        Fraction tick1 = m1->tick();
        Fraction tick2 = m2->endTick();
        auto spanners = s->spannerMap().findOverlapping(tick1.ticks(), tick2.ticks());
        for (auto i : spanners) {
            if (i.value->tick() >= tick1 && i.value->tick() < tick2) {
                undo(new RemoveElement(i.value));
            }
        }
        s->undoRemoveMeasures(m1, m2, true);

        Measure* nfm = 0;
        Measure* nlm = 0;
        Fraction tick     = fm->tick();
        for (int i = 0; i < nm; ++i) {
            Measure* m = Factory::createMeasure(s->dummy()->system());
            m->setPrev(nlm);
            if (nlm) {
                nlm->setNext(m);
            }
            m->setTimesig(ns);
            m->setTicks(ns);
            m->setTick(tick);
            tick += m->ticks();
            nlm = m;
            if (nfm == 0) {
                nfm = m;
            }
        }
//            nlm->setEndBarLineType(m2->endBarLineType(), m2->endBarLineGenerated(),
//               m2->endBarLineVisible(), m2->endBarLineColor());
        //
        // insert new calculated measures
        //
        nfm->setPrev(m1->prev());
        nlm->setNext(m2->next());
        s->undo(new InsertMeasures(nfm, nlm));
    }
    if (!fill.isZero()) {
        undoInsertTime(lm->endTick(), fill);
    }

    if (!range.write(masterScore(), fm->tick())) {
        return false;
    }
    connectTies(true);

    // reset start and end elements for slurs that overlap the rewritten measures
    for (auto spanner : m_spanner.findOverlapping(fm->tick().ticks(), lm->tick().ticks())) {
        Slur* slur = (spanner.value->isSlur() ? toSlur(spanner.value) : nullptr);
        if (slur) {
            EngravingItem* startEl = slur->startElement();
            EngravingItem* endEl = slur->endElement();
            if (!startEl || !endEl) {
                continue;
            }
            undo(new ChangeStartEndSpanner(spanner.value, slur->findStartCR(), slur->findEndCR()));
        }
    }
    // Attempt to move tremolos to correct chords
    for (auto tremPair : tremoloChordTicks) {
        Fraction chord1Tick = std::get<0>(tremPair);
        Fraction chord2Tick = std::get<1>(tremPair);
        Tremolo* trem       = std::get<2>(tremPair);
        int track      = std::get<3>(tremPair);

        undo(new MoveTremolo(trem->score(), chord1Tick, chord2Tick, trem, track));
    }

    if (noteEntryMode()) {
        // set input cursor to possibly re-written segment
        Fraction icTick = inputPos();
        Segment* icSegment = tick2segment(icTick, false, SegmentType::ChordRest);
        if (!icSegment) {
            // this can happen if cursor was on a rest
            // and in the rewriting it got subsumed into a full measure rest
            Measure* icMeasure = tick2measure(icTick);
            if (!icMeasure) {                         // shouldn't happen, but just in case
                icMeasure = firstMeasure();
            }
            icSegment = icMeasure->first(SegmentType::ChordRest);
        }
        inputState().setSegment(icSegment);
    }

    return true;
}

//---------------------------------------------------------
//   rewriteMeasures
//    rewrite all measures up to the next time signature or section break
//---------------------------------------------------------

bool Score::rewriteMeasures(Measure* fm, const Fraction& ns, staff_idx_t staffIdx)
{
    Measure* lm  = fm;
    Measure* fm1 = fm;
    Measure* nm  = nullptr;
    LayoutBreak* sectionBreak = nullptr;

    // disable local time sig modifications in linked staves
    if (staffIdx != mu::nidx && masterScore()->excerpts().size() > 0) {
        MScore::setError(MsError::CANNOT_CHANGE_LOCAL_TIMESIG_HAS_EXCERPTS);
        return false;
    }

    //
    // split into Measure segments fm-lm
    //
    for (MeasureBase* measure = fm;; measure = measure->next()) {
        if (!measure || !measure->isMeasure() || lm->sectionBreak()
            || (toMeasure(measure)->first(SegmentType::TimeSig) && measure != fm)) {
            // save section break to reinstate after rewrite
            LayoutBreak* layoutBreak = lm->sectionBreakElement();

            if (lm && layoutBreak) {
                sectionBreak = Factory::copyLayoutBreak(*layoutBreak);
            }

            if (!rewriteMeasures(fm1, lm, ns, staffIdx)) {
                if (staffIdx != mu::nidx) {
                    MScore::setError(MsError::CANNOT_CHANGE_LOCAL_TIMESIG_MEASURE_NOT_EMPTY);
                    // restore measure rests that were prematurely modified
                    Fraction fr(staff(staffIdx)->timeSig(fm->tick())->sig());
                    for (Measure* m = fm1; m; m = m->nextMeasure()) {
                        ChordRest* cr = m->findChordRest(m->tick(), staffIdx * VOICES);
                        if (cr && cr->isRest() && cr->durationType() == DurationType::V_MEASURE) {
                            cr->undoChangeProperty(Pid::DURATION, fr);
                        } else {
                            break;
                        }
                    }
                }
                for (Measure* m = fm1; m; m = m->nextMeasure()) {
                    if (m->first(SegmentType::TimeSig)) {
                        break;
                    }
                    Fraction fr(ns);
                    m->undoChangeProperty(Pid::TIMESIG_NOMINAL, fr);
                }
                return false;
            }

            // after rewrite, lm is not necessarily valid
            // m is first MeasureBase after rewritten range
            // m->prevMeasure () is new last measure of range
            // set nm to first true Measure after rewritten range
            // we may use this to reinstate time signatures
            if (measure && measure->prevMeasure()) {
                nm = measure->prevMeasure()->nextMeasure();
            } else {
                nm = nullptr;
            }

            if (sectionBreak) {
                // reinstate section break, then stop rewriting
                if (measure && measure->prevMeasure()) {
                    sectionBreak->setParent(measure->prevMeasure());
                    undoAddElement(sectionBreak);
                } else if (!measure) {
                    sectionBreak->setParent(lastMeasure());
                    undoAddElement(sectionBreak);
                } else {
                    LOGD("unable to restore section break");
                    nm = nullptr;
                    sectionBreak = nullptr;
                }
                break;
            }

            // stop rewriting at end of score
            // or at a measure (which means we found a time signature segment)
            if (!measure || measure->isMeasure()) {
                break;
            }

            // skip frames
            while (!measure->isMeasure()) {
                LayoutBreak* layoutBreak2 = measure->sectionBreakElement();

                if (layoutBreak2) {
                    // frame has a section break; we can stop skipping ahead
                    sectionBreak = layoutBreak2;
                    break;
                }
                measure = measure->next();
                if (!measure) {
                    break;
                }
            }
            // stop rewriting if we encountered a section break on a frame
            // or if there is a time signature on first measure after the frame
            if (sectionBreak || (measure && toMeasure(measure)->first(SegmentType::TimeSig))) {
                break;
            }

            // set up for next range to rewrite
            fm1 = toMeasure(measure);
            if (fm1 == 0) {
                break;
            }
        }

        // if we didn't break the loop already,
        // we must have an ordinary measure
        // add measure to range to rewrite
        lm = toMeasure(measure);
    }

    // if any staves don't have time signatures at the point where we stopped,
    // we need to reinstate their previous time signatures
    if (!nm) {
        return true;
    }
    Segment* s = nm->undoGetSegment(SegmentType::TimeSig, nm->tick());
    for (size_t i = 0; i < nstaves(); ++i) {
        if (!s->element(i * VOICES)) {
            TimeSig* ots = staff(i)->timeSig(nm->tick());
            if (ots) {
                TimeSig* nts = Factory::copyTimeSig(*ots);
                nts->setParent(s);
                if (sectionBreak) {
                    nts->setGenerated(false);
                    nts->setShowCourtesySig(false);
                }
                undoAddElement(nts);
            }
        }
    }

    return true;
}

//---------------------------------------------------------
//   cmdAddTimeSig
//
//    Add or change time signature at measure in response
//    to gui command (drop timesig on measure or timesig)
//---------------------------------------------------------

void Score::cmdAddTimeSig(Measure* fm, staff_idx_t staffIdx, TimeSig* ts, bool local)
{
    deselectAll();

    if (fm->isMMRest()) {
        fm = fm->mmRestFirst();
    }

    Fraction ns   = ts->sig();
    Fraction tick = fm->tick();
    TimeSig* lts  = staff(staffIdx)->timeSig(tick);
    if (local) {
        Fraction stretch = (ns / fm->timesig()).reduced();
        ts->setStretch(stretch);
    }

    Fraction stretch;
    Fraction lsig;                  // last signature
    if (lts) {
        stretch = lts->stretch();
        lsig    = lts->sig();
    } else {
        stretch.set(1, 1);
        lsig.set(4, 4);              // set to default
    }

    track_idx_t track = staffIdx * VOICES;
    Segment* seg = fm->undoGetSegment(SegmentType::TimeSig, tick);
    TimeSig* ots = toTimeSig(seg->element(track));

    if (ots && (*ots == *ts)) {
        //
        //  ignore if there is already a timesig
        //  with same values
        //
        delete ts;
        return;
    }

    auto getStaffIdxRange = [this, local, staffIdx](const Score* score) -> std::pair<staff_idx_t /*start*/, staff_idx_t /*end*/> {
        staff_idx_t startStaffIdx, endStaffIdx;
        if (local) {
            if (score == this) {
                startStaffIdx = staffIdx;
                endStaffIdx = startStaffIdx + 1;
            } else {
                // TODO: get index for this score
                LOGD("cmdAddTimeSig: unable to write local time signature change to linked score");
                startStaffIdx = 0;
                endStaffIdx = 0;
            }
        } else {
            startStaffIdx = 0;
            endStaffIdx = score->nstaves();
        }
        return std::make_pair(startStaffIdx, endStaffIdx);
    };

    if (ots && ots->sig() == ns && ots->stretch() == ts->stretch()) {
        //
        // the measure duration does not change,
        // so its ok to just update the time signatures
        //
        TimeSig* nts = staff(staffIdx)->nextTimeSig(tick + Fraction::fromTicks(1));
        const Fraction lmTick = nts ? nts->segment()->tick() : Fraction(-1, 1);
        for (Score* score : scoreList()) {
            Measure* mf = score->tick2measure(tick);
            Measure* lm = (lmTick != Fraction(-1, 1)) ? score->tick2measure(lmTick) : nullptr;
            for (Measure* m = mf; m != lm; m = m->nextMeasure()) {
                bool changeActual = m->ticks() == m->timesig();
                m->undoChangeProperty(Pid::TIMESIG_NOMINAL, ns);
                if (changeActual) {
                    m->undoChangeProperty(Pid::TIMESIG_ACTUAL, ns);
                }
            }
            std::pair<staff_idx_t, staff_idx_t> staffIdxRange = getStaffIdxRange(score);
            for (staff_idx_t si = staffIdxRange.first; si < staffIdxRange.second; ++si) {
                TimeSig* nsig = toTimeSig(seg->element(si * VOICES));
                if (!nsig) {
                    continue;
                }
                nsig->undoChangeProperty(Pid::SHOW_COURTESY, ts->showCourtesySig());
                nsig->undoChangeProperty(Pid::TIMESIG, ts->sig());
                nsig->undoChangeProperty(Pid::TIMESIG_TYPE, int(ts->timeSigType()));
                nsig->undoChangeProperty(Pid::NUMERATOR_STRING, ts->numeratorString());
                nsig->undoChangeProperty(Pid::DENOMINATOR_STRING, ts->denominatorString());
                nsig->undoChangeProperty(Pid::TIMESIG_STRETCH, ts->stretch());
                nsig->undoChangeProperty(Pid::GROUP_NODES, ts->groups().nodes());
                nsig->setSelected(false);
                nsig->setDropTarget(false);
            }
        }
    } else {
        Score* mScore = masterScore();
        Measure* mf  = mScore->tick2measure(tick);

        //
        // rewrite all measures up to the next time signature
        //
        if (mf == mScore->firstMeasure() && mf->nextMeasure() && (mf->ticks() != mf->timesig())) {
            // handle upbeat
            mf->undoChangeProperty(Pid::TIMESIG_NOMINAL, ns);
            Measure* m = mf->nextMeasure();
            Segment* s = m->findSegment(SegmentType::TimeSig, m->tick());
            mf = s ? 0 : mf->nextMeasure();
        } else {
            if (sigmap()->timesig(seg->tick().ticks()).nominal().identical(ns)) {
                // no change to global time signature,
                // but we need to rewrite any staves with local time signatures
                for (size_t i = 0; i < nstaves(); ++i) {
                    if (staff(i)->timeSig(tick) && staff(i)->timeSig(tick)->isLocal()) {
                        if (!mScore->rewriteMeasures(mf, ns, i)) {
                            undoStack()->current()->unwind();
                            return;
                        }
                    }
                }
                mf = 0;
            }
        }

        // try to rewrite the measures first
        // we will only add time signatures if this succeeds
        // this means, however, that the rewrite cannot depend on the time signatures being in place
        if (mf) {
            if (!mScore->rewriteMeasures(mf, ns, local ? staffIdx : mu::nidx)) {
                undoStack()->current()->unwind();
                return;
            }
        }
        // add the time signatures
        std::map<track_idx_t, TimeSig*> masterTimeSigs;
        for (Score* score : scoreList()) {
            Measure* nfm = score->tick2measure(tick);
            seg = nfm->undoGetSegment(SegmentType::TimeSig, nfm->tick());
            std::pair<staff_idx_t, staff_idx_t> staffIdxRange = getStaffIdxRange(score);
            for (staff_idx_t si = staffIdxRange.first; si < staffIdxRange.second; ++si) {
                if (fm->isMeasureRepeatGroup(si)) {
                    deleteItem(fm->measureRepeatElement(si));
                }
                TimeSig* nsig = toTimeSig(seg->element(si * VOICES));
                if (nsig == 0) {
                    nsig = Factory::copyTimeSig(*ts);
                    nsig->setScore(score);
                    nsig->setTrack(si * VOICES);
                    nsig->setParent(seg);
                    undoAddElement(nsig);
                    if (score->excerpt()) {
                        const track_idx_t masterTrack = mu::key(score->excerpt()->tracksMapping(), nsig->track());
                        TimeSig* masterTimeSig = masterTimeSigs[masterTrack];
                        if (masterTimeSig) {
                            undo(new Link(masterTimeSig, nsig));
                        }
                    }
                } else {
                    nsig->undoChangeProperty(Pid::SHOW_COURTESY, ts->showCourtesySig());
                    nsig->undoChangeProperty(Pid::TIMESIG_TYPE, int(ts->timeSigType()));
                    nsig->undoChangeProperty(Pid::TIMESIG, ts->sig());
                    nsig->undoChangeProperty(Pid::NUMERATOR_STRING, ts->numeratorString());
                    nsig->undoChangeProperty(Pid::DENOMINATOR_STRING, ts->denominatorString());

                    // HACK do it twice to accommodate undo
                    nsig->undoChangeProperty(Pid::TIMESIG_TYPE, int(ts->timeSigType()));
                    nsig->undoChangeProperty(Pid::TIMESIG_STRETCH, ts->stretch());
                    nsig->undoChangeProperty(Pid::GROUP_NODES, ts->groups().nodes());
                    nsig->setSelected(false);
                    nsig->setDropTarget(false);                 // DEBUG
                }

                if (score->isMaster()) {
                    masterTimeSigs[nsig->track()] = nsig;
                }
            }
        }
    }
    delete ts;
}

//---------------------------------------------------------
//   cmdRemoveTimeSig
//---------------------------------------------------------

void Score::cmdRemoveTimeSig(TimeSig* ts)
{
    if (ts->isLocal() && masterScore()->excerpts().size() > 0) {
        MScore::setError(MsError::CANNOT_CHANGE_LOCAL_TIMESIG_HAS_EXCERPTS);
        return;
    }

    Measure* m = ts->measure();
    Segment* s = ts->segment();

    //
    // we cannot remove a courtesy time signature
    //
    if (m->tick() != s->tick()) {
        return;
    }
    Fraction tick = m->tick();

    // if we remove all time sigs from segment, segment will be already removed by now
    // but this would leave us no means of detecting that we have measures in a local timesig
    // in cases where we try deleting the local time sig
    // known bug: this means we do not correctly detect non-empty measures when deleting global timesig change after a local one
    // see http://musescore.org/en/node/51596
    // Delete the time sig segment from the root score, we will rewriteMeasures from it
    // since it contains all the music while the part doesn't
    Score* rScore = masterScore();
    Measure* rm = rScore->tick2measure(m->tick());
    Segment* rs = rm->findSegment(SegmentType::TimeSig, s->tick());
    if (rs) {
        rScore->undoRemoveElement(rs);
    }

    Measure* pm = m->prevMeasure();
    Fraction ns(pm ? pm->timesig() : Fraction(4, 4));

    if (!rScore->rewriteMeasures(rm, ns, mu::nidx)) {
        undoStack()->current()->unwind();
    } else {
        m = tick2measure(tick);           // old m may have been replaced
        // hack: fix measure rest durations for staves with local time signatures
        // if a time signature was deleted to reveal a previous local one,
        // then rewriteMeasures() got the measure rest durations wrong
        // (if we fixed it to work for delete, it would fail for add)
        // so we will fix measure rest durations here
        // TODO: fix rewriteMeasures() to get this right
        for (size_t i = 0; i < nstaves(); ++i) {
            TimeSig* tsig = staff(i)->timeSig(tick);
            if (tsig && tsig->isLocal()) {
                for (Measure* nm = m; nm; nm = nm->nextMeasure()) {
                    // stop when time signature changes
                    if (staff(i)->timeSig(nm->tick()) != tsig) {
                        break;
                    }
                    // fix measure rest duration
                    ChordRest* cr = nm->findChordRest(nm->tick(), i * VOICES);
                    if (cr && cr->isRest() && cr->durationType() == DurationType::V_MEASURE) {
                        cr->undoChangeProperty(Pid::DURATION, nm->stretchedLen(staff(i)));
                    }
                    //cr->setTicks(nm->stretchedLen(staff(i)));
                }
            }
        }
    }
}

//---------------------------------------------------------
//  addTiedMidiPitch
//---------------------------------------------------------

Note* Score::addTiedMidiPitch(int pitch, bool addFlag, Chord* prevChord)
{
    Note* n = addMidiPitch(pitch, addFlag);
    if (prevChord) {
        Note* nn = prevChord->findNote(n->pitch());
        if (nn) {
            Tie* tie = Factory::createTie(this->dummy());
            tie->setStartNote(nn);
            tie->setEndNote(n);
            tie->setTick(tie->startNote()->tick());
            tie->setTick2(tie->endNote()->tick());
            tie->setTrack(n->track());
            n->setTieBack(tie);
            nn->setTieFor(tie);
            undoAddElement(tie);
        }
    }
    return n;
}

NoteVal Score::noteVal(int pitch) const
{
    NoteVal nval(pitch);
    Staff* st = staff(inputState().track() / VOICES);

    // if transposing, interpret MIDI pitch as representing desired written pitch
    // set pitch based on corresponding sounding pitch
    if (!style().styleB(Sid::concertPitch)) {
        nval.pitch += st->part()->instrument(inputState().tick())->transpose().chromatic;
    }
    // let addPitch calculate tpc values from pitch
    //Key key   = st->key(inputState().tick());
    //nval.tpc1 = pitch2tpc(nval.pitch, key, Prefer::NEAREST);

    return nval;
}

//---------------------------------------------------------
//  addMidiPitch
//---------------------------------------------------------

Note* Score::addMidiPitch(int pitch, bool addFlag)
{
    NoteVal nval = noteVal(pitch);
    return addPitch(nval, addFlag);
}

//---------------------------------------------------------
//   searchNote
//    search for note or rest before or at tick position tick
//    in staff
//---------------------------------------------------------

ChordRest* Score::searchNote(const Fraction& tick, track_idx_t track) const
{
    ChordRest* ipe = 0;
    SegmentType st = SegmentType::ChordRest;
    for (Segment* segment = firstSegment(st); segment; segment = segment->next1(st)) {
        ChordRest* cr = segment->cr(track);
        if (!cr) {
            continue;
        }
        if (cr->tick() == tick) {
            return cr;
        }
        if (cr->tick() > tick) {
            return ipe ? ipe : cr;
        }
        ipe = cr;
    }
    return 0;
}

//---------------------------------------------------------
//   regroupNotesAndRests
//    * combine consecutive rests into fewer rests of longer duration.
//    * combine tied notes/chords into fewer notes of longer duration.
//    Only operates on one voice - protects manual layout adjustment, etc.
//---------------------------------------------------------

void Score::regroupNotesAndRests(const Fraction& startTick, const Fraction& endTick, track_idx_t track)
{
    Segment* inputSegment = m_is.segment();   // store this so we can get back to it later.
    Segment* seg = tick2segment(startTick, true, SegmentType::ChordRest);
    for (Measure* msr = seg->measure(); msr && msr->tick() < endTick; msr = msr->nextMeasure()) {
        Fraction maxTick = endTick > msr->endTick() ? msr->endTick() : endTick;
        if (!seg || seg->measure() != msr) {
            seg = msr->first(SegmentType::ChordRest);
        }
        for (; seg; seg = seg->next(SegmentType::ChordRest)) {
            ChordRest* curr = seg->cr(track);
            if (!curr) {
                continue;         // this voice is empty here (CR overlaps with CR in other track)
            }
            if (seg->tick() + curr->actualTicks() > maxTick) {
                break;         // outside range
            }
            if (curr->isRest() && !(curr->tuplet()) && !(toRest(curr)->isGap())) {
                // combine consecutive rests
                ChordRest* lastRest = curr;
                for (Segment* s = seg->next(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                    ChordRest* cr = s->cr(track);
                    if (!cr) {
                        continue;             // this voice is empty here
                    }
                    if (!cr->isRest() || s->tick() + cr->actualTicks() > maxTick || toRest(cr)->isGap()) {
                        break;             // next element in the same voice is not a rest, or it exceeds the selection, or it is a gap
                    }
                    lastRest = cr;
                }
                Fraction restTicks = lastRest->tick() + lastRest->ticks() - curr->tick();
                seg = setNoteRest(seg, curr->track(), NoteVal(), restTicks, DirectionV::AUTO, false, {}, true);
            } else if (curr->isChord()) {
                // combine tied chords
                Chord* chord = toChord(curr);
                Chord* lastTiedChord = chord;
                for (Chord* next = chord->nextTiedChord(); next && next->tick() + next->ticks() <= maxTick; next = next->nextTiedChord()) {
                    lastTiedChord = next;
                }
                if (!lastTiedChord) {
                    lastTiedChord = chord;
                }
                Fraction noteTicks = lastTiedChord->tick() + lastTiedChord->ticks() - chord->tick();
                if (!(curr->tuplet())) {
                    // store start/end note for backward/forward ties ending/starting on the group of notes being rewritten
                    size_t numNotes = chord->notes().size();
#if (!defined (_MSCVER) && !defined (_MSC_VER))
                    Note* tieBack[numNotes];
                    Note* tieFor[numNotes];
#else
                    // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
                    //    heap allocation is slow, an optimization might be used.
                    std::vector<Note*> tieBack(numNotes);
                    std::vector<Note*> tieFor(numNotes);
#endif
                    for (size_t i = 0; i < numNotes; i++) {
                        Note* n = chord->notes()[i];
                        Note* nn = lastTiedChord->notes()[i];
                        if (n->tieBack()) {
                            tieBack[i] = n->tieBack()->startNote();
                        } else {
                            tieBack[i] = 0;
                        }
                        if (nn->tieFor()) {
                            tieFor[i] = nn->tieFor()->endNote();
                        } else {
                            tieFor[i] = 0;
                        }
                    }
                    Fraction tick = seg->tick();
                    track_idx_t tr = chord->track();
                    Fraction sd   = noteTicks;
                    Tie* tie      = 0;
                    Segment* segment = seg;
                    ChordRest* cr = toChordRest(segment->element(tr));
                    Chord* nchord = toChord(chord->clone());
                    for (size_t i = 0; i < numNotes; i++) {           // strip ties from cloned chord
                        Note* n = nchord->notes()[i];
                        if (Tie* tieFor2 = n->tieFor()) {
                            n->setTieFor(nullptr);
                            delete tieFor2;
                        }
                        if (Tie* tieBack2 = n->tieBack()) {
                            n->setTieBack(nullptr);
                            delete tieBack2;
                        }
                    }
                    Chord* startChord = nchord;
                    Measure* measure = nullptr;
                    bool firstpart = true;
                    for (;;) {
                        if (tr % VOICES) {
                            expandVoice(segment, tr);
                        }
                        // the returned gap ends at the measure boundary or at tuplet end
                        Fraction dd = makeGap(segment, tr, sd, cr->tuplet());
                        if (dd.isZero()) {
                            break;
                        }
                        measure = segment->measure();
                        std::vector<TDuration> dl;
                        dl = toRhythmicDurationList(dd, false, segment->rtick(), sigmap()->timesig(tick.ticks()).nominal(), measure, 1);
                        size_t n = dl.size();
                        for (size_t i = 0; i < n; ++i) {
                            const TDuration& d = dl[i];
                            Chord* nchord2 = toChord(nchord->clone());
                            if (!firstpart) {
                                nchord2->removeMarkings(true);
                            }
                            nchord2->setDurationType(d);
                            nchord2->setTicks(d.fraction());
                            std::vector<Note*> nl1 = nchord->notes();
                            std::vector<Note*> nl2 = nchord2->notes();
                            if (!firstpart) {
                                for (size_t j = 0; j < nl1.size(); ++j) {
                                    tie = Factory::createTie(this->dummy());
                                    tie->setStartNote(nl1[j]);
                                    tie->setEndNote(nl2[j]);
                                    tie->setTick(tie->startNote()->tick());
                                    tie->setTick2(tie->endNote()->tick());
                                    tie->setTrack(tr);
                                    nl1[j]->setTieFor(tie);
                                    nl2[j]->setTieBack(tie);
                                }
                            }
                            undoAddCR(nchord2, measure, tick);
                            segment = nchord2->segment();
                            tick += nchord2->actualTicks();
                            nchord = nchord2;
                            firstpart = false;
                        }
                        sd -= dd;
                        if (sd.isZero()) {
                            break;
                        }
                        Segment* nseg = tick2segment(tick, false, SegmentType::ChordRest);
                        if (nseg == 0) {
                            break;
                        }
                        segment = nseg;
                        cr = toChordRest(segment->element(tr));
                        if (cr == 0) {
                            if (tr % VOICES) {
                                cr = addRest(segment, tr, TDuration(DurationType::V_MEASURE), 0);
                            } else {
                                break;
                            }
                        }
                    }
                    if (m_is.slur()) {
                        // extend slur
                        m_is.slur()->undoChangeProperty(Pid::SPANNER_TICKS, nchord->tick() - m_is.slur()->tick());
                        for (EngravingObject* e : m_is.slur()->linkList()) {
                            Slur* slur = toSlur(e);
                            for (EngravingObject* ee : nchord->linkList()) {
                                EngravingItem* e1 = static_cast<EngravingItem*>(ee);
                                if (e1->score() == slur->score() && e1->track() == slur->track2()) {
                                    slur->score()->undo(new ChangeSpannerElements(slur, slur->startElement(), e1));
                                    break;
                                }
                            }
                        }
                    }
                    // recreate previously stored pending ties
                    for (size_t i = 0; i < numNotes; i++) {
                        Note* n = startChord->notes()[i];
                        Note* nn = nchord->notes()[i];
                        if (tieBack[i]) {
                            tie = Factory::createTie(this->dummy());
                            tie->setStartNote(tieBack[i]);
                            tie->setEndNote(n);
                            tie->setTick(tie->startNote()->tick());
                            tie->setTick2(tie->endNote()->tick());
                            tie->setTrack(track);
                            n->setTieBack(tie);
                            tieBack[i]->setTieFor(tie);
                            undoAddElement(tie);
                        }
                        if (tieFor[i]) {
                            tie = Factory::createTie(this->dummy());
                            tie->setStartNote(nn);
                            tie->setEndNote(tieFor[i]);
                            tie->setTick(tie->startNote()->tick());
                            tie->setTick2(tie->endNote()->tick());
                            tie->setTrack(track);
                            nn->setTieFor(tie);
                            tieFor[i]->setTieBack(tie);
                            undoAddElement(tie);
                        }
                    }
                    if (tie) {         // at least one tie was created
                        connectTies();
                    }
                }
            }
        }
    }
    // now put the input state back where it was before
    m_is.setSegment(inputSegment);
}

//---------------------------------------------------------
//   cmdTieNoteList
//---------------------------------------------------------

std::vector<Note*> Score::cmdTieNoteList(const Selection& selection, bool noteEntryMode)
{
    EngravingItem* el = selection.element();
    if (Note* n = InputState::note(el)) {
        if (noteEntryMode) {
            return n->chord()->notes();
        } else {
            return { n };
        }
    } else {
        ChordRest* cr = InputState::chordRest(el);
        if (cr && cr->isChord()) {
            return toChord(cr)->notes();
        }
    }
    return selection.noteList();
}

//---------------------------------------------------------
//   cmdAddTie
//---------------------------------------------------------

void Score::cmdAddTie(bool addToChord)
{
    const std::vector<Note*> noteList = cmdTieNoteList(selection(), noteEntryMode());

    if (noteList.empty()) {
        LOGD("no notes selected");
        return;
    }

    startCmd();
    Chord* lastAddedChord = 0;
    for (Note* note : noteList) {
        if (note->tieFor()) {
            LOGD("cmdAddTie: note %p has already tie? noteFor: %p", note, note->tieFor());
            if (addToChord) {
                continue;
            } else {
                undoRemoveElement(note->tieFor());
            }
        }

        if (noteEntryMode()) {
            ChordRest* cr = nullptr;
            Chord* c = note->chord();

            // set cursor at position after note
            if (c->isGraceBefore()) {
                // tie grace note before to main note
                cr = toChord(c->explicitParent());
                addToChord = true;
            } else {
                m_is.setSegment(note->chord()->segment());
                m_is.moveToNextInputPos();
                m_is.setLastSegment(m_is.segment());

                if (m_is.cr() == 0) {
                    expandVoice();
                }
                cr = m_is.cr();
            }
            if (cr == 0) {
                break;
            }

            bool addFlag = lastAddedChord != nullptr;

            // try to re-use existing note or chord
            Note* n = nullptr;
            if (addToChord && cr->isChord()) {
                Chord* chord = toChord(cr);
                Note* nn = chord->findNote(note->pitch());
                if (nn && nn->tpc() == note->tpc()) {
                    n = nn;                     // re-use note
                } else {
                    addFlag = true;             // re-use chord
                }
            }

            // if no note to re-use, create one
            NoteVal nval(note->noteVal());
            if (!n) {
                n = addPitch(nval, addFlag);
            } else {
                select(n);
            }

            if (n) {
                if (!lastAddedChord) {
                    lastAddedChord = n->chord();
                }
                // n is not necessarily next note if duration span over measure
                Note* nnote = searchTieNote(note);
                while (nnote) {
                    // DEBUG: if duration spans over measure
                    // this does not set line for intermediate notes
                    // tpc was set correctly already
                    //n->setLine(note->line());
                    //n->setTpc(note->tpc());
                    Tie* tie = Factory::createTie(this->dummy());
                    tie->setStartNote(note);
                    tie->setEndNote(nnote);
                    tie->setTrack(note->track());
                    tie->setTick(note->chord()->segment()->tick());
                    tie->setTicks(nnote->chord()->segment()->tick() - note->chord()->segment()->tick());
                    undoAddElement(tie);
                    if (!addFlag || nnote->chord()->tick() >= lastAddedChord->tick() || nnote->chord()->isGrace()) {
                        break;
                    } else {
                        note = nnote;
                        m_is.setLastSegment(m_is.segment());
                        nnote = addPitch(nval, true);
                    }
                }
            }
        } else {
            Note* note2 = searchTieNote(note);
            if (note2) {
                Tie* tie = Factory::createTie(this->dummy());
                tie->setStartNote(note);
                tie->setEndNote(note2);
                tie->setTrack(note->track());
                tie->setTick(note->chord()->segment()->tick());
                tie->setTicks(note2->chord()->segment()->tick() - note->chord()->segment()->tick());
                undoAddElement(tie);
            }
        }
    }
    if (lastAddedChord) {
        nextInputPos(lastAddedChord, false);
    }
    endCmd();
}

//---------------------------------------------------------
//   cmdRemoveTie
//---------------------------------------------------------

void Score::cmdToggleTie()
{
    const std::vector<Note*> noteList = cmdTieNoteList(selection(), noteEntryMode());

    if (noteList.empty()) {
        LOGD("no notes selected");
        return;
    }

    bool canAddTies = false;
    const size_t notes = noteList.size();
    std::vector<Note*> tieNoteList(notes);

    for (size_t i = 0; i < notes; ++i) {
        Note* n = noteList[i];
        if (n->tieFor()) {
            tieNoteList[i] = nullptr;
        } else {
            Note* tieNote = searchTieNote(n);
            tieNoteList[i] = tieNote;
            if (tieNote) {
                canAddTies = true;
            }
        }
    }

    startCmd();

    if (canAddTies) {
        for (size_t i = 0; i < notes; ++i) {
            Note* note2 = tieNoteList[i];
            if (note2) {
                Note* note = noteList[i];

                Tie* tie = Factory::createTie(this->dummy());
                tie->setStartNote(note);
                tie->setEndNote(note2);
                tie->setTrack(note->track());
                tie->setTick(note->chord()->segment()->tick());
                tie->setTicks(note2->chord()->segment()->tick() - note->chord()->segment()->tick());
                undoAddElement(tie);
            }
        }
    } else {
        for (Note* n : noteList) {
            Tie* tie = n->tieFor();
            if (tie) {
                undoRemoveElement(tie);
            }
        }
    }

    endCmd();
}

//---------------------------------------------------------
//   cmdAddOttava
//---------------------------------------------------------

void Score::cmdAddOttava(OttavaType type)
{
    const Selection sel = selection();   // copy selection state before the operation.
    // add on each staff if possible
    if (sel.isRange() && sel.staffStart() != sel.staffEnd() - 1) {
        for (staff_idx_t staffIdx = sel.staffStart(); staffIdx < sel.staffEnd(); ++staffIdx) {
            ChordRest* cr1 = sel.firstChordRest(staffIdx * VOICES);
            ChordRest* cr2 = sel.lastChordRest(staffIdx * VOICES);
            if (!cr1) {
                continue;
            }
            if (cr2 == 0) {
                cr2 = cr1;
            }
            Ottava* ottava = Factory::createOttava(this->dummy());
            ottava->setOttavaType(type);
            if (type == OttavaType::OTTAVA_8VB /*|| type == OttavaType::OTTAVA_15MB || type == OttavaType::OTTAVA_22MB*/) {
                ottava->setPlacement(PlacementV::BELOW);
                ottava->styleChanged();
            }
            ottava->setTrack(cr1->track());
            ottava->setTrack2(cr1->track());
            ottava->setTick(cr1->tick());
            ottava->setTick2(cr2->tick() + cr2->actualTicks());
            undoAddElement(ottava);
        }
    } else {
        ChordRest* cr1;
        ChordRest* cr2;
        getSelectedChordRest2(&cr1, &cr2);
        if (!cr1) {
            return;
        }
        if (cr2 == 0) {
            cr2 = cr1;
        }

        Ottava* ottava = Factory::createOttava(this->dummy());
        ottava->setOttavaType(type);
        if (type == OttavaType::OTTAVA_8VB /*|| type == OttavaType::OTTAVA_15MB || type == OttavaType::OTTAVA_22MB*/) {
            ottava->setPlacement(PlacementV::BELOW);
            ottava->styleChanged();
        }
        ottava->setTrack(cr1->track());
        ottava->setTrack2(cr1->track());
        ottava->setTick(cr1->tick());
        ottava->setTick2(cr2->tick() + cr2->actualTicks());
        undoAddElement(ottava);
        if (!noteEntryMode()) {
            select(ottava, SelectType::SINGLE, 0);
        }
    }
}

//---------------------------------------------------------
//   addHairpins
//---------------------------------------------------------

std::vector<Hairpin*> Score::addHairpins(HairpinType type)
{
    std::vector<Hairpin*> hairpins;

    // add hairpin on each staff if possible
    if (selection().isRange() && selection().staffStart() != selection().staffEnd() - 1) {
        for (staff_idx_t staffIdx = selection().staffStart(); staffIdx < selection().staffEnd(); ++staffIdx) {
            ChordRest* cr1 = selection().firstChordRest(staffIdx * VOICES);
            ChordRest* cr2 = selection().lastChordRest(staffIdx * VOICES);
            hairpins.push_back(addHairpin(type, cr1, cr2));
        }
    } else {
        // for single staff range selection, or single selection,
        // find start & end elements elements
        ChordRest* cr1 = nullptr;
        ChordRest* cr2 = nullptr;
        getSelectedChordRest2(&cr1, &cr2);
        hairpins.push_back(addHairpin(type, cr1, cr2));
    }

    return hairpins;
}

//---------------------------------------------------------
//   cmdAddNoteLine
//---------------------------------------------------------

void Score::addNoteLine()
{
    std::vector<Note*> selectedNotes;

    if (selection().isRange()) {
        track_idx_t startTrack = selection().staffStart() * VOICES;
        track_idx_t endTrack = selection().staffEnd() * VOICES;

        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            std::vector<Note*> notes = selection().noteList(track);
            selectedNotes.insert(selectedNotes.end(), notes.begin(), notes.end());
        }
    } else {
        std::vector<Note*> notes = selection().noteList();
        selectedNotes.insert(selectedNotes.end(), notes.begin(), notes.end());
    }

    Note* firstNote = nullptr;
    Note* lastNote  = nullptr;

    for (Note* note : selectedNotes) {
        if (firstNote == nullptr || firstNote->chord()->tick() > note->chord()->tick()) {
            firstNote = note;
        }
        if (lastNote == nullptr || lastNote->chord()->tick() < note->chord()->tick()) {
            lastNote = note;
        }
    }

    if (!firstNote || !lastNote) {
        LOGD("addNoteLine: no note %p %p", firstNote, lastNote);
        return;
    }

    if (firstNote == lastNote) {
        LOGD("addNoteLine: no support for note to same note line %p", firstNote);
        return;
    }

    TextLine* line = new TextLine(firstNote);
    line->setParent(firstNote);
    line->setStartElement(firstNote);
    line->setDiagonal(true);
    line->setAnchor(Spanner::Anchor::NOTE);
    line->setTick(firstNote->chord()->tick());
    line->setEndElement(lastNote);

    undoAddElement(line);
}

//---------------------------------------------------------
//   cmdSetBeamMode
//---------------------------------------------------------

void Score::cmdSetBeamMode(BeamMode mode)
{
    for (ChordRest* cr : getSelectedChordRests()) {
        if (cr) {
            cr->undoChangeProperty(Pid::BEAM_MODE, mode);
        }
    }
}

//---------------------------------------------------------
//   cmdFlip
//---------------------------------------------------------

void Score::cmdFlip()
{
    const std::vector<EngravingItem*>& el = selection().elements();
    if (el.empty()) {
        MScore::setError(MsError::NO_FLIPPABLE_SELECTED);
        return;
    }

    std::set<const EngravingItem*> alreadyFlippedElements;
    auto flipOnce = [&alreadyFlippedElements](const EngravingItem* element, std::function<void()> flipFunction) -> void {
        if (alreadyFlippedElements.insert(element).second) {
            flipFunction();
        }
    };

    for (EngravingItem* e : el) {
        if (e->isNote() || e->isStem() || e->isHook()) {
            Chord* chord = nullptr;
            if (e->isNote()) {
                chord = toNote(e)->chord();
            } else if (e->isStem()) {
                chord = toStem(e)->chord();
            } else {
                chord = toHook(e)->chord();
            }

            IF_ASSERT_FAILED(chord) {
                continue;
            }

            if (chord->beam()) {
                if (!selection().isRange()) {
                    e = chord->beam();
                } else {
                    continue;
                }
            } else if (chord->tremolo() && chord->tremolo()->twoNotes()) {
                if (!selection().isRange()) {
                    e = chord->tremolo();
                } else {
                    continue;
                }
            } else {
                flipOnce(chord, [chord]() {
                    DirectionV dir = chord->up() ? DirectionV::DOWN : DirectionV::UP;
                    chord->undoChangeProperty(Pid::STEM_DIRECTION, dir);
                });
            }
        }

        if (e->isBeam()) {
            auto beam = toBeam(e);
            flipOnce(beam, [beam]() {
                DirectionV dir = beam->up() ? DirectionV::DOWN : DirectionV::UP;
                beam->undoChangeProperty(Pid::STEM_DIRECTION, dir);
            });
        } else if (e->isTremolo()) {
            auto tremolo = toTremolo(e);
            flipOnce(tremolo, [tremolo]() {
                DirectionV dir = tremolo->up() ? DirectionV::DOWN : DirectionV::UP;
                tremolo->undoChangeProperty(Pid::STEM_DIRECTION, dir);
            });
        } else if (e->isSlurTieSegment()) {
            auto slurTieSegment = toSlurTieSegment(e)->slurTie();
            flipOnce(slurTieSegment, [slurTieSegment]() {
                DirectionV dir = slurTieSegment->up() ? DirectionV::DOWN : DirectionV::UP;
                slurTieSegment->undoChangeProperty(Pid::SLUR_DIRECTION, dir);
            });
        } else if (e->isArticulationFamily()) {
            auto artic = toArticulation(e);
            flipOnce(artic, [artic]() {
                ArticulationAnchor articAnchor = artic->anchor();
                switch (articAnchor) {
                case ArticulationAnchor::TOP:
                    articAnchor = ArticulationAnchor::BOTTOM;
                    break;
                case ArticulationAnchor::BOTTOM:
                    articAnchor = ArticulationAnchor::TOP;
                    break;
                case ArticulationAnchor::AUTO:
                    articAnchor = artic->up() ? ArticulationAnchor::BOTTOM : ArticulationAnchor::TOP;
                    break;
                }
                PropertyFlags pf = artic->propertyFlags(Pid::ARTICULATION_ANCHOR);
                if (pf == PropertyFlags::STYLED) {
                    pf = PropertyFlags::UNSTYLED;
                }
                artic->undoChangeProperty(Pid::ARTICULATION_ANCHOR, int(articAnchor), pf);
            });
        } else if (e->isTuplet()) {
            auto tuplet = toTuplet(e);
            flipOnce(tuplet, [tuplet]() {
                DirectionV dir = tuplet->isUp() ? DirectionV::DOWN : DirectionV::UP;
                tuplet->undoChangeProperty(Pid::DIRECTION, PropertyValue::fromValue<DirectionV>(dir), PropertyFlags::UNSTYLED);
            });
        } else if (e->isNoteDot() && e->explicitParent()->isNote()) {
            Note* note = toNote(e->explicitParent());
            DirectionV d = note->dotIsUp() ? DirectionV::DOWN : DirectionV::UP;
            note->undoChangeProperty(Pid::DOT_POSITION, PropertyValue::fromValue<DirectionV>(d));
        } else if (e->isGuitarBendSegment()) {
            GuitarBend* bend = toGuitarBendSegment(e)->guitarBend();
            flipOnce(bend, [bend] {
                DirectionV direction = bend->ldata()->up() ? DirectionV::DOWN : DirectionV::UP;
                bend->undoChangeProperty(Pid::DIRECTION, PropertyValue::fromValue<DirectionV>(direction));
            });
        } else if (e->isStaffText()
                   || e->isSystemText()
                   || e->isTempoText()
                   || e->isTripletFeel()
                   || e->isJump()
                   || e->isMarker()
                   || e->isRehearsalMark()
                   || e->isMeasureNumber()
                   || e->isInstrumentChange()
                   || e->isPlayTechAnnotation()
                   || e->isCapo()
                   || e->isStringTunings()
                   || e->isSticking()
                   || e->isFingering()
                   || e->isDynamic()
                   || e->isExpression()
                   || e->isHarmony()
                   || e->isFretDiagram()
                   || e->isHairpin()
                   || e->isHairpinSegment()
                   || e->isOttava()
                   || e->isOttavaSegment()
                   || e->isTrill()
                   || e->isTrillSegment()
                   || e->isTextLine()
                   || e->isTextLineSegment()
                   || e->isLetRing()
                   || e->isLetRingSegment()
                   || e->isVibrato()
                   || e->isVibratoSegment()
                   || e->isPalmMute()
                   || e->isPalmMuteSegment()
                   || e->isWhammyBar()
                   || e->isWhammyBarSegment()
                   || e->isRasgueado()
                   || e->isRasgueadoSegment()
                   || e->isHarmonicMark()
                   || e->isHarmonicMarkSegment()
                   || e->isGradualTempoChange()
                   || e->isGradualTempoChangeSegment()
                   || e->isPedal()
                   || e->isPedalSegment()
                   || e->isLyrics()
                   || e->isBreath()
                   || e->isFermata()) {
            e->undoChangeProperty(Pid::AUTOPLACE, true);
            // TODO: undoChangeProperty() should probably do this directly
            // see https://musescore.org/en/node/281432
            EngravingItem* ee = e->propertyDelegate(Pid::PLACEMENT);
            if (!ee) {
                ee = e;
            }

            flipOnce(ee, [ee]() {
                // getProperty() delegates call from spannerSegment to Spanner
                PlacementV p = PlacementV(ee->getProperty(Pid::PLACEMENT).toInt());
                p = (p == PlacementV::ABOVE) ? PlacementV::BELOW : PlacementV::ABOVE;
                PropertyFlags pf = ee->propertyFlags(Pid::PLACEMENT);
                if (pf == PropertyFlags::STYLED) {
                    pf = PropertyFlags::UNSTYLED;
                }
                double oldDefaultY = ee->propertyDefault(Pid::OFFSET).value<PointF>().y();
                ee->undoChangeProperty(Pid::PLACEMENT, int(p), pf);
                // flip and rebase user offset to new default now that placement has changed
                double newDefaultY = ee->propertyDefault(Pid::OFFSET).value<PointF>().y();
                if (ee->isSpanner()) {
                    Spanner* spanner = toSpanner(ee);
                    for (SpannerSegment* ss : spanner->spannerSegments()) {
                        if (!ss->isStyled(Pid::OFFSET)) {
                            PointF off = ss->getProperty(Pid::OFFSET).value<PointF>();
                            double oldY = off.y() - oldDefaultY;
                            off.ry() = newDefaultY - oldY;
                            ss->undoChangeProperty(Pid::OFFSET, off);
                            ss->setOffsetChanged(false);
                        }
                    }
                } else if (!ee->isStyled(Pid::OFFSET)) {
                    PointF off = ee->getProperty(Pid::OFFSET).value<PointF>();
                    double oldY = off.y() - oldDefaultY;
                    off.ry() = newDefaultY - oldY;
                    ee->undoChangeProperty(Pid::OFFSET, off);
                    ee->setOffsetChanged(false);
                }
            });
        }
    }
}

//---------------------------------------------------------
//   deleteItem
//---------------------------------------------------------

void Score::deleteItem(EngravingItem* el)
{
    if (!el) {
        return;
    }
    // cannot remove generated elements
    if (el->generated() && !(el->isBracket() || el->isBarLine() || el->isClef() || el->isMeasureNumber() || el->isKeySig())) {
        return;
    }
//      LOGD("%s", el->typeName());

    switch (el->type()) {
    case ElementType::INSTRUMENT_NAME: {
        Part* part = el->part();
        InstrumentName* in = toInstrumentName(el);
        if (in->instrumentNameType() == InstrumentNameType::LONG) {
            undo(new ChangeInstrumentLong(Fraction(0, 1), part, std::list<StaffName>()));
        } else if (in->instrumentNameType() == InstrumentNameType::SHORT) {
            undo(new ChangeInstrumentShort(Fraction(0, 1), part, std::list<StaffName>()));
        }
    }
    break;

    case ElementType::TIMESIG: {
        // timesig might already be removed
        TimeSig* ts = toTimeSig(el);
        Segment* s = ts->segment();
        Measure* m = s->measure();
        Segment* ns = m->findSegment(s->segmentType(), s->tick());
        if (!ns || (ns->element(ts->track()) != ts)) {
            LOGD("deleteItem: not found");
            break;
        }
        cmdRemoveTimeSig(ts);
    }
    break;

    case ElementType::KEYSIG:
    {
        KeySig* k = toKeySig(el);
        Measure* m = k->measure();
        if (m->isMMRest()) {
            m = m->mmRestFirst();
            if (Segment* s = m->findSegment(SegmentType::KeySig, k->tick())) {
                k = toKeySig(s->element(k->track()));
            }
            if (!k || k->generated()) {
                return;
            }
        }
        bool ic = k->segment()->next(SegmentType::ChordRest)->findAnnotation(ElementType::INSTRUMENT_CHANGE,
                                                                             el->part()->startTrack(), el->part()->endTrack() - 1);
        undoRemoveElement(k);
        if (ic) {
            KeySigEvent ke = k->keySigEvent();
            ke.setForInstrumentChange(true);
            Key cKey = k->staff()->keySigEvent(k->tick()).concertKey();
            Key tKey = cKey;
            if (!style().styleB(Sid::concertPitch)) {
                Interval v = k->part()->instrument(k->tick())->transpose();
                v.flip();
                tKey = transposeKey(cKey, v, k->part()->preferSharpFlat());
            }
            ke.setConcertKey(cKey);
            ke.setKey(tKey);
            undoChangeKeySig(k->staff(), k->tick(), ke);
        }
        for (size_t i = 0; i < k->part()->nstaves(); i++) {
            Staff* staff = k->part()->staff(i);
            KeySigEvent e = staff->keySigEvent(k->tick());
            updateInstrumentChangeTranspositions(e, staff, k->tick());
        }
    }
    break;

    case ElementType::NOTE:
    {
        Chord* chord = toChord(el->explicitParent());
        if (chord->notes().size() > 1) {
            undoRemoveElement(el);
            select(chord->downNote(), SelectType::SINGLE, 0);
            break;
        }
        // else fall through
        el = chord;
    }
    // fall through

    case ElementType::CHORD:
    {
        Chord* chord = toChord(el);
        removeChordRest(chord, false);

        // replace with rest
        if (chord->noteType() == NoteType::NORMAL) {
            Rest* rest = Factory::createRest(this->dummy()->segment(), chord->durationType());
            rest->setDurationType(chord->durationType());
            rest->setTicks(chord->ticks());

            rest->setTrack(el->track());
            rest->setParent(chord->explicitParent());

            Segment* segment = chord->segment();
            undoAddCR(rest, segment->measure(), segment->tick());

            Tuplet* tuplet = chord->tuplet();
            if (tuplet) {
                std::list<EngravingObject*> tl = tuplet->linkList();
                for (EngravingObject* e : rest->linkList()) {
                    DurationElement* de = toDurationElement(e);
                    for (EngravingObject* ee : tl) {
                        Tuplet* t = toTuplet(ee);
                        if (t->score() == de->score() && t->track() == de->track()) {
                            de->setTuplet(t);
                            t->add(de);
                            break;
                        }
                    }
                }
            }
            //select(rest, SelectType::SINGLE, 0);
        } else {
            // remove segment if empty
            Segment* seg = chord->segment();
            if (seg->empty()) {
                undoRemoveElement(seg);
            }
        }
    }
    break;

    case ElementType::MEASURE_REPEAT:
    {
        MeasureRepeat* mr = toMeasureRepeat(el);
        removeChordRest(mr, false);
        Rest* rest = Factory::createRest(this->dummy()->segment());
        rest->setDurationType(DurationType::V_MEASURE);
        rest->setTicks(mr->measure()->stretchedLen(mr->staff()));
        rest->setTrack(mr->track());
        rest->setParent(mr->explicitParent());
        Segment* segment = mr->segment();
        undoAddCR(rest, segment->measure(), segment->tick());

        // tell measures they're not part of measure repeat group anymore
        Measure* m = mr->firstMeasureOfGroup();
        for (int i = 1; i <= mr->numMeasures(); ++i) {
            undoChangeMeasureRepeatCount(m, 0, mr->staffIdx());
            // don't remove grouping if within measure repeat group on another staff
            bool otherStavesStillNeedGroup = false;
            for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                if (m->isMeasureRepeatGroupWithNextM(staffIdx) && staffIdx != mr->staffIdx()) {
                    otherStavesStillNeedGroup = true;
                    break;
                }
            }
            if (!otherStavesStillNeedGroup) {
                m->undoSetNoBreak(false);
            }
            m = m->nextMeasure();
        }
    }
    // fall through
    case ElementType::MMREST:
    case ElementType::REST:
        //
        // only allow for voices != 0
        //    e.g. voice 0 rests cannot be removed
        //
    {
        Rest* rest = toRest(el);
        if (rest->tuplet() && rest->tuplet()->elements().empty()) {
            undoRemoveElement(rest->tuplet());
        }
        if ((el->voice() != 0) && !rest->tuplet()) {
            rest->undoChangeProperty(Pid::GAP, true);
            rest->undoChangeProperty(Pid::BEAM_MODE, BeamMode::AUTO);
            for (EngravingObject* r : el->linkList()) {
                Rest* rr = toRest(r);
                if (rr->track() % VOICES) {
                    rr->undoChangeProperty(Pid::GAP, true);
                }
            }

            // delete them really when only gap rests are in the actual measure.
            Measure* m = toRest(el)->measure();
            track_idx_t track = el->track();
            if (m->isOnlyDeletedRests(track)) {
                static const SegmentType st { SegmentType::ChordRest };
                for (const Segment* s = m->first(st); s; s = s->next(st)) {
                    EngravingItem* del = s->element(track);
                    if (s->segmentType() != st || !del) {
                        continue;
                    }
                    if (toRest(del)->isGap()) {
                        undoRemoveElement(del);
                    }
                }
                checkSpanner(m->tick(), m->endTick());
            } else {
                // check if the other rest could be combined
                Segment* s = toRest(el)->segment();

                std::vector<Rest*> rests;
                // find previous segment with cr in this track
                EngravingItem* pe = 0;
                for (Segment* ps = s->prev(SegmentType::ChordRest); ps; ps = ps->prev(SegmentType::ChordRest)) {
                    EngravingItem* elm = ps->element(track);
                    if (elm && elm->isRest() && toRest(elm)->isGap()) {
                        pe = el;
                        rests.push_back(toRest(elm));
                    } else if (elm) {
                        break;
                    }
                }
                // find next segment with cr in this track
                Segment* ns;
                EngravingItem* ne = 0;
                for (ns = s->next(SegmentType::ChordRest); ns; ns = ns->next(SegmentType::ChordRest)) {
                    EngravingItem* elm = ns->element(track);
                    if (elm && elm->isRest() && toRest(elm)->isGap()) {
                        ne = elm;
                        rests.push_back(toRest(elm));
                    } else if (elm) {
                        break;
                    }
                }

                Fraction stick = pe ? pe->tick() : s->tick();
                Fraction ticks = { 0, 1 };

                if (ne) {
                    ticks = ne->tick() - stick + toRest(ne)->actualTicks();
                } else if (ns) {
                    ticks = ns->tick() - stick;
                } else {
                    ticks = m->ticks() + m->tick() - stick;
                }

                if (ticks != m->ticks() && ticks != s->ticks()) {
                    undoRemoveElement(rest);
                    for (Rest* r : rests) {
                        undoRemoveElement(r);
                    }

                    Fraction f = ticks;

                    std::vector<TDuration> dList = toDurationList(f, true);
                    if (dList.empty()) {
                        break;
                    }

                    for (const TDuration& d : dList) {
                        Rest* rr = Factory::createRest(this->dummy()->segment());
                        rr->setTicks(d.fraction());
                        rr->setDurationType(d);
                        rr->setTrack(track);
                        rr->setGap(true);
                        undoAddCR(rr, m, stick);
                    }
                }
            }
            // Set input position
            // TODO If deleted element is last of a sequence, use prev?
            if (noteEntryMode()) {
                score()->move(u"prev-chord");
            }
        }
    }
    break;

    case ElementType::ACCIDENTAL:
        if (el->explicitParent()->isNote()) {
            changeAccidental(toNote(el->explicitParent()), AccidentalType::NONE);
        } else {
            undoRemoveElement(el);
        }
        break;

    case ElementType::BAR_LINE: {
        BarLine* bl = toBarLine(el);
        Segment* s = bl->segment();
        Measure* m = s->measure();
        if (s->isBeginBarLineType() || s->isBarLineType()) {
            undoRemoveElement(el);
        } else {
            if (bl->barLineType() == BarLineType::START_REPEAT) {
                Measure* m2 = m->isMMRest() ? m->mmRestFirst() : m;
                for (Score* lscore : score()->scoreList()) {
                    Measure* lmeasure = lscore->tick2measure(m2->tick());
                    if (lmeasure) {
                        lmeasure->undoChangeProperty(Pid::REPEAT_START, false);
                    }
                }
            } else if (bl->barLineType() == BarLineType::END_REPEAT) {
                Measure* m2 = m->isMMRest() ? m->mmRestLast() : m;
                for (Score* lscore : score()->scoreList()) {
                    Measure* lmeasure = lscore->tick2measure(m2->tick());
                    if (lmeasure) {
                        lmeasure->undoChangeProperty(Pid::REPEAT_END, false);
                    }
                }
            } else {
                bl->undoChangeProperty(Pid::BARLINE_TYPE, PropertyValue::fromValue(BarLineType::NORMAL));
            }
        }
    }
    break;

    case ElementType::TUPLET:
        cmdDeleteTuplet(toTuplet(el), true);
        break;

    case ElementType::MEASURE: {
        Measure* m = toMeasure(el);
        undoRemoveMeasures(m, m);
        undoInsertTime(m->tick(), -(m->endTick() - m->tick()));
    }
    break;

    case ElementType::BRACKET:
        undoRemoveBracket(toBracket(el));
        break;

    case ElementType::LAYOUT_BREAK:
    {
        undoRemoveElement(el);
        LayoutBreak* lb = toLayoutBreak(el);
        MeasureBase* mb = lb->measure();
        Measure* m = mb && mb->isMeasure() ? toMeasure(mb) : nullptr;
        if (m && m->isMMRest()) {
            // propagate to original measure
            m = m->mmRestLast();
            for (EngravingItem* e : m->el()) {
                if (e->isLayoutBreak()) {
                    undoRemoveElement(e);
                    break;
                }
            }
        }
    }
    break;

    case ElementType::CLEF:
    {
        Clef* clef = toClef(el);
        Measure* m = clef->measure();
        if (m->isMMRest()) {
            // propagate to original measure
            m = m->mmRestLast();
            Segment* s = m->findSegment(SegmentType::Clef, clef->segment()->tick());
            if (s && s->element(clef->track())) {
                Clef* c = toClef(s->element(clef->track()));
                undoRemoveElement(c);
            }
        } else {
            if (clef->generated()) {
                // find the real clef if this is a cautionary one
                if (m && m->prevMeasure()) {
                    Fraction tick = m->tick();
                    m = m->prevMeasure();
                    Segment* s = m->findSegment(SegmentType::Clef, tick);
                    if (s && s->element(clef->track())) {
                        clef = toClef(s->element(clef->track()));
                    }
                }
            }
            undoRemoveElement(clef);
        }
    }
    break;

    case ElementType::MEASURE_NUMBER:
    {
        Measure* mea = toMeasure(el->explicitParent());
        switch (mea->measureNumberMode()) {
        // If the user tries to remove an automatically generated measure number,
        // we should force the measure not to show any measure number
        case MeasureNumberMode::AUTO:
            mea->undoChangeProperty(Pid::MEASURE_NUMBER_MODE, static_cast<int>(MeasureNumberMode::HIDE));
            break;

        // If the user tries to remove a measure number that he added manually,
        // then we should set the MeasureNumberMode to AUTO only if will not show if set to auto.
        // If after setting the MeasureNumberMode to AUTO, the measure number still shows,
        // We need to force the measure to hide its measure number.
        case MeasureNumberMode::SHOW:
            if (mea->showsMeasureNumberInAutoMode()) {
                mea->undoChangeProperty(Pid::MEASURE_NUMBER_MODE, static_cast<int>(MeasureNumberMode::HIDE));
            } else {
                mea->undoChangeProperty(Pid::MEASURE_NUMBER_MODE, static_cast<int>(MeasureNumberMode::AUTO));
            }
            break;
        case MeasureNumberMode::HIDE:
            break;
        }
    }
    break;
    case ElementType::REHEARSAL_MARK:
    case ElementType::TEMPO_TEXT:
    {
        Segment* s = toSegment(el->explicitParent());
        Measure* m = s->measure();
        if (m->isMMRest()) {
            // propagate to original measure/element
            m = m->mmRestFirst();
            Segment* ns = m->findSegment(SegmentType::ChordRest, s->tick());
            const auto annotations = ns->annotations(); // make a copy since we alter the list
            for (EngravingItem* e : annotations) {
                if (e->type() == el->type() && e->track() == el->track()) {
                    el = e;
                    undoRemoveElement(el);
                    break;
                }
            }
        } else {
            undoRemoveElement(el);
        }
    }
    break;

    case ElementType::OTTAVA_SEGMENT:
    case ElementType::HAIRPIN_SEGMENT:
    case ElementType::TRILL_SEGMENT:
    case ElementType::VIBRATO_SEGMENT:
    case ElementType::TEXTLINE_SEGMENT:
    case ElementType::VOLTA_SEGMENT:
    case ElementType::SLUR_SEGMENT:
    case ElementType::TIE_SEGMENT:
    case ElementType::LYRICSLINE_SEGMENT:
    case ElementType::PEDAL_SEGMENT:
    case ElementType::GLISSANDO_SEGMENT:
    case ElementType::LET_RING_SEGMENT:
    case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT:
    case ElementType::PALM_MUTE_SEGMENT:
    case ElementType::WHAMMY_BAR_SEGMENT:
    case ElementType::RASGUEADO_SEGMENT:
    case ElementType::HARMONIC_MARK_SEGMENT:
    case ElementType::PICK_SCRAPE_SEGMENT:
    case ElementType::GUITAR_BEND_SEGMENT:
    {
        el = toSpannerSegment(el)->spanner();
        undoRemoveElement(el);
    }
    break;

    case ElementType::STEM_SLASH:                   // cannot delete this elements
    case ElementType::HOOK:
        LOGD("cannot remove %s", el->typeName());
        break;

    case ElementType::TEXT:
        if ((el->explicitParent() && el->explicitParent()->isTBox()) || el->isTBox()) {
            el->undoChangeProperty(Pid::TEXT, String());
        } else {
            undoRemoveElement(el);
        }
        break;

    case ElementType::INSTRUMENT_CHANGE:
    {
        InstrumentChange* ic = static_cast<InstrumentChange*>(el);
        Fraction tickStart = ic->segment()->tick();
        Part* part = ic->part();
        Interval oldV = part->staff(0)->transpose(tickStart);
        undoRemoveElement(el);
        if (tickStart != Fraction(0, 1)) {
            for (KeySig* keySig : ic->keySigs()) {
                deleteItem(keySig);
            }
        }
        for (Clef* clef : ic->clefs()) {
            deleteItem(clef);
        }
        if (part->staff(0)->transpose(tickStart) != oldV) {
            auto i = part->instruments().upper_bound(tickStart.ticks());
            Fraction tickEnd;
            if (i == part->instruments().end()) {
                tickEnd = Fraction(-1, 1);
            } else {
                tickEnd = Fraction::fromTicks(i->first);
            }
            transpositionChanged(part, oldV, tickStart, tickEnd);
        }
    }
    break;

    case ElementType::MARKER:
    {
        Measure* m = toMeasure(el->explicitParent());
        if (m->isMMRest()) {
            // find corresponding marker in underlying measure
            bool found = false;
            // the marker may be in the first measure...
            for (EngravingItem* e : m->mmRestFirst()->el()) {
                if (e->isMarker() && e->subtype() == el->subtype()) {
                    undoRemoveElement(e);
                    found = true;
                    break;
                }
            }
            if (!found) {
                // ...or it may be in the last measure
                for (EngravingItem* e : m->mmRestLast()->el()) {
                    if (e->isMarker() && e->subtype() == el->subtype()) {
                        undoRemoveElement(e);
                        break;
                    }
                }
            }
        }
        // whether m is an mmrest or not, we still need to remove el
        undoRemoveElement(el);
    }
    break;

    case ElementType::JUMP:
    {
        Measure* m = toMeasure(el->explicitParent());
        if (m->isMMRest()) {
            // find corresponding jump in underlying measure
            for (EngravingItem* e : m->mmRestLast()->el()) {
                if (e->isJump() && e->subtype() == el->subtype()) {
                    undoRemoveElement(e);
                    break;
                }
            }
        }
        // whether m is an mmrest or not, we still need to remove el
        undoRemoveElement(el);
    }
    break;

    default:
        undoRemoveElement(el);
        break;
    }
}

//---------------------------------------------------------
//   deleteMeasures
//---------------------------------------------------------

void Score::deleteMeasures(MeasureBase* mbStart, MeasureBase* mbEnd, bool preserveTies)
{
    select(0, SelectType::SINGLE, 0);

    if (mbEnd->isMeasure()) {
        Measure* mbEndMeasure = toMeasure(mbEnd);
        if (mbEndMeasure->isMMRest()) {
            mbEnd = mbEndMeasure->mmRestLast();
        }
    }

    if (!preserveTies) {
        // we're not preserving slurs and ties where they are, so the endpoints need to be either
        // moved or the slur has to be deleted
        reconnectSlurs(mbStart, mbEnd);
    }

    // get the last deleted timesig & keysig in order to restore after deletion
    std::map<staff_idx_t, TimeSig*> lastDeletedTimeSigs;

    for (MeasureBase* mb = mbEnd;; mb = mb->prev()) {
        if (mb->isMeasure()) {
            Measure* m = toMeasure(mb);
            Segment* sts = m->findSegment(SegmentType::TimeSig, m->tick());

            if (sts) {
                for (staff_idx_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                    if (!mu::contains(lastDeletedTimeSigs, staffIdx)) {
                        if (TimeSig* ts = toTimeSig(sts->element(staffIdx * VOICES))) {
                            lastDeletedTimeSigs.insert({ staffIdx, ts });
                        }
                    }
                }
            }

            if (lastDeletedTimeSigs.size() == nstaves()) {
                break;
            }
        }
        if (mb == mbStart) {
            break;
        }
    }

    std::vector<KeySigEvent> lastDeletedKeySigEvents;

    for (Staff* staff : score()->staves()) {
        KeySigEvent kse = staff->keySigEvent(mbEnd->tick());
        lastDeletedKeySigEvents.push_back(kse);
    }

    Fraction startTick = mbStart->tick();
    Fraction endTick   = mbEnd->tick();

    for (Score* score : scoreList()) {
        Measure* startMeasure = score->tick2measure(startTick);
        Measure* endMeasure = score->tick2measure(endTick);

        score->undoRemoveMeasures(startMeasure, endMeasure, preserveTies);

        // adjust views
        Measure* focusOn = startMeasure->prevMeasure() ? startMeasure->prevMeasure() : score->firstMeasure();
        for (MuseScoreView* v : score->m_viewer) {
            v->adjustCanvasPosition(focusOn);
        }

        // insert correct timesig after deletion
        Measure* mBeforeSel = startMeasure->prevMeasure();
        Measure* mAfterSel  = mBeforeSel ? mBeforeSel->nextMeasure() : score->firstMeasure();
        if (mAfterSel) {
            Segment* s = mAfterSel->findSegment(SegmentType::TimeSig, mAfterSel->tick());

            for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                if (s && s->element(staffIdx * VOICES)) {
                    continue;
                }

                TimeSig* lastDeletedForThisStaff = mu::value(lastDeletedTimeSigs, staffIdx, nullptr);
                if (!lastDeletedForThisStaff) {
                    continue;
                }

                if (mBeforeSel) {
                    const Staff* staff = this->staff(staffIdx);
                    if (!staff) {
                        continue;
                    }

                    const TimeSig* timeSig = staff->timeSig(mBeforeSel->tick());
                    if (!timeSig || timeSig->sig() == lastDeletedForThisStaff->sig()) {
                        continue;
                    }
                }

                if (!s) {
                    s = mAfterSel->undoGetSegment(SegmentType::TimeSig, mAfterSel->tick());
                }

                TimeSig* nts = Factory::createTimeSig(s);
                nts->setTrack(staffIdx * VOICES);
                nts->setParent(s);
                nts->setFrom(lastDeletedForThisStaff);
                nts->setStretch(nts->sig() / mAfterSel->timesig());
                score->undoAddElement(nts);
            }
        }

        // insert correct keysig if necessary
        if (mAfterSel && !mBeforeSel) {
            Segment* s = mAfterSel->undoGetSegment(SegmentType::KeySig, mAfterSel->tick());

            bool userCustomizedKeySig = false;
            for (EngravingItem* e : s->elist()) {
                if (e && e->isKeySig() && !toKeySig(e)->generated()) {
                    userCustomizedKeySig = true;
                    break;
                }
            }
            if (userCustomizedKeySig) {
                continue;
            }

            bool concertPitch = score->style().styleB(Sid::concertPitch);
            for (Staff* staff : score->staves()) {
                Part* part = staff->part();
                Instrument* instrument = part->instrument(Fraction(0, 1));
                KeySigEvent nkse;
                staff_idx_t staffIdx = staff->idx();

                if (instrument->useDrumset()) {
                    nkse.setConcertKey(Key::C);
                    nkse.setCustom(true);
                    nkse.setMode(KeyMode::NONE);
                } else {
                    nkse = lastDeletedKeySigEvents.at(staffIdx);
                    if (!concertPitch && !nkse.isAtonal()) {
                        Interval v = instrument->transpose();
                        v.flip();
                        nkse.setKey(transposeKey(nkse.concertKey(), v, part->preferSharpFlat()));
                    }
                }

                KeySig* nks = (KeySig*)s->elementAt(staff2track(staffIdx));
                if (!nks) {
                    nks = Factory::createKeySig(s);
                    nks->setParent(s);
                    nks->setTrack(staffIdx * VOICES);
                    nks->setKeySigEvent(nkse);
                    score->undoAddElement(nks);
                } else {
                    nks->setGenerated(false);
                    nks->setKeySigEvent(nkse);
                    staff->setKey(mAfterSel->tick(), nkse);
                }
            }
        }
    }

    undoInsertTime(mbStart->tick(), -(mbEnd->endTick() - mbStart->tick()));
    m_is.setSegment(0);          // invalidate position
}

void Score::reconnectSlurs(MeasureBase* mbStart, MeasureBase* mbEnd)
{
    // Reconnect or remove slurs that intersect with these deleted measures
    Fraction sTick = mbStart->tick();
    Fraction eTick = mbEnd->tick();
    for (auto overlappingSlur : spannerMap().findOverlapping(sTick.ticks(), eTick.ticks())) {
        Spanner* sp = overlappingSlur.value;
        if (!sp->isSlur()) {
            continue;
        }
        ChordRest* cr1 = sp->startCR();
        ChordRest* cr2 = sp->endCR();
        if (!cr1 || !cr2) {
            // this is an invalid slur
            continue;
        }
        MeasureBase* m = mbStart;
        bool adjust1 = false;
        bool adjust2 = false;
        Measure* measure1 = cr1->measure();
        Measure* measure2 = cr2->measure();
        do {
            adjust1 = adjust1 ? true : measure1 == m;
            adjust2 = adjust2 ? true : measure2 == m;
            if (m == mbEnd) {
                break;
            } else {
                m = m->next();
            }
        } while (m && !(adjust1 && adjust2));
        if (adjust1 && adjust2) {
            // if both endpoints of this slur are inside this deleted range, remove the slur.
            undoRemoveElement(sp);
            continue;
        } else if (adjust1) {
            // endpoint 1 is in deleted range. move endpoint forward to first non-deleted CR after range
            Measure* mNext = mbEnd->next() && mbEnd->next()->isMeasure() ? toMeasure(mbEnd->next()) : nullptr;
            if (!mNext) {
                undoRemoveElement(sp);
                continue;
            }
            ChordRest* firstAvailableCr = nullptr;
            // search for last cr in this voice
            Segment* seg = mNext->first(SegmentType::ChordRest);
            track_idx_t spTrack = sp->track();
            ChordRest* spEndCr = sp->endCR();
            Fraction spEndCrTick = spEndCr->tick();
            while (seg) {
                firstAvailableCr = seg->cr(spTrack);
                if (!firstAvailableCr || !firstAvailableCr->isChord()) {
                    seg = seg->next(SegmentType::ChordRest);
                    continue;
                }
                auto gracesBefore = toChord(firstAvailableCr)->graceNotesBefore();
                if (firstAvailableCr->tick() > spEndCrTick) {
                    // we would create a negative-length slur. remove it entirely.
                    undoRemoveElement(sp);
                    break;
                } else if (spEndCr == firstAvailableCr) {
                    // this would be a zero-length slur, but a possibility exists that this cr has grace after
                    if (gracesBefore.empty()) {
                        undoRemoveElement(sp);
                        break;
                    }
                }
                if (!gracesBefore.empty()) {
                    firstAvailableCr = gracesBefore.front();
                }
                Spanner* newSp = toSpanner(sp->clone());
                newSp->setStartElement(firstAvailableCr);
                newSp->setTick(firstAvailableCr->tick());
                undoChangeElement(sp, newSp);
                break;
            }
        } else if (adjust2) {
            // endpoint 2 is in deleted range. move endpoint backward to last non-deleted CR before range
            Measure* mPrev = mbStart->prev() && mbStart->prev()->isMeasure() ? toMeasure(mbStart->prev()) : nullptr;
            if (!mPrev) {
                undoRemoveElement(sp);
                continue;
            }
            ChordRest* firstAvailableCr = nullptr;
            // search for last cr in this voice
            Segment* seg = mPrev->last();
            if (seg->segmentType() != SegmentType::ChordRest) {
                seg = seg->prev(SegmentType::ChordRest);
            }
            ChordRest* spStartCr = sp->startCR();
            Fraction spStartCrTick = spStartCr->tick();
            while (seg) {
                firstAvailableCr = seg->cr(sp->track2());
                if (!firstAvailableCr || !firstAvailableCr->isChord()) {
                    seg = seg->prev(SegmentType::ChordRest);
                    continue;
                }
                auto gracesAfter = toChord(firstAvailableCr)->graceNotesAfter();
                if (firstAvailableCr->tick() < spStartCrTick) {
                    // we would create a negative-length slur. remove it entirely.
                    undoRemoveElement(sp);
                    break;
                } else if (spStartCr == toChordRest(firstAvailableCr)) {
                    // this would be a zero-length slur, but a possibility exists that this cr has grace after
                    if (gracesAfter.empty()) {
                        undoRemoveElement(sp);
                        break;
                    }
                }
                if (!gracesAfter.empty()) {
                    firstAvailableCr = gracesAfter.back();
                }
                Spanner* newSp = toSpanner(sp->clone());
                newSp->setEndElement(firstAvailableCr);
                newSp->setTick2(firstAvailableCr->tick());
                undoChangeElement(sp, newSp);
                break;
            }
        } else {
            // both of the endpoints are outside of the deleted range. there is no reason to change or delete anything
        }
    }
}

//---------------------------------------------------------
//   deleteSpannersFromRange
///   Deletes spanners in the given range that match the
///   given selection filter.
//---------------------------------------------------------

void Score::deleteSpannersFromRange(const Fraction& t1, const Fraction& t2, track_idx_t track1, track_idx_t track2,
                                    const SelectionFilter& filter)
{
    auto spanners = m_spanner.findOverlapping(t1.ticks(), t2.ticks() - 1);
    for (auto i : spanners) {
        Spanner* sp = i.value;
        if (sp->isVolta() || sp->systemFlag()) {
            continue;
        }
        if (!filter.canSelectVoice(sp->track())) {
            continue;
        }
        if (sp->track() >= track1 && sp->track() < track2) {
            if (sp->tick() >= t1 && sp->tick() < t2
                && sp->tick2() >= t1 && sp->tick2() <= t2) {
                undoRemoveElement(sp);
            } else if (sp->isSlur() && ((sp->tick() >= t1 && sp->tick() < t2)
                                        || (sp->tick2() >= t1 && sp->tick2() < t2))) {
                undoRemoveElement(sp);
            }
        }
    }
}

//---------------------------------------------------------
//   deleteAnnotationsFromRange
///   Deletes annotations in the given range that match the
///   given selection filter.
//---------------------------------------------------------

void Score::deleteAnnotationsFromRange(Segment* s1, Segment* s2, track_idx_t track1, track_idx_t track2, const SelectionFilter& filter)
{
    if (!s1) {
        return;
    }
    if (s2 && (*s2) < (*s1)) {
        return;
    }

    for (track_idx_t track = track1; track < track2; ++track) {
        if (!filter.canSelectVoice(track)) {
            continue;
        }
        for (Segment* s = s1; s && s != s2; s = s->next1()) {
            const auto annotations = s->annotations(); // make a copy since we alter the list
            for (EngravingItem* annotation : annotations) {
                // skip if not included in selection (eg, filter)
                if (!filter.canSelect(annotation)) {
                    continue;
                }
                if (!annotation->systemFlag() && annotation->track() == track && !(annotation->type() == ElementType::INSTRUMENT_CHANGE)) {
                    deleteItem(annotation);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   deleteRange
///   Deletes elements in the given range that match the
///   given selection filter.
///   \return A chord/rest inside the selected range
///   that can be used to establish a selection after this
///   deletion operation.
//---------------------------------------------------------

ChordRest* Score::deleteRange(Segment* s1, Segment* s2, track_idx_t track1, track_idx_t track2, const SelectionFilter& filter)
{
    ChordRest* cr = nullptr;

    if (s1) {
        // delete content from measures underlying mmrests
        if (s1 && s1->measure() && s1->measure()->isMMRest()) {
            s1 = s1->measure()->mmRestFirst()->first();
        }
        if (s2 && s2->measure() && s2->measure()->isMMRest()) {
            s2 = s2->measure()->mmRestLast()->last();
        }

        const Fraction stick1 = s1->tick();
        const Fraction stick2 = s2 ? s2->tick() : lastMeasure()->endTick();

        Segment* ss1 = s1;
        if (!ss1->isChordRestType()) {
            ss1 = ss1->next1(SegmentType::ChordRest);
        }
        bool fullMeasure = ss1 && (ss1->measure()->first(SegmentType::ChordRest) == ss1)
                           && (s2 == 0 || s2->isEndBarLineType());

        Fraction tick2 = s2 ? s2->tick() : Fraction::max();

        deleteSpannersFromRange(stick1, stick2, track1, track2, filter);

        for (track_idx_t track = track1; track < track2; ++track) {
            if (!filter.canSelectVoice(track)) {
                continue;
            }
            Fraction f;
            Fraction tick  = Fraction(-1, 1);
            Tuplet* currentTuplet = 0;
            for (Segment* s = s1; s && (s->tick() < stick2); s = s->next1()) {
                if (s->element(track) && s->isBreathType()) {
                    deleteItem(s->element(track));
                    continue;
                }
                // delete annotations just from this segment and track
                deleteAnnotationsFromRange(s, s->next1(), track, track + 1, filter);

                EngravingItem* e = s->element(track);
                if (!e) {
                    continue;
                }
                if (!s->isChordRestType()) {
                    // do not delete TimeSig/KeySig,
                    // it doesn't make sense to do it, except on full system
                    if (s->segmentType() != SegmentType::TimeSig && s->segmentType() != SegmentType::KeySig) {
                        if (!(e->isBarLine())) {
                            undoRemoveElement(e);
                        }
                    }
                    continue;
                }
                ChordRest* cr1 = toChordRest(e);
                if (tick == Fraction(-1, 1)) {
                    // first ChordRest found:
                    Fraction offset = cr1->rtick();
                    if (cr1->measure()->tick() >= s1->tick() && offset.isNotZero()) {
                        f = offset;
                        tick = s->measure()->tick();
                    } else {
                        f = Fraction(0, 1);
                        tick = s->tick();
                    }
                    currentTuplet = cr1->tuplet();
                    if (currentTuplet && ((currentTuplet->tick()) >= stick1)
                        && ((currentTuplet->tick() + currentTuplet->actualTicks()) <= tick2)) {
                        // Find highest-level complete tuplet contained in range
                        Tuplet* t = cr1->tuplet();
                        while (t->tuplet() && ((t->tuplet()->tick()) >= stick1)
                               && ((t->tuplet()->tick() + t->tuplet()->actualTicks()) <= tick2)) {
                            t = t->tuplet();
                        }
                        cmdDeleteTuplet(t, false);
                        f += t->ticks();
                        currentTuplet = t->tuplet();
                        continue;
                    }
                }
                if (e->isMeasureRepeat()) {
                    deleteItem(e);
                    continue;
                }

                if (currentTuplet != cr1->tuplet()) {
                    if (f.isValid() && !fullMeasure) { // Set rests for the previous tuplet we were dealing with
                        setRest(tick, track, f, false, currentTuplet);
                    }
                    Tuplet* t = cr1->tuplet();
                    if (t && ((t->tick()) >= stick1) && (((t->tick() + t->actualTicks()) <= tick2) || fullMeasure)) { // If deleting a complete tuplet
                        // Find highest-level complete tuplet contained in range
                        while (t->tuplet() && ((t->tuplet()->tick()) >= stick1)
                               && ((t->tuplet()->tick() + t->tuplet()->actualTicks()) <= tick2)) {
                            t = t->tuplet();
                        }
                        cmdDeleteTuplet(t, false);
                        tick = t->tick();
                        f = t->ticks();
                        currentTuplet = t->tuplet();
                        continue;
                    }
                    // Not deleting a complete tuplet
                    tick = cr1->tick();
                    currentTuplet = cr1->tuplet();
                    removeChordRest(cr1, true);
                    f = cr1->ticks();
                } else {
                    removeChordRest(cr1, true);
                    f += cr1->ticks();
                }
            }
            if (f.isValid() && !f.isZero()) {
                if (fullMeasure) {
                    // handle this as special case to be able to
                    // fix broken measures:
                    Staff* staff = Score::staff(track / VOICES);
                    for (Measure* m = s1->measure(); m; m = m->nextMeasure()) {
                        Fraction tick3   = m->tick();
                        Fraction ff = m->stretchedLen(staff);
                        Rest* r = setRest(tick3, track, ff, false, 0);
                        if (!cr) {
                            cr = r;
                        }
                        if (s2 && (m == s2->measure())) {
                            break;
                        }
                    }
                } else {
                    Rest* r = setRest(tick, track, f, false, currentTuplet);
                    if (!cr) {
                        cr = r;
                    }
                }
            }
        }
    }
    return cr;
}

//---------------------------------------------------------
//   cmdDeleteSelection
//---------------------------------------------------------

void Score::cmdDeleteSelection()
{
    ChordRest* crSelectedAfterDeletion = 0;              // select something after deleting notes

    if (selection().isRange()) {
        Segment* s1 = selection().startSegment();
        Segment* s2 = selection().endSegment();
        const Fraction stick1 = selection().tickStart();
        const Fraction stick2 = selection().tickEnd();
        crSelectedAfterDeletion = deleteRange(s1, s2, staff2track(selection().staffStart()),
                                              staff2track(selection().staffEnd()), selectionFilter());
        s1 = tick2segment(stick1);
        s2 = tick2segment(stick2, true);
        if (s1 == 0 || s2 == 0) {
            deselectAll();
        } else {
            m_selection.setStartSegment(s1);
            m_selection.setEndSegment(s2);
            m_selection.updateSelectedElements();
        }
    } else {
        // deleteItem modifies selection().elements() list,
        // so we need a local copy:
        std::vector<EngravingItem*> el = selection().elements();

        // keep track of linked elements that are deleted implicitly
        // so we don't try to delete them twice if they are also in selection
        std::set<EngravingObject*> deletedElements;
        // Similarly, deleting one spanner segment, will delete all of them
        // so we don't try to delete them twice if they are also in selection
        std::set<Spanner*> deletedSpanners;

        for (EngravingItem* e : el) {
            // these are the linked elements we are about to delete
            std::list<EngravingObject*> links;
            if (e->links()) {
                links = *e->links();
            }

            // find location of element to select after deleting notes
            // get tick of element itself if that is valid
            // or of spanner or parent if that is more valid
            Fraction tick  = { -1, 1 };
            track_idx_t track = mu::nidx;
            if (!crSelectedAfterDeletion) {
                if (e->isNote()) {
                    tick = toNote(e)->chord()->tick();
                } else if (e->isRest() || e->isMMRest()) {
                    tick = toRest(e)->tick();
                } else if (e->isMeasureRepeat()) { // may be attached in different measure than it appears
                    tick = toMeasureRepeat(e)->firstMeasureOfGroup()->first()->tick();
                } else if (e->isSpannerSegment()) {
                    tick = toSpannerSegment(e)->spanner()->tick();
                } else if (e->isBreath()) {
                    // we want the tick of the ChordRest that precedes the breath mark (in the same track)
                    for (Segment* s = toBreath(e)->segment()->prev(); s; s = s->prev()) {
                        if (s->isChordRestType() && s->element(e->track())) {
                            tick = s->tick();
                            break;
                        }
                    }
                } else if (e->isBarLine() && toBarLine(e)->barLineType() != BarLineType::START_REPEAT) {
                    // we want the tick of the ChordRest that precedes the barline (in the same track)
                    for (Segment* s = toBarLine(e)->segment()->prev(); s; s = s->prev()) {
                        if (s->isChordRestType() && s->element(e->track())) {
                            tick = s->tick();
                            break;
                        }
                    }
                } else if (e->explicitParent()
                           && (e->explicitParent()->isSegment() || e->explicitParent()->isChord() || e->explicitParent()->isNote()
                               || e->explicitParent()->isRest())) {
                    tick = e->parentItem()->tick();
                }
                //else tick < 0
                track = e->track();
            }

            bool needFindCR = !crSelectedAfterDeletion && tick >= Fraction(0, 1) && track != mu::nidx;

            // We should not allow deleting the very first keySig of the piece, because it is
            // logically incorrect and leads to a state of undefined key/transposition.
            // Also instrument change key signatures should be undeletable.
            // The correct action is for the user to set an atonal/custom keySig as needed.
            if (e->isKeySig()) {
                if (e->tick() == Fraction(0, 1) || toKeySig(e)->forInstrumentChange()) {
                    MScore::setError(MsError::CANNOT_REMOVE_KEY_SIG);
                    if (needFindCR) {
                        crSelectedAfterDeletion = findCR(tick, track);
                    }
                    continue;
                }
            }

            // Don't allow deleting the trill cue note
            if (e->isNote() && toNote(e)->isTrillCueNote()) {
                if (needFindCR) {
                    crSelectedAfterDeletion = findCR(tick, track);
                }
                continue;
            }

            // delete element if we have not done so already
            if (deletedElements.find(e) == deletedElements.end()) {
                // do not delete two spanner segments from the same spanner
                if (e->isSpannerSegment()) {
                    Spanner* spanner = toSpannerSegment(e)->spanner();
                    if (deletedSpanners.find(spanner) != deletedSpanners.end()) {
                        continue;
                    } else {
                        std::list<EngravingObject*> linkedSpanners;
                        if (spanner->links()) {
                            linkedSpanners = *spanner->links();
                        } else {
                            linkedSpanners.push_back(spanner);
                        }
                        for (EngravingObject* se : linkedSpanners) {
                            deletedSpanners.insert(toSpanner(se));
                        }
                    }
                }
                deleteItem(e);
            }

            if (needFindCR) {
                crSelectedAfterDeletion = findCR(tick, track);
            }

            // add these linked elements to list of already-deleted elements
            for (EngravingObject* se : links) {
                deletedElements.insert(se);
            }
        }
    }

    deselectAll();
    // make new selection if appropriate
    if (noteEntryMode()) {
        if (crSelectedAfterDeletion) {
            m_is.setSegment(crSelectedAfterDeletion->segment());
        } else {
            crSelectedAfterDeletion = m_is.cr();
        }
    }
    if (crSelectedAfterDeletion) {
        if (crSelectedAfterDeletion->isChord()) {
            select(toChord(crSelectedAfterDeletion)->upNote(), SelectType::SINGLE);
        } else {
            select(crSelectedAfterDeletion, SelectType::SINGLE);
        }
    }
}

//---------------------------------------------------------
//   cmdFullMeasureRest
//---------------------------------------------------------

void Score::cmdFullMeasureRest()
{
    Segment* s1     = nullptr;
    Segment* s2     = nullptr;
    Fraction stick1 = { -1, 1 };
    Fraction stick2 = { -1, 1 };
    track_idx_t track1 = mu::nidx;
    track_idx_t track2 = mu::nidx;
    Rest* r = nullptr;

    if (noteEntryMode()) {
        s1 = inputState().segment();
        if (!s1 || s1->rtick().isNotZero()) {
            return;
        }
        Measure* m = s1->measure();
        s2 = m->last();
        stick1 = s1->tick();
        stick2 = s2->tick();
        track1 = inputState().track();
        track2 = track1 + 1;
    } else if (selection().isRange()) {
        s1 = selection().startSegment();
        s2 = selection().endSegment();
        if (style().styleB(Sid::createMultiMeasureRests)) {
            // use underlying measures
            if (s1 && s1->measure()->isMMRest()) {
                s1 = tick2segment(stick1);
            }
            if (s2 && s2->measure()->isMMRest()) {
                s2 = tick2segment(stick2, true);
            }
        }
        stick1 = selection().tickStart();
        stick2 = selection().tickEnd();
        Segment* ss1 = s1;
        if (ss1 && ss1->segmentType() != SegmentType::ChordRest) {
            ss1 = ss1->next1(SegmentType::ChordRest);
        }
        bool fullMeasure = ss1 && (ss1->measure()->first(SegmentType::ChordRest) == ss1)
                           && (s2 == 0 || (s2->segmentType() == SegmentType::EndBarLine)
                               || (s2->segmentType() == SegmentType::TimeSigAnnounce)
                               || (s2->segmentType() == SegmentType::KeySigAnnounce));
        if (!fullMeasure) {
            return;
        }
        track1 = selection().staffStart() * VOICES;
        track2 = selection().staffEnd() * VOICES;
    } else if (selection().cr()) {
        ChordRest* cr = selection().cr();
        if (!cr || cr->rtick().isNotZero()) {
            return;
        }
        Measure* m = cr->measure();
        s1 = m->first();
        s2 = m->last();
        stick1 = s1->tick();
        stick2 = s2->tick();
        track1 = selection().cr()->track();
        track2 = track1 + 1;
    } else {
        return;
    }

    for (track_idx_t track = track1; track < track2; ++track) {
        if (selection().isRange() && !selectionFilter().canSelectVoice(track)) {
            continue;
        }
        // first pass - remove non-initial rests from empty measures/voices
        for (Segment* s = s1; s != s2; s = s->next1()) {
            if (!(s->measure()->isOnlyRests(track))) {     // Don't remove anything from measures that contain notes
                continue;
            }
            if (s->segmentType() != SegmentType::ChordRest || !s->element(track)) {
                continue;
            }
            ChordRest* cr = toChordRest(s->element(track));
            // keep first rest of measure as placeholder (replaced in second pass)
            // but delete all others
            if (s->rtick().isNotZero()) {
                removeChordRest(cr, true);
            }
        }
        // second pass - replace placeholders with full measure rests
        for (Measure* m = s1->measure(); m; m = m->nextMeasure()) {
            if (m->isOnlyRests(track)) {
                ChordRest* cr = m->findChordRest(m->tick(), track);
                if (cr) {
                    removeChordRest(cr, true);
                    r = addRest(m->tick(), track, TDuration(DurationType::V_MEASURE), 0);
                } else if (noteEntryMode()) {
                    // might be no cr at input position
                    r = addRest(m->tick(), track, TDuration(DurationType::V_MEASURE), 0);
                }
            }
            if (s2 && (m == s2->measure())) {
                break;
            }
        }
    }

    // selected range is probably empty now and possibly subsumed by an mmrest
    // so updating selection requires forcing mmrests to be updated first
    s1 = tick2segmentMM(stick1);
    s2 = tick2segmentMM(stick2, true);
    if (selection().isRange() && s1 && s2) {
        m_selection.setStartSegment(s1);
        m_selection.setEndSegment(s2);
        m_selection.updateSelectedElements();
    } else if (r) {
        // note entry mode
        select(r, SelectType::SINGLE);
    } else {
        deselectAll();
    }
}

//---------------------------------------------------------
//   addLyrics
//    called from Keyboard Accelerator & menu
//---------------------------------------------------------

Lyrics* Score::addLyrics()
{
    EngravingItem* el = selection().element();
    if (el == 0 || (!el->isNote() && !el->isLyrics() && !el->isRest())) {
        MScore::setError(MsError::NO_LYRICS_SELECTED);
        return 0;
    }
    ChordRest* cr;
    if (el->isNote()) {
        cr = toNote(el)->chord();
        if (cr->isGrace()) {
            cr = toChordRest(cr->explicitParent());
        }
    } else if (el->isLyrics()) {
        cr = toLyrics(el)->chordRest();
    } else if (el->isRest()) {
        cr = toChordRest(el);
    } else {
        return 0;
    }

    int no = int(cr->lyrics().size());
    Lyrics* lyrics = Factory::createLyrics(cr);
    lyrics->setTrack(cr->track());
    lyrics->setParent(cr);
    lyrics->setNo(no);
    undoAddElement(lyrics);
    select(lyrics, SelectType::SINGLE, 0);
    return lyrics;
}

//---------------------------------------------------------
//   addHairpin
//---------------------------------------------------------

Hairpin* Score::addHairpin(HairpinType t, const Fraction& tickStart, const Fraction& tickEnd, track_idx_t track)
{
    Hairpin* pin = Factory::createHairpin(this->dummy()->segment());
    pin->setHairpinType(t);
    if (t == HairpinType::CRESC_LINE) {
        pin->setBeginText(u"cresc.");
        pin->setContinueText(u"(cresc.)");
    } else if (t == HairpinType::DECRESC_LINE) {
        pin->setBeginText(u"dim.");
        pin->setContinueText(u"(dim.)");
    }
    pin->setTrack(track);
    pin->setTrack2(track);
    pin->setTick(tickStart);
    pin->setTick2(tickEnd);
    undoAddElement(pin);
    return pin;
}

//---------------------------------------------------------
//   Score::addHairpin
//---------------------------------------------------------

Hairpin* Score::addHairpin(HairpinType type, ChordRest* cr1, ChordRest* cr2)
{
    if (!cr1) {
        return nullptr;
    }
    if (!cr2) {
        cr2 = cr1;
    }
    assert(cr1->staffIdx() == cr2->staffIdx());
    const Fraction end = cr2->tick() + cr2->actualTicks();
    return addHairpin(type, cr1->tick(), end, cr1->track());
}

//---------------------------------------------------------
//   cmdCreateTuplet
//    replace cr with tuplet
//---------------------------------------------------------

void Score::cmdCreateTuplet(ChordRest* ocr, Tuplet* tuplet)
{
    track_idx_t track = ocr->track();
    Measure* measure = ocr->measure();
    Fraction tick = ocr->tick();
    Fraction an = (tuplet->ticks() * tuplet->ratio()) / tuplet->baseLen().fraction();
    if (!an.denominator()) {
        return;
    }

    if (ocr->tuplet()) {
        tuplet->setTuplet(ocr->tuplet());
    }
    removeChordRest(ocr, false);

    ChordRest* cr;
    if (ocr->isChord()) {
        cr = Factory::createChord(this->dummy()->segment());
        toChord(cr)->setStemDirection(toChord(ocr)->stemDirection());
        for (Note* oldNote : toChord(ocr)->notes()) {
            Note* note = Factory::createNote(toChord(cr));
            note->setPitch(oldNote->pitch());
            note->setTpc1(oldNote->tpc1());
            note->setTpc2(oldNote->tpc2());
            cr->add(note);
        }
    } else {
        cr = Factory::createRest(this->dummy()->segment());
    }

    int actualNotes = an.numerator() / an.denominator();

    tuplet->setTrack(track);
    cr->setTuplet(tuplet);
    cr->setTrack(track);
    cr->setDurationType(tuplet->baseLen());
    cr->setTicks(tuplet->baseLen().fraction());

    undoAddCR(cr, measure, tick);

    Fraction ticks = cr->actualTicks();

    for (int i = 0; i < (actualNotes - 1); ++i) {
        tick += ticks;
        Rest* rest = Factory::createRest(this->dummy()->segment());
        rest->setTuplet(tuplet);
        rest->setTrack(track);
        rest->setDurationType(tuplet->baseLen());
        rest->setTicks(tuplet->baseLen().fraction());
        undoAddCR(rest, measure, tick);
    }
}

//---------------------------------------------------------
//   cmdExchangeVoice
//---------------------------------------------------------

void Score::cmdExchangeVoice(int s, int d)
{
    if (!selection().isRange()) {
        MScore::setError(MsError::NO_STAFF_SELECTED);
        return;
    }
    Fraction t1 = selection().tickStart();
    Fraction t2 = selection().tickEnd();

    Measure* m1 = tick2measure(t1);
    Measure* m2 = tick2measure(t2);

    if (selection().score()->excerpt()) {
        return;
    }

    if (t2 > m2->tick()) {
        m2 = m2->nextMeasure();
    }

    for (;;) {
        undoExchangeVoice(m1, s, d, selection().staffStart(), selection().staffEnd());
        m1 = m1->nextMeasure();
        if ((m1 == 0) || (m2 && (m1->tick() == m2->tick()))) {
            break;
        }
    }
}

//---------------------------------------------------------
//   cmdEnterRest
//---------------------------------------------------------

void Score::cmdEnterRest(const TDuration& d)
{
    if (m_is.track() == mu::nidx) {
        LOGD("cmdEnterRest: track invalid");
        return;
    }
    startCmd();
    enterRest(d);
    endCmd();
}

//---------------------------------------------------------
//   enterRest
//---------------------------------------------------------

void Score::enterRest(const TDuration& d, InputState* externalInputState)
{
    InputState& is = externalInputState ? (*externalInputState) : m_is;

    expandVoice(is.segment(), is.track());

    if (!is.cr()) {
        LOGD("cannot enter rest here");
        return;
    }

    const track_idx_t track = is.track();
    NoteVal nval;
    setNoteRest(is.segment(), track, nval,
                d.fraction(), DirectionV::AUTO, /* forceAccidental */ false, is.articulationIds(), /* rhythmic */ false,
                externalInputState);
    is.moveToNextInputPos();
    if (!is.noteEntryMode() || is.usingNoteEntryMethod(NoteEntryMethod::STEPTIME)) {
        is.setRest(false);  // continue with normal note entry
    }
}

//---------------------------------------------------------
//   removeChordRest
//    remove chord or rest
//    remove associated segment if empty
//    remove beam
//    remove slurs
//---------------------------------------------------------

void Score::removeChordRest(ChordRest* cr, bool clearSegment)
{
    std::set<Segment*> segments;
    for (EngravingObject* e : cr->linkList()) {
        undo(new RemoveElement(static_cast<EngravingItem*>(e)));
        if (clearSegment) {
            Segment* s = cr->segment();
            if (segments.find(s) == segments.end()) {
                segments.insert(s);
            }
        }
    }
    for (Segment* s : segments) {
        if (s->empty()) {
            undo(new RemoveElement(s));
        }
    }
    if (cr->beam()) {
        Beam* beam = cr->beam();
        if (beam->generated()) {
            beam->parentItem()->remove(beam);
            delete beam;
        } else {
            undoRemoveElement(beam);
        }
    }
    if (cr->isChord()) {
        for (Spanner* spanner : toChord(cr)->startingSpanners()) {
            if (spanner->isTrill()) {
                undo(new RemoveElement(spanner));
            }
        }
    }
}

//---------------------------------------------------------
//   cmdDeleteTuplet
//    remove tuplet and replace with rest
//---------------------------------------------------------

void Score::cmdDeleteTuplet(Tuplet* tuplet, bool replaceWithRest)
{
    std::vector<DurationElement*> elements = tuplet->elements();
    for (DurationElement* de : elements) {
        if (de->isChordRest()) {
            removeChordRest(toChordRest(de), true);
        } else {
            assert(de->isTuplet());
            cmdDeleteTuplet(toTuplet(de), false);
        }
    }
    if (replaceWithRest) {
        setRest(tuplet->tick(), tuplet->track(), tuplet->ticks(), true, tuplet->tuplet());
    }
}

//---------------------------------------------------------
//   nextInputPos
//---------------------------------------------------------

void Score::nextInputPos(ChordRest* cr, bool doSelect)
{
    ChordRest* ncr = nextChordRest(cr);
    if ((ncr == 0) && (m_is.track() % VOICES)) {
        Segment* s = tick2segment(cr->tick() + cr->actualTicks(), false, SegmentType::ChordRest);
        track_idx_t track = (cr->track() / VOICES) * VOICES;
        ncr = s ? toChordRest(s->element(track)) : 0;
    }
    if (ncr) {
        m_is.setSegment(ncr->segment());
        if (doSelect) {
            select(ncr, SelectType::SINGLE, 0);
        }
        setPlayPos(ncr->tick());
        for (MuseScoreView* v : m_viewer) {
            v->moveCursor();
        }
    }
}

//---------------------------------------------------------
//   insertMeasure
//    if inserting Measure, act on master score
//    (and add to all linked scores)
//    if inserting Box, add to local score only
//---------------------------------------------------------

MeasureBase* Score::insertMeasure(ElementType type, MeasureBase* beforeMeasure, const InsertMeasureOptions& options)
{
    MeasureBase* localInsertMeasureBase = nullptr;
    if (type == ElementType::MEASURE) {
        if (MeasureBase* masterInsertMeasure = masterScore()->insertMeasure(beforeMeasure, options)) {
            localInsertMeasureBase = tick2measureBase(masterInsertMeasure->tick());
        }
    } else {
        localInsertMeasureBase = insertBox(type, beforeMeasure, options);
    }

    return localInsertMeasureBase;
}

//---------------------------------------------------------
//   insertBox
//---------------------------------------------------------

MeasureBase* Score::insertBox(ElementType type, MeasureBase* beforeMeasure, const InsertMeasureOptions& options)
{
    const bool isFrame = type == ElementType::FBOX || type == ElementType::HBOX || type == ElementType::TBOX || type == ElementType::VBOX;

    if (!isFrame) {
        return nullptr;
    }

    Fraction tick;
    if (beforeMeasure) {
        if (beforeMeasure->isMeasure()) {
            Measure* m = toMeasure(beforeMeasure);
            if (m->isMMRest()) {
                beforeMeasure = m->mmRestFirst();
                deselectAll();
            }
            for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                if (m->isMeasureRepeatGroupWithPrevM(staffIdx)) {
                    MScore::setError(MsError::CANNOT_SPLIT_MEASURE_REPEAT);
                    return nullptr;
                }
            }
        } else {
            beforeMeasure = beforeMeasure->top(); // don't try to insert in front of nested frame
        }
        tick = beforeMeasure->tick();
    } else {
        tick = last() ? last()->endTick() : Fraction(0, 1);
    }

    MeasureBase* newMeasureBase = toMeasureBase(Factory::createItem(type, dummy()));
    newMeasureBase->setTick(tick);
    newMeasureBase->setNext(beforeMeasure);
    newMeasureBase->setPrev(beforeMeasure ? beforeMeasure->prev() : last());

    undo(new InsertMeasures(newMeasureBase, newMeasureBase));

    if (options.needDeselectAll) {
        deselectAll();
    }

    if (options.cloneBoxToAllParts) {
        newMeasureBase->manageExclusionFromParts(/*exclude =*/ false);
    }

    return newMeasureBase;
}

void Score::restoreInitialKeySigAndTimeSig()
{
    bool concertPitch = style().styleB(Sid::concertPitch);
    static constexpr Fraction startTick = Fraction(0, 1);

    Measure* firstMeas = firstMeasure();
    for (Staff* staff : m_staves) {
        Key concertKey = Key::C;
        Key transposedKey = concertKey;
        const Part* part = staff->part();
        const Instrument* instrument = part->instrument();
        int transpose = -instrument->transpose().chromatic;
        if (!concertPitch) {
            transposedKey = mu::engraving::transposeKey(transposedKey, transpose, part->preferSharpFlat());
        }

        Segment* keySegment = firstMeas->undoGetSegment(SegmentType::KeySig, startTick);

        KeySigEvent keySigEvent;
        keySigEvent.setConcertKey(concertKey);
        keySigEvent.setKey(transposedKey);
        // use atonal KS for drums
        if (instrument->useDrumset()) {
            keySigEvent.setCustom(true);
            keySigEvent.setMode(KeyMode::NONE);
        }

        KeySig* newKeySig = Factory::createKeySig(keySegment);
        newKeySig->setTrack(staff->idx() * VOICES);
        keySegment->add(newKeySig);
        newKeySig->setKeySigEvent(keySigEvent);
        undoAddElement(newKeySig);

        staff->setKey(Fraction(0, 1), keySigEvent);

        Segment* timeSegment = firstMeas->undoGetSegment(SegmentType::TimeSig, startTick);
        TimeSig* newTimeSig = Factory::createTimeSig(timeSegment);
        newTimeSig->setTrack(staff->idx() * VOICES);
        newTimeSig->setSig(Fraction(4, 4));
        timeSegment->add(newTimeSig);
        undoAddElement(newTimeSig);
    }

    firstMeas->setTimesig(Fraction(4, 4));
    firstMeas->setTicks(Fraction(4, 4));
}

//---------------------------------------------------------
//   checkSpanner
//    check if spanners are still valid as anchors may
//    have changed or be removed.
//    Spanners need to have a start anchor. Slurs need a
//    start and end anchor.
//---------------------------------------------------------

void Score::checkSpanner(const Fraction& startTick, const Fraction& endTick, bool removeOrphans)
{
    std::list<Spanner*> sl;       // spanners to remove
    std::list<Spanner*> sl2;      // spanners to shorten
    auto spanners = m_spanner.findOverlapping(startTick.ticks(), endTick.ticks());

    // DEBUG: check all spanner
    //        there may be spanners outside of score bc. some measures were deleted

    Fraction lastTick = lastMeasure()->endTick();

    for (auto i : m_spanner.map()) {
        Spanner* s = i.second;

        if (s->isSlur()) {
            Segment* seg = tick2segmentMM(s->tick(), false, SegmentType::ChordRest);
            if (!seg || !seg->element(s->track())) {
                sl.push_back(s);
            } else {
                seg = tick2segmentMM(s->tick2(), false, SegmentType::ChordRest);
                if (!seg || !seg->element(s->track2())) {
                    sl.push_back(s);
                }
            }
        } else {
            // remove spanner if there is no start element
            s->computeStartElement();
            if (!s->startElement()) {
                sl.push_back(s);
                LOGD("checkSpanner::remove (3)");
            } else {
                if (s->tick2() > lastTick) {
                    sl2.push_back(s);              //s->undoChangeProperty(Pid::SPANNER_TICKS, lastTick - s->tick());
                } else {
                    s->computeEndElement();
                }
            }
        }
    }
    if (removeOrphans) {
        for (auto s : sl) {       // actually remove scheduled spanners
            undo(new RemoveElement(s));
        }
    }
    for (auto s : sl2) {      // shorten spanners that extended past end of score
        undo(new ChangeProperty(s, Pid::SPANNER_TICKS, lastTick - s->tick()));
        s->computeEndElement();
    }
}

static constexpr SegmentType CR_TYPE = SegmentType::ChordRest;

//---------------------------------------------------------
//   checkTimeDelete
//---------------------------------------------------------

bool Score::checkTimeDelete(Segment* startSegment, Segment* endSegment)
{
    Measure* startMeasure = startSegment->measure();
    Measure* endMeasure;

    if (endSegment) {
        endMeasure = endSegment->prev() ? endSegment->measure() : endSegment->measure()->prevMeasure();
    } else {
        endMeasure = lastMeasure();
    }

    Fraction endTick = endSegment ? endSegment->tick() : endMeasure->endTick();
    Fraction tick = startSegment->tick();
    Fraction etick = (startMeasure == endMeasure ? endTick : startMeasure->endTick());

    // check for MeasureRepeat
    bool startsAtBeginningOfMeasure = (tick == startMeasure->tick());
    bool endsAtEndOfMeasure = (endTick == endMeasure->endTick());
    for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
        if ((startMeasure->isMeasureRepeatGroup(staffIdx) && !startsAtBeginningOfMeasure)
            || (endMeasure->isMeasureRepeatGroup(staffIdx) && !endsAtEndOfMeasure)
            || startMeasure->isMeasureRepeatGroupWithPrevM(staffIdx)
            || endMeasure->isMeasureRepeatGroupWithNextM(staffIdx)) {
            MScore::setError(MsError::CANNOT_REMOVE_TIME_MEASURE_REPEAT);
            return false;
        }
    }

    bool canDeleteTime = true;
    while (canDeleteTime) {
        for (size_t track = 0; canDeleteTime && track < m_staves.size() * VOICES; ++track) {
            if (startMeasure->hasVoice(track)) {
                Segment* fs = startMeasure->first(CR_TYPE);
                for (Segment* s = fs; s; s = s->next(CR_TYPE)) {
                    if (s->element(track)) {
                        ChordRest* cr       = toChordRest(s->element(track));
                        Tuplet* t           = cr->tuplet();
                        DurationElement* de = t ? toDurationElement(t) : toDurationElement(cr);
                        Fraction f          = de->tick() + de->actualTicks();
                        Fraction cetick     = f;
                        if (cetick <= tick) {
                            continue;
                        }
                        if (de->tick() >= etick) {
                            break;
                        }
                        if (t && (t->tick() < tick || cetick > etick)) {
                            canDeleteTime = false;
                            break;
                        }
                    }
                }
            }
        }
        if (startMeasure == endMeasure) {
            break;
        }
        startMeasure = endMeasure;
        tick  = startMeasure->tick();
        etick = endTick;
    }
    if (!canDeleteTime) {
        MScore::setError(MsError::CANNOT_REMOVE_TIME_TUPLET);
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   cmdTimeDelete
///    delete time by decreasing measure length if partial measures are selected
//---------------------------------------------------------

void Score::cmdTimeDelete()
{
    EngravingItem* e = selection().element();

    if (e && e->isBarLine() && toBarLine(e)->segment()->isEndBarLineType()) {
        Measure* m = toBarLine(e)->segment()->measure();
        cmdJoinMeasure(m, m->nextMeasure());
        return;
    }

    Segment* startSegment = nullptr;
    Segment* endSegment = nullptr;

    if (selection().state() != SelState::RANGE) {
        if (!e) {
            return;
        }

        ChordRest* cr = nullptr;
        if (e->isNote()) {
            cr = toNote(e)->chord();
        } else if (e->isChordRest()) {
            cr = toChordRest(e);
        } else {
            return;
        }

        startSegment     = cr->segment();
        Fraction endTick = startSegment->tick() + cr->actualTicks();
        endSegment       = tick2measure(endTick)->findSegment(CR_TYPE, endTick);
    } else {
        startSegment = selection().startSegment();
        endSegment   = selection().endSegment();
    }

    if (!isMaster() && masterScore()) {
        masterScore()->doTimeDelete(startSegment, endSegment);
    } else {
        doTimeDelete(startSegment, endSegment);
    }

    if (noteEntryMode()) {
        Segment* currentSegment = endSegment;
        ChordRest* cr = nullptr;
        if (!currentSegment && lastMeasureMM()) {
            // deleted to end of score - get last cr on current track
            currentSegment = lastMeasureMM()->last();
            if (currentSegment) {
                cr = currentSegment->nextChordRest(m_is.track(), true);
                if (cr) {
                    currentSegment = cr->segment();
                }
            }
        }
        if (!currentSegment) {
            // no cr found - append a new measure
            appendMeasures(1);
            currentSegment = lastMeasureMM()->first(SegmentType::ChordRest);
        }
        m_is.setSegment(currentSegment);
        cr = m_is.cr();
        if (cr) {
            if (cr->isChord()) {
                select(toChord(cr)->upNote(), SelectType::SINGLE);
            } else {
                select(cr, SelectType::SINGLE);
            }
        } else {
            // could not find cr to select,
            // may be that there is a "hole" in the current track
            deselectAll();
        }
    } else {
        deselectAll();
    }
}

void Score::doTimeDelete(Segment* startSegment, Segment* endSegment)
{
    if (!checkTimeDelete(startSegment, endSegment)) {
        return;
    }

    MeasureBase* mbStart = startSegment->measure();
    if (mbStart->isMeasure() && toMeasure(mbStart)->isMMRest()) {
        mbStart = toMeasure(mbStart)->mmRestFirst();
    }
    MeasureBase* mbEnd;

    if (endSegment) {
        mbEnd = endSegment->prev(SegmentType::ChordRest) ? endSegment->measure() : endSegment->measure()->prev();
    } else {
        mbEnd = lastMeasure();
    }

    Fraction endTick = endSegment ? endSegment->tick() : mbEnd->endTick();

    for (;;) {
        if (mbStart->tick() != startSegment->tick()) {
            Fraction tick = startSegment->tick();
            Fraction len;
            if (mbEnd == mbStart) {
                len = endTick - tick;
            } else {
                len = mbStart->endTick() - tick;
            }
            doTimeDeleteForMeasure(toMeasure(mbStart), startSegment, len);
            if (mbStart == mbEnd) {
                break;
            }
            mbStart = mbStart->next();
        }
        endTick = endSegment ? endSegment->tick() : mbEnd->endTick();
        if (mbEnd->endTick() != endTick) {
            Fraction len = endTick - mbEnd->tick();
            doTimeDeleteForMeasure(toMeasure(mbEnd), toMeasure(mbEnd)->first(), len);
            if (mbStart == mbEnd) {
                break;
            }
            mbEnd = mbEnd->prev();
        }
        deleteMeasures(mbStart, mbEnd);
        break;
    }
}

void Score::doTimeDeleteForMeasure(Measure* m, Segment* startSegment, const Fraction& f)
{
    if (f.isZero()) {
        return;
    }

    const Fraction tick  = startSegment->rtick();
    const Fraction len   = f;
    const Fraction etick = tick + len;

    Segment* fs = m->first(CR_TYPE);

    for (size_t track = 0; track < m_staves.size() * VOICES; ++track) {
        if (m->hasVoice(track)) {
            for (Segment* s = fs; s; s = s->next(CR_TYPE)) {
                if (s->element(track)) {
                    ChordRest* cr   = toChordRest(s->element(track));
                    Fraction cetick = cr->rtick() + cr->actualTicks();

                    if (cetick <= tick) {
                        continue;
                    }
                    if (s->rtick() >= etick) {
                        break;
                    }

                    if (cr->isFullMeasureRest()) {
                        if (cr->rtick() >= tick) {
                            // Move full-measure rest from the deleted area
                            undoRemoveElement(cr);
                            ChordRest* newCR = toChordRest(cr->clone());
                            newCR->setTicks(cr->ticks() - f);
                            undoAddCR(newCR, m, m->tick() + etick);
                        } else {
                            cr->undoChangeProperty(Pid::DURATION, cr->ticks() - f);
                        }
                    }
                    // inside deleted area
                    else if (s->rtick() >= tick && cetick <= etick) {
                        // inside
                        undoRemoveElement(cr);
                    } else if (s->rtick() >= tick) {
                        // running out
                        Fraction ff = cetick - etick;
                        undoRemoveElement(cr);
                        createCRSequence(ff, cr, tick + len);
                    } else if (s->rtick() < tick && cetick <= etick) {
                        // running in
                        Fraction f1 = tick - s->rtick();
                        changeCRlen(cr, f1, false);
                    } else {
                        // running in/out
                        Fraction f1 = cr->ticks() - f;
                        changeCRlen(cr, f1, false);
                    }
                }
            }
        }
    }
    const Fraction abstick = startSegment->tick();
    undoInsertTime(abstick, -len);

    std::vector<Segment*> emptySegments;

    for (Score* score : masterScore()->scoreList()) {
        Measure* localMeasure = score->tick2measure(abstick);

        undo(new InsertTime(score, abstick, -len));

        Fraction updatedTick = tick;
        for (Segment* s = localMeasure->first(CR_TYPE); s; s = s->next()) {
            if (s->rtick() < etick || s->rtick() == updatedTick) {
                continue;
            }

            s->undoChangeProperty(Pid::TICK, updatedTick);
            updatedTick += s->ticks();

            if (score->isMaster()) {
                if (s->isChordRestType() && !s->hasElements()) {
                    emptySegments.push_back(s);
                }
            }
        }

        undo(new ChangeMeasureLen(localMeasure, localMeasure->ticks() - f));
    }

    for (Segment* s : emptySegments) {
        if (Segment* ns = s->next(CR_TYPE)) {
            // Move annotations from the empty segment.
            // TODO: do we need to preserve annotations at all?
            // Maybe only some types (Tempo etc.)?
            const auto annotations = s->annotations(); // make a copy since we alter the list
            for (EngravingItem* a : annotations) {
                EngravingItem* a1 = a->clone();
                a1->setParent(ns);
                undoRemoveElement(a);
                undoAddElement(a1);
            }
        }
    }
}

//---------------------------------------------------------
//   cloneVoice
//---------------------------------------------------------

void Score::cloneVoice(track_idx_t strack, track_idx_t dtrack, Segment* sf, const Fraction& lTick, bool link, bool spanner)
{
    Fraction start = sf->tick();
    TieMap tieMap;
    TupletMap tupletMap;      // tuplets cannot cross measure boundaries
    Score* score = sf->score();
    Tremolo* tremolo = 0;

    for (Segment* oseg = sf; oseg && oseg->tick() < lTick; oseg = oseg->next1()) {
        Segment* ns = 0;            //create segment later, on demand
        Measure* dm = tick2measure(oseg->tick());

        EngravingItem* oe = oseg->element(strack);

        if (oe && !oe->generated() && oe->isChordRest()) {
            EngravingItem* ne;
            //does a linked clone to create just this element
            //otherwise element will be add in every linked stave
            if (link) {
                ne = oe->linkedClone();
            } else {
                ne = oe->clone();
            }
            ne->setTrack(dtrack);

            //Don't clone gaps to a first voice
            if (!(ne->track() % VOICES) && ne->isRest()) {
                toRest(ne)->setGap(false);
            }

            ne->setScore(this);
            ChordRest* ocr = toChordRest(oe);
            ChordRest* ncr = toChordRest(ne);

            //Handle beams
            if (ocr->beam() && !ocr->beam()->empty() && ocr->beam()->elements().front() == ocr) {
                Beam* nb = ocr->beam()->clone();
                nb->clear();
                nb->setTrack(dtrack);
                nb->setScore(this);
                nb->add(ncr);
                ncr->setBeam(nb);
            }

            // clone Tuplets
            Tuplet* ot = ocr->tuplet();
            if (ot) {
                ot->setTrack(strack);
                Tuplet* nt = tupletMap.findNew(ot);
                if (nt == 0) {
                    if (link) {
                        nt = toTuplet(ot->linkedClone());
                    } else {
                        nt = toTuplet(ot->clone());
                    }
                    nt->setTrack(dtrack);
                    nt->setParent(dm);
                    tupletMap.add(ot, nt);

                    Tuplet* nt1 = nt;
                    while (ot->tuplet()) {
                        Tuplet* nt2 = tupletMap.findNew(ot->tuplet());
                        if (nt2 == 0) {
                            if (link) {
                                nt2 = toTuplet(ot->tuplet()->linkedClone());
                            } else {
                                nt2 = toTuplet(ot->tuplet()->clone());
                            }
                            nt2->setTrack(dtrack);
                            nt2->setParent(dm);
                            tupletMap.add(ot->tuplet(), nt2);
                        }
                        nt2->add(nt1);
                        nt1->setTuplet(nt2);
                        ot = ot->tuplet();
                        nt1 = nt2;
                    }
                }
                nt->add(ncr);
                ncr->setTuplet(nt);
            }

            // clone additional settings
            if (oe->isChordRest()) {
                if (oe->isRest()) {
                    Rest* ore = toRest(ocr);
                    // If we would clone a full measure rest just don't clone this rest
                    if (ore->isFullMeasureRest() && (dtrack % VOICES)) {
                        continue;
                    }
                }

                if (oe->isChord()) {
                    Chord* och = toChord(ocr);
                    Chord* nch = toChord(ncr);

                    size_t n = och->notes().size();
                    for (size_t i = 0; i < n; ++i) {
                        Note* on = och->notes().at(i);
                        Note* nn = nch->notes().at(i);
                        staff_idx_t idx = track2staff(dtrack);
                        Fraction tick = oseg->tick();
                        Interval v = staff(idx) ? staff(idx)->transpose(tick) : Interval();
                        nn->setTpc1(on->tpc1());
                        if (v.isZero()) {
                            nn->setTpc2(on->tpc1());
                        } else {
                            v.flip();
                            nn->setTpc2(transposeTpc(nn->tpc1(), v, true));
                        }

                        if (on->tieFor()) {
                            Tie* tie;
                            if (link) {
                                tie = toTie(on->tieFor()->linkedClone());
                            } else {
                                tie = toTie(on->tieFor()->clone());
                            }
                            tie->setScore(this);
                            nn->setTieFor(tie);
                            tie->setStartNote(nn);
                            tie->setTrack(nn->track());
                            tie->setEndNote(nn);
                            tieMap.add(on->tieFor(), tie);
                        }
                        if (on->tieBack()) {
                            Tie* tie = tieMap.findNew(on->tieBack());
                            if (tie) {
                                nn->setTieBack(tie);
                                tie->setEndNote(nn);
                            } else {
                                LOGD("cloneVoices: cannot find tie");
                            }
                        }
                        // add back spanners (going back from end to start spanner element
                        // makes sure the 'other' spanner anchor element is already set up)
                        // 'on' is the old spanner end note and 'nn' is the new spanner end note
                        for (Spanner* oldSp : on->spannerBack()) {
                            Note* newStart = Spanner::startElementFromSpanner(oldSp, nn);
                            if (newStart) {
                                Spanner* newSp;
                                if (link) {
                                    newSp = toSpanner(oldSp->linkedClone());
                                } else {
                                    newSp = toSpanner(oldSp->clone());
                                }
                                newSp->setNoteSpan(newStart, nn);
                                addElement(newSp);
                            } else {
                                LOGD("cloneVoices: cannot find spanner start note");
                            }
                        }
                    }
                    // two note tremolo
                    if (och->tremolo() && och->tremolo()->twoNotes()) {
                        if (och == och->tremolo()->chord1()) {
                            if (tremolo) {
                                LOGD("unconnected two note tremolo");
                            }
                            if (link) {
                                tremolo = toTremolo(och->tremolo()->linkedClone());
                            } else {
                                tremolo = toTremolo(och->tremolo()->clone());
                            }
                            tremolo->setScore(nch->score());
                            tremolo->setParent(nch);
                            tremolo->setTrack(nch->track());
                            tremolo->setChords(nch, 0);
                            nch->setTremolo(tremolo);
                        } else if (och == och->tremolo()->chord2()) {
                            if (!tremolo) {
                                LOGD("first note for two note tremolo missing");
                            } else {
                                tremolo->setChords(tremolo->chord1(), nch);
                                nch->setTremolo(tremolo);
                            }
                        } else {
                            LOGD("inconsistent two note tremolo");
                        }
                    }
                }

                // Add element (link -> just in this measure)
                if (link) {
                    if (!ns) {
                        ns = dm->getSegment(oseg->segmentType(), oseg->tick());
                    }
                    ns->add(ne);
                } else {
                    undoAddCR(toChordRest(ne), dm, oseg->tick());
                }
            }
        }
        Segment* tst = dm->segments().firstCRSegment();
        if (strack % VOICES && !(dtrack % VOICES) && (!tst || (!tst->element(dtrack)))) {
            Rest* rest = Factory::createRest(this->dummy()->segment());
            rest->setTicks(dm->ticks());
            rest->setDurationType(DurationType::V_MEASURE);
            rest->setTrack(dtrack);
            if (link) {
                Segment* segment = dm->getSegment(SegmentType::ChordRest, dm->tick());
                segment->add(rest);
            } else {
                undoAddCR(toChordRest(rest), dm, dm->tick());
            }
        }
    }

    if (spanner) {
        // Find and add corresponding slurs
        auto spanners = score->spannerMap().findOverlapping(start.ticks(), lTick.ticks());
        for (auto i = spanners.begin(); i < spanners.end(); i++) {
            Spanner* sp      = i->value;
            Fraction spStart = sp->tick();
            track_idx_t track = sp->track();
            track_idx_t track2 = sp->track2();
            Fraction spEnd = spStart + sp->ticks();

            if (sp->isSlur() && (spStart >= start && spEnd < lTick)) {
                if (track == strack && track2 == strack) {
                    Spanner* ns = toSpanner(link ? sp->linkedClone() : sp->clone());

                    ns->setScore(this);
                    ns->setParent(0);
                    ns->setTrack(dtrack);
                    ns->setTrack2(dtrack);

                    // set start/end element for slur
                    ChordRest* cr1 = sp->startCR();
                    ChordRest* cr2 = sp->endCR();

                    ns->setStartElement(0);
                    ns->setEndElement(0);
                    if (cr1 && cr1->links()) {
                        for (EngravingObject* e : *cr1->links()) {
                            ChordRest* cr = toChordRest(e);
                            if (cr == cr1) {
                                continue;
                            }
                            if ((cr->score() == this) && (cr->tick() == ns->tick()) && cr->track() == dtrack) {
                                ns->setStartElement(cr);
                                break;
                            }
                        }
                    }
                    if (cr2 && cr2->links()) {
                        for (EngravingObject* e : *cr2->links()) {
                            ChordRest* cr = toChordRest(e);
                            if (cr == cr2) {
                                continue;
                            }
                            if ((cr->score() == this) && (cr->tick() == ns->tick2()) && cr->track() == dtrack) {
                                ns->setEndElement(cr);
                                break;
                            }
                        }
                    }
                    undo(new AddElement(ns));
                }
            }
        }
    }
}

//---------------------------------------------------------
//   undoPropertyChanged
//    return true if an property was actually changed
//---------------------------------------------------------

bool Score::undoPropertyChanged(EngravingItem* item, Pid propId, const PropertyValue& propValue, PropertyFlags propFlags)
{
    bool changed = false;

    const PropertyValue currentPropValue = item->getProperty(propId);
    const PropertyFlags currentPropFlags = item->propertyFlags(propId);

    if ((currentPropValue != propValue) || (currentPropFlags != propFlags)) {
        item->setPropertyFlags(propId, propFlags);
        undoStack()->push1(new ChangeProperty(item, propId, propValue, propFlags));
        changed = true;
    }

    const std::list<EngravingObject*> linkedItems = item->linkListForPropertyPropagation();

    for (EngravingObject* linkedItem : linkedItems) {
        if (linkedItem == item) {
            continue;
        }
        PropertyPropagation propertyPropagate = item->propertyPropagation(toEngravingItem(linkedItem), propId);
        switch (propertyPropagate) {
        case PropertyPropagation::PROPAGATE:
            if (linkedItem->getProperty(propId) != currentPropValue) {
                undoStack()->push(new ChangeProperty(linkedItem, propId, currentPropValue, propFlags), nullptr);
                changed = true;
            }
            break;
        case PropertyPropagation::UNLINK:
            item->unlinkPropertyFromMaster(propId);
            break;
        default:
            break;
        }
    }

    return changed;
}

void Score::undoPropertyChanged(EngravingObject* e, Pid t, const PropertyValue& st, PropertyFlags ps)
{
    if (e->getProperty(t) != st) {
        undoStack()->push1(new ChangeProperty(e, t, st, ps));
    }
}

//---------------------------------------------------------
//   undoChangeStyleVal
//---------------------------------------------------------

void Score::undoChangeStyleVal(Sid idx, const PropertyValue& v)
{
    undo(new ChangeStyleVal(this, idx, v));
}

//---------------------------------------------------------
//   undoChangePageNumberOffset
//---------------------------------------------------------

void Score::undoChangePageNumberOffset(int po)
{
    undo(new ChangePageNumberOffset(this, po));
}

void Score::undoChangeParent(EngravingItem* element, EngravingItem* parent, staff_idx_t staffIdx)
{
    if (!element || !parent) {
        return;
    }

    if (element->parentItem() == parent && staffIdx == element->staffIdx()) {
        return;
    }
    Staff* originStaff = element->staff();
    Staff* destStaff = staff(staffIdx);
    bool recreateItemNeeded = false;

    const std::list<EngravingObject*> links = element->linkList();
    for (EngravingObject* obj : links) {
        EngravingItem* item = toEngravingItem(obj);
        Score* linkedScore = item->score();
        Staff* linkedOrigin = originStaff->findLinkedInScore(linkedScore);
        Staff* linkedDest = destStaff->findLinkedInScore(linkedScore);

        if (!linkedScore) {
            continue;
        }
        if (item == element) {
            // Master score
            undo(new ChangeParent(element, parent, staffIdx));
        } else if (linkedOrigin && linkedDest) {
            // Part - origin and destination staves are in this part
            EngravingItem* linkedParent;
            if (parent->isSegment()) {
                // Get parent segment in linked score
                Segment* oldSeg = toSegment(parent);
                Measure* m = linkedScore->tick2measure(oldSeg->tick());
                linkedParent = m->tick2segment(oldSeg->tick(), oldSeg->segmentType());
            } else {
                linkedParent = parent->findLinkedInScore(linkedScore);
            }
            linkedScore->undo(new ChangeParent(item, linkedParent, linkedScore->staffIdx(linkedDest)));
        } else if (linkedOrigin && !linkedDest) {
            // Part - move is to a different staff
            // Remove original item
            linkedScore->undoRemoveElement(item, false);
            recreateItemNeeded = true;
        }
    }

    if (recreateItemNeeded) {
        // Need to create item in some parts
        const std::list<EngravingObject*> destStaffLinks = destStaff->linkList();
        for (EngravingObject* obj : destStaffLinks) {
            Staff* linkedDest = toStaff(obj);
            Score* linkedScore = linkedDest->score();
            if (linkedDest != destStaff && !element->findLinkedInScore(linkedScore)) {
                // Item should be in this score and isn't already
                EngravingItem* newItem = element->linkedClone();
                EngravingItem* linkedParent;

                if (parent->isSegment()) {
                    // Get parent segment in linked score
                    Segment* oldSeg = toSegment(parent);
                    Measure* m = linkedScore->tick2measure(oldSeg->tick());
                    linkedParent = m->tick2segment(oldSeg->tick(), oldSeg->segmentType());
                } else {
                    linkedParent = parent->findLinkedInScore(linkedScore);
                }

                newItem->setParent(linkedParent);
                newItem->setTrack(linkedScore->staffIdx(linkedDest) * VOICES);
                newItem->setOffset(PointF());
                linkedParent->undoAddElement(newItem, false);
            }
        }
    }
}

//---------------------------------------------------------
//   undoChangeElement
//---------------------------------------------------------

void Score::undoChangeElement(EngravingItem* oldElement, EngravingItem* newElement)
{
    if (!oldElement) {
        undoAddElement(newElement);
    } else {
        const std::list<EngravingObject*> links = oldElement->linkList();
        for (EngravingObject* obj : links) {
            EngravingItem* item = toEngravingItem(obj);
            if (item == oldElement) {
                undo(new ChangeElement(oldElement, newElement));
            } else {
                if (item->score()) {
                    item->score()->undo(new ChangeElement(item, newElement->linkedClone()));
                }
            }
        }
    }
}

//---------------------------------------------------------
//   undoChangePitch
//---------------------------------------------------------

void Score::undoChangePitch(Note* note, int pitch, int tpc1, int tpc2)
{
    for (EngravingObject* e : note->linkList()) {
        Note* n = toNote(e);
        undoStack()->push(new ChangePitch(n, pitch, tpc1, tpc2), 0);
    }
}

//---------------------------------------------------------
//   undoChangeFretting
//
//    To use with tablatures to force a specific note fretting;
//    Pitch, string and fret must be changed all together; otherwise,
//    if they are not consistent among themselves, the refretting algorithm may re-assign
//    fret and string numbers for (potentially) all the notes of all the chords of a segment.
//---------------------------------------------------------

void Score::undoChangeFretting(Note* note, int pitch, int string, int fret, int tpc1, int tpc2)
{
    const LinkedObjects* l = note->links();
    if (l) {
        for (EngravingObject* e : *l) {
            Note* n = toNote(e);
            undo(new ChangeFretting(n, pitch, string, fret, tpc1, tpc2));
        }
    } else {
        undo(new ChangeFretting(note, pitch, string, fret, tpc1, tpc2));
    }
}

//---------------------------------------------------------
//   undoChangeKeySig
//---------------------------------------------------------

void Score::undoChangeKeySig(Staff* ostaff, const Fraction& tick, KeySigEvent key)
{
    KeySig* lks = 0;
    bool needsUpdate = false;

    for (Staff* staff : ostaff->staffList()) {
        if (staff->isDrumStaff(tick)) {
            continue;
        }

        Score* score = staff->score();
        Measure* measure = score->tick2measure(tick);
        KeySigEvent currentKeySigEvent = staff->keySigEvent(tick);
        if (!measure) {
            LOGW("measure for tick %d not found!", tick.ticks());
            continue;
        }
        Segment* s   = measure->undoGetSegment(SegmentType::KeySig, tick);

        staff_idx_t staffIdx = staff->idx();
        track_idx_t track    = staffIdx * VOICES;
        KeySig* ks   = toKeySig(s->element(track));

        Interval interval = staff->part()->instrument(tick)->transpose();
        Interval oldStaffInterval = staff->transpose(tick);
        KeySigEvent nkey  = key;
        bool concertPitch = score->style().styleB(Sid::concertPitch);

        if (interval.chromatic && !concertPitch && !nkey.isAtonal()) {
            interval.flip();
            nkey.setKey(transposeKey(key.concertKey(), interval, staff->part()->preferSharpFlat()));
            interval.flip();
        }

        updateInstrumentChangeTranspositions(key, staff, tick);
        if (ks) {
            ks->undoChangeProperty(Pid::GENERATED, false);
            undo(new ChangeKeySig(ks, nkey, ks->showCourtesy()));
        } else {
            KeySig* nks = Factory::createKeySig(s);
            nks->setParent(s);
            nks->setTrack(track);
            nks->setKeySigEvent(nkey);
            undo(new AddElement(nks));
            if (lks) {
                undo(new Link(lks, nks));
            } else {
                lks = nks;
            }
        }
        if (interval != staff->transpose(tick) || interval != oldStaffInterval) {
            needsUpdate = true;
        }
    }
    if (needsUpdate) {
        Fraction tickEnd = Fraction::fromTicks(ostaff->keyList()->nextKeyTick(tick.ticks()));
        transpositionChanged(ostaff->part(), ostaff->transpose(tick), tick, tickEnd);
    }
}

void Score::updateInstrumentChangeTranspositions(KeySigEvent& key, Staff* staff, const Fraction& tick)
{
    if (!key.forInstrumentChange()) {
        KeyList* kl = staff->keyList();
        int nextTick = kl->nextKeyTick(tick.ticks());

        while (nextTick != -1) {
            KeySigEvent e = kl->key(nextTick);
            if (e.forInstrumentChange()) {
                Measure* m = tick2measure(Fraction::fromTicks(nextTick));
                Segment* s = m->tick2segment(Fraction::fromTicks(nextTick), SegmentType::KeySig);
                track_idx_t track = staff->idx() * VOICES;
                if (key.isAtonal() && !e.isAtonal()) {
                    e.setMode(KeyMode::NONE);
                    e.setConcertKey(Key::C);
                } else {
                    e.setMode(key.mode());
                    Interval transposeInterval = staff->part()->instrument(Fraction::fromTicks(nextTick))->transpose();
                    transposeInterval.flip();
                    Key ckey = key.concertKey();
                    Key nkey = transposeKey(ckey, transposeInterval, staff->part()->preferSharpFlat());
                    e.setConcertKey(ckey);
                    e.setKey(nkey);
                }
                KeySig* keySig = nullptr;
                EngravingItem* keySigElem = s ? s->element(track) : nullptr;
                if (keySigElem && keySigElem->isKeySig()) {
                    keySig = toKeySig(keySigElem);
                }
                if (keySig) {
                    undo(new ChangeKeySig(keySig, e, keySig->showCourtesy()));
                }
                nextTick = kl->nextKeyTick(nextTick);
            } else {
                nextTick = -1;
            }
        }
    }
}

//---------------------------------------------------------
//   undoChangeClef
//    change clef if e is a clef
//    else
//    create a clef before element e
//---------------------------------------------------------

void Score::undoChangeClef(Staff* ostaff, EngravingItem* e, ClefType ct, bool forInstrumentChange, Clef* clefToRelink)
{
    IF_ASSERT_FAILED(ostaff && e) {
        return;
    }

    bool moveClef = false;
    SegmentType st = SegmentType::Clef;
    if (e->isMeasure()) {
        if (toMeasure(e)->prevMeasure()) {
            moveClef = true;
        } else {
            st = SegmentType::HeaderClef;
        }
    } else if (e->isClef()) {
        Clef* clef = toClef(e);
        if (clef->segment()->isHeaderClefType()) {
            if (clef->measure()->prevMeasure()) {
                moveClef = true;
            } else {
                st = SegmentType::HeaderClef;
            }
        } else if (clef->rtick() == clef->measure()->ticks()) {
            moveClef = true;
        }
    } else if (e->rtick() == Fraction(0, 1)) {
        Measure* curMeasure = e->findMeasure();
        Measure* prevMeasure = curMeasure ? curMeasure->prevMeasure() : nullptr;
        if (prevMeasure && !prevMeasure->sectionBreak()) {
            moveClef = true;
        }
    }

    Clef* gclef = 0;
    Fraction tick = e->tick();
    Fraction rtick = e->rtick();
    bool isSmall = (st == SegmentType::Clef);
    for (Staff* staff : ostaff->staffList()) {
        if (clefToRelink && ostaff == staff) {
            continue;
        }

        Score* score     = staff->score();
        Measure* measure = score->tick2measure(tick);

        if (!measure) {
            LOGW("measure for tick %d not found!", tick.ticks());
            continue;
        }

        Segment* destSeg;
        Fraction rt;
        if (moveClef) {                // if at start of measure and there is a previous measure
            measure = measure->prevMeasure();
            rt      = measure->ticks();
        } else {
            rt = rtick;
        }
        destSeg = measure->undoGetSegmentR(st, rt);

        staff_idx_t staffIdx = staff->idx();
        track_idx_t track    = staffIdx * VOICES;
        Clef* clef   = toClef(destSeg->element(track));

        if (clef) {
            //
            // for transposing instruments, differentiate
            // clef type for concertPitch
            //
            Instrument* i = staff->part()->instrument(tick);
            ClefType cp, tp;
            if (i->transpose().isZero()) {
                cp = ct;
                tp = ct;
            } else {
                bool concertPitch = clef->concertPitch();
                if (concertPitch) {
                    cp = ct;
                    tp = clef->transposingClef();
                } else {
                    cp = clef->concertClef();
                    tp = ct;
                }
            }
            clef->setGenerated(false);
            score->undo(new ChangeClefType(clef, cp, tp));
            Clef* oClef = clef->otherClef();
            if (oClef && !(oClef->generated())) {
                score->undo(new ChangeClefType(oClef, cp, tp));
            }
            // change the clef in the mmRest if any
            if (measure->hasMMRest()) {
                Measure* mmMeasure = measure->mmRest();
                Segment* mmDestSeg = mmMeasure->findSegment(SegmentType::Clef, tick);
                if (mmDestSeg) {
                    Clef* mmClef = toClef(mmDestSeg->element(clef->track()));
                    if (mmClef) {
                        score->undo(new ChangeClefType(mmClef, cp, tp));
                    }
                }
            }
        } else {
            if (gclef) {
                clef = toClef(gclef->linkedClone());
                clef->setScore(score);
            } else {
                clef = Factory::createClef(score->dummy()->segment());
                gclef = clef;
            }
            clef->setTrack(track);
            clef->setClefType(ct);
            clef->setParent(destSeg);
            clef->setIsHeader(st == SegmentType::HeaderClef);
            score->undo(new AddElement(clef));

            renderer()->layoutItem(clef);
        }
        if (forInstrumentChange) {
            clef->setForInstrumentChange(true);
        }
        clef->setSmall(isSmall);

        if (clefToRelink) {
            LinkedObjects* links = clef->links();
            if (!links) {
                clef->linkTo(clefToRelink);
            } else if (!clef->isLinked(clefToRelink)) {
                clefToRelink->setLinks(links);
                links->push_back(clefToRelink);
            }
        }
    }
}

//---------------------------------------------------------
//   findLinkedChord
//---------------------------------------------------------

static Chord* findLinkedChord(Chord* c, Staff* nstaff)
{
    Excerpt* se = c->score()->excerpt();
    Excerpt* de = nstaff->score()->excerpt();
    track_idx_t strack = c->track();
    track_idx_t dtrack = nstaff->idx() * VOICES + c->voice();

    if (se) {
        strack = mu::key(se->tracksMapping(), strack);
    }

    if (de) {
        std::vector<track_idx_t> l = mu::values(de->tracksMapping(), strack);
        if (l.empty()) {
            // simply return the first linked chord whose staff is equal to nstaff
            for (EngravingObject* ee : c->linkList()) {
                Chord* ch = toChord(ee);
                if (ch->staff() == nstaff) {
                    return ch;
                }
            }
            return 0;
        }
        for (track_idx_t i : l) {
            if (nstaff->idx() * VOICES <= i && (nstaff->idx() + 1) * VOICES > i) {
                dtrack = i;
                break;
            }
        }
    }

    Segment* s = c->segment();
    if (!s) {
        s = c->segment();
    }
    Measure* nm = nstaff->score()->tick2measure(s->tick());
    Segment* ns = nm->findSegment(s->segmentType(), s->tick());
    EngravingItem* ne = ns->element(dtrack);
    if (!ne->isChord()) {
        return 0;
    }
    Chord* nc = toChord(ne);
    if (c->isGrace()) {
        Chord* pc = toChord(c->explicitParent());
        size_t index = 0;
        for (Chord* gc : pc->graceNotes()) {
            if (c == gc) {
                break;
            }
            index++;
        }
        if (index < nc->graceNotes().size()) {
            nc = nc->graceNotes().at(index);
        }
    }
    return nc;
}

//---------------------------------------------------------
//   undoChangeChordRestLen
//---------------------------------------------------------

void Score::undoChangeChordRestLen(ChordRest* cr, const TDuration& d)
{
    cr->undoChangeProperty(Pid::DURATION_TYPE_WITH_DOTS, d.typeWithDots());
    cr->undoChangeProperty(Pid::DURATION, d.fraction());
}

//---------------------------------------------------------
//   undoTransposeHarmony
//---------------------------------------------------------

void Score::undoTransposeHarmony(Harmony* h, int rootTpc, int baseTpc)
{
    undo(new TransposeHarmony(h, rootTpc, baseTpc));
}

//---------------------------------------------------------
//   undoExchangeVoice
//---------------------------------------------------------

void Score::undoExchangeVoice(Measure* measure, voice_idx_t srcVoice, voice_idx_t dstVoice, staff_idx_t srcStaff, staff_idx_t dstStaff)
{
    Fraction tick = measure->tick();

    for (staff_idx_t staffIdx = srcStaff; staffIdx < dstStaff; ++staffIdx) {
        std::set<Staff*> staffList;
        for (Staff* s : staff(staffIdx)->staffList()) {
            staffList.insert(s);
        }

        track_idx_t srcStaffTrack = staffIdx * VOICES;
        track_idx_t srcTrack = srcStaffTrack + srcVoice;
        track_idx_t dstTrack = srcStaffTrack + dstVoice;
        int trackDiff = static_cast<int>(dstVoice - srcVoice);

        //handle score and complete measures first
        undo(new ExchangeVoice(measure, srcTrack, dstTrack, staffIdx));

        for (Staff* st : staffList) {
            track_idx_t staffTrack = st->idx() * VOICES;
            Measure* measure2 = st->score()->tick2measure(tick);
            Excerpt* ex = st->score()->excerpt();

            if (ex) {
                const TracksMap& tracks = ex->tracksMapping();
                std::vector<track_idx_t> srcTrackList = mu::values(tracks, srcTrack);
                std::vector<track_idx_t> dstTrackList = mu::values(tracks, dstTrack);

                for (track_idx_t srcTrack2 : srcTrackList) {
                    // don't care about other linked staves
                    if (!(staffTrack <= srcTrack2) || !(srcTrack2 < staffTrack + VOICES)) {
                        continue;
                    }

                    track_idx_t tempTrack = srcTrack;
                    std::vector<track_idx_t> testTracks = mu::values(tracks, tempTrack + trackDiff);
                    bool hasVoice = false;
                    for (track_idx_t testTrack : testTracks) {
                        if (staffTrack <= testTrack && testTrack < staffTrack + VOICES && mu::contains(dstTrackList, testTrack)) {
                            hasVoice = true;
                            // voice is simply exchangeable now (deal directly)
                            undo(new ExchangeVoice(measure2, srcTrack2, testTrack, staffTrack / 4));
                        }
                    }

                    // only source voice is in this staff
                    if (!hasVoice) {
                        undo(new CloneVoice(measure->first(), measure2->endTick(), measure2->first(), tempTrack, srcTrack2,
                                            tempTrack + trackDiff));
                        mu::remove(srcTrackList, srcTrack2);
                    }
                }

                for (track_idx_t dstTrack2 : dstTrackList) {
                    // don't care about other linked staves
                    if (!(staffTrack <= dstTrack2) || !(dstTrack2 < staffTrack + VOICES)) {
                        continue;
                    }

                    track_idx_t tempTrack = dstTrack;
                    std::vector<track_idx_t> testTracks = mu::values(tracks, tempTrack - trackDiff);
                    bool hasVoice = false;
                    for (track_idx_t testTrack : testTracks) {
                        if (staffTrack <= testTrack && testTrack < staffTrack + VOICES && mu::contains(srcTrackList, testTrack)) {
                            hasVoice = true;
                        }
                    }

                    // only destination voice is in this staff
                    if (!hasVoice) {
                        undo(new CloneVoice(measure->first(), measure2->endTick(), measure2->first(), tempTrack, dstTrack2,
                                            tempTrack - trackDiff));
                        mu::remove(dstTrackList, dstTrack2);
                    }
                }
            } else if (srcStaffTrack != staffTrack) {
                // linked staff in same score (all voices present can be assumed)
                undo(new ExchangeVoice(measure2, staffTrack + srcVoice, staffTrack + dstVoice, st->idx()));
            }
        }
    }

    // make sure voice 0 is complete

    if (srcVoice == 0 || dstVoice == 0) {
        for (staff_idx_t staffIdx = srcStaff; staffIdx < dstStaff; ++staffIdx) {
            // check for complete timeline of voice 0
            Fraction ctick  = measure->tick();
            track_idx_t track = staffIdx * VOICES;
            for (Segment* s = measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                ChordRest* cr = toChordRest(s->element(track));
                if (cr == 0) {
                    continue;
                }
                if (cr->isRest()) {
                    Rest* r = toRest(cr);
                    if (r->isGap()) {
                        r->undoChangeProperty(Pid::GAP, false);
                    }
                }
                if (ctick < s->tick()) {
                    setRest(ctick, track, s->tick() - ctick, false, 0);             // fill gap
                }
                ctick = s->tick() + cr->actualTicks();
            }
            Fraction etick = measure->endTick();
            if (ctick < etick) {
                setRest(ctick, track, etick - ctick, false, 0);               // fill gap
            }
        }
    }
}

//---------------------------------------------------------
//   undoRemovePart
//---------------------------------------------------------

void Score::undoRemovePart(Part* part, size_t partIdx)
{
    undo(new RemovePart(part, partIdx));
}

//---------------------------------------------------------
//   undoInsertPart
//---------------------------------------------------------

void Score::undoInsertPart(Part* part, size_t targetPartIndex)
{
    undo(new InsertPart(part, targetPartIndex));
}

//---------------------------------------------------------
//   undoRemoveStaff
//    idx - index of staff in part
//---------------------------------------------------------

void Score::undoRemoveStaff(Staff* staff)
{
    const staff_idx_t staffIndex = staff->idx();
    assert(staffIndex != mu::nidx);

    std::vector<Spanner*> spannersToRemove;

    for (auto it = m_spanner.cbegin(); it != m_spanner.cend(); ++it) {
        Spanner* spanner = it->second;

        if (allowRemoveWhenRemovingStaves(spanner, staffIndex)) {
            spannersToRemove.push_back(spanner);
        }
    }

    for (Spanner* spanner : m_unmanagedSpanner) {
        if (allowRemoveWhenRemovingStaves(spanner, staffIndex)) {
            spannersToRemove.push_back(spanner);
        }
    }

    for (Spanner* spanner : spannersToRemove) {
        spanner->undoUnlink();
        undo(new RemoveElement(spanner));
    }

    //
    //    adjust measures
    //
    for (Measure* m = staff->score()->firstMeasure(); m; m = m->nextMeasure()) {
        m->cmdRemoveStaves(staffIndex, staffIndex + 1);
        if (m->hasMMRest()) {
            m->mmRest()->cmdRemoveStaves(staffIndex, staffIndex + 1);
        }
    }

    staff->undoUnlink();

    undo(new RemoveStaff(staff));
}

//---------------------------------------------------------
//   undoInsertStaff
//    idx - index of staff in part
//---------------------------------------------------------

void Score::undoInsertStaff(Staff* staff, staff_idx_t ridx, bool createRests)
{
    undo(new InsertStaff(staff, ridx));
    staff_idx_t idx = staffIdx(staff->part()) + ridx;
    for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
        m->cmdAddStaves(idx, idx + 1, createRests);
        if (m->hasMMRest()) {
            m->mmRest()->cmdAddStaves(idx, idx + 1, false);
        }
    }
    // when newly adding an instrument,
    // this was already set when we created the staff
    // we don't have any better info at this point
    // and it doesn't work to adjust bracket & barlines until all staves are added
    // TODO: adjust brackets only when appropriate
    //adjustBracketsIns(idx, idx+1);
}

static bool chordHasVisibleNote(const Chord* chord)
{
    for (const Note* note : chord->notes()) {
        if (note->visible()) {
            return true;
        }
    }

    return false;
}

static void undoChangeOrnamentVisibility(Ornament* ornament, bool visible);

static void undoChangeNoteVisibility(Note* note, bool visible)
{
    note->undoChangeProperty(Pid::VISIBLE, visible);

    for (NoteDot* dot : note->dots()) {
        dot->undoChangeProperty(Pid::VISIBLE, visible);
    }

    for (EngravingItem* e : note->el()) {
        e->undoChangeProperty(Pid::VISIBLE, visible);
    }

    if (note->accidental()) {
        note->accidental()->undoChangeProperty(Pid::VISIBLE, visible);
    }

    Chord* noteChord = note->chord();
    Beam* beam = noteChord->beam();
    std::vector<Chord*> chords;

    bool chordHasVisibleNote_ = visible || chordHasVisibleNote(noteChord);
    bool beamHasVisibleNote_ = chordHasVisibleNote_;

    if (beam) {
        for (EngravingItem* item : beam->elements()) {
            if (!item->isChord()) {
                continue;
            }

            Chord* chord = toChord(item);
            chords.push_back(chord);

            if (!beamHasVisibleNote_ && chord != noteChord) {
                beamHasVisibleNote_ = chordHasVisibleNote(chord);
            }
        }
    } else {
        chords.push_back(noteChord);
    }

    static const std::unordered_set<ElementType> IGNORED_TYPES {
        ElementType::NOTE,
        ElementType::LYRICS,
        ElementType::SLUR,
        ElementType::CHORD, // grace notes
        ElementType::LEDGER_LINE, // temporary objects, impossible to change visibility
    };

    for (Chord* chord : chords) {
        if (chord != noteChord) {
            chordHasVisibleNote_ = chordHasVisibleNote(chord);
        }

        for (EngravingObject* child : chord->scanChildren()) {
            ElementType type = child->type();

            if (mu::contains(IGNORED_TYPES, type)) {
                continue;
            }

            if (beam) {
                if (type == ElementType::STEM || type == ElementType::BEAM) {
                    child->undoChangeProperty(Pid::VISIBLE, beamHasVisibleNote_);
                    continue;
                }
            }
            if (child->isOrnament()) {
                undoChangeOrnamentVisibility(toOrnament(child), visible);
            } else {
                child->undoChangeProperty(Pid::VISIBLE, chordHasVisibleNote_);
            }
        }
    }
}

static void undoChangeRestVisibility(Rest* rest, bool visible)
{
    rest->undoChangeProperty(Pid::VISIBLE, visible);

    for (NoteDot* dot : rest->dotList()) {
        dot->undoChangeProperty(Pid::VISIBLE, visible);
    }
}

static void undoChangeOrnamentVisibility(Ornament* ornament, bool visible)
{
    ornament->undoChangeProperty(Pid::VISIBLE, visible);
    Chord* cueNoteChord = ornament->cueNoteChord();
    if (cueNoteChord) {
        undoChangeNoteVisibility(cueNoteChord->upNote(), visible);
    }
    if (ornament->accidentalAbove()) {
        ornament->accidentalAbove()->undoChangeProperty(Pid::VISIBLE, visible);
    }
    if (ornament->accidentalBelow()) {
        ornament->accidentalBelow()->undoChangeProperty(Pid::VISIBLE, visible);
    }
}

void Score::undoChangeVisible(EngravingItem* item, bool visible)
{
    if (item->isNote()) {
        undoChangeNoteVisibility(toNote(item), visible);
    } else if (item->isRest()) {
        undoChangeRestVisibility(toRest(item), visible);
    } else if (item->isOrnament()) {
        undoChangeOrnamentVisibility(toOrnament(item), visible);
    } else if (item->isTrillSegment()) {
        item->undoChangeProperty(Pid::VISIBLE, visible);
        Ornament* orn = toTrillSegment(item)->trill()->ornament();
        if (orn) {
            undoChangeOrnamentVisibility(orn, visible);
        }
    } else {
        item->undoChangeProperty(Pid::VISIBLE, visible);
    }
}

//---------------------------------------------------------
//   undoAddElement
//---------------------------------------------------------

void Score::undoAddElement(EngravingItem* element, bool addToLinkedStaves, bool ctrlModifier, EngravingItem* elementToRelink)
{
    Staff* ostaff = element->staff();
    track_idx_t strack = mu::nidx;
    if (ostaff) {
        strack = ostaff->idx() * VOICES + element->track() % VOICES;

        if (mu::engraving::Excerpt* excerpt = ostaff->score()->excerpt()) {
            const TracksMap& tracks = excerpt->tracksMapping();

            if (!tracks.empty() && strack != mu::nidx) {
                strack = mu::key(tracks, strack, mu::nidx);
            }
        }
    }

    ElementType et = element->type();

    //
    // some elements are replicated for all parts regardless of
    // linking:
    //

    bool isSystemLine = isSystemTextLine(element);

    if ((et == ElementType::REHEARSAL_MARK)
        || (et == ElementType::SYSTEM_TEXT)
        || (et == ElementType::TRIPLET_FEEL)
        || (et == ElementType::JUMP)
        || (et == ElementType::MARKER)
        || (et == ElementType::TEMPO_TEXT)
        || isSystemLine
        ) {
        std::list<Staff* > staffList;

        if (!addToLinkedStaves) {
            staffList.push_back(element->staff());
        } else if (ctrlModifier && isSystemLine) {
            staffList = ostaff->staffList();
            element->setSystemFlag(false);
        } else {
            for (Score* s : scoreList()) {
                staffList.push_back(s->staff(0)); // system objects always appear on the top staff
                for (Staff* staff : s->systemObjectStaves()) {
                    IF_ASSERT_FAILED(staff->idx() != mu::nidx) {
                        continue;
                    }

                    staffList.push_back(staff);
                }
            }
        }

        if (elementToRelink) {
            staffList.remove(ostaff);
        }

        bool originalAdded = false;
        for (Staff* staff : staffList) {
            if (!staff) {
                continue;
            }

            Score* score  = staff->score();
            staff_idx_t staffIdx = staff->idx();
            size_t ntrack = staffIdx * VOICES;
            EngravingItem* ne;

            if (ostaff && staff->score() == ostaff->score() && staff == staffList.front() && !originalAdded) {
                // add the element itself to the first system object staff in the score
                ne = element;
                originalAdded = true;
            } else {
                // add linked clones to the other staves
                ne = element->linkedClone();
                ne->setScore(score);
                ne->setSelected(false);
                ne->setTrack(staffIdx * VOICES + element->voice());
            }

            if (isSystemLine) {
                Spanner* nsp = toSpanner(ne);
                Spanner* sp = toSpanner(element);
                long long diff = sp->track2() - sp->track();
                nsp->setTrack2(nsp->track() + diff);
                nsp->computeStartElement();
                nsp->computeEndElement();
                undo(new AddElement(nsp));
            } else if (et == ElementType::MARKER || et == ElementType::JUMP) {
                Measure* om = toMeasure(element->explicitParent());
                Measure* m  = score->tick2measure(om->tick());
                ne->setTrack(ntrack);
                ne->setParent(m);
                undo(new AddElement(ne));
            } else if (et == ElementType::MEASURE_NUMBER) {
                toMeasure(element->explicitParent())->undoChangeProperty(Pid::MEASURE_NUMBER_MODE,
                                                                         static_cast<int>(MeasureNumberMode::SHOW));
            } else {
                Segment* segment  = toSegment(element->explicitParent());
                Fraction tick     = segment->tick();
                Measure* m        = score->tick2measure(tick);
                Segment* seg      = m->undoGetSegment(SegmentType::ChordRest, tick);
                ne->setTrack(ntrack);
                ne->setParent(seg);
                undo(new AddElement(ne));
            }
        }

        return;
    }

    if (et == ElementType::FINGERING
        || (et == ElementType::IMAGE && !element->explicitParent()->isSegment())
        || (et == ElementType::SYMBOL && !element->explicitParent()->isSegment())
        || et == ElementType::NOTE
        || et == ElementType::TEXT
        || et == ElementType::GLISSANDO
        || et == ElementType::GUITAR_BEND
        || et == ElementType::BEND
        || (et == ElementType::CHORD && toChord(element)->isGrace())
        ) {
        const EngravingItem* parent = element->parentItem();
        const LinkedObjects* links = parent ? parent->links() : nullptr;

        // don't link part name
        if (et == ElementType::TEXT) {
            Text* t = toText(element);
            if (t->textStyleType() == TextStyleType::INSTRUMENT_EXCERPT) {
                links = 0;
            }
        }
        if (links == 0 || !addToLinkedStaves) {
            undo(new AddElement(element));
            return;
        }
        for (EngravingObject* ee : *links) {
            EngravingItem* e = static_cast<EngravingItem*>(ee);
            EngravingItem* ne;
            if (e == parent) {
                ne = element;
            } else {
                if (element->isGlissando() || element->isGuitarBend()) {            // and other spanners with Anchor::NOTE
                    Note* newEnd = Spanner::endElementFromSpanner(toSpanner(element), e);
                    if (newEnd) {
                        ne = element->linkedClone();
                        toSpanner(ne)->setNoteSpan(toNote(e), newEnd);
                        if (element->isGuitarBend() && element->staffType()->isTabStaff()) {
                            toGuitarBend(ne)->endNote()->setVisible(true);
                        }
                    } else {                    //couldn't find suitable start note
                        continue;
                    }
                } else if (element->isFingering()) {
                    bool tabFingering = e->staff()->staffType(e->tick())->showTabFingering();
                    if (e->staff()->isTabStaff(e->tick()) && !tabFingering) {
                        continue;
                    }
                    ne = element->linkedClone();
                } else {
                    ne = element->linkedClone();
                }
            }
            ne->setScore(e->score());
            ne->setSelected(false);
            ne->setParent(e);
            undo(new AddElement(ne));
        }
        return;
    }

    if (et == ElementType::LAYOUT_BREAK) {
        LayoutBreak* lb = toLayoutBreak(element);
        if (lb->layoutBreakType() == LayoutBreakType::SECTION) {
            undo(new AddElement(lb));
            MeasureBase* m = lb->measure();
            if (m->isBox()) {
                // for frames, use linked frames
                LinkedObjects* links = m->links();
                if (links) {
                    for (EngravingObject* lo : *links) {
                        if (lo->isBox()) {
                            Box* box = toBox(lo);
                            Score* score = box->score();
                            if (score != lb->score()) {
                                EngravingItem* e = lb->linkedClone();
                                e->setScore(score);
                                e->setParent(box);
                                undo(new AddElement(e));
                            }
                        }
                    }
                    return;
                } else { // if thera are not linked frames, use previous measure
                    m = m->prevMeasure();
                    if (!m) {
                        return;
                    }
                }
            }
            for (Score* s : scoreList()) {
                if (s != lb->score()) {
                    EngravingItem* e = lb->linkedClone();
                    e->setScore(s);
                    Measure* nm = s->tick2measure(m->tick());
                    e->setParent(nm);
                    undo(new AddElement(e));
                }
            }
            return;
        }
    }

    if (ostaff == 0 || (
            et != ElementType::ARTICULATION
            && et != ElementType::ORNAMENT
            && et != ElementType::CHORDLINE
            && et != ElementType::LYRICS
            && et != ElementType::SLUR
            && et != ElementType::TIE
            && et != ElementType::NOTE
            && et != ElementType::INSTRUMENT_CHANGE
            && et != ElementType::HAIRPIN
            && et != ElementType::OTTAVA
            && et != ElementType::TRILL
            && et != ElementType::VIBRATO
            && et != ElementType::TEXTLINE
            && et != ElementType::PEDAL
            && et != ElementType::BREATH
            && et != ElementType::DYNAMIC
            && et != ElementType::EXPRESSION
            && et != ElementType::STAFF_TEXT
            && et != ElementType::SYSTEM_TEXT
            && et != ElementType::TRIPLET_FEEL
            && et != ElementType::PLAYTECH_ANNOTATION
            && et != ElementType::CAPO
            && et != ElementType::STRING_TUNINGS
            && et != ElementType::STICKING
            && et != ElementType::TREMOLO
            && et != ElementType::ARPEGGIO
            && et != ElementType::SYMBOL
            && et != ElementType::IMAGE
            && et != ElementType::TREMOLOBAR
            && et != ElementType::FRET_DIAGRAM
            && et != ElementType::FERMATA
            && et != ElementType::HARMONY
            && et != ElementType::HARP_DIAGRAM
            && et != ElementType::FIGURED_BASS)
        ) {
        undo(new AddElement(element));
        return;
    }

    // For linked staves the length of staffList is always > 1 since the list contains the staff itself too!
    const bool linked = ostaff->staffList().size() > 1;

    std::list<Staff*> staves;
    if (addToLinkedStaves) {
        staves = ostaff->staffList();
    } else {
        staves.push_back(ostaff);
    }

    if (elementToRelink) {
        staves.remove(ostaff);
    }

    for (Staff* staff : staves) {
        Score* score = staff->score();
        staff_idx_t staffIdx = staff->idx();

        std::vector<track_idx_t> tr;
        if (!staff->score()->excerpt()) {
            // On masterScore.
            track_idx_t track = staff->idx() * VOICES + (strack % VOICES);
            tr.push_back(track);
        } else {
            const TracksMap& mapping = staff->score()->excerpt()->tracksMapping();
            if (mapping.empty()) {
                // This can happen during reading the score and there is
                // no Tracklist tag specified.
                // TODO solve this in read302.cpp.
                tr.push_back(strack);
            } else {
                for (track_idx_t track : mu::values(mapping, strack)) {
                    // linkedPart : linked staves within same part/instrument.
                    // linkedScore: linked staves over different scores via excerpts.
                    const bool linkedPart  = linked && (staff != ostaff) && (staff->score() == ostaff->score());
                    const bool linkedScore = linked && (staff != ostaff) && (staff->score() != ostaff->score());
                    if (linkedPart && !linkedScore) {
                        tr.push_back(staff->idx() * VOICES + mu::value(mapping, track) % VOICES);
                    } else if (!linkedPart && linkedScore) {
                        if ((track >> 2) != staffIdx) {
                            track += (staffIdx - (track >> 2)) * VOICES;
                        }
                        tr.push_back(track);
                    } else {
                        tr.push_back(track);
                    }
                }
            }
        }

        // Some elements in voice 1 of a staff should be copied to every track which has a linked voice in this staff

        if (tr.empty() && (element->isSymbol()
                           || element->isImage()
                           || element->isTremoloBar()
                           || element->isDynamic()
                           || element->isExpression()
                           || element->isStaffText()
                           || element->isPlayTechAnnotation()
                           || element->isCapo()
                           || element->isStringTunings()
                           || element->isSticking()
                           || element->isFretDiagram()
                           || element->isHarmony()
                           || element->isHairpin()
                           || element->isOttava()
                           || element->isTrill()
                           || element->isSlur()
                           || element->isVibrato()
                           || element->isTextLine()
                           || element->isPedal()
                           || element->isLyrics())) {
            tr.push_back(staffIdx * VOICES);
        }

        for (track_idx_t ntrack : tr) {
            EngravingItem* ne;
            if (staff == ostaff) {
                ne = element;
            } else {
                if (staff->rstaff() != ostaff->rstaff()) {
                    switch (element->type()) {
                    // exclude certain element types except on corresponding staff in part
                    // this should be same list excluded in cloneStaff()
                    case ElementType::STAFF_TEXT:
                    case ElementType::SYSTEM_TEXT:
                    case ElementType::TRIPLET_FEEL:
                    case ElementType::PLAYTECH_ANNOTATION:
                    case ElementType::CAPO:
                    case ElementType::STRING_TUNINGS:
                    case ElementType::FRET_DIAGRAM:
                    case ElementType::HARMONY:
                    case ElementType::FIGURED_BASS:
                    case ElementType::DYNAMIC:
                    case ElementType::EXPRESSION:
                    case ElementType::LYRICS:                   // not normally segment-attached
                        continue;
                    default:
                        break;
                    }
                }
                ne = element->linkedClone();
                ne->setScore(score);
                ne->setSelected(false);
                ne->setTrack(staffIdx * VOICES + element->voice());

                if (ne->isFretDiagram()) {
                    FretDiagram* fd = toFretDiagram(ne);
                    Harmony* fdHarmony = fd->harmony();
                    if (fdHarmony) {
                        fdHarmony->setScore(score);
                        fdHarmony->setSelected(false);
                        fdHarmony->setTrack(staffIdx * VOICES + element->voice());
                    }
                }
            }

            if (element->isArticulationFamily()) {
                Articulation* a  = toArticulation(element);
                Segment* segment;
                SegmentType st;
                Measure* m;
                Fraction tick;
                if (a->explicitParent()->isChordRest()) {
                    ChordRest* cr = a->chordRest();
                    segment       = cr->segment();
                    st            = SegmentType::ChordRest;
                    tick          = segment->tick();
                    m             = score->tick2measure(tick);
                } else {
                    segment  = toSegment(a->explicitParent()->explicitParent());
                    st       = SegmentType::EndBarLine;
                    tick     = segment->tick();
                    m        = score->tick2measure(tick);
                    if (m->tick() == tick) {
                        m = m->prevMeasure();
                    }
                }
                Segment* seg = m->findSegment(st, tick);
                if (seg == 0) {
                    LOGW("undoAddSegment: segment not found");
                    break;
                }
                Articulation* na = toArticulation(ne);
                na->setTrack(ntrack);
                if (a->explicitParent()->isChordRest()) {
                    ChordRest* cr = a->chordRest();
                    ChordRest* ncr;
                    if (cr->isGrace()) {
                        ncr = findLinkedChord(toChord(cr), score->staff(staffIdx));
                    } else {
                        ncr = toChordRest(seg->element(ntrack));
                    }
                    na->setParent(ncr);
                } else {
                    BarLine* bl = toBarLine(seg->element(ntrack));
                    na->setParent(bl);
                }
                undo(new AddElement(na));
            } else if (element->isChordLine() || element->isLyrics()) {
                ChordRest* cr    = toChordRest(element->explicitParent());
                Segment* segment = cr->segment();
                Fraction tick    = segment->tick();
                Measure* m       = score->tick2measure(tick);
                Segment* seg     = m->findSegment(SegmentType::ChordRest, tick);
                if (seg == 0) {
                    LOGW("undoAddSegment: segment not found");
                    break;
                }
                ne->setTrack(ntrack);
                ChordRest* ncr = toChordRest(seg->element(ntrack));
                ne->setParent(ncr);
                if (element->isChordLine()) {
                    ChordLine* oldChordLine = toChordLine(element);
                    ChordLine* newChordLine = toChordLine(ne);
                    // Chordline also needs to know the new note
                    Note* newNote = toChord(ncr)->findNote(oldChordLine->note()->pitch());
                    newChordLine->setNote(newNote);
                }
                undo(new AddElement(ne));
            }
            //
            // elements with Segment as parent
            //
            else if (element->isSymbol()
                     || element->isImage()
                     || element->isTremoloBar()
                     || element->isDynamic()
                     || element->isExpression()
                     || element->isStaffText()
                     || element->isPlayTechAnnotation()
                     || element->isCapo()
                     || element->isStringTunings()
                     || element->isSticking()
                     || element->isFretDiagram()
                     || element->isFermata()
                     || element->isHarmony()
                     || element->isHarpPedalDiagram()
                     || element->isFiguredBass()) {
                Segment* segment
                    = element->explicitParent()->isFretDiagram() ? toSegment(element->explicitParent()->explicitParent()) : toSegment(
                          element->explicitParent());
                Fraction tick    = segment->tick();
                Measure* m       = score->tick2measure(tick);
                if ((segment->segmentType() == SegmentType::EndBarLine) && (m->tick() == tick)) {
                    m = m->prevMeasure();
                }
                Segment* seg     = m->undoGetSegment(segment->segmentType(), tick);
                ne->setTrack(ntrack);
                ne->setParent(seg);

                // make harmony child of fret diagram if possible
                if (ne->isHarmony()) {
                    for (EngravingItem* segel : segment->annotations()) {
                        if (segel && segel->isFretDiagram() && segel->track() == ntrack) {
                            segel->add(ne);
                            break;
                        }
                    }
                } else if (ne->isFretDiagram()) {
                    // update track of child harmony
                    FretDiagram* fd = toFretDiagram(ne);
                    if (fd->harmony()) {
                        fd->harmony()->setTrack(ntrack);
                    }
                } else if (ne->isStringTunings()) {
                    StringTunings* stringTunings = toStringTunings(ne);
                    if (stringTunings->stringData()->isNull()) {
                        const StringData* stringData = stringTunings->part()->stringData(tick, staff->idx());
                        int frets = stringData->frets();
                        std::vector<mu::engraving::instrString> stringList = stringData->stringList();

                        stringTunings->setStringData(StringData(frets, stringList));
                    }
                }

                undo(new AddElement(ne));
                // transpose harmony if necessary
                if (element->isHarmony() && ne != element) {
                    Harmony* h = toHarmony(ne);
                    if (score->style().styleB(Sid::concertPitch) != element->style().styleB(Sid::concertPitch)) {
                        Staff* staffDest = h->staff();
                        Interval interval = staffDest->transpose(tick);
                        if (!interval.isZero()) {
                            if (!score->style().styleB(Sid::concertPitch)) {
                                interval.flip();
                            }
                            int rootTpc = transposeTpc(h->rootTpc(), interval, true);
                            int baseTpc = transposeTpc(h->baseTpc(), interval, true);
                            score->undoTransposeHarmony(h, rootTpc, baseTpc);
                        }
                    }
                }
            } else if (element->isSlur()
                       || element->isHairpin()
                       || element->isOttava()
                       || element->isTrill()
                       || element->isVibrato()
                       || element->isTextLine()
                       || element->isPedal()) {
                Spanner* sp   = toSpanner(element);
                Spanner* nsp  = toSpanner(ne);
                staff_idx_t staffIdx1 = sp->track() / VOICES;
                track_idx_t tr2 = sp->effectiveTrack2();
                staff_idx_t staffIdx2 = tr2 / VOICES;
                int diff = static_cast<int>(staffIdx2 - staffIdx1);
                nsp->setTrack2((staffIdx + diff) * VOICES + (tr2 % VOICES));
                nsp->setTrack(ntrack);

                // determine start/end element for slurs
                // this is only necessary if start/end element is
                //   a grace note, otherwise the element can be set to zero
                //   and will later be calculated from tick/track values
                //
                if (element->isSlur() && sp != nsp) {
                    if (sp->startElement()) {
                        std::list<EngravingObject*> sel = sp->startElement()->linkList();
                        for (EngravingObject* ee : sel) {
                            EngravingItem* e = static_cast<EngravingItem*>(ee);
                            if (e->score() == nsp->score() && e->track() == nsp->track()) {
                                nsp->setStartElement(e);
                                break;
                            }
                        }
                    }
                    if (sp->endElement()) {
                        std::list<EngravingObject*> eel = sp->endElement()->linkList();
                        for (EngravingObject* ee : eel) {
                            EngravingItem* e = static_cast<EngravingItem*>(ee);
                            if (e->score() == nsp->score() && e->track() == nsp->track2()) {
                                nsp->setEndElement(e);
                                break;
                            }
                        }
                    }
                }
                undo(new AddElement(nsp));
            } else if (et == ElementType::GLISSANDO || et == ElementType::GUITAR_BEND) {
                undo(new AddElement(toSpanner(ne)));
            } else if (element->isTremolo() && toTremolo(element)->twoNotes()) {
                Tremolo* tremolo = toTremolo(element);
                ChordRest* cr1 = toChordRest(tremolo->chord1());
                ChordRest* cr2 = toChordRest(tremolo->chord2());
                Segment* s1    = cr1->segment();
                Segment* s2    = cr2->segment();
                Measure* m1    = s1->measure();
                Measure* m2    = s2->measure();
                Measure* nm1   = score->tick2measure(m1->tick());
                Measure* nm2   = score->tick2measure(m2->tick());
                Segment* ns1   = nm1->findSegment(s1->segmentType(), s1->tick());
                Segment* ns2   = nm2->findSegment(s2->segmentType(), s2->tick());
                Chord* c1      = toChord(ns1->element(staffIdx * VOICES + cr1->voice()));
                Chord* c2      = toChord(ns2->element(staffIdx * VOICES + cr2->voice()));
                Tremolo* ntremolo = toTremolo(ne);
                ntremolo->setChords(c1, c2);
                ntremolo->setParent(c1);
                undo(new AddElement(ntremolo));
            } else if (element->isTremolo() && !toTremolo(element)->twoNotes()) {
                Chord* cr = toChord(element->explicitParent());
                Chord* c1 = findLinkedChord(cr, score->staff(staffIdx));
                ne->setParent(c1);
                undo(new AddElement(ne));
            } else if (element->isArpeggio()) {
                ChordRest* cr = toChordRest(element->explicitParent());
                Segment* s    = cr->segment();
                Measure* m    = s->measure();
                Measure* nm   = score->tick2measure(m->tick());
                Segment* ns   = nm->findSegment(s->segmentType(), s->tick());
                Chord* c1     = toChord(ns->element(staffIdx * VOICES + cr->voice()));
                ne->setParent(c1);
                undo(new AddElement(ne));
            } else if (element->isTie()) {
                Tie* tie       = toTie(element);
                Note* n1       = tie->startNote();
                Note* n2       = tie->endNote();
                Chord* cr1     = n1->chord();
                Chord* cr2     = n2 ? n2->chord() : 0;

                // find corresponding notes in linked staff
                // accounting for grace notes and cross-staff notation
                int sm = 0;
                if (cr1->staffIdx() != cr2->staffIdx()) {
                    sm = static_cast<int>(cr2->staffIdx() - cr1->staffIdx());
                }
                Chord* c1 = findLinkedChord(cr1, score->staff(staffIdx));
                Chord* c2 = findLinkedChord(cr2, score->staff(staffIdx + sm));
                Note* nn1 = c1->findNote(n1->pitch(), n1->unisonIndex());
                Note* nn2 = c2 ? c2->findNote(n2->pitch(), n2->unisonIndex()) : 0;

                // create tie
                Tie* ntie = toTie(ne);
                ntie->eraseSpannerSegments();
                ntie->setTrack(c1->track());
                ntie->setStartNote(nn1);
                ntie->setEndNote(nn2);
                undo(new AddElement(ntie));
            } else if (element->isInstrumentChange()) {
                InstrumentChange* is = toInstrumentChange(element);
                Segment* s1    = is->segment();
                Measure* m1    = s1->measure();
                Measure* nm1   = score->tick2measure(m1->tick());
                Segment* ns1   = nm1->findSegment(s1->segmentType(), s1->tick());
                InstrumentChange* nis = toInstrumentChange(ne);
                nis->setParent(ns1);
                Fraction tickStart = nis->segment()->tick();
                Part* part = nis->part();
                Interval oldV = nis->staff()->transpose(tickStart);
                // ws: instrument should not be changed here
                if (is->instrument()->channel().empty() || is->instrument()->channel(0)->program() == -1) {
                    nis->setInstrument(*staff->part()->instrument(s1->tick()));
                } else if (nis != is) {
                    nis->setInstrument(*is->instrument());
                }
                undo(new AddElement(nis));
                // transpose root score; parts will follow
                if (score->isMaster() && nis->staff()->transpose(tickStart) != oldV) {
                    auto i = part->instruments().upper_bound(tickStart.ticks());
                    Fraction tickEnd = i == part->instruments().end() ? Fraction(-1, 1) : Fraction::fromTicks(i->first);
                    transpositionChanged(part, oldV, tickStart, tickEnd);
                }
            } else if (element->isBreath()) {
                Breath* breath   = toBreath(element);
                Fraction tick    = breath->segment()->tick();
                Measure* m       = score->tick2measure(tick);
                // breath appears before barline
                if (m->tick() == tick) {
                    m = m->prevMeasure();
                }
                Segment* seg     = m->undoGetSegment(SegmentType::Breath, tick);
                Breath* nbreath  = toBreath(ne);
                nbreath->setScore(score);
                nbreath->setTrack(ntrack);
                nbreath->setParent(seg);
                undo(new AddElement(nbreath));
            } else {
                LOGW("undoAddElement: unhandled: <%s>", element->typeName());
            }
            ne->styleChanged();

            if (elementToRelink) {
                LinkedObjects* links = ne->links();
                if (!links) {
                    ne->linkTo(elementToRelink);
                } else {
                    elementToRelink->setLinks(links);
                    links->push_back(elementToRelink);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   undoAddCR
//---------------------------------------------------------

void Score::undoAddCR(ChordRest* cr, Measure* measure, const Fraction& tick)
{
    assert(!cr->isChord() || !(toChord(cr)->notes()).empty());
    if (!cr->lyrics().empty()) {
        // Add chordrest and lyrics separately for correct
        // handling of adding lyrics to linked staves.
        std::vector<Lyrics*> lyrics;
        std::swap(lyrics, cr->lyrics());
        undoAddCR(cr, measure, tick);
        for (Lyrics* l : lyrics) {
            undoAddElement(l);
        }
        return;
    }

    Staff* ostaff = cr->staff();
    track_idx_t strack = ostaff->idx() * VOICES + cr->voice();

    if (mu::engraving::Excerpt* excerpt = ostaff->score()->excerpt()) {
        const TracksMap& tracks = excerpt->tracksMapping();
        if (!tracks.empty()) {
            strack = mu::key(tracks, strack, mu::nidx);
        }
    }

    SegmentType segmentType = SegmentType::ChordRest;

    // For linked staves the length of staffList is always > 1 since the list contains the staff itself too!
    const bool linked = ostaff->staffList().size() > 1;

    for (const Staff* staff : ostaff->staffList()) {
        std::list<track_idx_t> tracks;
        if (!staff->score()->excerpt()) {
            // On masterScore.
            track_idx_t track = staff->idx() * VOICES + (strack % VOICES);
            tracks.push_back(track);
        } else {
            const TracksMap& mapping = staff->score()->excerpt()->tracksMapping();
            if (mapping.empty()) {
                // This can happen during reading the score and there is
                // no Tracklist tag specified.
                // TODO solve this in read302.cpp.
                tracks.push_back(strack);
            } else {
                // linkedPart : linked staves within same part/instrument.
                // linkedScore: linked staves over different scores via excerpts.
                const bool linkedPart  = linked && (staff != ostaff) && (staff->score() == ostaff->score());
                const bool linkedScore = linked && (staff != ostaff) && (staff->score() != ostaff->score());
                for (track_idx_t track : mu::values(mapping, strack)) {
                    if (linkedPart && !linkedScore) {
                        tracks.push_back(staff->idx() * VOICES + mu::value(mapping, track) % VOICES);
                    } else if (!linkedPart && linkedScore) {
                        if ((track >> 2) != staff->idx()) {
                            track += (staff->idx() - (track >> 2)) * VOICES;
                        }
                        tracks.push_back(track);
                    } else {
                        tracks.push_back(track);
                    }
                }
            }
        }

        for (track_idx_t ntrack : tracks) {
            if (ntrack < staff->part()->startTrack() || ntrack >= staff->part()->endTrack()) {
                continue;
            }

            Score* score = staff->score();
            Measure* m   = (score == this) ? measure : score->tick2measure(tick);
            if (!m) {
                LOGD("measure not found");
                break;
            }
            Segment* seg = m->undoGetSegment(segmentType, tick);

            assert(seg->segmentType() == segmentType);

            ChordRest* newcr = (staff == ostaff) ? cr : toChordRest(cr->linkedClone());
            newcr->setScore(score);

            newcr->setTrack(ntrack);
            newcr->setParent(seg);

#ifndef QT_NO_DEBUG
            if (newcr->isChord()) {
                Chord* chord = toChord(newcr);
                // setTpcFromPitch needs to know the note tick position
                for (Note* note : chord->notes()) {
                    // if (note->tpc() == Tpc::TPC_INVALID)
                    //      note->setTpcFromPitch();
                    assert(note->tpc() != Tpc::TPC_INVALID);
                }
            }
#endif
            // Climb up the (possibly nested) tuplets from this chordRest
            // Make sure all tuplets are cloned and correctly nested
            DurationElement* elementBelow = cr;
            Tuplet* tupletAbove = elementBelow->tuplet();
            while (tupletAbove) {
                DurationElement* linkedElementBelow = (DurationElement*)elementBelow->findLinkedInStaff(staff);
                if (!linkedElementBelow) { // shouldn't happen
                    break;
                }
                Tuplet* linkedTuplet = (Tuplet*)tupletAbove->findLinkedInStaff(staff);
                if (!linkedTuplet) {
                    linkedTuplet = toTuplet(tupletAbove->linkedClone());
                    linkedTuplet->setScore(score);
                    linkedTuplet->setTrack(newcr->track());
                    linkedTuplet->setParent(m);
                }
                linkedElementBelow->setTuplet(linkedTuplet);

                elementBelow = tupletAbove;
                tupletAbove = tupletAbove->tuplet();
            }

            if (newcr->isRest() && (toRest(newcr)->isGap()) && !(toRest(newcr)->track() % VOICES)) {
                toRest(newcr)->setGap(false);
            }

            undo(new AddElement(newcr));
        }
    }
}

//---------------------------------------------------------
//   undoRemoveElement
//---------------------------------------------------------

void Score::undoRemoveElement(EngravingItem* element, bool removeLinked)
{
    if (!element) {
        return;
    }
    std::list<Segment*> segments;
    for (EngravingObject* ee : element->linkList()) {
        EngravingItem* e = static_cast<EngravingItem*>(ee);
        if (e == element || removeLinked) {
            undo(new RemoveElement(e));

            if (e->explicitParent() && (e->explicitParent()->isSegment())) {
                Segment* s = toSegment(e->explicitParent());
                if (!mu::contains(segments, s)) {
                    segments.push_back(s);
                }
            }
            if (e->explicitParent() && e->explicitParent()->isSystem()) {
                e->setParent(0);   // systems will be regenerated upon redo, so detach
            }
        }
    }

    if (element->isMeasureRepeat()) {
        const MeasureRepeat* repeat = toMeasureRepeat(element);
        Measure* measure = repeat->firstMeasureOfGroup();
        size_t staffIdx = repeat->staffIdx();

        for (int i = 0; i < repeat->numMeasures(); ++i) {
            undoChangeMeasureRepeatCount(measure, 0, staffIdx);
            measure = measure->nextMeasure();
        }
    }

    for (Segment* s : segments) {
        if (s->empty()) {
            if (s->header() || s->trailer()) {        // probably more segment types (system header)
                s->setEnabled(false);
            } else {
                undo(new RemoveElement(s));
            }
        }
    }
}

//---------------------------------------------------------
//   undoChangeSpannerElements
//---------------------------------------------------------

void Score::undoChangeSpannerElements(Spanner* spanner, EngravingItem* startElement, EngravingItem* endElement)
{
    EngravingItem* oldStartElement = spanner->startElement();
    EngravingItem* oldEndElement = spanner->endElement();
    track_idx_t startDeltaTrack = startElement && oldStartElement ? startElement->track() - oldStartElement->track() : 0;
    track_idx_t endDeltaTrack = endElement && oldEndElement ? endElement->track() - oldEndElement->track() : 0;
    // scan all spanners linked to this one
    for (EngravingObject* el : spanner->linkList()) {
        Spanner* sp = toSpanner(el);
        EngravingItem* newStartElement = nullptr;
        EngravingItem* newEndElement = nullptr;
        // if not the current spanner, but one linked to it, determine its new start and end elements
        // as modifications 'parallel' to the modifications of the current spanner's start and end elements
        if (sp != spanner) {
            if (startElement) {
                // determine the track where to expect the 'parallel' start element
                track_idx_t newTrack = sp->startElement() ? sp->startElement()->track() + startDeltaTrack : sp->track();
                // look in elements linked to new start element for an element with
                // same score as linked spanner and appropriate track
                for (EngravingObject* ee : startElement->linkList()) {
                    EngravingItem* e = toEngravingItem(ee);
                    if (e->score() == sp->score() && e->track() == newTrack) {
                        newStartElement = e;
                        break;
                    }
                }
            }
            // similarly to determine the 'parallel' end element
            if (endElement) {
                track_idx_t newTrack = sp->endElement() ? sp->endElement()->track() + endDeltaTrack : sp->track2();
                for (EngravingObject* ee : endElement->linkList()) {
                    EngravingItem* e = toEngravingItem(ee);
                    if (e->score() == sp->score() && e->track() == newTrack) {
                        newEndElement = e;
                        break;
                    }
                }
            }
        }
        // if current spanner, just use stored start and end elements
        else {
            newStartElement = startElement;
            newEndElement = endElement;
        }
        sp->score()->undo(new ChangeSpannerElements(sp, newStartElement, newEndElement));
    }
}

//---------------------------------------------------------
//   undoChangeTuning
//---------------------------------------------------------

void Score::undoChangeTuning(Note* n, double v)
{
    n->undoChangeProperty(Pid::TUNING, v);
}

void Score::undoChangeUserMirror(Note* n, DirectionH d)
{
    n->undoChangeProperty(Pid::MIRROR_HEAD, int(d));
}

//---------------------------------------------------------
//   undoChangeTpc
//    TODO-TPC: check
//---------------------------------------------------------

void Score::undoChangeTpc(Note* note, int v)
{
    note->undoChangeProperty(Pid::TPC1, v);
}

//---------------------------------------------------------
//   undoAddBracket
//---------------------------------------------------------

void Score::undoAddBracket(Staff* staff, size_t level, BracketType type, size_t span)
{
    staff_idx_t startStaffIdx = staff->idx();
    staff_idx_t totStaves = nstaves();

    // Make sure this brackets won't overlap with others sharing same column.
    // If overlaps are found, move the other brackets outwards (i.e. increase column).
    for (staff_idx_t staffIdx = startStaffIdx; staffIdx < startStaffIdx + span && staffIdx < totStaves; ++staffIdx) {
        const std::vector<BracketItem*>& brackets = m_staves.at(staffIdx)->brackets();

        for (int i = static_cast<int>(brackets.size()) - 1; i >= static_cast<int>(level); --i) {
            if (i >= static_cast<int>(brackets.size())) {
                // This might theoretically happen when a lot of brackets get cleaned up
                // after changing the column of the first bracket we see
                continue;
            }

            if (brackets[i]->bracketType() == BracketType::NO_BRACKET) {
                // Better not get brackets with type NO_BRACKET in the UndoStack,
                // as they might be cleaned up (Staff::cleanupBrackets())
                continue;
            }

            brackets[i]->undoChangeProperty(Pid::BRACKET_COLUMN, brackets[i]->column() + 1);
        }
    }

    undo(new AddBracket(staff, level, type, span));
}

//---------------------------------------------------------
//   undoRemoveBracket
//---------------------------------------------------------

void Score::undoRemoveBracket(Bracket* b)
{
    undo(new RemoveBracket(b->staff(), b->column(), b->bracketType(), b->span()));
}

//---------------------------------------------------------
//   undoInsertTime
//   acts on the linked scores as well
//---------------------------------------------------------

void Score::undoInsertTime(const Fraction& tick, const Fraction& len)
{
    if (len.isZero()) {
        return;
    }

    std::list<Spanner*> sl;
    for (auto i : m_spanner.map()) {
        Spanner* s = i.second;
        if (s->tick2() < tick) {
            continue;
        }
        bool append = false;
        if (len > Fraction(0, 1)) {
            if (tick > s->tick() && tick < s->tick2()) {
                append = true;
            } else if (tick <= s->tick()) {
                append = true;
            }
        } else {
            Fraction tick2 = tick - len;
            if (s->tick() >= tick2) {
                append = true;
            } else if (s->tick() >= tick && s->tick2() <= tick2) {
                append = true;
            } else if ((s->tick() <= tick) && (s->tick2() >= tick2)) {
                Fraction t2 = s->tick2() + len;
                if (t2 > s->tick()) {
                    append = true;
                }
            } else if (s->tick() > tick && s->tick2() > tick2) {
                append = true;
            } else if (s->tick() < tick && s->tick2() < tick2) {
                append = true;
            }
        }
        for (Spanner* ss : sl) {
            if (mu::contains(ss->linkList(), static_cast<EngravingObject*>(s))) {
                append = false;
                break;
            }
        }
        if (append) {
            sl.push_back(s);
        }
    }
    for (Spanner* s : sl) {
        if (len > Fraction(0, 1)) {
            if (tick > s->tick() && tick < s->tick2()) {
                //
                //  case a:
                //  +----spanner--------+
                //    +---add---
                //
                s->undoChangeProperty(Pid::SPANNER_TICKS, s->ticks() + len);
            } else if (tick <= s->tick()) {
                //
                //  case b:
                //       +----spanner--------
                //  +---add---
                // and
                //            +----spanner--------
                //  +---add---+
                EngravingItem* startElement = s->startElement();
                EngravingItem* endElement = s->endElement();
                undoChangeSpannerElements(s, nullptr, nullptr);
                s->undoChangeProperty(Pid::SPANNER_TICK, s->tick() + len);
                undoChangeSpannerElements(s, startElement, endElement);
            }
        } else {
            Fraction tick2 = tick - len;
            if (s->tick() >= tick2) {
                //
                //  case A:
                //  +----remove---+ +---spanner---+
                //
                Fraction t = s->tick() + len;
                if (t < Fraction(0, 1)) {
                    t = Fraction(0, 1);
                }
                EngravingItem* startElement = s->startElement();
                EngravingItem* endElement = s->endElement();
                undoChangeSpannerElements(s, nullptr, nullptr);
                s->undoChangeProperty(Pid::SPANNER_TICK, t);
                undoChangeSpannerElements(s, startElement, endElement);
            } else if (s->tick() >= tick && s->tick2() <= tick2) {
                //
                //  case B:
                //    +---spanner---+
                //  +----remove--------+
                //
                undoRemoveElement(s);
            } else if ((s->tick() <= tick) && (s->tick2() >= tick2)) {
                //
                //  case C:
                //  +----spanner--------+
                //    +---remove---+
                //
                Fraction t2 = s->tick2() + len;
                if (t2 > s->tick()) {
                    s->undoChangeProperty(Pid::SPANNER_TICKS, s->ticks() + len);
                }
            } else if (s->tick() > tick && s->tick2() > tick2) {
                //
                //  case D:
                //       +----spanner--------+
                //  +---remove---+
                //
                Fraction d1 = s->tick() - tick;
                Fraction d2 = tick2 - s->tick();
                Fraction le = s->ticks() - d2;
                if (le.isZero()) {
                    undoRemoveElement(s);
                } else {
                    s->undoChangeProperty(Pid::SPANNER_TICK, s->tick() - d1);
                    s->undoChangeProperty(Pid::SPANNER_TICKS, le);
                }
            } else if (s->tick() < tick && s->tick2() < tick2) {
                //
                //  case E:
                //       +----spanner--------+
                //                     +---remove---+
                //
                Fraction d  = s->tick2() - tick;
                Fraction le = s->ticks() - d;
                if (le.isZero()) {
                    undoRemoveElement(s);
                } else {
                    s->undoChangeProperty(Pid::SPANNER_TICKS, le);
                }
            }
        }
    }

    undo(new InsertTimeUnmanagedSpanner(this, tick, len));
}

//---------------------------------------------------------
//   undoRemoveMeasures
//---------------------------------------------------------

void Score::undoRemoveMeasures(Measure* m1, Measure* m2, bool preserveTies)
{
    assert(m1 && m2);

    const Fraction startTick = m1->tick();
    const Fraction endTick = m2->endTick();
    std::set<Spanner*> spannersToRemove;
    std::set<EngravingItem*> annotationsToRemove;

    //
    //  handle ties which start before m1 and end in (m1-m2)
    //
    for (Segment* s = m1->first(); s != m2->last(); s = s->next1()) {
        IF_ASSERT_FAILED(s) {
            // This score is corrupted; handle it without crashing,
            // to help the user to fix corruptions by deleting the affected measures
            LOGE() << "Missing segments detected while deleting measures " << m1->no() << " to " << m2->no()
                   << ". This score (" << name() << ") is corrupted. Continuing without deleting measure contents.";
            break;
        }

        if (!s->isChordRestType()) {
            continue;
        }

        for (EngravingItem* ee : s->annotations()) {
            annotationsToRemove.insert(ee);
        }

        for (track_idx_t track = 0; track < ntracks(); ++track) {
            EngravingItem* e = s->element(track);
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* c = toChord(e);
            for (Note* n : c->notes()) {
                // Remove ties crossing measure range boundaries
                Tie* t = n->tieBack();
                if (t && (t->startNote()->chord()->tick() < startTick)) {
                    if (preserveTies) {
                        t->setEndNote(0);
                    } else {
                        undoRemoveElement(t);
                    }
                }
                t = n->tieFor();
                if (t && (t->endNote()->chord()->tick() >= endTick)) {
                    undoRemoveElement(t);
                }

                // Do the same for other note-anchored spanners (e.g. glissandi).
                // Delay actual removing to avoid modifying lists inside loops over them.
                for (Spanner* sb : n->spannerBack()) {
                    if (sb->tick() < startTick) {
                        spannersToRemove.insert(sb);
                    }
                }
                for (Spanner* sf : n->spannerFor()) {
                    if (sf->tick2() >= endTick) {
                        spannersToRemove.insert(sf);
                    }
                }
            }
        }
    }

    for (Spanner* s : spannersToRemove) {
        undoRemoveElement(s);
    }

    for (EngravingItem* a : annotationsToRemove) {
        undoRemoveElement(a);
    }

    undo(new RemoveMeasures(m1, m2));
}

//---------------------------------------------------------
//   undoChangeMeasureRepeatCount
//---------------------------------------------------------

void Score::undoChangeMeasureRepeatCount(Measure* m, int newCount, staff_idx_t staffIdx)
{
    for (Staff* st : staff(staffIdx)->staffList()) {
        Score* linkedScore = st->score();
        staff_idx_t linkedStaffIdx = st->idx();
        Measure* linkedMeasure = linkedScore->tick2measure(m->tick());

        int currCount = linkedMeasure->measureRepeatCount(linkedStaffIdx);
        if (currCount != newCount) {
            linkedScore->undo(new ChangeMeasureRepeatCount(linkedMeasure, newCount, linkedStaffIdx));
        }
    }
}
}
