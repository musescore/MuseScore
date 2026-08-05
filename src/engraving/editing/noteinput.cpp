/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "noteinput.h"

#include <algorithm>
#include <iterator>
#include <tuple>

#include "translation.h"

#include "editmeasures.h"
#include "editnote.h"
#include "inserttime.h"
#include "mscoreview.h"
#include "regroup.h"
#include "transaction/transaction.h"
#include "transpose.h"

#include "../infrastructure/messagebox.h"

#include "../dom/accidental.h"
#include "../dom/chord.h"
#include "../dom/chordline.h"
#include "../dom/drumset.h"
#include "../dom/durationtype.h"
#include "../dom/excerpt.h"
#include "../dom/factory.h"
#include "../dom/input.h"
#include "../dom/masterscore.h"
#include "../dom/measure.h"
#include "../dom/measurerepeat.h"
#include "../dom/note.h"
#include "../dom/noteval.h"
#include "../dom/part.h"
#include "../dom/range.h"
#include "../dom/rest.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"
#include "../dom/slur.h"
#include "../dom/staff.h"
#include "../dom/stafftype.h"
#include "../dom/stringdata.h"
#include "../dom/tie.h"
#include "../dom/tremolotwochord.h"
#include "../dom/tuplet.h"
#include "../dom/utils.h"

#include "navigation.h"

#include "log.h"

using namespace muse;
using namespace mu;
using namespace mu::engraving;

//---------------------------------------------------------
//   resolveNoteInputParams
//---------------------------------------------------------

bool NoteInput::resolveNoteInputParams(const Score* score, int note, bool addFlag, NoteInputParams& out)
{
    const InputState& is = score->inputState();

    //! NOTE: Drumset params should be defined explicitly (see NotationViewInputController::tryPercussionShortcut)
    if (!is.isValid() || is.drumset()) {
        return false;
    }

    int octave = 4;

    static const int tab[] = { 0, 2, 4, 5, 7, 9, 11 };

    // if adding notes, add above the upNote of the current chord
    EngravingItem* el = score->selection().element();
    if (addFlag && el && el->isNote()) {
        Chord* chord = toNote(el)->chord();
        Note* n      = chord->upNote();
        int tpc = n->tpc();
        octave = (n->epitch() - int(tpc2alter(tpc))) / PITCH_DELTA_OCTAVE;
        if (note <= tpc2step(tpc)) {
            octave++;
        }
    } else {
        int curPitch = 60;
        if (is.segment()) {
            const Staff* staff = score->staff(is.track() / VOICES);
            Segment* seg = is.segment()->prev1(SegmentType::ChordRest | SegmentType::Clef | SegmentType::HeaderClef);
            while (seg) {
                if (seg->isChordRestType()) {
                    EngravingItem* p = seg->element(is.track());
                    if (p && p->isChord()) {
                        Note* n = toChord(p)->downNote();
                        // forget any accidental and/or adjustment due to key signature
                        curPitch = n->epitch() - static_cast<int>(tpc2alter(n->tpc()));
                        break;
                    }
                } else if (seg->isClefType() || seg->isHeaderClefType()) {
                    EngravingItem* p = seg->element(trackZeroVoice(is.track()));                  // clef on voice 1
                    if (p && p->isClef()) {
                        Clef* clef = toClef(p);
                        // check if it's an actual change or just a courtesy
                        ClefType ctb = staff->clef(clef->tick() - Fraction::eps());
                        if (ctb != clef->clefType() || clef->tick().isZero()) {
                            curPitch = line2pitch(4, clef->clefType(), Key::C);                     // C 72 for treble clef
                            break;
                        }
                    }
                }
                seg = seg->prev1MM(SegmentType::ChordRest | SegmentType::Clef | SegmentType::HeaderClef);
            }
            octave = curPitch / 12;
        }

        int delta = octave * 12 + tab[note] - curPitch;
        if (delta > 6) {
            --octave;
        } else if (delta < -6) {
            ++octave;
        }
    }

    out.step = octave * 7 + note;
    return true;
}

//---------------------------------------------------------
//   noteVal
//---------------------------------------------------------

NoteVal NoteInput::noteVal(const Score* score, int pitch, staff_idx_t staffIdx, bool allowTransposition)
{
    NoteVal nval(pitch);

    const Staff* st = score->staff(staffIdx);
    if (!st) {
        return nval;
    }

    bool concertPitch = score->style().styleB(Sid::concertPitch);
    Interval v = st->part()->instrument(score->inputState().tick())->transpose();

    // If an accidental is set in the input state, use it as a hint for the pitch spelling
    if (AccidentalType at = score->inputState().accidentalType(); at == AccidentalType::FLAT || at == AccidentalType::SHARP) {
        Prefer prefer = at == AccidentalType::SHARP ? Prefer::SHARPS : Prefer::FLATS;
        if (concertPitch || v.isZero()) {
            // Note: using Key::C always and ignoring actual key signature. Otherwise, flat mode would still use sharps
            // sometimes if they're in the key, and vice versa, which seems contrary to the intent of the hint.
            nval.tpc1 = pitch2tpc(nval.pitch, Key::C, prefer);
            Interval vFlipped = v;
            vFlipped.flip();
            nval.tpc2 = Transpose::transposeTpc(nval.tpc1, vFlipped, true);
        } else {
            // Spell the transposed pitch first, then convert to concert pitch
            int writtenPitch = nval.pitch;
            if (!allowTransposition) {
                writtenPitch -= v.chromatic;
            }
            nval.tpc2 = pitch2tpc(writtenPitch, Key::C, prefer);
            nval.tpc1 = Transpose::transposeTpc(nval.tpc2, v, true);
        }
    }

    // if transposing, interpret MIDI pitch as representing desired written pitch
    // set pitch based on corresponding sounding pitch
    if (!concertPitch && allowTransposition) {
        nval.pitch += v.chromatic;
    }

    return nval;
}

//---------------------------------------------------------
//   noteValForPosition
//---------------------------------------------------------

