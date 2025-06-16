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

#include "translation.h"
#include "infrastructure/messagebox.h"

#include "accidental.h"
#include "chord.h"
#include "chordline.h"
#include "drumset.h"
#include "excerpt.h"
#include "factory.h"
#include "masterscore.h"
#include "measure.h"
#include "measurerepeat.h"
#include "navigate.h"
#include "part.h"
#include "range.h"
#include "score.h"
#include "slur.h"
#include "staff.h"
#include "stringdata.h"
#include "tie.h"
#include "tuplet.h"
#include "undo.h"
#include "utils.h"

#include "log.h"

using namespace muse;
using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   noteValForPosition
//---------------------------------------------------------

NoteVal Score::noteValForPosition(Position pos, AccidentalType at, bool& error)
{
    error           = false;
    Segment* s      = pos.segment;
    int line        = pos.line;
    Fraction tick   = s->tick();
    staff_idx_t staffIdx = pos.staffIdx;
    Staff* st       = staff(staffIdx);
    ClefType clef   = st->clef(tick);
    const Instrument* instr = st->part()->instrument(s->tick());
    NoteVal nval;
    const StringData* stringData = 0;

    // pitched/unpitched note entry depends on instrument (override StaffGroup)
    StaffGroup staffGroup = st->staffType(tick)->group();
    if (staffGroup != StaffGroup::TAB) {
        staffGroup = instr->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
    }

    switch (staffGroup) {
    case StaffGroup::PERCUSSION: {
        if (m_is.rest()) {
            break;
        }
        const Drumset* ds = instr->drumset();
        nval.pitch = m_is.drumNote();
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
        if (m_is.rest()) {
            return nval;
        }
        stringData = st->part()->stringData(s->tick(), st->idx());
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
            m_is.setString(line);
            nval.fret = 0;
        }
        // reduce within fret limit
        if (nval.fret > stringData->frets()) {
            nval.fret = stringData->frets();
        }
        // for open strings, only accepts fret 0 (strings in StringData are from bottom to top)
        size_t strgDataIdx = stringData->strings() - line - 1;
        if (nval.fret > 0 && stringData->stringList().at(strgDataIdx).open == true) {
            nval.fret = 0;
        }
        nval.pitch = stringData->getPitch(line, nval.fret, st);
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
        if (style().styleB(Sid::concertPitch)) {
            nval.tpc1 = step2tpc(step % 7, acci);
        } else {
            nval.pitch += instr->transpose().chromatic;
            nval.tpc2 = step2tpc(step % 7, acci);
            Interval v = st->transpose(tick);
            if (v.isZero()) {
                nval.tpc1 = nval.tpc2;
            } else {
                nval.tpc1 = mu::engraving::transposeTpc(nval.tpc2, v, true);
            }
        }
    }
    break;
    }
    return nval;
}

