/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "editnote.h"
#include "editchord.h"

#include <set>

#include "dom/accidental.h"
#include "dom/articulation.h"
#include "dom/chord.h"
#include "dom/drumset.h"
#include "dom/factory.h"
#include "dom/linkedobjects.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/ornament.h"
#include "dom/part.h"
#include "dom/score.h"
#include "dom/staff.h"
#include "dom/stringdata.h"
#include "dom/tapping.h"
#include "dom/utils.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   toggleOrnament
//---------------------------------------------------------

void EditNote::toggleOrnament(Score* score, SymId attr)
{
    std::set<Chord*> set;
    for (EngravingItem* el : score->selection().elements()) {
        if (el->isNote() || el->isChord()) {
            Chord* cr = 0;
            // apply articulation on a given chord only once
            if (el->isNote()) {
                cr = toNote(el)->chord();
                if (muse::contains(set, cr)) {
                    continue;
                }
            }
            Ornament* na = Factory::createOrnament(score->dummy()->chord());
            na->setSymId(attr);
            if (!EditChord::toggleArticulation(score, el, na)) {
                delete na;
            }

            if (cr) {
                set.insert(cr);
            }
        }
    }
}

//---------------------------------------------------------
//   toggleAccidental
//---------------------------------------------------------

void EditNote::toggleAccidental(Score* score, AccidentalType at)
{
    bool applyNaturalToInputNotes = false;
    if (score->inputState().accidentalType() == at && at != AccidentalType::NONE) {
        at = AccidentalType::NONE; // NONE also means "search for previous accidental and use it if found"
        applyNaturalToInputNotes = true;
    }

    if (score->noteEntryMode()) {
        score->inputState().setAccidentalType(at);
        score->inputState().setRest(false);

        if (!score->inputState().notes().empty()) {
            EditNote::applyAccidentalToInputNotes(score,
                                                  applyNaturalToInputNotes ? AccidentalType::NATURAL : at);
        }
    } else {
        if (score->selection().isNone()) {
            score->inputState().setAccidentalType(at);
            score->inputState().setDuration(DurationType::V_QUARTER);
            score->inputState().setRest(false);
        } else {
            EditNote::changeAccidental(score, at);
        }
    }
}

//---------------------------------------------------------
//   applyAccidentalToInputNotes
//---------------------------------------------------------

void EditNote::applyAccidentalToInputNotes(Score* score, AccidentalType accidentalType)
{
    NoteValList notes;
    notes.reserve(score->inputState().notes().size());

    Position pos;
    pos.segment = score->inputState().segment();
    pos.staffIdx = score->inputState().staffIdx();

    for (const NoteVal& oldVal : score->inputState().notes()) {
        pos.line = noteValToLine(oldVal, score->inputState().staff(), score->inputState().tick());

        bool error = false;
        const NoteVal newVal = score->noteValForPosition(pos, accidentalType, error);

        if (error) {
            notes.push_back(oldVal);
        } else {
            notes.push_back(newVal);
        }
    }

    score->inputState().setNotes(notes);
}

//---------------------------------------------------------
//   changeAccidental
///   Change accidental to subtype \a idx for all selected
///   notes.
//---------------------------------------------------------

void EditNote::changeAccidental(Score* score, AccidentalType idx)
{
    for (EngravingItem* item : score->selection().elements()) {
        switch (item->type()) {
        case ElementType::ACCIDENTAL: {
            Accidental* accidental = toAccidental(item);
            if (accidental->accidentalType() == idx) {
                EditNote::changeAccidental(score, accidental->note(), AccidentalType::NONE);
            } else {
                EditNote::changeAccidental(score, accidental->note(), idx);
            }
            break;
        }
        case ElementType::NOTE:
            EditNote::changeAccidental(score, toNote(item), idx);
            break;
        default:
            break;
        }
    }
}

//---------------------------------------------------------
//   changeAccidental2
//---------------------------------------------------------