NoteVal NoteInput::noteValForPosition(const Score* score, Position pos, AccidentalType at, bool& error)
{
    const InputState& is = score->inputState();

    error           = false;
    const Segment* s = pos.segment;
    int line        = pos.line;
    Fraction tick   = s->tick();
    staff_idx_t staffIdx = pos.staffIdx;
    const Staff* st = score->staff(staffIdx);
    ClefType clef   = st->clef(tick);
    const Instrument* instr = st->part()->instrument(s->tick());
    NoteVal nval;
    const StringData* stringData = nullptr;

    // pitched/unpitched note entry depends on instrument (override StaffGroup)
    StaffGroup staffGroup = st->staffType(tick)->group();
    if (staffGroup != StaffGroup::TAB) {
        staffGroup = instr->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
    }

    if (staffGroup != StaffGroup::PERCUSSION) {
        stringData = st->part()->stringData(s->tick(), st->idx());
    }

    switch (staffGroup) {
    case StaffGroup::PERCUSSION: {
        if (is.rest()) {
            break;
        }
        const Drumset* ds = instr->drumset();
        nval.pitch = is.drumNote();
        if (!ds->isValid(nval.pitch) || ds->line(nval.pitch) != line) {
            // Drum note from input state is not valid - fall back to the first valid pitch for this line...
            const int defaultPitch = ds->defaultPitchForLine(line);
            if (ds->isValid(defaultPitch)) {
                nval.pitch = defaultPitch;
            }
        }
        if (nval.pitch < 0) {
            error = true;
            return nval;
        }
        nval.headGroup = ds->noteHead(nval.pitch);
        if (nval.headGroup == NoteHeadGroup::HEAD_INVALID) {
            error = true;
            return nval;
        }
        break;
    }
    case StaffGroup::TAB: {
        if (is.rest()) {
            return nval;
        }
        line = st->staffType(tick)->visualStringToPhys(line);
        if (line < 0 || line >= static_cast<int>(stringData->strings())) {
            error = true;
            return nval;
        }
        // build a default NoteVal for that string
        nval.string = line;
        if (pos.fret != INVALID_FRET_INDEX) {                  // if a fret is given, use it
            nval.fret = pos.fret;
        } else {                                      // if no fret, use 0 as default
            nval.fret = 0;
        }
        // reduce within fret limit
        if (nval.fret > stringData->frets()) {
            nval.fret = stringData->frets();
        }
        // for open strings, only accepts fret 0 (strings in StringData are from bottom to top)
        size_t strgDataIdx = stringData->strings() - line - 1;
        if (nval.fret > 0 && stringData->stringList().at(strgDataIdx).open) {
            nval.fret = 0;
        }
        nval.pitch = stringData->getPitch(line, nval.fret, st, pos.segment->tick());
        break;
    }

    case StaffGroup::STANDARD: {
        AccidentalVal acci
            = (at == AccidentalType::NONE ? s->measure()->findAccidental(s, staffIdx, line, error) : Accidental::subtype2value(at));
        if (error) {
            return nval;
        }
        int step           = absStep(line, clef);
        int octave         = step / 7;
        nval.pitch         = step2pitch(step) + octave * 12 + int(acci);
        if (score->style().styleB(Sid::concertPitch)) {
            nval.tpc1 = step2tpc(step % 7, acci);
        } else {
            nval.pitch += instr->transpose().chromatic;
            nval.tpc2 = step2tpc(step % 7, acci);
            Interval v = st->transpose(tick);
            if (v.isZero()) {
                nval.tpc1 = nval.tpc2;
            } else {
                nval.tpc1 = Transpose::transposeTpc(nval.tpc2, v, true);
            }
        }
        stringData->convertPitch(nval.pitch, st, pos.segment->tick(), &nval.string, &nval.fret);
    }
    break;
    }
    return nval;
}

//---------------------------------------------------------
//   addNote from NoteVal
//---------------------------------------------------------

Note* NoteInput::addNote(Transaction&, Score* score, Chord* chord, const NoteVal& noteVal, bool forceAccidental,
                         const std::set<SymId>& articulationIds, InputState* externalInputState)
{
    InputState& is = externalInputState ? (*externalInputState) : score->inputState();

    Note* note = Factory::createNote(chord);
    note->setParent(chord);
    note->setTrack(chord->track());
    note->setNval(noteVal);
    score->undoAddElement(note);
    if (forceAccidental) {
        int tpc = score->style().styleB(Sid::concertPitch) ? noteVal.tpc1 : noteVal.tpc2;
        AccidentalVal alter = tpc2alter(tpc);
        AccidentalType at = Accidental::value2subtype(alter);
        Accidental* a = Factory::createAccidental(note);
        a->setAccidentalType(at);
        a->setRole(AccidentalRole::USER);
        a->setParent(note);
        score->undoAddElement(a);
    }

    if (!articulationIds.empty()) {
        chord->updateArticulations(articulationIds);
    }

    score->setPlayNote(true);
    score->setPlayChord(true);

    if (externalInputState) {
        is.setTrack(note->track());
        is.setLastSegment(is.segment());
        is.setSegment(note->chord()->segment());
    } else {
        score->select(note, SelectType::SINGLE, 0);
    }

    if (!chord->staff()->isTabStaff(chord->tick())) {
        NoteEntryMethod entryMethod = is.noteEntryMethod();
        if (entryMethod != NoteEntryMethod::REPITCH && entryMethod != NoteEntryMethod::REALTIME_AUTO
            && entryMethod != NoteEntryMethod::REALTIME_MANUAL) {
            is.moveToNextInputPos();
        }
    }
    return note;
}

Note* NoteInput::addNoteToTiedChord(Transaction& tx, Score* score, Chord* chord, const NoteVal& noteVal, bool forceAccidental,
                                    const std::set<SymId>& articulationIds)
{
    IF_ASSERT_FAILED(!chord->notes().empty()) {
        return nullptr;
    };
    Note* referenceNote = chord->notes().at(0);

    while (true) {
        // don't add note if it is already part of tied notes previously
        if (referenceNote->chord()->findNote(noteVal.pitch)) {
            return nullptr;
        }
        if (!referenceNote->tieBack() || referenceNote->incomingPartialTie()) {
            break;
        }
        referenceNote = referenceNote->tieBack()->startNote();
    }

    Tie* tie = nullptr;
    Note* newNote = nullptr;

    while (referenceNote->tieFor()) {
        chord = referenceNote->chord();
        newNote = addNote(tx, score, chord, noteVal, forceAccidental, articulationIds);
        if (!newNote) {
            return nullptr;
        }
        if (tie) {
            tie->setEndNote(newNote);
            tie->setTick2(newNote->tick());
            newNote->setTieBack(tie);
            score->undoAddElement(tie);
        }

        tie = Factory::createTie(newNote);
        tie->setStartNote(newNote);
        tie->setTick(newNote->tick());
        tie->setTrack(newNote->track());
        newNote->setTieFor(tie);

        if (!referenceNote->tieFor()->endNote()) {
            break;
        }

        referenceNote = referenceNote->tieFor()->endNote();
    }

    chord = referenceNote->chord();
    newNote = addNote(tx, score, chord, noteVal, forceAccidental, articulationIds);

    if (!newNote) {
        return nullptr;
    }

    if (tie) {
        tie->setEndNote(newNote);
        tie->setTick2(newNote->tick());
        newNote->setTieBack(tie);
        score->undoAddElement(tie);
    }

    score->connectTies();

    return newNote;
}

//---------------------------------------------------------
//   addPitch
//---------------------------------------------------------