Note* Score::addPitch(NoteVal& nval, bool addFlag, InputState* externalInputState)
{
    InputState& is = externalInputState ? (*externalInputState) : m_is;

    if (addFlag) {
        ChordRest* c = toChordRest(is.lastSegment()->element(is.track()));
        if (!c || !c->isChord()) {
            LOGD("Score::addPitch: cr %s", c ? c->typeName() : "zero");
            return nullptr;
        }

        return addPitchToChord(nval, toChord(c), externalInputState);
    }

    expandVoice(is.segment(), is.track());

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
        expandVoice(is.segment(), is.track());
    }

    if (!is.cr()) {
        handleOverlappingChordRest(is);
    }

    Measure* measure = is.segment()->measure();
    if (measure->isMeasureRepeatGroup(track2staff(track))) {
        MeasureRepeat* mr = measure->measureRepeatElement(track2staff(track));
        deleteItem(mr); // resets any measures related to mr
    }
    Fraction duration;
    if (is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
        duration = is.cr()->ticks();
    } else if (is.usingNoteEntryMethod(NoteEntryMethod::REALTIME_AUTO) || is.usingNoteEntryMethod(NoteEntryMethod::REALTIME_MANUAL)) {
        // FIXME: truncate duration at barline in real-time modes.
        //   The user might try to enter a duration that is too long to fit in the remaining space in the measure.
        //   We could split the duration at the barline and continue into the next bar, but this would create extra
        //   notes, extra ties, and extra pain. Instead, we simply truncate the duration at the barline.
        Fraction ticks2measureEnd = is.segment()->measure()->ticks() - is.segment()->rtick();
        duration = is.duration().fraction() > ticks2measureEnd ? ticks2measureEnd : is.duration().fraction();
    } else {
        duration = is.duration().fraction();
    }
    Note* note = 0;
    Note* firstTiedNote = 0;
    Note* lastTiedNote = 0;
    if (is.usingNoteEntryMethod(NoteEntryMethod::REPITCH) && is.cr()->isChord()) {
        // repitch mode for MIDI input (where we are given a pitch) is handled here
        // for keyboard input (where we are given a staff position), there is a separate function Score::repitchNote()
        // the code is similar enough that it could possibly be refactored
        Chord* chord = toChord(is.cr());
        note = Factory::createNote(chord);
        note->setParent(chord);
        note->setTrack(chord->track());
        note->setNval(nval);
        lastTiedNote = note;
        if (!addFlag) {
            std::vector<Note*> notes = chord->notes();
            // break all ties into current chord
            // these will exist only if user explicitly moved cursor to a tied-into note
            // in ordinary use, cursor will automatically skip past these during note entry
            for (Note* n : notes) {
                if (n->tieBack()) {
                    undoRemoveElement(n->tieBack());
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
                            undoRemoveElement(n);
                        }
                        assert(tc->notes().size() == 1 && tc->notes().front() == tn);
                    }
                    if (!firstTiedNote) {
                        firstTiedNote = tn;
                    }
                    lastTiedNote = tn;
                    undoChangePitch(tn, note->pitch(), note->tpc1(), note->tpc2());
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
                undoRemoveElement(chord->notes().front());
            }
        }
        // add new note to chord
        undoAddElement(note);
        setPlayNote(true);
        // recreate tie forward if there is a note to tie to
        // one-sided ties will not be recreated
        if (firstTiedNote) {
            Tie* tie = Factory::createTie(note);
            tie->setStartNote(note);
            tie->setEndNote(firstTiedNote);
            tie->setTick(tie->startNote()->tick());
            tie->setTick2(tie->endNote()->tick());
            tie->setTrack(note->track());
            undoAddElement(tie);
        }
        select(lastTiedNote);
    } else if (!is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
        Segment* seg = setNoteRest(
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
        ChordRest* e = searchNote(is.tick(), is.track());
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
        ChordRest* next = lastTiedNote ? nextChordRest(lastTiedNote->chord()) : nextChordRest(is.cr());
        while (next && !next->isChord()) {
            next = nextChordRest(next);
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

Note* Score::addPitchToChord(NoteVal& nval, Chord* chord, InputState* externalInputState)
{
    IF_ASSERT_FAILED(chord) {
        return nullptr;
    }

    InputState& is = externalInputState ? (*externalInputState) : m_is;

    auto isTied = [](const Chord* ch) {
        if (ch->notes().empty()) {
            return false;
        }
        Note* n = ch->notes().at(0);
        return n->tieFor() || n->tieBack();
    };

    Note* note = nullptr;
    if (isTied(chord)) {
        note = addNoteToTiedChord(chord, nval, /* forceAccidental */ false);
        if (!note) {
            note = addNote(chord, nval, /* forceAccidental */ false, /* articulationIds */ {}, externalInputState);
        }
    } else {
        note = addNote(chord, nval, /* forceAccidental */ false, is.articulationIds(), externalInputState);
    }

    if (is.usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
        // move cursor to next note
        ChordRest* next = nextChordRest(note->chord());
        while (next && !next->isChord()) {
            next = nextChordRest(next);
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
//   putNote
//    mouse click in state NoteType::ENTRY
//---------------------------------------------------------

Ret Score::putNote(const PointF& pos, bool replace, bool insert)
{
    Position p;
    if (!getPosition(&p, pos, m_is.voice())) {
        LOGD("cannot put note here, get position failed");
        return make_ret(Ret::Code::UnknownError);
    }

    Score* score = p.segment->score();
    // it is not safe to call Score::repitchNote() if p is on a TAB staff
    bool isTablature = staff(p.staffIdx)->isTabStaff(p.segment->tick());

    //  calculate actual clicked line from staffType offset and stepOffset
    Staff* ss = score->staff(p.staffIdx);
    int stepOffset = ss->staffType(p.segment->tick())->stepOffset();
    double stYOffset = ss->staffType(p.segment->tick())->yoffset().val();
    double lineDist = ss->staffType(p.segment->tick())->lineDistance().val();
    p.line -= stepOffset + 2 * stYOffset / lineDist;

    if (score->inputState().usingNoteEntryMethod(NoteEntryMethod::REPITCH) && !isTablature) {
        return score->repitchNote(p, replace);
    } else {
        if (insert || score->inputState().usingNoteEntryMethod(NoteEntryMethod::TIMEWISE)) {
            return score->insertChordByInsertingTime(p);
        } else {
            return score->putNote(p, replace);
        }
    }
}

Ret Score::putNote(const Position& p, bool replace)
{
    Staff* st   = staff(p.staffIdx);
    Segment* s  = p.segment;

    m_is.setTrack(p.staffIdx * VOICES + m_is.voice());
    m_is.setSegment(s);

    if (mu::engraving::Excerpt* excerpt = score()->excerpt()) {
        const TracksMap& tracks = excerpt->tracksMapping();

        if (!tracks.empty() && muse::key(tracks, m_is.track(), muse::nidx) == muse::nidx) {
            return make_ret(Ret::Code::UnknownError);
        }
    }

    DirectionV stemDirection = DirectionV::AUTO;
    bool error = false;
    NoteVal nval = noteValForPosition(p, m_is.accidentalType(), error);
    if (error) {
        return make_ret(Ret::Code::UnknownError);
    }

    // warn and delete MeasureRepeat if necessary
    Measure* m = m_is.segment()->measure();
    staff_idx_t staffIdx = track2staff(m_is.track());
    if (m->isMeasureRepeatGroup(staffIdx)) {
        auto b = MessageBox(iocContext()).warning(muse::trc("engraving", "Note input will remove measure repeat"),
                                                  muse::trc("engraving", "This measure contains a measure repeat."
                                                                         " If you enter notes here, it will be deleted."
                                                                         " Do you want to continue?"));
        if (b == MessageBox::Cancel) {
            return make_ret(Ret::Code::Cancel);
        }

        Score::deleteItem(m->measureRepeatElement(staffIdx));
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
            m_is.setVoice(ds->voice(nval.pitch));
        }
        break;
    }
    case StaffGroup::TAB:
        stringData = st->part()->stringData(s->tick(), st->idx());
        m_is.setDrumNote(-1);
        break;
    case StaffGroup::STANDARD:
        m_is.setDrumNote(-1);
        break;
    }

    expandVoice();

    // If there's an overlapping ChordRest at the current input position, shorten it...
    if (!m_is.cr()) {
        handleOverlappingChordRest(m_is);
    }

    ChordRest* cr = m_is.cr();

    auto checkTied = [&]() {
        if (!cr || !cr->isChord()) {
            return false;
        }
        auto ch = toChord(cr);
        return !ch->notes().empty() && !ch->notes()[0]->tieBack() && ch->notes()[0]->tieFor();
    };

    bool addToChord = false;
    bool shouldAddAsTied = checkTied();

    if (cr) {
        // retrieve total duration of current chord
        TDuration d = cr->durationType();
        // if not in replace mode AND chord duration == input duration AND not rest input
        // we need to add to current chord (otherwise, we will need to replace it or create a new one)
        if (!replace
            && ((d == m_is.duration()) || shouldAddAsTied)
            && cr->isChord()
            && !m_is.rest()) {
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
                        undoChangeFretting(note, nval.pitch, nval.string, nval.fret, tpc1, tpc2);
                        setPlayNote(true);
                        return muse::make_ok();
                    }
                }
            } else {                            // not TAB
                // if a note with the same pitch already exists in the chord, remove it
                Chord* chord = toChord(cr);
                Note* note = chord->findNote(nval.pitch);
                if (note) {
                    if (chord->notes().size() > 1) {
                        undoRemoveElement(note);
                    }
                    return muse::make_ok();
                }
            }
            addToChord = true;                  // if no special case, add note to chord
        }
    }
    bool forceAccidental = false;
    if (m_is.accidentalType() != AccidentalType::NONE) {
        NoteVal nval2 = noteValForPosition(p, AccidentalType::NONE, error);
        forceAccidental = (nval.pitch == nval2.pitch);
    }

    Ret ret = muse::make_ok();

    if (addToChord && cr->isChord()) {
        // if adding, add!
        Chord* chord = toChord(cr);
        if (shouldAddAsTied) {
            Note* n = addNoteToTiedChord(chord, nval, forceAccidental, m_is.articulationIds());
            if (!n) {
                ret = make_ret(Ret::Code::UnknownError);
            }

            m_is.setAccidentalType(AccidentalType::NONE);
            return ret;
        }
        Note* note = addNote(chord, nval, forceAccidental, m_is.articulationIds());
        if (!note) {
            ret = make_ret(Ret::Code::UnknownError);
        }

        m_is.setAccidentalType(AccidentalType::NONE);
        return ret;
    } else {
        // if not adding, replace current chord (or create a new one)
        if (m_is.rest()) {
            nval.pitch = -1;
        }

        Segment* seg = setNoteRest(m_is.segment(), m_is.track(), nval,
                                   m_is.duration().fraction(), stemDirection, forceAccidental, m_is.articulationIds());
        if (!seg) {
            ret = make_ret(Ret::Code::UnknownError);
        }

        m_is.setAccidentalType(AccidentalType::NONE);
    }

    if (cr && !st->isTabStaff(cr->tick())) {
        m_is.moveToNextInputPos();
    }

    return ret;
}

void Score::handleOverlappingChordRest(InputState& inputState)
{
    MasterScore* ms = masterScore();
    ChordRest* prevCr = inputState.segment()->nextChordRest(inputState.track(), /*backwards*/ true, /*stopAtMeasureBoundary*/ true);
    if (prevCr && prevCr->endTick() > inputState.tick()) {
        const Fraction overlapDuration = prevCr->endTick() - inputState.tick();
        const Fraction desiredDuration = prevCr->ticks() - overlapDuration;

        const InputState inputStateToRestore = inputState; // because changeCRlen will alter the input state
        ms->changeCRlen(prevCr, desiredDuration, /*fillWithRest*/ true);

        // Fill the difference with tied notes if necessary...
        const Fraction difference = desiredDuration - prevCr->ticks();
        if (prevCr->isChord() && difference.isNotZero()) {
            Fraction startTick = prevCr->endTick();
            Chord* prevChord = toChord(prevCr);
            const std::vector<TDuration> durationList = toDurationList(difference, true);
            for (const TDuration& dur : durationList) {
                prevChord = ms->addChord(startTick, dur, prevChord, /*genTie*/ bool(prevChord), prevChord->tuplet());
                startTick += dur.fraction();
            }
        }

        inputState = inputStateToRestore;
    }
}

//---------------------------------------------------------
//   repitchNote
//---------------------------------------------------------

Ret Score::repitchNote(const Position& p, bool replace)
{
    Segment* s      = p.segment;
    Fraction tick   = s->tick();
    Staff* st       = staff(p.staffIdx);
    ClefType clef   = st->clef(tick);

    NoteVal nval;
    bool error = false;
    AccidentalType at = m_is.accidentalType();
    if (m_is.drumset() && m_is.drumNote() != -1) {
        nval.pitch = m_is.drumNote();
    } else {
        AccidentalVal acci
            = (at == AccidentalType::NONE ? s->measure()->findAccidental(s, p.staffIdx, p.line, error) : Accidental::subtype2value(at));
        if (error) {
            return make_ret(Ret::Code::UnknownError);
        }

        int step   = absStep(p.line, clef);
        int octave = step / 7;
        nval.pitch = step2pitch(step) + octave * 12 + int(acci);

        if (style().styleB(Sid::concertPitch)) {
            nval.tpc1 = step2tpc(step % 7, acci);
        } else {
            nval.pitch += st->part()->instrument(s->tick())->transpose().chromatic;
            nval.tpc2 = step2tpc(step % 7, acci);
        }
    }

    if (!m_is.segment()) {
        return make_ret(Ret::Code::UnknownError);
    }

    Chord* chord;
    ChordRest* cr = m_is.cr();
    if (!cr) {
        cr = m_is.segment()->nextChordRest(m_is.track());
        if (!cr) {
            return make_ret(Ret::Code::UnknownError);
        }
    }
    if (cr->isRest()) {   //skip rests
        ChordRest* next = nextChordRest(cr);
        while (next && !next->isChord()) {
            next = nextChordRest(next);
        }
        if (next) {
            m_is.moveInputPos(next->segment());
        }
        return muse::make_ok();
    } else {
        chord = toChord(cr);
    }
    Note* note = Factory::createNote(chord);
    note->setParent(chord);
    note->setTrack(chord->track());
    note->setNval(nval);

    Note* firstTiedNote = 0;
    Note* lastTiedNote = note;
    ChordLine* chordLine = nullptr;
    if (replace) {
        std::vector<Note*> notes = chord->notes();
        // break all ties into current chord
        // these will exist only if user explicitly moved cursor to a tied-into note
        // in ordinary use, cursor will automatically skip past these during note entry
        for (Note* n : notes) {
            if (n->tieBack()) {
                undoRemoveElement(n->tieBack());
            }
        }
        // Keep first chordline only
        chordLine = chord->chordLine() ? chord->chordLine()->clone() : nullptr;
        std::vector<EngravingItem*> chordEls = chord->el();
        for (EngravingItem* e : chordEls) {
            if (e->isChordLine()) {
                undoRemoveElement(e);
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
                        undoRemoveElement(n);
                    }
                    assert(tc->notes().size() == 1 && tc->notes().front() == tn);
                }
                if (!firstTiedNote) {
                    firstTiedNote = tn;
                }
                lastTiedNote = tn;
                undoChangePitch(tn, note->pitch(), note->tpc1(), note->tpc2());
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
            undoRemoveElement(chord->notes().front());
        }
    }
    // add new note to chord
    undoAddElement(note);

    if (chordLine) {
        chordLine->setNote(note);
        undoAddElement(chordLine);
    }

    bool forceAccidental = false;
    if (m_is.accidentalType() != AccidentalType::NONE) {
        NoteVal nval2 = noteValForPosition(p, AccidentalType::NONE, error);
        forceAccidental = (nval.pitch == nval2.pitch);
    }
    if (forceAccidental) {
        int tpc = style().styleB(Sid::concertPitch) ? nval.tpc1 : nval.tpc2;
        AccidentalVal alter = tpc2alter(tpc);
        at = Accidental::value2subtype(alter);
        Accidental* a = Factory::createAccidental(note);
        a->setAccidentalType(at);
        a->setRole(AccidentalRole::USER);
        a->setParent(note);
        undoAddElement(a);
    }
    setPlayNote(true);
    setPlayChord(true);
    // recreate tie forward if there is a note to tie to
    // one-sided ties will not be recreated
    if (firstTiedNote) {
        Tie* tie = Factory::createTie(note);
        tie->setStartNote(note);
        tie->setEndNote(firstTiedNote);
        tie->setTick(tie->startNote()->tick());
        tie->setTick2(tie->endNote()->tick());
        tie->setTrack(note->track());
        undoAddElement(tie);
    }
    select(lastTiedNote);
    // move to next Chord
    ChordRest* next = nextChordRest(lastTiedNote->chord());
    while (next && !next->isChord()) {
        next = nextChordRest(next);
    }
    if (next) {
        m_is.moveInputPos(next->segment());
    }

    return muse::make_ok();
}