void EditNote::changeAccidental2(Note* n, int pitch, int tpc)
{
    Score* score  = n->score();
    Chord* chord  = n->chord();
    Staff* st     = chord->staff();
    int fret      = n->fret();
    int string    = n->string();

    if (st->isTabStaff(chord->tick())) {
        if (pitch != n->pitch()) {
            //
            // as pitch has changed, calculate new
            // string & fret
            //
            const StringData* stringData = n->part()->stringData(n->tick(), st->idx());
            if (stringData) {
                stringData->convertPitch(pitch, st, &string, &fret);
            }
        }
    }
    int tpc1;
    int tpc2 = n->transposeTpc(tpc);
    if (n->style().styleB(Sid::concertPitch)) {
        tpc1 = tpc;
    } else {
        tpc1 = tpc2;
        tpc2 = tpc;
    }

    if (!st->isTabStaff(chord->tick())) {
        //
        // handle ties
        //
        if (n->tieBack()) {
            if (pitch != n->pitch()) {
                score->undoRemoveElement(n->tieBack());
                if (n->tieFor()) {
                    score->undoRemoveElement(n->tieFor());
                }
            }
        } else {
            Note* nn = n;
            while (nn && nn->tieFor()) {
                nn = nn->tieFor()->endNote();
                if (nn) {
                    score->undo(new ChangePitch(nn, pitch, tpc1, tpc2));
                }
            }
        }
    }
    undoChangePitch(score, n, pitch, tpc1, tpc2);
}

//---------------------------------------------------------
//   changeAccidental
///   Change accidental to subtype \accidental for
///   note \a note.
//---------------------------------------------------------

void EditNote::changeAccidental(Score* score, Note* note, AccidentalType accidental)
{
    Chord* chord = note ? note->chord() : nullptr;
    if (!chord) {
        return;
    }
    Segment* segment = chord->segment();
    if (!segment) {
        return;
    }
    Measure* measure = segment->measure();
    if (!measure) {
        return;
    }
    Fraction tick = segment->tick();
    Staff* estaff = score->staff(chord->vStaffIdx());
    if (!estaff) {
        return;
    }
    ClefType clef = estaff->clef(tick);
    if (clef == ClefType::TAB
        || clef == ClefType::TAB4
        || clef == ClefType::TAB_SERIF
        || clef == ClefType::TAB4_SERIF) {
        return;
    }
    int step      = ClefInfo::pitchOffset(clef) - note->line();
    while (step < 0) {
        step += 7;
    }
    step %= 7;
    //
    // accidental change may result in pitch change
    //
    AccidentalVal acc2 = measure->findAccidental(note);
    AccidentalVal acc = (accidental == AccidentalType::NONE) ? acc2 : Accidental::subtype2value(accidental);

    int pitch = line2pitch(note->line(), clef, Key::C) + int(acc);
    if (!note->concertPitch()) {
        pitch += note->transposition();
    }

    int tpc = step2tpc(step, acc);

    bool forceRemove = false;
    bool forceAdd = false;

    // delete accidental
    // both for this note and for any linked notes
    if (accidental == AccidentalType::NONE) {
        forceRemove = true;
    }
    // precautionary or microtonal accidental
    // either way, we display it unconditionally
    // both for this note and for any linked notes
    else if (acc == acc2 || (pitch == note->pitch() && !Accidental::isMicrotonal(note->accidentalType()))
             || Accidental::isMicrotonal(accidental)) {
        forceAdd = true;
    }

    for (EngravingObject* se : note->linkList()) {
        Note* ln = toNote(se);
        if (ln->concertPitch() != note->concertPitch()) {
            continue;
        }
        Score* lns    = ln->score();
        Accidental* a = ln->accidental();
        if (forceRemove) {
            if (a) {
                lns->undoRemoveElement(a);
            }
            if (ln->tieBack()) {
                continue;
            }
        } else if (forceAdd) {
            if (a) {
                score->undoRemoveElement(a);
            }
            Accidental* a1 = Factory::createAccidental(ln);
            a1->setParent(ln);
            a1->setAccidentalType(accidental);
            a1->setRole(AccidentalRole::USER);
            lns->undoAddElement(a1);
        } else if (a && Accidental::isMicrotonal(a->accidentalType())) {
            lns->undoRemoveElement(a);
        }
        changeAccidental2(ln, pitch, tpc);
    }
    score->setPlayNote(true);
    score->setSelectionChanged(true);
}

//---------------------------------------------------------
//   upDownChromatic
//---------------------------------------------------------