Note* NoteInput::addPitch(Transaction& tx, Score* score, NoteVal& nval, bool addFlag, InputState* externalInputState)
{
    InputState& is = externalInputState ? (*externalInputState) : score->inputState();

    if (addFlag) {
        ChordRest* c = toChordRest(is.lastSegment()->element(is.track()));
        if (!c || !c->isChord()) {
            LOGD("NoteInput::addPitch: cr %s", c ? c->typeName() : "zero");
            return nullptr;
        }

        return addPitchToChord(tx, score, nval, toChord(c), externalInputState);
    }

    if (is.beyondScore()) {
        score->appendMeasures(1);
        is.moveToNextInputPos();
    }

    score->expandVoice(is.segment(), is.track());

    // insert note
    DirectionV stemDirection = DirectionV::AUTO;
    track_idx_t track = is.track();
    if (is.drumset()) {
        const Drumset* ds = is.drumset();
        if (!ds->isValid(nval.pitch)) {
            return nullptr;
        }

        nval.headGroup    = ds->noteHead(nval.pitch);
        stemDirection     = ds->stemDirection(nval.pitch);
        track             = ds->voice(nval.pitch) + (is.track() / VOICES) * VOICES;
        is.setTrack(track);
        score->expandVoice(is.segment(), is.track());
    }

    if (!is.cr()) {
        ChordRest* prevCr = is.segment()->nextChordRest(is.track(), /*backwards*/ true, /*stopAtMeasureBoundary*/ true);
        if (prevCr && prevCr->endTick() > is.tick()) {
            const InputState inputStateToRestore = is; // because truncateChordRest will alter the input state
            truncateChordRest(tx, score, prevCr, is.tick(), /*fillWithRest*/ true);
            is = inputStateToRestore;
        }
    }

    Measure* measure = is.segment()->measure();
    if (measure->isMeasureRepeatGroup(track2staff(track))) {
        MeasureRepeat* mr = measure->measureRepeatElement(track2staff(track));
        score->deleteItem(mr); // resets any measures related to mr
    }
    Fraction duration;
    if (is.usingNoteEntryMethod(NoteEntryMethod::REALTIME_AUTO) || is.usingNoteEntryMethod(NoteEntryMethod::REALTIME_MANUAL)) {
        // FIXME: truncate duration at barline in real-time modes.
        //   The user might try to enter a duration that is too long to fit in the remaining space in the measure.
        //   We could split the duration at the barline and continue into the next bar, but this would create extra
        //   notes, extra ties, and extra pain. Instead, we simply truncate the duration at the barline.
        Fraction ticks2measureEnd = is.segment()->measure()->ticks() - is.segment()->rtick();
        duration = is.duration().fraction() > ticks2measureEnd ? ticks2measureEnd : is.duration().fraction();
    } else {
        duration = is.duration().fraction();
    }
    Note* note = nullptr;
    Note* lastTiedNote = nullptr;
    if (is.usingNoteEntryMethod(NoteEntryMethod::REPITCH) && is.cr()->isChord()) {
        Chord* chord = toChord(is.cr());
        std::tie(note, lastTiedNote) = repitchReplaceNote(tx, score, chord, nval);  // the add (not replace) case was handled above
    } else if (!is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
        Segment* seg = score->setNoteRest(
            is.segment(), track, nval, duration, stemDirection, /* forceAccidental */ false, is.articulationIds(), /* rhythmic */ false,
            externalInputState);
        if (seg) {
            note = toChord(seg->element(track))->upNote();
        }
    }

    if (is.slur()) {
        //
        // extend slur
        //
        ChordRest* e = score->searchNote(is.tick(), is.track());
        EngravingItem* ee = is.slur()->startElement();
        if (e && ee) {
            Fraction stick = Fraction(0, 1);
            if (ee->isChordRest()) {
                stick = toChordRest(ee)->tick();
            } else if (ee->isNote()) {
                stick = toNote(ee)->chord()->tick();
            }
            if (stick == e->tick()) {
                is.slur()->setTick(stick);
                is.slur()->setStartElement(e);
            } else {
                is.slur()->setTick2(e->tick());
                is.slur()->setEndElement(e);
            }
        } else {
            LOGD("addPitch: cannot find slur note");
        }
    }
    if (is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
        // move cursor to next note, but skip tied notes (they were already repitched above)
        ChordRest* next = lastTiedNote ? Navigation::nextChordRest(lastTiedNote->chord()) : Navigation::nextChordRest(is.cr());
        while (next && !next->isChord()) {
            next = Navigation::nextChordRest(next);
        }
        if (next) {
            is.moveInputPos(next->segment());
        }
    } else {
        NoteEntryMethod entryMethod = is.noteEntryMethod();
        if (entryMethod != NoteEntryMethod::REALTIME_AUTO && entryMethod != NoteEntryMethod::REALTIME_MANUAL) {
            is.moveToNextInputPos();
        }
    }
    return note;
}

Note* NoteInput::addPitchToChord(Transaction& tx, Score* score, NoteVal& nval, Chord* chord, InputState* externalInputState,
                                 bool forceAccidental)
{
    IF_ASSERT_FAILED(chord) {
        return nullptr;
    }

    InputState& is = externalInputState ? (*externalInputState) : score->inputState();

    auto isTied = [](const Chord* ch) {
        if (ch->notes().empty()) {
            return false;
        }
        Note* n = ch->notes().at(0);
        return (n->tieFor() && !n->tieFor()->isLaissezVib()) || n->tieBack();
    };

    Note* note = nullptr;
    if (isTied(chord)) {
        note = addNoteToTiedChord(tx, score, chord, nval, forceAccidental);
        if (!note) {
            note = addNote(tx, score, chord, nval, forceAccidental, /* articulationIds */ {}, externalInputState);
        }
    } else {
        note = addNote(tx, score, chord, nval, forceAccidental, /* articulationIds */ {}, externalInputState);
    }

    if (is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
        // move cursor to next note
        ChordRest* next = Navigation::nextChordRest(note->chord());
        while (next && !next->isChord()) {
            next = Navigation::nextChordRest(next);
        }
        if (next) {
            is.moveInputPos(next->segment());
        }
    } else if (is.lastSegment() == is.segment()) {
        NoteEntryMethod entryMethod = is.noteEntryMethod();
        if (entryMethod != NoteEntryMethod::REALTIME_AUTO && entryMethod != NoteEntryMethod::REALTIME_MANUAL) {
            is.moveToNextInputPos();
        }
    }

    return note;
}

//---------------------------------------------------------
//   addMidiPitch
//---------------------------------------------------------

Note* NoteInput::addMidiPitch(Transaction& tx, Score* score, int pitch, bool addFlag, bool allowTransposition)
{
    NoteVal nval = noteVal(score, pitch, score->inputState().staffIdx(), allowTransposition);
    return addPitch(tx, score, nval, addFlag);
}

//---------------------------------------------------------
//   addTiedMidiPitch
//---------------------------------------------------------

Note* NoteInput::addTiedMidiPitch(Transaction& tx, Score* score, int pitch, bool addFlag, Chord* prevChord, bool allowTransposition)
{
    Note* n = addMidiPitch(tx, score, pitch, addFlag, allowTransposition);
    if (prevChord) {
        Note* nn = prevChord->findNote(n->pitch());
        if (nn) {
            Tie* tie = Factory::createTie(score->dummy());
            tie->setStartNote(nn);
            tie->setEndNote(n);
            tie->setTick(tie->startNote()->tick());
            tie->setTick2(tie->endNote()->tick());
            tie->setTrack(n->track());
            n->setTieBack(tie);
            nn->setTieFor(tie);
            score->undoAddElement(tie);
        }
    }
    return n;
}

//---------------------------------------------------------
//   repitchReplaceNote
//---------------------------------------------------------