//---------------------------------------------------------
//   insertChordByInsertingTime
//---------------------------------------------------------

Ret Score::insertChordByInsertingTime(const Position& pos)
{
    // insert by increasing measure length
    //
    // TODO:
    //    - check voices
    //    - split chord/rest

    EngravingItem* el = selection().element();
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
            undoRemoveElement(t);
        }
    }

    const TDuration duration = m_is.duration();
    const Fraction fraction  = duration.fraction();
    const Fraction len       = fraction;
    Fraction tick            = seg->tick();
    Measure* measure         = seg->measure()->isMMRest() ? seg->measure()->mmRestFirst() : seg->measure();
    const Fraction targetMeasureLen = measure->ticks() + fraction;

    // Shift spanners, enlarge the measure.
    // The approach is similar to that in Measure::adjustToLen() but does
    // insert time to the middle of the measure rather than to the end.
    undoInsertTime(tick, len);

    for (Score* score : scoreList()) {
        score->undo(new InsertTime(score, tick, len));
        Measure* m = score->tick2measure(tick);
        undo(new ChangeMeasureLen(m, targetMeasureLen));
        Segment* scoreSeg = m->tick2segment(tick);
        for (Segment* s = scoreSeg; s; s = s->next()) {
            s->undoChangeProperty(Pid::TICK, s->rtick() + len);
        }
    }

    // Fill the inserted time with rests.
    // This is better to be done in master score to cover all staves.
    MasterScore* ms = masterScore();
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
            if (cr && cr->tick() < tick && (cr->tick() + cr->actualTicks()) > tick) {
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
    putNote(p, true);

    return muse::make_ok();
}
} // namespace mu::engraving