void EditNote::upDownChromatic(bool up, int pitch, Note* n, Key key, int tpc1, int tpc2,
                               int& newPitch, int& newTpc1, int& newTpc2)
{
    bool concertPitch = n->concertPitch();
    AccidentalVal noteAccVal = tpc2alter(concertPitch ? tpc1 : tpc2);
    AccidentalVal accState = AccidentalVal::NATURAL;
    if (Measure* m = n->findMeasure()) {
        accState = m->findAccidental(n);
    }
    if (up && pitch < 127) {
        newPitch = pitch + 1;
        if (concertPitch) {
            if (tpc1 > Tpc::TPC_A + int(key) && noteAccVal >= accState) {
                newTpc1 = tpc1 - 5;           // up semitone diatonic
            } else {
                newTpc1 = tpc1 + 7;           // up semitone chromatic
            }
            newTpc2 = n->transposeTpc(newTpc1);
        } else {
            if (tpc2 > Tpc::TPC_A + int(key) && noteAccVal >= accState) {
                newTpc2 = tpc2 - 5;           // up semitone diatonic
            } else {
                newTpc2 = tpc2 + 7;           // up semitone chromatic
            }
            newTpc1 = n->transposeTpc(newTpc2);
        }
    } else if (!up && pitch > 0) {
        newPitch = pitch - 1;
        if (concertPitch) {
            if (tpc1 > Tpc::TPC_C + int(key) || noteAccVal > accState) {
                newTpc1 = tpc1 - 7;           // down semitone chromatic
            } else {
                newTpc1 = tpc1 + 5;           // down semitone diatonic
            }
            newTpc2 = n->transposeTpc(newTpc1);
        } else {
            if (tpc2 > Tpc::TPC_C + int(key) || noteAccVal > accState) {
                newTpc2 = tpc2 - 7;           // down semitone chromatic
            } else {
                newTpc2 = tpc2 + 5;           // down semitone diatonic
            }
            newTpc1 = n->transposeTpc(newTpc2);
        }
    }
}

//---------------------------------------------------------
//   upDown
//---------------------------------------------------------