std::pair<Note*, Note*> NoteInput::repitchReplaceNote(Transaction&, Score* score, Chord* chord, const NoteVal& nval,
                                                      bool forceAccidental)
{
    Note* note = Factory::createNote(chord);
    note->setParent(chord);
    note->setTrack(chord->track());
    note->setNval(nval);

    Note* firstTiedNote = nullptr;
    Note* lastTiedNote = note;
    ChordLine* chordLine = nullptr;
    std::vector<Note*> notes = chord->notes();
    // break all ties into current chord
    // these will exist only if user explicitly moved cursor to a tied-into note
    // in ordinary use, cursor will automatically skip past these during note entry
    for (Note* n : notes) {
        if (n->tieBack()) {
            score->undoRemoveElement(n->tieBack());
        }
    }
    // Keep first chordline only
    chordLine = chord->chordLine() ? chord->chordLine()->clone() : nullptr;
    std::vector<EngravingItem*> chordEls = chord->el();
    for (EngravingItem* e : chordEls) {
        if (e->isChordLine()) {
            score->undoRemoveElement(e);
        }
    }
    // for the first note of the chord only, preserve ties by changing pitch of all forward notes
    // the tie forward itself will be added later
    // multi-note chords get reduced to single note chords anyhow since we remove the old notes below
    if (notes.front()->tieFor()) {
        Note* tn = notes.front()->tieFor()->endNote();
        while (tn) {
            Chord* tc = tn->chord();
            if (tc->notes().size() != 1) {
                std::vector<Note*> notesToRemove;
                for (Note* n : tc->notes()) {
                    if (n != tn) {
                        notesToRemove.push_back(n);
                    }
                }
                for (Note* n : notesToRemove) {
                    score->undoRemoveElement(n);
                }
                assert(tc->notes().size() == 1 && tc->notes().front() == tn);
            }
            if (!firstTiedNote) {
                firstTiedNote = tn;
            }
            lastTiedNote = tn;
            EditNote::undoChangePitch(score, tn, note->pitch(), note->tpc1(), note->tpc2());
            if (tn->tieFor()) {
                tn = tn->tieFor()->endNote();
            } else {
                break;
            }
        }
    }
    // remove all notes from chord
    // the new note will be added below
    while (!chord->notes().empty()) {
        score->undoRemoveElement(chord->notes().front());
    }
    notes.clear();  // get rid of vector full of dangling pointers!

    // add new note to chord
    score->undoAddElement(note);

    if (chordLine) {
        chordLine->setNote(note);
        score->undoAddElement(chordLine);
    }

    if (forceAccidental) {
        int tpc = score->style().styleB(Sid::concertPitch) ? nval.tpc1 : nval.tpc2;
        AccidentalVal alter = tpc2alter(tpc);
        AccidentalType at = Accidental::value2subtype(alter);
        Accidental* a = Factory::createAccidental(note);
        a->setAccidentalType(at);
        a->setRole(AccidentalRole::USER);
        a->setParent(note);
        score->undoAddElement(a);
    }
    score->setPlayNote(true);
    // recreate tie forward if there is a note to tie to
    // one-sided ties will not be recreated
    if (firstTiedNote) {
        Tie* tie = Factory::createTie(note);
        tie->setStartNote(note);
        tie->setEndNote(firstTiedNote);
        tie->setTick(tie->startNote()->tick());
        tie->setTick2(tie->endNote()->tick());
        tie->setTrack(note->track());
        score->undoAddElement(tie);
    }
    score->select(lastTiedNote);

    return { note, lastTiedNote };
}

//---------------------------------------------------------
//   putNote
//    mouse click in state NoteType::ENTRY
//---------------------------------------------------------

Ret NoteInput::putNote(Transaction& tx, Score* score, const PointF& pos, bool replace, bool insert)
{
    Position p;
    if (!score->getPosition(&p, pos, score->inputState().voice())) {
        LOGD("cannot put note here, get position failed");
        return make_ret(Ret::Code::UnknownError);
    }

    Score* posScore = p.segment->score();
    // it is not safe to call repitchNote() if p is on a TAB staff
    bool isTablature = posScore->staff(p.staffIdx)->isTabStaff(p.segment->tick());

    //  calculate actual clicked line from staffType offset and stepOffset
    Staff* ss = posScore->staff(p.staffIdx);
    int stepOffset = ss->staffType(p.segment->tick())->stepOffset();
    double stYOffset = ss->staffType(p.segment->tick())->yoffset().val();
    double lineDist = ss->staffType(p.segment->tick())->lineDistance().val();
    p.line -= stepOffset + 2 * stYOffset / lineDist;

    if (posScore->inputState().usingNoteEntryMethod(NoteEntryMethod::REPITCH) && !isTablature) {
        return repitchNote(tx, posScore, p, replace);
    } else {
        if (insert || posScore->inputState().usingNoteEntryMethod(NoteEntryMethod::TIMEWISE)) {
            return insertChordByInsertingTime(tx, posScore, p);
        } else {
            return putNote(tx, posScore, p, replace);
        }
    }
}

Ret NoteInput::putNote(Transaction& tx, Score* score, const Position& p, bool replace)
{
    InputState& is = score->inputState();

    Staff* st   = score->staff(p.staffIdx);
    Segment* s  = p.segment;

    is.setTrack(p.staffIdx * VOICES + is.voice());
    is.setSegment(s);

    if (p.beyondScore) {
        score->appendMeasures(1);
        is.moveToNextInputPos();
    }

    if (mu::engraving::Excerpt* excerpt = score->excerpt()) {
        const TracksMap& tracks = excerpt->tracksMapping();

        if (!tracks.empty() && muse::key(tracks, is.track(), muse::nidx) == muse::nidx) {
            return make_ret(Ret::Code::UnknownError);
        }
    }

    DirectionV stemDirection = DirectionV::AUTO;
    bool error = false;
    NoteVal nval = noteValForPosition(score, p, is.accidentalType(), error);
    if (error) {
        return make_ret(Ret::Code::UnknownError);
    }

    // warn and delete MeasureRepeat if necessary
    Measure* m = is.segment()->measure();
    staff_idx_t staffIdx = track2staff(is.track());
    if (m->isMeasureRepeatGroup(staffIdx)) {
        auto b = MessageBox(score->iocContext()).warning(muse::trc("engraving", "Note input will remove measure repeat"),
                                                         muse::trc("engraving", "This measure contains a measure repeat."
                                                                                " If you enter notes here, it will be deleted."
                                                                                " Do you want to continue?"));
        if (b == MessageBox::Cancel) {
            return make_ret(Ret::Code::Cancel);
        }

        score->deleteItem(m->measureRepeatElement(staffIdx));
    }

    const StringData* stringData = 0;

    // pitched/unpitched note entry depends on instrument (override StaffGroup)
    StaffGroup staffGroup = st->staffType(s->tick())->group();
    if (staffGroup != StaffGroup::TAB) {
        staffGroup = st->part()->instrument(s->tick())->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
    }

    switch (staffGroup) {
    case StaffGroup::PERCUSSION: {
        const Drumset* ds = st->part()->instrument(s->tick())->drumset();
        DO_ASSERT(ds);

        if (ds) {
            stemDirection = ds->stemDirection(nval.pitch);
            is.setVoice(ds->voice(nval.pitch));
        }
        break;
    }
    case StaffGroup::TAB:
        stringData = st->part()->stringData(s->tick(), st->idx());
        is.setDrumNote(-1);
        break;
    case StaffGroup::STANDARD:
        is.setDrumNote(-1);
        break;
    }

    score->expandVoice();

    // If there's an overlapping ChordRest at the current input position, shorten it...
    if (!is.cr()) {
        ChordRest* prevCr = is.segment()->nextChordRest(is.track(), /*backwards*/ true, /*stopAtMeasureBoundary*/ true);
        if (prevCr && prevCr->endTick() > is.tick()) {
            const InputState inputStateToRestore = is; // because truncateChordRest will alter the input state
            truncateChordRest(tx, score, prevCr, is.tick(), /*fillWithRest*/ true);
            is = inputStateToRestore;
        }
    }

    ChordRest* cr = is.cr();

    auto checkTied = [&]() {
        if (!cr || !cr->isChord()) {
            return false;
        }
        auto ch = toChord(cr);
        return (!ch->notes().empty() && !ch->notes()[0]->tieBack())
               && (ch->notes()[0]->tieFor() && !ch->notes()[0]->tieFor()->isLaissezVib());
    };

    bool addToChord = false;
    bool shouldAddAsTied = checkTied();

    if (cr) {
        // retrieve total duration of current chord
        TDuration d = cr->durationType();
        // if not in replace mode AND chord duration == input duration AND not rest input
        // we need to add to current chord (otherwise, we will need to replace it or create a new one)
        if (!replace
            && ((d == is.duration()) || shouldAddAsTied)
            && cr->isChord()
            && !is.rest()) {
            if (st->isTabStaff(cr->tick())) {            // TAB
                // if a note on same string already exists, update to new pitch/fret
                for (Note* note : toChord(cr)->notes()) {
                    if (note->string() == nval.string) {                 // if string is the same
                        // if adding a new digit will keep fret number within fret limit,
                        // add a digit to existing fret number
                        if (stringData) {
                            int fret = note->fret() * 10 + nval.fret;
                            if (fret <= stringData->frets()) {
                                nval.fret = fret;
                                nval.pitch = stringData->getPitch(nval.string, nval.fret, st);
                            } else {
                                LOGD("can't increase fret to %d", fret);
                            }
                        }
                        // set fret number (original or combined) in all linked notes
                        int tpc1 = note->tpc1default(nval.pitch);
                        int tpc2 = note->tpc2default(nval.pitch);
                        EditNote::undoChangeFretting(score, note, nval.pitch, nval.string, nval.fret, tpc1, tpc2);
                        score->setPlayNote(true);
                        return muse::make_ok();
                    }
                }
            } else {                            // not TAB
                // if a note with the same pitch already exists in the chord, remove it
                Chord* chord = toChord(cr);
                Note* note = chord->findNote(nval.pitch);
                if (note) {
                    if (chord->notes().size() > 1) {
                        score->undoRemoveElement(note);
                    }
                    return muse::make_ok();
                }
            }
            addToChord = true;                  // if no special case, add note to chord
        }
    }
    bool forceAccidental = false;
    if (is.accidentalType() != AccidentalType::NONE) {
        NoteVal nval2 = noteValForPosition(score, p, AccidentalType::NONE, error);
        forceAccidental = (nval.pitch == nval2.pitch);
    }

    Ret ret = muse::make_ok();

    if (addToChord && cr->isChord()) {
        // if adding, add!
        Chord* chord = toChord(cr);
        if (shouldAddAsTied) {
            Note* n = addNoteToTiedChord(tx, score, chord, nval, forceAccidental, is.articulationIds());
            if (!n) {
                ret = make_ret(Ret::Code::UnknownError);
            }

            is.setAccidentalType(AccidentalType::NONE);
            return ret;
        }
        Note* note = addNote(tx, score, chord, nval, forceAccidental, is.articulationIds());
        if (!note) {
            ret = make_ret(Ret::Code::UnknownError);
        }

        is.setAccidentalType(AccidentalType::NONE);
        return ret;
    } else {
        // if not adding, replace current chord (or create a new one)
        if (is.rest()) {
            nval.pitch = -1;
        }

        Segment* seg = score->setNoteRest(is.segment(), is.track(), nval,
                                          is.duration().fraction(), stemDirection, forceAccidental, is.articulationIds());
        if (!seg) {
            ret = make_ret(Ret::Code::UnknownError);
        }

        is.setAccidentalType(AccidentalType::NONE);
    }

    if (cr && !st->isTabStaff(cr->tick())) {
        is.moveToNextInputPos();
    }

    return ret;
}

