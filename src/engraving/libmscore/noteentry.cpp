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

#include "translation.h"
#include "infrastructure/messagebox.h"

#include "accidental.h"
#include "chord.h"
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
        if (_is.rest()) {
            break;
        }
        const Drumset* ds = instr->drumset();
        nval.pitch        = _is.drumNote();
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
        if (_is.rest()) {
            error = true;
            return nval;
        }
        stringData = instr->stringData();
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
            _is.setString(line);
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
        if (styleB(Sid::concertPitch)) {
            nval.tpc1 = step2tpc(step % 7, acci);
        } else {
            nval.pitch += instr->transpose().chromatic;
            nval.tpc2 = step2tpc(step % 7, acci);
            Interval v = st->part()->instrument(tick)->transpose();
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

//---------------------------------------------------------
//   addPitch
//---------------------------------------------------------

Note* Score::addPitch(NoteVal& nval, bool addFlag, InputState* externalInputState)
{
    InputState& is = externalInputState ? (*externalInputState) : _is;

    if (addFlag) {
        ChordRest* c = toChordRest(is.lastSegment()->element(is.track()));

        if (c == 0 || !c->isChord()) {
            LOGD("Score::addPitch: cr %s", c ? c->typeName() : "zero");
            return 0;
        }
        Note* note = addNote(toChord(c), nval, /* forceAccidental */ false, is.articulationIds(), externalInputState);
        if (is.lastSegment() == is.segment()) {
            NoteEntryMethod entryMethod = is.noteEntryMethod();
            if (entryMethod != NoteEntryMethod::REALTIME_AUTO && entryMethod != NoteEntryMethod::REALTIME_MANUAL) {
                is.moveToNextInputPos();
            }
        }
        return note;
    }
    expandVoice(is.segment(), is.track());

    // insert note
    DirectionV stemDirection = DirectionV::AUTO;
    track_idx_t track = is.track();
    if (is.drumset()) {
        const Drumset* ds = is.drumset();
        nval.headGroup    = ds->noteHead(nval.pitch);
        stemDirection     = ds->stemDirection(nval.pitch);
        track             = ds->voice(nval.pitch) + (is.track() / VOICES) * VOICES;
        is.setTrack(track);
        expandVoice(is.segment(), is.track());
    }

    if (!is.cr()) {
        return 0;
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
            // for single note chords only, preserve ties by changing pitch of all forward notes
            // the tie forward itself will be added later
            // multi-note chords get reduced to single note chords anyhow since we remove the old notes below
            // so there will be no way to preserve those ties
            if (notes.size() == 1 && notes.front()->tieFor()) {
                Note* tn = notes.front()->tieFor()->endNote();
                while (tn) {
                    Chord* tc = tn->chord();
                    if (tc->notes().size() != 1) {
                        undoRemoveElement(tn->tieBack());
                        break;
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
            is.segment(), track, nval, duration, stemDirection, /* forceAccidental */ false, {}, /* rhythmic */ false,
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
        if (e) {
            Fraction stick = Fraction(0, 1);
            EngravingItem* ee = is.slur()->startElement();
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

//---------------------------------------------------------
//   putNote
//    mouse click in state NoteType::ENTRY
//---------------------------------------------------------

Ret Score::putNote(const PointF& pos, bool replace, bool insert)
{
    Position p;
    if (!getPosition(&p, pos, _is.voice())) {
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
            return score->insertChord(p);
        } else {
            return score->putNote(p, replace);
        }
    }

    return make_ok();
}

Ret Score::putNote(const Position& p, bool replace)
{
    Staff* st   = staff(p.staffIdx);
    Segment* s  = p.segment;

    _is.setTrack(p.staffIdx * VOICES + _is.voice());
    _is.setSegment(s);

    if (mu::engraving::Excerpt* excerpt = score()->excerpt()) {
        const TracksMap& tracks = excerpt->tracksMapping();

        if (!tracks.empty() && mu::key(tracks, _is.track(), mu::nidx) == mu::nidx) {
            return make_ret(Ret::Code::UnknownError);
        }
    }

    DirectionV stemDirection = DirectionV::AUTO;
    bool error = false;
    NoteVal nval = noteValForPosition(p, _is.accidentalType(), error);
    if (error) {
        return make_ret(Ret::Code::UnknownError);
    }

    // warn and delete MeasureRepeat if necessary
    Measure* m = _is.segment()->measure();
    staff_idx_t staffIdx = track2staff(_is.track());
    if (m->isMeasureRepeatGroup(staffIdx)) {
        auto b = MessageBox::warning(trc("engraving", "Note input will remove measure repeat"),
                                     trc("engraving", "This measure contains a measure repeat."
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
        stemDirection     = ds->stemDirection(nval.pitch);
        break;
    }
    case StaffGroup::TAB:
        stringData = st->part()->instrument(s->tick())->stringData();
        _is.setDrumNote(-1);
        break;
    case StaffGroup::STANDARD:
        _is.setDrumNote(-1);
        break;
    }

    expandVoice();
    ChordRest* cr = _is.cr();
    bool addToChord = false;

    if (cr) {
        // retrieve total duration of current chord
        TDuration d = cr->durationType();
        // if not in replace mode AND chord duration == input duration AND not rest input
        // we need to add to current chord (otherwise, we will need to replace it or create a new one)
        if (!replace
            && (d == _is.duration())
            && cr->isChord()
            && !_is.rest()) {
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
                        return make_ok();
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
                    return make_ok();
                }
            }
            addToChord = true;                  // if no special case, add note to chord
        }
    }
    bool forceAccidental = false;
    if (_is.accidentalType() != AccidentalType::NONE) {
        NoteVal nval2 = noteValForPosition(p, AccidentalType::NONE, error);
        forceAccidental = (nval.pitch == nval2.pitch);
    }

    Ret ret = make_ok();

    if (addToChord && cr->isChord()) {
        // if adding, add!
        Note* note = addNote(toChord(cr), nval, forceAccidental, _is.articulationIds());
        if (!note) {
            ret = make_ret(Ret::Code::UnknownError);
        }

        _is.setAccidentalType(AccidentalType::NONE);
        return ret;
    } else {
        // if not adding, replace current chord (or create a new one)
        if (_is.rest()) {
            nval.pitch = -1;
        }

        Segment* seg = setNoteRest(_is.segment(), _is.track(), nval,
                                   _is.duration().fraction(), stemDirection, forceAccidental, _is.articulationIds());
        if (!seg) {
            ret = make_ret(Ret::Code::UnknownError);
        }

        _is.setAccidentalType(AccidentalType::NONE);
    }

    if (cr && !st->isTabStaff(cr->tick())) {
        _is.moveToNextInputPos();
    }

    return ret;
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
    AccidentalType at = _is.accidentalType();
    if (_is.drumset() && _is.drumNote() != -1) {
        nval.pitch = _is.drumNote();
    } else {
        AccidentalVal acci
            = (at == AccidentalType::NONE ? s->measure()->findAccidental(s, p.staffIdx, p.line, error) : Accidental::subtype2value(at));
        if (error) {
            return make_ret(Ret::Code::UnknownError);
        }

        int step   = absStep(p.line, clef);
        int octave = step / 7;
        nval.pitch = step2pitch(step) + octave * 12 + int(acci);

        if (styleB(Sid::concertPitch)) {
            nval.tpc1 = step2tpc(step % 7, acci);
        } else {
            nval.pitch += st->part()->instrument(s->tick())->transpose().chromatic;
            nval.tpc2 = step2tpc(step % 7, acci);
        }
    }

    if (!_is.segment()) {
        return make_ret(Ret::Code::UnknownError);
    }

    Chord* chord;
    ChordRest* cr = _is.cr();
    if (!cr) {
        cr = _is.segment()->nextChordRest(_is.track());
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
            _is.moveInputPos(next->segment());
        }
        return make_ok();
    } else {
        chord = toChord(cr);
    }
    Note* note = Factory::createNote(chord);
    note->setParent(chord);
    note->setTrack(chord->track());
    note->setNval(nval);

    Note* firstTiedNote = 0;
    Note* lastTiedNote = note;
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
        // for single note chords only, preserve ties by changing pitch of all forward notes
        // the tie forward itself will be added later
        // multi-note chords get reduced to single note chords anyhow since we remove the old notes below
        // so there will be no way to preserve those ties
        if (notes.size() == 1 && notes.front()->tieFor()) {
            Note* tn = notes.front()->tieFor()->endNote();
            while (tn) {
                Chord* tc = tn->chord();
                if (tc->notes().size() != 1) {
                    undoRemoveElement(tn->tieBack());
                    break;
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
    bool forceAccidental = false;
    if (_is.accidentalType() != AccidentalType::NONE) {
        NoteVal nval2 = noteValForPosition(p, AccidentalType::NONE, error);
        forceAccidental = (nval.pitch == nval2.pitch);
    }
    if (forceAccidental) {
        int tpc = styleB(Sid::concertPitch) ? nval.tpc1 : nval.tpc2;
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
        _is.moveInputPos(next->segment());
    }

    return make_ok();
}

//---------------------------------------------------------
//   insertChord
//---------------------------------------------------------

Ret Score::insertChord(const Position& pos)
{
    // insert
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
    if (_is.insertMode()) {
        globalInsertChord(pos);
    } else {
        localInsertChord(pos);
    }

    return make_ok();
}

//---------------------------------------------------------
//   localInsertChord
//---------------------------------------------------------

void Score::localInsertChord(const Position& pos)
{
    const TDuration duration = _is.duration();
    const Fraction fraction  = duration.fraction();
    const Fraction len       = fraction;
    Segment* seg             = pos.segment;
    Fraction tick            = seg->tick();
    Measure* measure         = seg->measure()->isMMRest() ? seg->measure()->mmRestFirst() : seg->measure();
    const Fraction targetMeasureLen = measure->ticks() + fraction;

    // Shift spanners, enlarge the measure.
    // The approach is similar to that in Measure::adjustToLen() but does
    // insert time to the middle of the measure rather than to the end.
    undoInsertTime(tick, len);
    undo(new InsertTime(this, tick, len));

    for (Score* score : scoreList()) {
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
}

//---------------------------------------------------------
//   globalInsertChord
//---------------------------------------------------------

void Score::globalInsertChord(const Position& pos)
{
    ChordRest* cr = selection().cr();
    track_idx_t track = cr ? cr->track() : mu::nidx;
    deselectAll();
    Segment* s1        = pos.segment;
    Segment* s2        = lastSegment();
    TDuration duration = _is.duration();
    Fraction fraction  = duration.fraction();
    ScoreRange r;

    r.read(s1, s2, false);

    track_idx_t strack = 0;                        // for now for all tracks
    track_idx_t etrack = nstaves() * VOICES;
    Fraction stick  = s1->tick();
    Fraction etick  = s2->tick();
    Fraction ticks  = fraction;
    Fraction len    = r.ticks();

    if (!r.truncate(fraction)) {
        appendMeasures(1);
    }

    putNote(pos, true);
    Fraction dtick = s1->tick() + ticks;
    int voiceOffsets[VOICES] { 0, 0, 0, 0 };
    len = r.ticks();
    for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
        makeGap1(dtick, staffIdx, r.ticks(), voiceOffsets);
    }
    r.write(this, dtick);

    for (auto i :  spanner()) {
        Spanner* s = i.second;
        if (s->track() >= strack && s->track() < etrack) {
            if (s->tick() >= stick && s->tick() < etick) {
                s->undoChangeProperty(Pid::SPANNER_TICK, s->tick() + ticks);
            } else if (s->tick2() >= stick && s->tick2() < etick) {
                s->undoChangeProperty(Pid::SPANNER_TICKS, s->ticks() + ticks);
            }
        }
    }

    if (track != mu::nidx) {
        Measure* m = tick2measure(dtick);
        Segment* s = m->findSegment(SegmentType::ChordRest, dtick);
        EngravingItem* e = s->element(track);
        if (e) {
            select(e->isChord() ? toChord(e)->notes().front() : e);
        }
    }
}
} // namespace mu::engraving