void EditNote::upDown(Score* score, bool up, UpDownMode mode)
{
    std::list<Note*> el = score->selection().uniqueNotes();

    el.sort([up](Note* a, Note* b) {
        if (up) {
            return a->string() < b->string();
        } else {
            return a->string() > b->string();
        }
    });

    for (Note* oNote : el) {
        Fraction tick     = oNote->chord()->tick();
        Staff* staff = oNote->staff();
        Part* part   = staff->part();
        Key key      = staff->key(tick);
        int tpc1     = oNote->tpc1();
        int tpc2     = oNote->tpc2();
        int pitch    = oNote->pitch();
        int pitchOffset = staff->pitchOffset(tick);
        int newTpc1  = tpc1;          // default to unchanged
        int newTpc2  = tpc2;          // default to unchanged
        int newPitch = pitch;         // default to unchanged
        int string   = oNote->string();
        int fret     = oNote->fret();

        StaffGroup staffGroup = staff->staffType(oNote->chord()->tick())->group();
        // if not tab, check for instrument instead of staffType (for pitched to unpitched instrument changes)
        if (staffGroup != StaffGroup::TAB) {
            staffGroup = staff->part()->instrument(oNote->tick())->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
        }

        switch (staffGroup) {
        case StaffGroup::PERCUSSION:
        {
            const Drumset* ds = part->instrument(tick)->drumset();
            if (ds) {
                newPitch = up ? ds->nextPitch(pitch) : ds->prevPitch(pitch);
                newTpc1 = pitch2tpc(newPitch, Key::C, Prefer::NEAREST);
                newTpc2 = newTpc1;
            }
        }
        break;
        case StaffGroup::TAB:
        {
            const StringData* stringData = part->stringData(tick, staff->idx());
            switch (mode) {
            case UpDownMode::OCTAVE:
            {
                const StaffType* stt = staff->staffType(tick);
                string = stt->physStringToVisual(string);
                string += (up ? -1 : 1);
                if (string < 0 || string >= static_cast<int>(stringData->strings())) {
                    return;                                 // no next string to move to
                }
                string = stt->visualStringToPhys(string);
                fret = stringData->fret(pitch, string, staff, tick);
                if (fret == -1) {
                    return;
                }
            }
            break;

            case UpDownMode::DIATONIC:
                upDownChromatic(up, pitch, oNote, key, tpc1, tpc2, newPitch, newTpc1, newTpc2);
                break;

            case UpDownMode::CHROMATIC:
            {
                if (!stringData->frets()) {
                    LOGD("upDown tab chromatic: no frets?");
                    return;
                }
                fret += (up ? 1 : -1);
                if (fret < 0 || fret > stringData->frets()) {
                    LOGD("upDown tab in-string: out of fret range");
                    return;
                }
                upDownChromatic(up, pitch, oNote, key, tpc1, tpc2, newPitch, newTpc1, newTpc2);
                if (newPitch + pitchOffset != stringData->getPitch(string, fret, staff, oNote->tick()) && !oNote->bendBack()) {
                    LOGD("upDown tab in-string: pitch mismatch");
                    return;
                }
                oNote->undoChangeProperty(Pid::FRET, fret);
            }
            break;
            }
        }
        break;
        case StaffGroup::STANDARD:
            switch (mode) {
            case UpDownMode::OCTAVE:
                if (up) {
                    if (pitch < 116) {
                        newPitch = pitch + 12;
                    }
                } else {
                    if (pitch > 11) {
                        newPitch = pitch - 12;
                    }
                }
                break;

            case UpDownMode::CHROMATIC:
                upDownChromatic(up, pitch, oNote, key, tpc1, tpc2, newPitch, newTpc1, newTpc2);
                break;

            case UpDownMode::DIATONIC:
            {
                Note* firstTiedNote = oNote->firstTiedNote();
                int newLine = firstTiedNote->line() + (up ? -1 : 1);
                Staff* vStaff = score->staff(firstTiedNote->chord()->vStaffIdx());

                bool error = false;
                AccidentalVal accOffs = firstTiedNote->chord()->measure()->findAccidental(
                    firstTiedNote->chord()->segment(), firstTiedNote->chord()->vStaffIdx(), newLine, error);
                if (error) {
                    accOffs = Accidental::subtype2value(AccidentalType::NONE);
                }
                int nStep = absStep(newLine, vStaff->clef(tick));
                int octave = nStep / 7;
                int testPitch = step2pitch(nStep) + octave * 12 + int(accOffs);

                if (testPitch <= 127 && testPitch > 0) {
                    newPitch = testPitch;
                    newTpc1 = newTpc2 = step2tpc(nStep % 7, accOffs);
                    if (firstTiedNote->concertPitch()) {
                        newTpc2 = firstTiedNote->transposeTpc(newTpc1);
                    } else {
                        newPitch += vStaff->transpose(tick).chromatic;
                        newTpc1 = firstTiedNote->transposeTpc(newTpc2);
                    }
                }
            }
            break;
            }
            break;
        }

        if ((oNote->pitch() != newPitch) || (oNote->tpc1() != newTpc1) || oNote->tpc2() != newTpc2) {
            if (mode != UpDownMode::OCTAVE) {
                auto l = oNote->linkList();
                for (EngravingObject* e : l) {
                    Note* ln = toNote(e);
                    if (ln->accidental()) {
                        score->doUndoRemoveElement(ln->accidental());
                    }
                }
            }
            EditNote::undoChangePitch(score, oNote, newPitch, newTpc1, newTpc2);
            if (mode == UpDownMode::DIATONIC) {
                part->stringData(tick, staff->idx())->convertPitch(newPitch, staff, tick, &string, &fret);
                EditNote::undoChangeFretting(score, oNote, newPitch, string, fret, newTpc1, newTpc2);
            }
        } else if (staff->staffType(tick)->group() == StaffGroup::TAB) {
            bool refret = false;
            if (oNote->string() != string) {
                oNote->undoChangeProperty(Pid::STRING, string);
                refret = true;
            }
            if (oNote->fret() != fret) {
                oNote->undoChangeProperty(Pid::FRET, fret);
                refret = true;
            }
            if (refret) {
                const StringData* stringData = part->stringData(tick, staff->idx());
                stringData->fretChords(oNote->chord());
            }
        }

        score->setPlayNote(true);
    }
    score->setSelectionChanged(true);
}

//---------------------------------------------------------
//   undoChangePitch
//---------------------------------------------------------

void EditNote::undoChangePitch(Score* score, Note* note, int pitch, int tpc1, int tpc2)
{
    for (EngravingObject* e : note->linkList()) {
        Note* n = toNote(e);
        score->undoStack()->pushAndPerform(new ChangePitch(n, pitch, tpc1, tpc2), 0);
    }
}

//---------------------------------------------------------
//   undoChangeFretting
//---------------------------------------------------------

void EditNote::undoChangeFretting(Score* score, Note* note, int pitch, int string, int fret, int tpc1, int tpc2)
{
    const LinkedObjects* l = note->links();
    if (l) {
        for (EngravingObject* e : *l) {
            Note* n = toNote(e);
            score->undo(new ChangeFretting(n, pitch, string, fret, tpc1, tpc2));
        }
    } else {
        score->undo(new ChangeFretting(note, pitch, string, fret, tpc1, tpc2));
    }
}