//---------------------------------------------------------
//   repitchNote
//---------------------------------------------------------

Ret NoteInput::repitchNote(Transaction& tx, Score* score, const Position& p, bool replace)
{
    InputState& is = score->inputState();

    Segment* s      = p.segment;
    Fraction tick   = s->tick();
    Staff* st       = score->staff(p.staffIdx);
    ClefType clef   = st->clef(tick);

    NoteVal nval;
    bool error = false;
    AccidentalType at = is.accidentalType();
    if (is.drumset() && is.drumNote() != -1) {
        nval.pitch = is.drumNote();
    } else {
        AccidentalVal acci
            = (at == AccidentalType::NONE ? s->measure()->findAccidental(s, p.staffIdx, p.line, error) : Accidental::subtype2value(at));
        if (error) {
            return make_ret(Ret::Code::UnknownError);
        }

        int step   = absStep(p.line, clef);
        int octave = step / 7;
        nval.pitch = step2pitch(step) + octave * 12 + int(acci);

        if (score->style().styleB(Sid::concertPitch)) {
            nval.tpc1 = step2tpc(step % 7, acci);
        } else {
            nval.pitch += st->part()->instrument(s->tick())->transpose().chromatic;
            nval.tpc2 = step2tpc(step % 7, acci);
        }
    }

    if (!is.segment()) {
        return make_ret(Ret::Code::UnknownError);
    }

    Chord* chord;
    ChordRest* cr = is.cr();
    if (!cr) {
        cr = is.segment()->nextChordRest(is.track());
        if (!cr) {
            return make_ret(Ret::Code::UnknownError);
        }
    }
    if (cr->isRest()) {   //skip rests
        ChordRest* next = Navigation::nextChordRest(cr);
        while (next && !next->isChord()) {
            next = Navigation::nextChordRest(next);
        }
        if (next) {
            is.moveInputPos(next->segment());
        }
        return muse::make_ok();
    } else {
        chord = toChord(cr);
    }

    bool forceAccidental = false;
    if (is.accidentalType() != AccidentalType::NONE) {
        NoteVal nval2 = noteValForPosition(score, p, AccidentalType::NONE, error);
        forceAccidental = (nval.pitch == nval2.pitch);
    }

    // Note: not sure this is ever called with replace == false, since add (not replace) case is handled already in cmdAddPitch
    if (!replace) {
        return addPitchToChord(tx, score, nval, chord, /* externalInputState */ nullptr, forceAccidental);
    }

    auto [note, lastTiedNote] = repitchReplaceNote(tx, score, chord, nval, forceAccidental);
    score->setPlayChord(true);

    // move to next Chord
    ChordRest* next = Navigation::nextChordRest(lastTiedNote->chord());
    while (next && !next->isChord()) {
        next = Navigation::nextChordRest(next);
    }
    if (next) {
        is.moveInputPos(next->segment());
    }

    return muse::make_ok();
}

//---------------------------------------------------------
//   insertChordByInsertingTime
//---------------------------------------------------------

Ret NoteInput::insertChordByInsertingTime(Transaction& tx, Score* score, const Position& pos)
{
    // insert by increasing measure length
    //
    // TODO:
    //    - check voices
    //    - split chord/rest

    EngravingItem* el = score->selection().element();
    if (!el || !(el->isNote() || el->isRest())) {
        return make_ret(Ret::Code::UnknownError);
    }

    Segment* seg = pos.segment;
    if (seg->splitsTuplet()) {
        MScore::setError(MsError::CANNOT_INSERT_TUPLET);
        return make_ret(Ret::Code::UnknownError);
    }

    // remove all two-note tremolos that end on this tick
    for (EngravingItem* e : seg->elist()) {
        if (!e || !e->isChord()) {
            continue;
        }
        Chord* c = toChord(e);
        TremoloTwoChord* t = c->tremoloTwoChord();
        if (t && t->chord2() == c) {
            // we have to remove this tremolo because we are adding time in the middle of it
            // (if c is chord1 then we're inserting before the trem so it's fine)
            score->undoRemoveElement(t);
        }
    }

    const TDuration duration = score->inputState().duration();
    const Fraction fraction  = duration.fraction();
    const Fraction len       = fraction;
    Fraction tick            = seg->tick();
    Measure* measure         = seg->measure()->isMMRest() ? seg->measure()->mmRestFirst() : seg->measure();
    const Fraction targetMeasureLen = measure->ticks() + fraction;

    // Shift spanners, enlarge the measure.
    // The approach is similar to that in Measure::adjustToLen() but does
    // insert time to the middle of the measure rather than to the end.
    score->undoInsertTime(tick, len);

    for (Score* lscore : score->scoreList()) {
        tx.push(new InsertTime(lscore, tick, len));
        Measure* m = lscore->tick2measure(tick);
        tx.push(new ChangeMeasureLen(m, targetMeasureLen));
        Segment* scoreSeg = m->tick2segment(tick);
        for (Segment* s = scoreSeg; s; s = s->next()) {
            s->undoChangeProperty(Pid::TICK, s->rtick() + len);
        }
    }

    // Fill the inserted time with rests.
    // This is better to be done in master score to cover all staves.
    MasterScore* ms = score->masterScore();
    Measure* msMeasure = ms->tick2measure(tick);
    const size_t msTracks = ms->ntracks();

    Segment* firstSeg = msMeasure->first(SegmentType::ChordRest);
    for (track_idx_t track = 0; track < msTracks; ++track) {
        EngravingItem* maybeRest = firstSeg->element(track);
        bool measureIsFull = false;

        // I. Convert any measure rests into normal (non-measure) rest(s) of equivalent duration
        if (maybeRest && maybeRest->isRest() && toRest(maybeRest)->durationType().isMeasure()) {
            ms->undoRemoveElement(maybeRest);
            Rest* measureRest = toRest(maybeRest);
            // If measure rest is situated at measure start we will fill
            // the whole measure with rests.
            measureIsFull = measureRest->rtick().isZero();
            const Fraction fillLen = measureIsFull ? targetMeasureLen : measureRest->ticks();
            ms->setRest(measureRest->tick(), track, fillLen, /* useDots */ false, /* tuplet */ nullptr, /* useFullMeasureRest */ false);
        }

        // II. Make chord or rest in other track longer if it crosses the insert area
        if (!measureIsFull) {
            ChordRest* cr = ms->findCR(tick, track);
            if (cr && cr->tick() < tick && cr->endTick() > tick) {
                if (cr->isRest()) {
                    const Fraction fillLen = cr->ticks() + fraction;
                    ms->undoRemoveElement(cr);
                    ms->setRest(cr->tick(), track, fillLen, /* useDots */ false, /* tuplet */ nullptr, /* useFullMeasureRest */ false);
                } else if (cr->isChord()) {
                    Chord* chord = toChord(cr);
                    std::vector<TDuration> durations = toDurationList(chord->ticks() + fraction, /* useDots */ true);
                    Fraction p = chord->tick();
                    ms->undoRemoveElement(chord);
                    Chord* prevChord = nullptr;
                    for (const TDuration& dur : durations) {
                        Chord* prototype = prevChord ? prevChord : chord;
                        const bool genTie = bool(prevChord);
                        prevChord = ms->addChord(p, dur, prototype, genTie, /* tuplet */ nullptr);
                        p += dur.fraction();
                    }
                    // TODO: reconnect ties if this chord was tied to other
                }
                measureIsFull = true;
            }
        }

        // III. insert rest(s) to fill the inserted space
        if (!measureIsFull && msMeasure->hasVoice(track)) {
            ms->setRest(tick, track, fraction, /* useDots */ false, /* tuplet */ nullptr);
        }
    }

    // Put the note itself.
    Segment* s = measure->undoGetSegment(SegmentType::ChordRest, tick);
    Position p(pos);
    p.segment = s;
    putNote(tx, score, p, true);

    return muse::make_ok();
}

//---------------------------------------------------------
//   truncateChordRest
//---------------------------------------------------------

void NoteInput::truncateChordRest(Transaction&, Score* score, ChordRest* cr, const Fraction& tick, bool fillWithRest)
{
    MasterScore* ms = score->masterScore();
    IF_ASSERT_FAILED(ms && cr && cr->endTick() > tick) {
        return;
    }

    const Fraction timeStretch = cr->staff()->timeStretch(cr->tick());
    Tuplet* tuplet = cr->tuplet();
    const Fraction overlapDuration = (cr->endTick() - tick) / actualTicks(Fraction(1, 1), tuplet, timeStretch);
    const Fraction desiredDuration = cr->ticks() - overlapDuration;

    ms->changeCRlen(cr, desiredDuration, fillWithRest);

    // Fill the difference with tied notes if necessary...
    const Fraction difference = desiredDuration - cr->ticks();
    if (cr->isChord() && difference.isNotZero()) {
        Fraction startTick = cr->endTick();
        Chord* prevChord = toChord(cr);
        const std::vector<TDuration> durationList = toDurationList(difference, true);
        for (const TDuration& dur : durationList) {
            prevChord = ms->addChord(startTick, dur, prevChord, /*genTie*/ bool(prevChord), prevChord->tuplet());
            startTick += actualTicks(dur.fraction(), prevChord->tuplet(), prevChord->staff()->timeStretch(startTick));
        }
    }
}

//---------------------------------------------------------
//   nextInputPos
//---------------------------------------------------------

void NoteInput::nextInputPos(Transaction&, Score* score, const ChordRest* cr, bool doSelect)
{
    InputState& is = score->inputState();

    ChordRest* ncr = Navigation::nextChordRest(cr);
    if ((!ncr) && (is.track() % VOICES)) {
        Segment* s = score->tick2segment(cr->endTick(), false, SegmentType::ChordRest);
        ncr = s ? toChordRest(s->element(cr->staffIdx() * VOICES)) : nullptr;
    }
    if (ncr) {
        is.setSegment(ncr->segment());
        if (doSelect) {
            score->select(ncr, SelectType::SINGLE, 0);
        }
        for (MuseScoreView* v : score->getViewer()) {
            v->moveCursor();
        }
    }
}

//---------------------------------------------------------
//   addPitch
///   insert note or add note to chord
//---------------------------------------------------------

void NoteInput::addPitch(Transaction& tx, Score* score, const NoteInputParams& params, bool addFlag, bool insert)
{
    InputState& is = score->inputState();
    if (!is.isValid()) {
        LOGD("cannot enter notes here (no chord rest at current position)");
        return;
    }

    is.setRest(false);

    const Drumset* ds = is.drumset();
    if (ds) {
        is.setDrumNote(params.drumPitch);
        is.setVoice(ds->voice(params.drumPitch));
    }

    addPitch(tx, score, params.step, addFlag, insert);
}

void NoteInput::addPitch(Transaction& tx, Score* score, int step, bool addFlag, bool insert)
{
    InputState& is = score->inputState();

    insert = insert || is.usingNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    Position pos;
    if (addFlag) {
        EngravingItem* el = score->selection().element();
        if (el && el->isNote()) {
            Note* selectedNote = toNote(el);
            Chord* chord  = selectedNote->chord();
            Segment* seg  = chord->segment();
            pos.segment   = seg;
            pos.staffIdx  = chord->vStaffIdx();
            ClefType clef = score->staff(pos.staffIdx)->clef(seg->tick());
            pos.line      = relStep(step, clef);
            bool error;
            NoteVal nval = noteValForPosition(score, pos, is.accidentalType(), error);
            if (error) {
                return;
            }
            bool forceAccidental = false;
            if (is.accidentalType() != AccidentalType::NONE) {
                NoteVal nval2 = noteValForPosition(score, pos, AccidentalType::NONE, error);
                forceAccidental = (nval.pitch == nval2.pitch);
            }
            if (is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
                addPitchToChord(tx, score, nval, chord, /* externalInputState */ nullptr, forceAccidental);
            } else {
                addNote(tx, score, chord, nval, forceAccidental, is.articulationIds());
            }
            is.setAccidentalType(AccidentalType::NONE);
            return;
        }
    }

    pos.segment   = is.segment();
    pos.staffIdx  = is.track() / VOICES;
    ClefType clef = score->staff(pos.staffIdx)->clef(pos.segment->tick());
    pos.line      = relStep(step, clef);
    pos.beyondScore = is.beyondScore();

    if (is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
        repitchNote(tx, score, pos, !addFlag);
    } else {
        if (insert) {
            insertChordByInsertingTime(tx, score, pos);
        } else {
            putNote(tx, score, pos, !addFlag);
        }
    }
    is.setAccidentalType(AccidentalType::NONE);
}