//---------------------------------------------------------
//   ChangePitch
//---------------------------------------------------------

ChangePitch::ChangePitch(Note* _note, int _pitch, int _tpc1, int _tpc2)
{
    note  = _note;
    pitch = _pitch;
    tpc1  = _tpc1;
    tpc2  = _tpc2;
}

void ChangePitch::flip(EditData*)
{
    int f_pitch = note->pitch();
    int f_tpc1  = note->tpc1();
    int f_tpc2  = note->tpc2();
    // do not change unless necessary
    if (f_pitch == pitch && f_tpc1 == tpc1 && f_tpc2 == tpc2) {
        return;
    }

    note->setPitch(pitch, tpc1, tpc2);
    pitch = f_pitch;
    tpc1  = f_tpc1;
    tpc2  = f_tpc2;

    note->triggerLayout();
}

//---------------------------------------------------------
//   ChangeFretting
//
//    To use with tablatures to force a specific note fretting;
//    Pitch, string and fret must be changed all together; otherwise,
//    if they are not consistent among themselves, the refretting algorithm may re-assign
//    fret and string numbers for (potentially) all the notes of all the chords of a segment.
//---------------------------------------------------------

ChangeFretting::ChangeFretting(Note* _note, int _pitch, int _string, int _fret, int _tpc1, int _tpc2)
{
    note  = _note;
    pitch = _pitch;
    string= _string;
    fret  = _fret;
    tpc1  = _tpc1;
    tpc2  = _tpc2;
}

void ChangeFretting::flip(EditData*)
{
    int f_pitch = note->pitch();
    int f_string= note->string();
    int f_fret  = note->fret();
    int f_tpc1  = note->tpc1();
    int f_tpc2  = note->tpc2();
    // do not change unless necessary
    if (f_pitch == pitch && f_string == string && f_fret == fret && f_tpc1 == tpc1 && f_tpc2 == tpc2) {
        return;
    }

    note->setPitch(pitch, tpc1, tpc2);
    note->setString(string);
    note->setFret(fret);
    pitch = f_pitch;
    string= f_string;
    fret  = f_fret;
    tpc1  = f_tpc1;
    tpc2  = f_tpc2;
    note->triggerLayout();
}

//---------------------------------------------------------
//   ChangeVelocity
//---------------------------------------------------------

ChangeVelocity::ChangeVelocity(Note* n, int o)
    : note(n), userVelocity(o)
{
}

void ChangeVelocity::flip(EditData*)
{
    int v = note->userVelocity();
    note->setUserVelocity(userVelocity);
    userVelocity = v;
}

//---------------------------------------------------------
//   ChangeNoteEventList::flip
//---------------------------------------------------------

void ChangeNoteEventList::flip(EditData*)
{
    note->score()->setPlaylistDirty();
    // Get copy of current list.
    NoteEventList nel = note->playEvents();
    // Replace current copy with new list.
    note->setPlayEvents(newEvents);
    // Save copy of replaced list.
    newEvents = nel;
    // Get a copy of the current playEventType.
    PlayEventType petval = note->chord()->playEventType();
    // Replace current setting with new setting.
    note->chord()->setPlayEventType(newPetype);
    // Save copy of old setting.
    newPetype = petval;
}

//---------------------------------------------------------
//   ChangeNoteEvent::flip
//---------------------------------------------------------

void ChangeNoteEvent::flip(EditData*)
{
    note->score()->setPlaylistDirty();
    NoteEvent e = *oldEvent;
    *oldEvent   = newEvent;
    newEvent    = e;
    // Get a copy of the current playEventType.
    PlayEventType petval = note->chord()->playEventType();
    // Replace current setting with new setting.
    note->chord()->setPlayEventType(newPetype);
    // Save copy of old setting.
    newPetype = petval;
}

//---------------------------------------------------------
//   ChangeChordPlayEventType::flip
//---------------------------------------------------------

void ChangeChordPlayEventType::flip(EditData*)
{
    chord->score()->setPlaylistDirty();
    // Flips data between NoteEventList's.
    size_t n = chord->notes().size();
    for (size_t i = 0; i < n; ++i) {
        Note* note = chord->notes()[i];
        note->playEvents().swap(events[int(i)]);
    }
    // Flips PlayEventType between chord and undo.
    PlayEventType curPetype = chord->playEventType();
    chord->setPlayEventType(petype);
    petype = curPetype;
}