//---------------------------------------------------------
//   addFret
///   insert note with given fret on current string
//---------------------------------------------------------

void NoteInput::addFret(Transaction& tx, Score* score, int fret)
{
    InputState& is = score->inputState();
    if (is.track() == muse::nidx) { // invalid state
        return;
    }
    if (!is.segment()) {
        LOGD("cannot enter notes here (no chord rest at current position)");
        return;
    }
    is.setRest(false);
    Position pos;
    pos.segment   = is.segment();
    pos.staffIdx  = is.track() / VOICES;
    pos.line      = score->staff(pos.staffIdx)->staffType(is.tick())->physStringToVisual(is.string());
    pos.fret      = fret;
    pos.beyondScore = is.beyondScore();
    putNote(tx, score, pos, false);
}

//---------------------------------------------------------
//   noteValueChangeAllowed
//    Note value changes are disallowed while a trill cue note is selected outside note entry mode.
//---------------------------------------------------------

bool NoteInput::noteValueChangeAllowed(const Score* score)
{
    if (score->noteEntryMode()) {
        return true;
    }

    for (const ChordRest* cr : score->getSelectedChordRests()) {
        if (cr->isChord() && toChord(cr)->isTrillCueNote()) {
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------
//   durationTooShortForDots
//    A duration cannot hold `dots` augmentation dots when it is one of the `dots` shortest
//    representable durations (the shortest being V_1024TH).
//---------------------------------------------------------

static bool durationTooShortForDots(const TDuration& duration, int dots)
{
    // Ordered shortest-first; a duration cannot hold `dots` dots if it is one of the `dots` shortest.
    static constexpr DurationType shortestTypes[] = {
        DurationType::V_1024TH,
        DurationType::V_512TH,
        DurationType::V_256TH,
        DurationType::V_128TH,
    };

    const int count = std::min(dots, int(std::size(shortestTypes)));
    for (int i = 0; i < count; ++i) {
        if (duration == shortestTypes[i]) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//   setDuration
//    Set the note input duration (and apply it to the selection).
//---------------------------------------------------------

void NoteInput::setDuration(Transaction& tx, Score* score, DurationType duration)
{
    if (!noteValueChangeAllowed(score)) {
        return;
    }

    InputState& is = score->inputState();

    const TDuration oldDuration = is.duration();
    const bool oldRest = is.rest();
    const AccidentalType oldAccidentalType = is.accidentalType();

    is.setDuration(duration);
    is.setDots(0);

    //
    // if in "note enter" mode, reset rest flag
    //
    if (score->noteEntryMode()) {
        if (score->usingNoteEntryMethod(NoteEntryMethod::BY_DURATION) || score->usingNoteEntryMethod(NoteEntryMethod::RHYTHM)) {
            // Preserve the number of dots from the previous duration
            if (oldDuration.dots() > 0) {
                toggleDots(tx, score, oldDuration.dots());
            }

            if (is.rest()) {
                // Enter a rest
                score->setNoteRest(is.segment(), is.track(), NoteVal(), is.duration().fraction());
                is.moveToNextInputPos();
            } else {
                if (score->usingNoteEntryMethod(NoteEntryMethod::RHYTHM)) {
                    const EngravingItem* selectedItem = score->selection().element();
                    if (selectedItem && selectedItem->isNote()) {
                        is.setNotes({ toNote(selectedItem)->noteVal() });
                    }
                }

                if (is.beyondScore()) {
                    score->appendMeasures(1);
                    is.moveToNextInputPos();
                }
                ChordRest* cr = is.cr();
                Chord* chord = nullptr;

                if (cr && cr->isChord() && cr->durationType() == is.duration()) {
                    chord = toChord(cr);
                }

                for (const NoteVal& nval : is.notes()) {
                    if (chord && chord->findNote(nval.pitch)) {
                        is.moveToNextInputPos();
                        continue;
                    }

                    NoteVal copy(nval);
                    const Note* note = nullptr;

                    if (chord) {
                        note = addPitchToChord(tx, score, copy, chord);
                    } else {
                        note = addPitch(tx, score, copy, false /*addFlag*/);
                    }

                    if (note) {
                        chord = note->chord();
                    }
                }
            }
        } else {
            is.setRest(false);
        }
    }

    applyToSelection(score, oldDuration, oldRest, oldAccidentalType, false /*toggleForSelectionOnly*/, false /*restToggle*/);
}

//---------------------------------------------------------
//   toggleRest
//    Toggle the note input rest flag (or turn the selection into rests).
//---------------------------------------------------------

void NoteInput::toggleRest(Transaction& /*tx*/, Score* score)
{
    if (!noteValueChangeAllowed(score)) {
        return;
    }

    InputState& is = score->inputState();

    const TDuration oldDuration = is.duration();
    const bool oldRest = is.rest();
    const AccidentalType oldAccidentalType = is.accidentalType();

    if (score->noteEntryMode()) {
        is.setRest(!is.rest());
        is.setAccidentalType(AccidentalType::NONE);
    } else if (score->selection().isNone()) {
        is.setDuration(DurationType::V_QUARTER);
        is.setRest(true);
    } else {
        for (ChordRest* cr : score->getSelectedChordRests()) {
            if (!cr->isRest()) {
                score->setNoteRest(cr->segment(), cr->track(), NoteVal(), cr->durationTypeTicks(), DirectionV::AUTO, false,
                                   is.articulationIds());
            }
        }
    }

    applyToSelection(score, oldDuration, oldRest, oldAccidentalType, false /*toggleForSelectionOnly*/, true /*restToggle*/);
}

//---------------------------------------------------------
//   toggleDots
//    Toggle `dots` augmentation dots (1..4) on the note input duration and the selection.
//---------------------------------------------------------

void NoteInput::toggleDots(Transaction& /*tx*/, Score* score, int dots, bool toggleForSelectionOnly)
{
    IF_ASSERT_FAILED(dots >= 1 && dots <= 4) {
        return;
    }

    if (!noteValueChangeAllowed(score)) {
        return;
    }

    InputState& is = score->inputState();

    const TDuration oldDuration = is.duration();
    const bool oldRest = is.rest();
    const AccidentalType oldAccidentalType = is.accidentalType();

    if (is.duration().dots() == dots || durationTooShortForDots(is.duration(), dots)) {
        is.setDots(0);
    } else {
        is.setDots(dots);
    }

    applyToSelection(score, oldDuration, oldRest, oldAccidentalType, toggleForSelectionOnly, false /*restToggle*/);
}

//---------------------------------------------------------
//   applyToSelection
//    Shared epilogue for setDuration/toggleRest/toggleDots: apply the (possibly changed) input state
//    duration/dots to the current selection.
//---------------------------------------------------------

void NoteInput::applyToSelection(Score* score, const TDuration& oldDuration, bool oldRest,
                                 AccidentalType oldAccidentalType, bool toggleForSelectionOnly, bool restToggle)
{
    InputState& is = score->inputState();

    if (score->noteEntryMode()) {
        if (!toggleForSelectionOnly || score->selection().isNone()) {
            return;
        }
    }

    std::vector<ChordRest*> crs;
    std::vector<EngravingItem*> elementsToSelect;

    if (score->selection().isSingle()) {
        EngravingItem* e = score->selection().element();
        ChordRest* cr = InputState::chordRest(e);

        // do not allow to add a dot on a full measure rest
        if (cr && cr->isRest()) {
            Rest* r = toRest(cr);
            if (r->isFullMeasureRest()) {
                is.setDots(0);
            }
        }

        // on measure rest, select the first actual rest
        if (cr && cr->isMMRest()) {
            Measure* m = cr->measure()->mmRestFirst();
            if (m) {
                cr = m->findChordRest(m->tick(), 0);
            }
        }

        if (cr) {
            crs.push_back(cr);
        } else {
            score->deselect(e);
        }
    } else if (score->selection().isNone() && !restToggle) {
        TDuration td = is.duration();
        is.setDuration(td);
        is.setAccidentalType(AccidentalType::NONE);
    } else {
        const auto elements = score->selection().uniqueElements();
        for (EngravingItem* e : elements) {
            if (score->selection().isList() || e->isRest() || e->isNote()) {
                elementsToSelect.push_back(e);
                score->deselect(e);
            }
        }
        bool canAdjustLength = true;
        for (EngravingItem* e : elements) {
            ChordRest* cr = InputState::chordRest(e);
            if (!cr) {
                continue;
            }
            if (cr->isMeasureRepeat() || cr->isMMRest()) {
                canAdjustLength = false;
                break;
            }
            crs.push_back(cr);
        }

        if (canAdjustLength) {
            // Change length from last to first chord/rest
            std::sort(crs.begin(), crs.end(), [](const ChordRest* cr1, const ChordRest* cr2) {
                if (cr2->track() == cr1->track()) {
                    return cr2->isBefore(cr1);
                }
                return cr2->track() < cr1->track();
            });
            // Remove duplicates from the list
            crs.erase(std::unique(crs.begin(), crs.end()), crs.end());
        } else {
            crs.clear();
        }
    }

    const int dots = is.duration().dots();

    if (toggleForSelectionOnly && dots > 0 && !crs.empty()) {
        bool shouldRemoveDots = true;
        for (const ChordRest* cr : crs) {
            if (dots != cr->dots()) {
                shouldRemoveDots = false;
                break;
            }
        }

        if (shouldRemoveDots) {
            is.setDots(0);
        }
    }

    for (ChordRest* cr : crs) {
        if (cr->isChord() && (toChord(cr)->isGrace())) {
            //
            // handle appoggiatura and acciaccatura
            //
            score->undoChangeChordRestLen(cr, is.duration());
        } else {
            score->changeCRlen(cr, is.duration());
        }
    }

    if (!elementsToSelect.empty()) {
        std::vector<EngravingItem*> selectList;
        for (EngravingItem* e : elementsToSelect) {
            if (score->canReselectItem(e)) {
                selectList.push_back(e);
            }
        }

        score->select(selectList, SelectType::ADD, 0);
        score->selection().updateSelectedElements();
    }

    if (toggleForSelectionOnly) {
        is.setDuration(oldDuration);
        is.setRest(oldRest);
        is.setAccidentalType(oldAccidentalType);

        if (score->noteEntryMode()) {
            if (is.lastSegment() == is.segment()) {
                is.moveToNextInputPos();
            }
        }
    }
}

void NoteInput::increaseDuration(Transaction& tx, Score* score)
{
    switch (score->inputState().duration().type()) {
// cycle back from longest to shortest?
//          case TDuration::V_LONG:
//                setDuration(tx, score, DurationType::V_128TH);
//                break;
    case DurationType::V_BREVE:
        setDuration(tx, score, DurationType::V_LONG);
        break;
    case DurationType::V_WHOLE:
        setDuration(tx, score, DurationType::V_BREVE);
        break;
    case DurationType::V_HALF:
        setDuration(tx, score, DurationType::V_WHOLE);
        break;
    case DurationType::V_QUARTER:
        setDuration(tx, score, DurationType::V_HALF);
        break;
    case DurationType::V_EIGHTH:
        setDuration(tx, score, DurationType::V_QUARTER);
        break;
    case DurationType::V_16TH:
        setDuration(tx, score, DurationType::V_EIGHTH);
        break;
    case DurationType::V_32ND:
        setDuration(tx, score, DurationType::V_16TH);
        break;
    case DurationType::V_64TH:
        setDuration(tx, score, DurationType::V_32ND);
        break;
    case DurationType::V_128TH:
        setDuration(tx, score, DurationType::V_64TH);
        break;
    case DurationType::V_256TH:
        setDuration(tx, score, DurationType::V_128TH);
        break;
    case DurationType::V_512TH:
        setDuration(tx, score, DurationType::V_256TH);
        break;
    case DurationType::V_1024TH:
        setDuration(tx, score, DurationType::V_512TH);
        break;
    default:
        break;
    }
}

void NoteInput::decreaseDuration(Transaction& tx, Score* score)
{
    switch (score->inputState().duration().type()) {
    case DurationType::V_LONG:
        setDuration(tx, score, DurationType::V_BREVE);
        break;
    case DurationType::V_BREVE:
        setDuration(tx, score, DurationType::V_WHOLE);
        break;
    case DurationType::V_WHOLE:
        setDuration(tx, score, DurationType::V_HALF);
        break;
    case DurationType::V_HALF:
        setDuration(tx, score, DurationType::V_QUARTER);
        break;
    case DurationType::V_QUARTER:
        setDuration(tx, score, DurationType::V_EIGHTH);
        break;
    case DurationType::V_EIGHTH:
        setDuration(tx, score, DurationType::V_16TH);
        break;
    case DurationType::V_16TH:
        setDuration(tx, score, DurationType::V_32ND);
        break;
    case DurationType::V_32ND:
        setDuration(tx, score, DurationType::V_64TH);
        break;
    case DurationType::V_64TH:
        setDuration(tx, score, DurationType::V_128TH);
        break;
    case DurationType::V_128TH:
        setDuration(tx, score, DurationType::V_256TH);
        break;
    case DurationType::V_256TH:
        setDuration(tx, score, DurationType::V_512TH);
        break;
    case DurationType::V_512TH:
        setDuration(tx, score, DurationType::V_1024TH);
        break;
// cycle back from shortest to longest?
//          case DurationType::V_1024TH:
//                setDuration(tx, score, DurationType::V_LONG);
//                break;
    default:
        break;
    }
}

//---------------------------------------------------------
//   realtimeAdvance
//---------------------------------------------------------

void NoteInput::realtimeAdvance(Transaction& tx, Score* score, bool allowTransposition, const std::vector<int>& activeMidiPitches)
{
    InputState& is = score->inputState();
    if (!is.noteEntryMode()) {
        return;
    }

    Fraction ticks2measureEnd = is.segment()->measure()->ticks() - is.segment()->rtick();
    if (!is.cr() || (is.cr()->ticks() != is.duration().fraction() && is.duration() < ticks2measureEnd)) {
        score->setNoteRest(is.segment(), is.track(), NoteVal(), is.duration().fraction(), DirectionV::AUTO);
    }

    ChordRest* prevCR = toChordRest(is.cr());
    if (is.beyondScore()) {
        score->appendMeasures(1);
    }

    is.moveToNextInputPos();

    if (activeMidiPitches.empty()) {
        score->setNoteRest(is.segment(), is.track(), NoteVal(), is.duration().fraction(), DirectionV::AUTO);
    } else {
        Chord* prevChord = prevCR->isChord() ? toChord(prevCR) : 0;
        bool partOfChord = false;
        for (int pitch : activeMidiPitches) {
            NoteInput::addTiedMidiPitch(tx, score, pitch, partOfChord, prevChord, allowTransposition);
            partOfChord = true;
        }
    }

    if (prevCR->measure() != is.segment()->measure()) {
        // just advanced across barline. Now simplify tied notes.
        Regroup::regroupNotesAndRests(tx, score, prevCR->measure()->tick(), is.segment()->measure()->tick(), is.track());
    }
}
