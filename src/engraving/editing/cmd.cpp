/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

/**
 \file
 Handling of several GUI commands.
*/

#include <assert.h>
#include <set>

#include "translation.h"

#include "infrastructure/messagebox.h"
#include "style/style.h"

#include "rw/xmlreader.h"

#include "../dom/accidental.h"
#include "../dom/articulation.h"
#include "../dom/barline.h"
#include "../dom/box.h"
#include "../dom/chord.h"
#include "../dom/chordrest.h"
#include "../dom/clef.h"
#include "../dom/drumset.h"
#include "../dom/durationtype.h"
#include "../dom/dynamic.h"
#include "../dom/factory.h"
#include "../dom/glissando.h"
#include "../dom/guitarbend.h"
#include "../dom/hairpin.h"
#include "../dom/harmony.h"
#include "../dom/key.h"
#include "../dom/laissezvib.h"
#include "../dom/layoutbreak.h"
#include "../dom/linkedobjects.h"
#include "../dom/lyrics.h"
#include "../dom/masterscore.h"
#include "../dom/measure.h"
#include "../dom/measurerepeat.h"
#include "../dom/mscore.h"
#include "../dom/note.h"
#include "../dom/ornament.h"
#include "../dom/page.h"
#include "../dom/part.h"
#include "../dom/pitchspelling.h"
#include "../dom/rehearsalmark.h"
#include "../dom/rest.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/sig.h"
#include "../dom/slur.h"
#include "../dom/spacer.h"
#include "../dom/staff.h"
#include "../dom/stafftype.h"
#include "../dom/stafftypechange.h"
#include "../dom/stem.h"
#include "../dom/stringdata.h"
#include "../dom/system.h"
#include "../dom/tapping.h"
#include "../dom/tie.h"
#include "../dom/timesig.h"
#include "../dom/tremolotwochord.h"
#include "../dom/tuplet.h"
#include "../dom/utils.h"

#include "editchord.h"
#include "editnote.h"
#include "noteinput.h"
#include "editpagelocks.h"
#include "editproperty.h"
#include "editspanner.h"
#include "editstaff.h"
#include "editsystemlocks.h"
#include "mscoreview.h"
#include "navigation.h"
#include "transaction/transaction.h"
#include "transaction/undostack.h"
#include "transpose.h"

#include "log.h"

using namespace muse::io;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//    For use with Score::scanElements.
//    Reset positions and autoplacement for the given
//    element.
//---------------------------------------------------------

static void resetElementPosition(EngravingItem* e)
{
    if (e->generated()) {
        return;
    }

    e->undoResetProperty(Pid::AUTOPLACE);
    e->undoResetProperty(Pid::OFFSET);
    e->undoResetProperty(Pid::LEADING_SPACE);
    e->setOffsetChanged(false);
    if (e->isSpanner()) {
        e->undoResetProperty(Pid::OFFSET2);
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void CmdState::reset()
{
    layoutFlags         = LayoutFlag::NO_FLAGS;
    m_updateMode         = UpdateMode::DoNothing;
    m_startTick          = Fraction(-1, 1);
    m_endTick            = Fraction(-1, 1);

    m_startStaff = muse::nidx;
    m_endStaff = muse::nidx;
    m_el = nullptr;
    m_oneElement = true;
    m_mb = nullptr;
    m_oneMeasureBase = true;
    m_locked = false;
}

//---------------------------------------------------------
//   setTick
//---------------------------------------------------------

void CmdState::setTick(const Fraction& t)
{
    if (m_locked) {
        return;
    }

    if (m_startTick == Fraction(-1, 1) || t < m_startTick) {
        m_startTick = t;
    }
    if (m_endTick == Fraction(-1, 1) || t > m_endTick) {
        m_endTick = t;
    }
    setUpdateMode(UpdateMode::Layout);
}

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void CmdState::setStaff(staff_idx_t st)
{
    if (m_locked || st == muse::nidx) {
        return;
    }

    if (m_startStaff == muse::nidx || st < m_startStaff) {
        m_startStaff = st;
    }
    if (m_endStaff == muse::nidx || st > m_endStaff) {
        m_endStaff = st;
    }
}

//---------------------------------------------------------
//   setMeasureBase
//---------------------------------------------------------

void CmdState::setMeasureBase(const MeasureBase* mb)
{
    if (!mb || m_mb == mb || m_locked) {
        return;
    }

    m_oneMeasureBase = !m_mb;
    m_mb = mb;
}

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void CmdState::setElement(const EngravingItem* e)
{
    if (!e || m_el == e || m_locked) {
        return;
    }

    m_oneElement = !m_el;
    m_el = e;

    if (m_oneMeasureBase) {
        setMeasureBase(e->findMeasureBase());
    }
}

//---------------------------------------------------------
//   unsetElement
//---------------------------------------------------------

void CmdState::unsetElement(const EngravingItem* e)
{
    if (m_el == e) {
        m_el = nullptr;
    }
    if (m_mb == e) {
        m_mb = nullptr;
    }
}

//---------------------------------------------------------
//   element
//---------------------------------------------------------

const EngravingItem* CmdState::element() const
{
    if (m_oneElement) {
        return m_el;
    }
    if (m_oneMeasureBase) {
        return m_mb;
    }
    return nullptr;
}

//---------------------------------------------------------
//   setUpdateMode
//---------------------------------------------------------

void CmdState::_setUpdateMode(UpdateMode m)
{
    m_updateMode = m;
}

void CmdState::setUpdateMode(UpdateMode m)
{
    if (int(m) > int(m_updateMode)) {
        _setUpdateMode(m);
    }
}

//---------------------------------------------------------
//   deleteLater
//---------------------------------------------------------

void CmdState::deleteLater(EngravingObject* e)
{
    m_postponedDeletions.push_back(e);
}

std::vector<EngravingObject*> CmdState::takePostponedDeletions()
{
    std::vector<EngravingObject*> result;
    result.swap(m_postponedDeletions);
    return result;
}

static void deletePostponed(CmdState& cmdState)
{
    for (EngravingObject* e : cmdState.takePostponedDeletions()) {
        if (e->isSystem()) {
            System* s = toSystem(e);
            std::list<SpannerSegment*> spanners = s->spannerSegments();
            for (SpannerSegment* ss : spanners) {
                if (ss->system() == s) {
                    ss->setSystem(0);
                }
            }
        }
        delete e;
    }
}

//---------------------------------------------------------
//   startCmd
///   Start a GUI command by clearing the redraw area
///   and starting a user-visible undo.
//---------------------------------------------------------

void Score::startCmd(const TranslatableString& actionName)
{
    masterScore()->transactionManager()->beginTransaction(actionName);
}

//---------------------------------------------------------
//   undoRedo
//---------------------------------------------------------

void Score::undoRedo(bool undo, EditData* ed)
{
    masterScore()->transactionManager()->undoRedo(undo, ed);
}

//---------------------------------------------------------
//   endCmd
///   End a GUI command by (if \a undo) ending a user-visible undo
///   and (always) updating the redraw area.
//---------------------------------------------------------

void Score::endCmd(bool rollback, bool layoutAllParts, bool keepRolledBackElements)
{
    masterScore()->transactionManager()->endTransaction(rollback, layoutAllParts, keepRolledBackElements);
}

//---------------------------------------------------------
//   undo
//    Deprecated: use Transaction::push instead.
//---------------------------------------------------------

void Score::undo(UndoableCommand* cmd) const
{
    Transaction* tx = masterScore()->transactionManager()->currentTransaction();
    if (!tx) {
        // this can happen for layout() outside of a transaction (load)
        if (!ScoreLoad::loading()) {
            LOGW() << "called outside of transaction";
        }

        cmd->redo();
        delete cmd;
        return;
    }

    tx->push(cmd);
}

#ifndef NDEBUG
//---------------------------------------------------------
//   CmdState::dump
//---------------------------------------------------------

void CmdState::dump()
{
    LOGD("CmdState: mode %d %d-%d", int(m_updateMode), m_startTick.ticks(), m_endTick.ticks());
    // bool _excerptsChanged     { false };
    // bool _instrumentsChanged  { false };
}

#endif

//---------------------------------------------------------
//   update
//    layout & update
//---------------------------------------------------------

void Score::update()
{
    masterScore()->update();
}

void MasterScore::update(bool resetCmdState, bool layoutAllParts)
{
    if (m_updatesLocked) {
        return;
    }

    TRACEFUNC;

    deletePostponed(m_cmdState);

    bool updateAll = false;
    if (m_cmdState.layoutRange()) {
        for (Score* s : scoreList()) {
            if (s != this && !s->isOpen() && scoreList().size() > 1 && !layoutAllParts) {
                continue;
            }
            s->doLayoutRange(m_cmdState.startTick(), m_cmdState.endTick());
        }
        updateAll = true;
    }

    if (needSetUpTempoMap()) {
        setUpTempoMap();
    }

    if (updateAll || m_cmdState.updateAll()) {
        for (Score* s : scoreList()) {
            for (MuseScoreView* v : s->getViewer()) {
                v->updateAll();
            }
        }
    } else if (m_cmdState.updateRange()) {
        // Any score that accumulated a refresh rect via addRefresh() calls dataChanged() on its viewers.
        for (Score* s : scoreList()) {
            if (s->refreshRect().isNull()) {
                continue;
            }
            const std::list<MuseScoreView*>& viewers = s->getViewer();
            if (!viewers.empty()) {
                double d = s->style().spatium() * .5;
                RectF rect = s->refreshRect().adjusted(-d, -d, 2 * d, 2 * d);
                for (MuseScoreView* v : viewers) {
                    v->dataChanged(rect);
                }
            }
            s->clearRefreshRect();
        }
    }

    if (resetCmdState) {
        m_cmdState.reset();
    }

    for (Score* score : scoreList()) {
        Selection& sel = score->selection();
        if (sel.isRange() && !sel.isLocked()) {
            sel.updateSelectedElements();
        }
    }
}

void MasterScore::lockUpdates(bool locked)
{
    m_updatesLocked = locked;
}

//---------------------------------------------------------
//   expandVoice
//    fills gaps in voice with rests,
//    from previous cr (or beginning of measure) to next cr (or end of measure)
//---------------------------------------------------------

void Score::expandVoice(Segment* s, track_idx_t track)
{
    if (!s) {
        LOGD("expand voice: no segment");
        return;
    }
    if (s->element(track)) {
        return;
    }

    // find previous segment with cr in this track
    Segment* ps;
    for (ps = s; ps; ps = ps->prev(SegmentType::ChordRest)) {
        if (ps->element(track)) {
            break;
        }
    }
    if (ps) {
        ChordRest* cr = toChordRest(ps->element(track));
        Fraction tick = cr->endTick();
        if (tick > s->tick()) {
            // previous cr extends past current segment
            LOGD("expandVoice: cannot insert element here");
            return;
        }
        if (cr->isChord()) {
            // previous cr ends on or before current segment
            // for chords, move ps to just after cr ends
            // so we can fill any gap that might exist
            // but don't move ps if previous cr is a rest
            // this will be combined with any new rests needed to fill up to s->tick() below
            ps = ps->measure()->undoGetSegment(SegmentType::ChordRest, tick);
        }
    }
    //
    // fill up to s->tick() with rests
    //
    Measure* m = s->measure();
    Fraction stick  = ps ? ps->tick() : m->tick();
    Fraction stretch = staff(track2staff(track))->timeStretch(stick);
    Fraction ticks  = s->tick() - stick;
    if (ticks.isNotZero()) {
        setRest(stick, track, ticks * stretch, false, 0);
    }

    //
    // fill from s->tick() until next chord/rest in measure
    //
    Segment* ns;
    for (ns = s->next(SegmentType::ChordRest); ns; ns = ns->next(SegmentType::ChordRest)) {
        if (ns->element(track)) {
            break;
        }
    }
    ticks  = ns ? (ns->tick() - s->tick()) : (m->ticks() - s->rtick());
    if (ticks == m->ticks()) {
        addRest(s, track, TDuration(DurationType::V_MEASURE), 0);
    } else {
        setRest(s->tick(), track, ticks * stretch, false, 0);
    }
}

void Score::expandVoice()
{
    Segment* s = m_is.segment();
    track_idx_t track = m_is.track();
    expandVoice(s, track);
}

//---------------------------------------------------------
//   addInterval
//---------------------------------------------------------

void Score::addInterval(int val, const std::vector<Note*>& nl)
{
    // Prepare note selection in case there are not selected tied notes and sort them
    std::vector<Note*> tmpnl;
    std::vector<Note*> _nl = nl;
    bool selIsList = selection().isList();
    bool selIsSingle = selIsList && _nl.size() == 1;
    bool shouldSelectFirstNote = selIsSingle && _nl.front()->tieFor();

    std::sort(_nl.begin(), _nl.end(), [](const Note* a, const Note* b) -> bool {
        return a->tick() < b->tick();
    });

    for (Note* currNote : _nl) {
        for (Note* n = currNote; n && !muse::contains(tmpnl, n); n = n->tieFor() ? n->tieFor()->endNote() : nullptr) {
            if (selIsList && n->selected()) {
                deselect(n);
            }
            tmpnl.emplace_back(n);
        }
    }

    Note* prevTied = nullptr;
    std::vector<EngravingItem*> notesToSelect;
    int deltaLine = val < 0 ? val + 1 : val - 1;
    bool accidental = m_is.noteEntryMode() && m_is.accidentalType() != AccidentalType::NONE;
    bool useOctaveRule = (deltaLine % STEP_DELTA_OCTAVE == 0) && !accidental;  // Both octaves and unison

    for (Note* on : tmpnl) {
        Chord* chord = on->chord();
        Fraction tick = chord->tick();
        NoteVal nval;
        bool forceAccidental = false;
        bool noteValid = false;
        if (on->staff() && on->staff()->isDrumStaff(tick)) {
            // For drum staves: Don't calculate the new note based on pitch, but choose one using lines
            const Drumset* ds = on->staff()->part()->instrument(tick)->drumset();
            nval.pitch = ds->defaultPitchForLine(on->line() - deltaLine);
            nval.headGroup = ds->noteHead(nval.pitch);
            noteValid = ds->isValid(nval.pitch) && nval.headGroup != NoteHeadGroup::HEAD_INVALID;
        } else {
            if (useOctaveRule) {
                int octaves = deltaLine / STEP_DELTA_OCTAVE;
                nval.pitch = on->pitch() + octaves * PITCH_DELTA_OCTAVE;
                nval.tpc1 = on->tpc1();
                nval.tpc2 = on->tpc2();
            } else {
                int line      = on->line() - deltaLine;
                Staff* estaff = staff(chord->vStaffIdx());
                ClefType clef = estaff->clef(tick);
                Key key       = estaff->key(tick);
                int ntpc;
                if (accidental) {
                    AccidentalVal acci = Accidental::subtype2value(m_is.accidentalType());
                    int step = absStep(line, clef);
                    int octave = step / 7;
                    nval.pitch = step2pitch(step) + octave * PITCH_DELTA_OCTAVE + int(acci);
                    forceAccidental = (nval.pitch == line2pitch(line, clef, key));
                    ntpc = step2tpc(step % STEP_DELTA_OCTAVE, acci);
                } else {
                    nval.pitch = line2pitch(line, clef, key);
                    ntpc = pitch2tpc(nval.pitch, key, Prefer::NEAREST);
                }

                Interval v = estaff->transpose(tick);
                if (v.isZero()) {
                    nval.tpc1 = nval.tpc2 = ntpc;
                } else {
                    if (style().styleB(Sid::concertPitch)) {
                        v.flip();
                        nval.tpc1 = ntpc;
                        nval.tpc2 = Transpose::transposeTpc(ntpc, v, true);
                    } else {
                        nval.pitch += v.chromatic;
                        nval.tpc2 = ntpc;
                        nval.tpc1 = Transpose::transposeTpc(ntpc, v, true);
                    }
                }
            }
            noteValid = pitchIsValid(nval.pitch);
        }

        if (!noteValid) {
            notesToSelect.push_back(toEngravingItem(on));
            continue;
        }

        Note* note = Factory::createNote(chord);
        note->setParent(chord);
        note->setTrack(chord->track());
        note->setNval(nval, tick);
        undoAddElement(note);

        if (forceAccidental) {
            Accidental* a = Factory::createAccidental(note);
            a->setAccidentalType(m_is.accidentalType());
            a->setRole(AccidentalRole::USER);
            a->setParent(note);
            undoAddElement(a);
        }
        if (on->tieBack() && prevTied) {
            Tie* tie = prevTied->tieFor();
            tie->setEndNote(note);
            tie->setTick2(note->tick());
            note->setTieBack(tie);
            undoAddElement(tie);
            prevTied = nullptr;
        }

        Tie* tieFor = on->tieFor();
        if (tieFor) {
            Tie* tie = tieFor->isLaissezVib() ? Factory::createLaissezVib(this->dummy()->note()) : Factory::createTie(this->dummy());
            tie->setStartNote(note);
            tie->setTick(note->tick());
            tie->setTrack(note->track());
            note->setTieFor(tie);
            prevTied = note;
        }

        setPlayNote(true);
        if (selIsList && note) {
            notesToSelect.push_back(toEngravingItem(note));
        }
    }
    if (m_is.noteEntryMode()) {
        m_is.setAccidentalType(AccidentalType::NONE);
    }
    if (notesToSelect.empty()) {
        return;
    }
    if (shouldSelectFirstNote) {
        select(notesToSelect.front(), SelectType::SINGLE, 0);
    } else {
        select(notesToSelect, SelectType::ADD, 0);
    }
    if (m_is.cr() == toChordRest(_nl.front()->chord()) && selIsSingle) {
        m_is.moveToNextInputPos();
    }
}

//---------------------------------------------------------
//   setGraceNote
///   Create a grace note in front of a normal note.
///   \arg ch is the chord of the normal note
///   \arg pitch is the pitch of the grace note
///   \arg is the grace note type
///   \len is the visual duration of the grace note (1/16 or 1/32)
//---------------------------------------------------------

Note* Score::setGraceNote(Chord* ch, int pitch, NoteType type, int len)
{
    Chord* chord = Factory::createChord(this->dummy()->segment());
    Note* note = Factory::createNote(chord);

    // allow grace notes to be added to other grace notes
    // by really adding to parent chord
    if (ch->noteType() != NoteType::NORMAL) {
        ch = toChord(ch->explicitParent());
    }

    chord->setTrack(ch->track());
    chord->setParent(ch);
    chord->add(note);

    // find corresponding note within chord and use its tpc information
    // if no note with same pitch found, derive tpc from pitch / key
    if (Note* n = ch->findNote(pitch)) {
        note->setNval(n->noteVal(), ch->tick());
    } else {
        note->setPitch(pitch);
        note->setTpcFromPitch();
    }

    TDuration d;
    d.setVal(len);
    chord->setDurationType(d);
    chord->setTicks(d.fraction());
    chord->setNoteType(type);
    chord->setShowStemSlashInAdvance();
    chord->mutldata()->setMag(ch->staff()->staffMag(chord->tick()) * style().styleD(Sid::graceNoteMag));

    undoAddElement(chord);
    return note;
}

GuitarBend* Score::addGuitarBend(GuitarBendType type, Note* note, Note* endNote)
{
    if (note->isPreBendOrDiveStart()) {
        if (type == GuitarBendType::BEND && note->staffType()->isTabStaff()) {
            GuitarBend* preBend = note->bendFor();
            Note* mainNote = preBend ? preBend->endNote() : nullptr;
            if (mainNote) {
                return addGuitarBend(type, mainNote, nullptr);
            }
        }
        return nullptr;
    }

    if (note->isGraceBendStart() && type != GuitarBendType::PRE_BEND) {
        return nullptr;
    }

    if (GuitarBend* bendBack = note->bendBack(); bendBack && !bendBack->isDive()
        && (type == GuitarBendType::PRE_BEND || type == GuitarBendType::GRACE_NOTE_BEND)) {
        return nullptr;
    }

    if (GuitarBend* bendFor = note->bendFor(); bendFor && !bendFor->isDive()
        && (type == GuitarBendType::BEND || type == GuitarBendType::SLIGHT_BEND)) {
        return nullptr;
    }

    if (type == GuitarBendType::DIP) {
        for (Note* n : note->chord()->notes()) {
            if (GuitarBend* bendFor = n->bendFor(); bendFor && bendFor->bendType() == GuitarBendType::DIP) {
                return nullptr; // Only one dip per chord
            }
        }
    }

    Chord* chord = note->chord();
    bool isDive = static_cast<int>(type) >= static_cast<int>(GuitarBendType::DIVE)
                  && static_cast<int>(type) <= static_cast<int>(GuitarBendType::SCOOP);

    if (type == GuitarBendType::BEND || type == GuitarBendType::DIVE) {
        for (Spanner* sp : note->spannerFor()) {
            if ((sp->isGuitarBend() && toGuitarBend(sp)->isDive() == isDive) || sp->isGlissando()) {
                return nullptr;
            }
        }

        if (!endNote) {
            endNote = SLine::guessFinalNote(note);
        }

        bool suitableEndNote = endNote;
        if (endNote) {
            for (Spanner* sp : endNote->spannerBack()) {
                if (sp->isTie() || sp->isGlissando() || (sp->isGuitarBend() && toGuitarBend(sp)->isDive() == isDive)) {
                    suitableEndNote = false;
                    break;
                }
            }
        }

        if (!suitableEndNote) {
            endNote = GuitarBend::createEndNote(note, type);
        }

        if (!endNote) {
            return nullptr;
        }
    }

    GuitarBend* bend = new GuitarBend(score()->dummy()->note());
    bend->setTick(chord->tick());
    bend->setTrack(chord->track());

    if (type == GuitarBendType::BEND || type == GuitarBendType::DIVE) {
        bend->setBendType(chord->isGrace() && type == GuitarBendType::BEND ? GuitarBendType::GRACE_NOTE_BEND : type);
        bend->setStartElement(note);
        bend->setTick2(endNote->tick());
        bend->setTrack2(endNote->track());
        bend->setEndElement(endNote);
        bend->setParent(note);
        if (type == GuitarBendType::BEND) {
            GuitarBend::fixNotesFrettingForStandardBend(note, endNote);
        }
    } else {
        bend->setBendType(type);
        bend->setTick2(chord->tick());
        bend->setTrack2(chord->track());

        if (type == GuitarBendType::PRE_BEND || type == GuitarBendType::GRACE_NOTE_BEND || type == GuitarBendType::PRE_DIVE) {
            const GraceNotesGroup& gracesBefore = chord->graceNotesBefore();

            // Create grace note
            Note* graceNote = gracesBefore.empty()
                              ? setGraceNote(chord, note->pitch(), NoteType::APPOGGIATURA, Constants::DIVISION / 2)
                              : NoteInput::addNote(transactionManager()->currentOrDummyTransaction(), this, gracesBefore.back(),
                                                   note->noteVal());
            graceNote->transposeDiatonic(type == GuitarBendType::PRE_DIVE ? 1 : -1, true, false);
            GuitarBend::fixNotesFrettingForGraceBend(graceNote, note);

            Chord* graceChord = graceNote->chord();
            for (EngravingObject* item : graceChord->linkList()) {
                Chord* linkedGrace = toChord(item);
                linkedGrace->setNoStem(true);
                linkedGrace->setBeamMode(BeamMode::NONE);
            }

            // Add bend
            bend->setParent(graceNote);
            bend->setStartElement(graceNote);
            bend->setEndElement(note);
        } else if (type == GuitarBendType::SLIGHT_BEND || type == GuitarBendType::DIP || type == GuitarBendType::SCOOP) {
            bend->setParent(note);
            bend->setStartElement(note);
            // Slight bends don't end on another note
            bend->setEndElement(note);
        }
    }

    Chord* startChord = bend->startNote()->chord();
    if (startChord->isGrace()) {
        for (EngravingObject* item : startChord->linkList()) {
            Chord* linkedGrace = toChord(item);
            if (linkedGrace->staffType()->isTabStaff()) {
                linkedGrace->setNoStem(true);
                linkedGrace->setBeamMode(BeamMode::NONE);
            }
        }
    }

    score()->undoAddElement(bend);

    if (GuitarBend* overlapping = bend->overlappingBendOrDive()) {
        int halfBendAmount = std::floor(overlapping->bendAmountInQuarterTones() / 2);
        if (bend->isDive()) {
            bend->undoChangeProperty(Pid::GUITAR_BEND_AMOUNT, halfBendAmount);
        } else {
            overlapping->undoChangeProperty(Pid::GUITAR_BEND_AMOUNT, halfBendAmount);
        }
    }

    return bend;
}

//---------------------------------------------------------
//   createCRSequence
//    Create a rest or chord of len f.
//    If f is not a basic len, create several rests or
//    tied chords.
//
//    f     total len of ChordRest
//    cr    prototype CR
//    tick  start position in measure
//---------------------------------------------------------

void Score::createCRSequence(const Fraction& f, ChordRest* cr, const Fraction& t)
{
    Fraction tick(t);
    Measure* measure = cr->measure();
    ChordRest* ocr = 0;
    for (TDuration d : toDurationList(f, true)) {
        ChordRest* ncr = toChordRest(cr->clone());
        ncr->setDurationType(d);
        ncr->setTicks(d.fraction());
        undoAddCR(ncr, measure, measure->tick() + tick);
        if (cr->isChord() && ocr) {
            Chord* nc = toChord(ncr);
            Chord* oc = toChord(ocr);
            for (unsigned int i = 0; i < oc->notes().size(); ++i) {
                Note* on = oc->notes()[i];
                Note* nn = nc->notes()[i];
                Tie* tie = Factory::createTie(this->dummy());
                tie->setStartNote(on);
                tie->setEndNote(nn);
                tie->setTick(tie->startNote()->tick());
                tie->setTick2(tie->endNote()->tick());
                tie->setTrack(cr->track());
                on->setTieFor(tie);
                nn->setTieBack(tie);
                undoAddElement(tie);
            }
        }

        tick += ncr->actualTicks();
        ocr = ncr;
    }
}

//---------------------------------------------------------
//   setNoteRest
//    pitch == -1  -> set rest
//    "sd" is in local (stretched) time
//    return segment of last created note/rest
//---------------------------------------------------------

Segment* Score::setNoteRest(Segment* segment, track_idx_t track, NoteVal nval, Fraction sd, DirectionV stemDirection,
                            bool forceAccidental, const std::set<SymId>& articulationIds, bool rhythmic, InputState* externalInputState)
{
    assert(segment->segmentType() == SegmentType::ChordRest);
    InputState& is = externalInputState ? (*externalInputState) : m_is;

    bool isRest   = nval.isRest();
    Fraction tick = segment->tick();
    EngravingItem* nr   = nullptr;
    Tie* tie      = nullptr;
    ChordRest* cr = toChordRest(segment->element(track));
    Tuplet* tuplet = cr ? cr->tuplet() : nullptr;
    Measure* measure = nullptr;
    bool targetIsRest = cr && cr->isRest();

    // preserve lyrics before the ChordRest is removed or replaced
    std::vector<Lyrics*> lyricsToPreserve;
    bool shouldPreserveLyrics = false;
    if (!isRest && cr) {
        lyricsToPreserve = cr->lyrics();
        shouldPreserveLyrics = !lyricsToPreserve.empty();
    }
    bool lyricsPreserved = false;

    for (;;) {
        if (track % VOICES) {
            expandVoice(segment, track);
        }
        if (targetIsRest && !cr->isRest()) {
            undoRemoveElement(cr);
            segment = addRest(segment, track, cr->ticks(), cr->tuplet())->segment();
        }
        // the returned gap ends at the measure boundary or at tuplet end
        Fraction dd = makeGap(segment, track, sd, tuplet);

        if (dd.isZero()) {
            LOGD("cannot get gap at %d type: %d/%d", tick.ticks(), sd.numerator(),
                 sd.denominator());
            break;
        }

        measure = segment->measure();
        Fraction timeStretch = staff(track2staff(track))->timeStretch(tick);
        std::vector<TDuration> dl;
        if (rhythmic) {
            dl = toRhythmicDurationList(dd, isRest, segment->rtick() * timeStretch, sigmap()->timesig(
                                            tick).nominal(), measure, 1, timeStretch);
        } else {
            dl = toDurationList(dd, true);
        }
        size_t n = dl.size();
        for (size_t i = 0; i < n; ++i) {
            const TDuration& d = dl[i];
            ChordRest* ncr = nullptr;
            Note* note = nullptr;
            Tie* addTie = nullptr;
            if (isRest) {
                nr = ncr = Factory::createRest(this->dummy()->segment());
                nr->setTrack(track);
                ncr->setDurationType(d);
                ncr->setTicks(d.isMeasure() ? measure->ticks() * timeStretch : d.fraction());
            } else {
                nr = note = Factory::createNote(this->dummy()->chord());

                if (tie) {
                    tie->setEndNote(note);
                    note->setTieBack(tie);
                    addTie = tie;
                }
                Chord* chord = Factory::createChord(this->dummy()->segment());
                chord->setTrack(track);
                chord->setDurationType(d);
                chord->setTicks(d.fraction());
                chord->setStemDirection(stemDirection);
                chord->add(note);
                if (cr && cr->isChord()) {
                    std::vector<Chord*> graceNotes = toChord(cr)->graceNotes();
                    for (Chord* grace : graceNotes) {
                        undoChangeParent(grace, chord, chord->staffIdx());
                    }
                }
                note->setNval(nval, tick);
                if (forceAccidental) {
                    int tpc = style().styleB(Sid::concertPitch) ? nval.tpc1 : nval.tpc2;
                    AccidentalVal alter = tpc2alter(tpc);
                    AccidentalType at = Accidental::value2subtype(alter);
                    Accidental* a = Factory::createAccidental(note);
                    a->setAccidentalType(at);
                    a->setRole(AccidentalRole::USER);
                    note->add(a);
                }

                ncr = chord;
                if (i + 1 < n) {
                    tie = Factory::createTie(this->dummy());
                    tie->setStartNote(note);
                    tie->setTick(tie->startNote()->tick());
                    tie->setTrack(track);
                    note->setTieFor(tie);
                }
            }
            if (tuplet) {
                ncr->setTuplet(tuplet);
            }
            tuplet = 0;
            undoAddCR(ncr, measure, tick);

            if (shouldPreserveLyrics && !lyricsPreserved) {
                // reattach the preserved lyrics to the first replacement ChordRest
                // to ensure they are not duplicated during subsequent operations
                lyricsPreserved = true;
                for (Lyrics* lyric : lyricsToPreserve) {
                    undoChangeParent(lyric, ncr, ncr->staffIdx());
                }
            }

            if (addTie) {
                undoAddElement(addTie);
            }
            setPlayNote(true);
            segment = ncr->segment();
            tick += ncr->actualTicks();

            if (!articulationIds.empty()) {
                if (ncr->isChord()) {
                    toChord(ncr)->updateArticulations(articulationIds);
                }
            }
        }

        sd -= dd;
        if (sd.isZero()) {
            break;
        }

        if (tick >= score()->endTick()) {
            appendMeasures(1);
        }
        Segment* nseg = tick2segment(tick, false, SegmentType::ChordRest);
        if (!nseg) {
            break;
        }
        segment = nseg;

        cr = toChordRest(segment->element(track));
        // if the cr was too big for the tuplet, get the next tuplet so we can spill into that one
        tuplet = cr && cr->tuplet() ? cr->tuplet() : nullptr;
        if (!cr) {
            if (track % VOICES) {
                cr = addRest(segment, track, TDuration(DurationType::V_MEASURE), 0);
            } else {
                LOGD("no rest in voice 0");
                break;
            }
        }
        //
        //  Note does not fit on current measure, create Tie to
        //  next part of note
        if (!isRest) {
            tie = Factory::createTie(this->dummy());
            tie->setStartNote((Note*)nr);
            tie->setTick(tie->startNote()->tick());
            tie->setTrack(nr->track());
            ((Note*)nr)->setTieFor(tie);
        }
    }
    if (tie) {
        connectTies();
    }

    if (nr) {
        if (is.slur() && nr->isNote()) {
            // If the start element was the same as the end element when the slur was created,
            // the end grip of the front slur segment was given an x-offset of 3.0 * spatium().
            // Now that the slur is about to be given a new end element, this should be reset.
            if (is.slur()->endElement() == is.slur()->startElement()) {
                is.slur()->frontSegment()->reset();
            }
            //
            // extend slur
            //
            Chord* chord = toNote(nr)->chord();
            is.slur()->undoChangeProperty(Pid::SPANNER_TICKS, chord->tick() - is.slur()->tick());
            for (EngravingObject* se : is.slur()->linkList()) {
                Slur* slur = toSlur(se);
                for (EngravingObject* ee : chord->linkList()) {
                    EngravingItem* e = static_cast<EngravingItem*>(ee);
                    if (e->score() == slur->score() && e->track() == slur->track2()) {
                        slur->score()->undo(new ChangeSpannerElements(slur, slur->startElement(), e));
                        break;
                    }
                }
            }
        }
        if (externalInputState) {
            is.setTrack(nr->track());
            cr = nr->isRest() ? toChordRest(nr) : toNote(nr)->chord();
            is.setLastSegment(is.segment());
            is.setSegment(cr->segment());
        } else {
            select(nr, SelectType::SINGLE, 0);
        }
    }
    return segment;
}

//---------------------------------------------------------
//   makeGap
//    make time gap at tick by removing/shortening
//    chord/rest
//
//    if keepChord, the chord at tick is not removed
//
//    "sd" is in local (stretched) time
//
//    gap does not exceed measure or scope of tuplet
//
//    return size of actual gap in local time
//---------------------------------------------------------

Fraction Score::makeGap(Segment* segment, track_idx_t track, const Fraction& _sd, Tuplet* tuplet, bool keepChord, bool deleteAnnotations)
{
    assert(_sd.numerator());

    Measure* measure = segment->measure();
    Staff* stf = staff(track2staff(track));
    Fraction accumulated;
    Fraction sd = _sd;

    //
    // remember first segment which should
    // not be deleted (it may contain other elements we want to preserve)
    //
    Segment* firstSegment = segment;
    const Fraction firstSegmentEnd = firstSegment->tick() + firstSegment->ticks();
    Fraction nextTick = segment->tick();

    for (Segment* seg = firstSegment; seg; seg = seg->next(SegmentType::ChordRest)) {
        //
        // voices != 0 may have gaps:
        //
        ChordRest* cr = toChordRest(seg->element(track));
        Fraction timeStretch = stf->timeStretch(seg->tick());
        if (!cr) {
            if (seg->tick() < nextTick) {
                continue;
            }
            Segment* seg1 = seg->next(SegmentType::ChordRest);
            Fraction tick2 = seg1 ? seg1->tick() : seg->measure()->endTick();
            Fraction td(tick2 - seg->tick());
            td /= actualTicks(Fraction(1, 1), tuplet, timeStretch);
            if (td > sd) {
                td = sd;
            }
            accumulated += td;
            sd -= td;
            if (sd.isZero()) {
                break;
            }
            nextTick = tick2;
            continue;
        }
        if (seg->tick() > nextTick) {
            // there was a gap
            Fraction td(seg->tick() - nextTick);
            td /= actualTicks(Fraction(1, 1), tuplet, timeStretch);
            if (td > sd) {
                td = sd;
            }
            accumulated += td;
            sd -= td;
            if (sd.isZero()) {
                break;
            }
        }
        //
        // limit to tuplet level
        //
        if (tuplet) {
            bool tupletEnd = true;
            Tuplet* t = cr->tuplet();
            while (t) {
                if (cr->tuplet() == tuplet) {
                    tupletEnd = false;
                    break;
                }
                t = t->tuplet();
            }
            if (tupletEnd) {
                break;
            }
        }
        Fraction td(cr->ticks());

        // remove tremolo between 2 notes, if present
        if (cr->isChord()) {
            Chord* c = toChord(cr);
            if (c->tremoloTwoChord()) {
                undoRemoveElement(c->tremoloTwoChord());
            }
        }
        Tuplet* ltuplet = cr->tuplet();
        if (ltuplet != tuplet) {
            //
            // Current location points to the start of a (nested)tuplet.
            // We have to remove the complete tuplet.

            // get top level tuplet
            while (ltuplet->tuplet()) {
                ltuplet = ltuplet->tuplet();
            }

            // get last segment of tuplet, drilling down to leaf nodes as necessary
            Tuplet* t = ltuplet;
            while (t->elements().back()->isTuplet()) {
                t = toTuplet(t->elements().back());
            }
            seg = toChordRest(t->elements().back())->segment();

            // now delete the full tuplet
            td = ltuplet->ticks();
            cmdDeleteTuplet(ltuplet, false);
            tuplet = 0;
        } else {
            if (seg != firstSegment || !keepChord) {
                undoRemoveElement(cr);
            }
            // even if there was a tuplet, we didn't remove it
            ltuplet = 0;
        }
        nextTick += actualTicks(td, tuplet, timeStretch);
        if (sd < td) {
            //
            // we removed too much
            //
            accumulated = _sd;
            Fraction rd = td - sd;
            Fraction tick = cr->tick() + actualTicks(sd, tuplet, timeStretch);

            std::vector<TDuration> dList;
            if (tuplet) {
                dList = toDurationList(rd, false);
                std::reverse(dList.begin(), dList.end());
            } else {
                TimeSig* timeSig = stf->timeSig(tick);
                TimeSigFrac refTimeSig = timeSig ? timeSig->sig() : sigmap()->timesig(tick).nominal();
                Fraction rTickStart = (tick - measure->tick()) * stf->timeStretch(tick);
                dList = toRhythmicDurationList(rd, true, rTickStart, refTimeSig, measure, 0, stf->timeStretch(tick));
            }
            if (dList.empty()) {
                break;
            }

            for (TDuration d : dList) {
                if (ltuplet) {
                    // take care not to recreate tuplet we just deleted
                    Rest* r = setRest(tick, track, d.fraction(), false, 0, false);
                    tick += r->actualTicks();
                } else {
                    tick += addClone(cr, tick, d)->actualTicks();
                }
            }
            break;
        }
        accumulated += td;
        sd          -= td;
        if (sd.isZero()) {
            break;
        }
    }

    const Fraction t1 = firstSegmentEnd;
    const Fraction t2 = firstSegment->tick() + actualTicks(accumulated, tuplet, stf->timeStretch(firstSegment->tick()));
    if (t1 < t2) {
        // Delete annotations that require an anchor to the previous segment
        Segment* s1 = tick2rightSegment(t1);
        Segment* s2 = tick2rightSegment(t2);
        const bool segsValid = s1 && s2 && (*s2) > (*s1);
        if (segsValid && deleteAnnotations) {
            for (Segment* s = s1; s && s != s2; s = s->next1()) {
                const auto annotations = s->annotations(); // make a copy since we alter the list
                for (EngravingItem* annotation : annotations) {
                    if (!annotation->systemFlag() && !annotation->allowTimeAnchor() && annotation->track() == track) {
                        deleteItem(annotation);
                    }
                }
            }
        }

        SelectionFilter filter;
        deleteSlursFromRange(t1, t2, track, track + 1, filter);
    }

    return accumulated;
}

bool Score::makeGapVoice(Segment* seg, track_idx_t track, Fraction len, const Fraction& tick, bool deleteAnnotations)
{
    ChordRest* cr = 0;
    cr = toChordRest(seg->element(track));
    if (!cr) {
        // check if we are in the middle of a chord/rest
        Segment* seg1 = seg->prev(SegmentType::ChordRest);
        for (;;) {
            if (seg1 == 0) {
                if (!(track % VOICES)) {
                    LOGD("no segment before tick %d", tick.ticks());
                }
                // this happens only for voices other than voice 1
                expandVoice(seg, track);
                return makeGapVoice(seg, track, len, tick, deleteAnnotations);
            }
            if (seg1->element(track)) {
                break;
            }
            seg1 = seg1->prev(SegmentType::ChordRest);
        }
        ChordRest* cr1 = toChordRest(seg1->element(track));
        Fraction srcF = cr1->ticks();
        Fraction dstF = (tick - cr1->tick()) * cr1->staff()->timeStretch(cr1->tick());
        std::vector<TDuration> dList = toDurationList(dstF, true);
        if (dList.empty()) {
            LOGD("Could not make durations for: %d/%d", dstF.numerator(), dstF.denominator());
            return false;
        }
        size_t n = dList.size();
        undoChangeChordRestLen(cr1, TDuration(dList[0]));
        if (n > 1) {
            Fraction crtick = cr1->endTick();
            Measure* measure = tick2measure(crtick);
            if (cr1->isChord()) {
                // split Chord
                Chord* c = toChord(cr1);
                for (size_t i = 1; i < n; ++i) {
                    TDuration d = dList[i];
                    Chord* c2 = addChord(crtick, d, c, true, c->tuplet());
                    c = c2;
                    seg1 = c->segment();
                    crtick += c->actualTicks();
                }
            } else {
                // split Rest
                Rest* r       = toRest(cr1);
                for (size_t i = 1; i < n; ++i) {
                    TDuration d = dList[i];
                    Rest* r2      = toRest(r->clone());
                    r2->setTicks(d.fraction());
                    r2->setDurationType(d);
                    undoAddCR(r2, measure, crtick);
                    seg1 = r2->segment();
                    crtick += r2->actualTicks();
                }
            }
        }
        setRest(tick, track, srcF - dstF, true, 0);
        for (;;) {
            seg1 = seg1->next1(SegmentType::ChordRest);
            if (seg1 == 0) {
                LOGD("no segment");
                return false;
            }
            if (seg1->element(track)) {
                cr = toChordRest(seg1->element(track));
                break;
            }
        }
    }

    for (;;) {
        if (!cr) {
            LOGD("cannot make gap");
            return false;
        }
        Fraction l = makeGap(cr->segment(), cr->track(), len, nullptr, /*keepChord*/ false, deleteAnnotations);
        if (l.isZero()) {
            LOGD("returns zero gap");
            return false;
        }
        len -= l;
        if (len.isZero()) {
            break;
        }
        // go to next cr
        Measure* m = cr->measure()->nextMeasure();
        if (!m) {
            LOGD("EOS reached");
            InsertMeasureOptions options;
            options.createMeasureRests = false;
            insertMeasure(ElementType::MEASURE, nullptr, options);
            m = cr->measure()->nextMeasure();
            if (!m) {
                LOGD("===EOS reached");
                return true;
            }
        }
        // first segment in measure was removed, have to recreate it
        Segment* s = m->undoGetSegment(SegmentType::ChordRest, m->tick());
        track_idx_t t = cr->track();
        cr = toChordRest(s->element(t));
        if (!cr) {
            addRest(s, t, TDuration(DurationType::V_MEASURE), 0);
            cr = toChordRest(s->element(t));
        }
    }
    return true;
}

//---------------------------------------------------------
//   splitGapToMeasureBoundaries
//    cr  - start of gap
//    gap - gap len in local (stretched) time
//---------------------------------------------------------

std::vector<Fraction> Score::splitGapToMeasureBoundaries(ChordRest* cr, Fraction gap)
{
    std::vector<Fraction> flist;

    Tuplet* tuplet = cr->tuplet();
    if (tuplet) {
        Fraction rest = tuplet->elementsDuration();
        for (DurationElement* de : tuplet->elements()) {
            if (de == cr) {
                break;
            }
            rest -= de->ticks();
        }
        if (rest < gap) {
            LOGD("does not fit in tuplet");
        } else {
            flist.push_back(gap);
        }
        return flist;
    }

    Segment* s = cr->segment();
    while (gap > Fraction(0, 1)) {
        Measure* m    = s->measure();
        Fraction timeStretch = cr->staff()->timeStretch(s->tick());
        Fraction rest = (m->ticks() - s->rtick()) * timeStretch;
        if (rest >= gap) {
            flist.push_back(gap);
            return flist;
        }
        flist.push_back(rest);
        gap -= rest;
        if (m->nextMeasure() != 0) {
            m = m->nextMeasure();
        }
        s = m->first(SegmentType::ChordRest);
    }
    return flist;
}

//---------------------------------------------------------
//   changeCRlen
//    - dstF is in local (stretched) time
//---------------------------------------------------------

void Score::changeCRlen(ChordRest* cr, const TDuration& d)
{
    Fraction dstF;
    if (d.type() == DurationType::V_MEASURE) {
        dstF = cr->measure()->stretchedLen(cr->staff());
    } else {
        dstF = d.fraction();
    }
    changeCRlen(cr, dstF);
}

void Score::changeCRlen(ChordRest* cr, const Fraction& dstF, bool fillWithRest)
{
    if (cr->isMeasureRepeat()) {
        // it is not clear what should this
        // operation mean for measure repeats.
        return;
    }
    Fraction srcF(cr->ticks());
    if (srcF == dstF) {
        if (cr->isFullMeasureRest()) {
            undoChangeChordRestLen(cr, dstF);
        }
        return;
    }

    //keep selected element if any
    EngravingItem* selElement = selection().isSingle() ? getSelectedElement() : 0;

    track_idx_t track = cr->track();
    Tuplet* tuplet = cr->tuplet();

    if (srcF > dstF) {
        //
        // make shorter and fill with rest
        //
        deselectAll();
        if (cr->isChord()) {
            //
            // remove ties and tremolo between 2 notes
            //
            Chord* c = toChord(cr);
            if (c->tremoloTwoChord()) {
                undoRemoveElement(c->tremoloTwoChord());
            }
            for (Note* n : c->notes()) {
                if (Tie* tie = n->tieFor()) {
                    if (tie->tieJumpPoints()) {
                        tie->undoRemoveTiesFromJumpPoints();
                    }
                    undoRemoveElement(tie);
                }
                for (Spanner* sp : n->spannerFor()) {
                    if (sp->isGlissando() || sp->isGuitarBend()) {
                        undoRemoveElement(sp);
                    }
                }
            }
        }
        Fraction timeStretch = cr->staff()->timeStretch(cr->tick());
        std::vector<TDuration> dList = toDurationList(dstF, true);
        if (dList.empty()) {
            LOGD("Could not make durations for: %d/%d", dstF.numerator(), dstF.denominator());
            return;
        }
        undoChangeChordRestLen(cr, dList[0]);
        Fraction tick2 = cr->tick();
        for (unsigned i = 1; i < dList.size(); ++i) {
            tick2 += actualTicks(dList[i - 1].ticks(), tuplet, timeStretch);
            TDuration d = dList[i];
            setRest(tick2, track, d.fraction(), (d.dots() > 0), tuplet);
        }
        if (fillWithRest) {
            setRest(cr->endTick(), track, srcF - dstF, false, tuplet);
        }

        if (selElement) {
            select(selElement, SelectType::SINGLE, 0);
        }
        return;
    }

    //
    // make longer
    //
    // split required len into Measures
    std::vector<Fraction> flist = splitGapToMeasureBoundaries(cr, dstF);
    if (flist.empty()) {
        return;
    }

    deselectAll();
    EngravingItem* elementToSelect = nullptr;

    Fraction tick  = cr->tick();
    Fraction f     = dstF;
    ChordRest* cr1 = cr;
    Chord* oc      = 0;
    Segment* s     = cr->segment();

    bool first = true;
    for (const Fraction& f2 : flist) {
        if (!cr1) {
            expandVoice(s, track);
            cr1 = toChordRest(s->element(track));
        }

        f  -= f2;
        makeGap(cr1->segment(), cr1->track(), f2, tuplet, first);

        if (cr->isRest()) {
            Fraction timeStretch = cr1->staff()->timeStretch(cr1->tick());
            Rest* r = toRest(cr);
            if (first) {
                std::vector<TDuration> dList = toDurationList(f2, true);
                undoChangeChordRestLen(cr, dList[0]);
                Fraction tick2 = cr->tick();
                for (unsigned i = 1; i < dList.size(); ++i) {
                    tick2 += actualTicks(dList[i - 1].ticks(), tuplet, timeStretch);
                    TDuration d = dList[i];
                    setRest(tick2, track, d.fraction(), (d.dots() > 0), tuplet);
                }
            } else {
                r = setRest(tick, track, f2, false, tuplet);
            }
            if (first) {
                elementToSelect = r;
                first = false;
            }
            tick += actualTicks(f2, tuplet, timeStretch);
        } else {
            std::vector<TDuration> dList = toDurationList(f2, true);
            Measure* measure             = tick2measure(tick);
            Fraction etick                    = measure->tick();

            if (((tick - etick).ticks() % dList[0].ticks().ticks()) == 0) {
                for (TDuration du : dList) {
                    Chord* cc = nullptr;
                    if (oc) {
                        cc = oc;
                        oc = addChord(tick, du, cc, true, tuplet);
                    } else {
                        cc = toChord(cr);
                        undoChangeChordRestLen(cr, du);
                        oc = cc;
                    }
                    if (oc && first) {
                        elementToSelect = selElement ? selElement : oc;
                        first = false;
                    }
                    if (oc) {
                        tick += oc->actualTicks();
                    }
                }
            } else {
                for (size_t i = dList.size(); i > 0; --i) {         // loop probably needs to be in this reverse order
                    Chord* cc;
                    if (oc) {
                        cc = oc;
                        oc = addChord(tick, dList[i - 1], cc, true, tuplet);
                    } else {
                        cc = toChord(cr);
                        undoChangeChordRestLen(cr, dList[i - 1]);
                        oc = cc;
                    }
                    if (first) {
                        elementToSelect = selElement;
                        first = false;
                    }
                    tick += oc->actualTicks();
                }
            }
        }
        const Measure* m  = cr1->measure();
        if (m->nextMeasure() == 0) {
            if (f.isNotZero()) {
                appendMeasures(1);
            }
        }
        const Measure* m1 = m->nextMeasure();
        if (m1 == 0) {
            break;
        }
        s = m1->first(SegmentType::ChordRest);
        cr1 = toChordRest(s->element(track));
    }
    connectTies();

    if (elementToSelect) {
        if (canReselectItem(elementToSelect)) {
            select(elementToSelect, SelectType::SINGLE, 0);
        }
    }
}

static void resetBeamOffSet(EngravingItem* e)
{
    // Reset completely cross staff beams from MU1&2
    if (e->isBeam() && toBeam(e)->fullCross()) {
        e->reset();
    }
}

void Score::resetAutoplace()
{
    TRACEFUNC;

    scanElements(resetElementPosition);
}

void Score::resetCrossBeams()
{
    TRACEFUNC;

    scanElements(resetBeamOffSet);
}

//---------------------------------------------------------
//   cmdMoveRest
//---------------------------------------------------------

void Score::cmdMoveRest(Rest* rest, DirectionV dir)
{
    PointF pos(rest->offset());
    if (dir == DirectionV::UP) {
        pos.ry() -= rest->spatium() * rest->staffType()->lineDistance().val();
    } else if (dir == DirectionV::DOWN) {
        pos.ry() += rest->spatium() * rest->staffType()->lineDistance().val();
    }
    rest->undoChangeProperty(Pid::OFFSET, pos);
}

//---------------------------------------------------------
//   cmdMoveLyrics
//---------------------------------------------------------

void Score::cmdMoveLyrics(Lyrics* lyrics, DirectionV dir)
{
    int verse = lyrics->verse() + (dir == DirectionV::UP ? -1 : 1);
    if (verse < 0) {
        return;
    }
    lyrics->undoChangeProperty(Pid::VERSE, verse);
}

//---------------------------------------------------------
//   cmdAddGrace
///   adds grace note of specified type to selected notes
//---------------------------------------------------------

void Score::cmdAddGrace(NoteType graceType, int duration)
{
    const std::vector<EngravingItem*> copyOfElements = selection().elements();
    for (EngravingItem* e : copyOfElements) {
        if (e->isNote()) {
            Note* n = toNote(e);
            Note* graceNote = setGraceNote(n->chord(), n->pitch(), graceType, duration);
            select(graceNote, SelectType::SINGLE, 0);
        }
    }
}

//---------------------------------------------------------
//   cmdRemoveEmptyTrailingMeasures
//---------------------------------------------------------

void Score::cmdRemoveEmptyTrailingMeasures()
{
    Score* mScore = masterScore();
    auto beginMeasure = mScore->firstTrailingMeasure();
    if (beginMeasure) {
        mScore->deleteMeasures(beginMeasure, mScore->lastMeasure());
    }
}

//---------------------------------------------------------
//   cmdToggleLayoutBreak
//---------------------------------------------------------

void Score::cmdToggleLayoutBreak(LayoutBreakType type)
{
    // find measure(s)
    std::vector<MeasureBase*> mbl;
    bool allNoBreaks = true; // NOBREAK is not removed unless every measure in selection already has one
    if (selection().isRange()) {
        Measure* startMeasure = nullptr;
        Measure* endMeasure = nullptr;
        if (!selection().measureRange(&startMeasure, &endMeasure)) {
            return;
        }
        if (!startMeasure || !endMeasure) {
            return;
        }
        if (type == LayoutBreakType::NOBREAK) {
            // add throughout the selection
            // or remove if already on every measure
            if (startMeasure == endMeasure) {
                mbl.push_back(startMeasure);
                allNoBreaks = startMeasure->noBreak();
            } else {
                for (Measure* m = startMeasure; m; m = m->nextMeasureMM()) {
                    if (m == endMeasure) {
                        break;
                    }
                    mbl.push_back(m);
                    if (!toMeasureBase(m)->noBreak()) {
                        allNoBreaks = false;
                    }
                }
            }
        } else {
            // toggle break on the last measure of the range
            mbl.push_back(endMeasure);
            // if more than one measure selected,
            // also toggle break *before* the range (to try to fit selection on a single line)
            if (startMeasure != endMeasure && startMeasure->prev()) {
                mbl.push_back(startMeasure->prev());
            }
        }
    } else {
        MeasureBase* mb = nullptr;
        for (EngravingItem* el : selection().elements()) {
            switch (el->type()) {
            case ElementType::HBOX:
            case ElementType::VBOX:
            case ElementType::TBOX:
            case ElementType::FBOX:
                mb = toMeasureBase(el);
                break;
            default: {
                // find measure
                mb = toMeasure(el->findMeasure());
                // for start repeat, attach break to previous measure
                if (mb && el->isBarLine()) {
                    BarLine* bl = toBarLine(el);
                    if (bl->barLineType() == BarLineType::START_REPEAT) {
                        mb = mb->prevMeasure();
                    }
                }

                if (mb) {
                    allNoBreaks = mb->noBreak();
                }
            }
            }
        }
        if (mb) {
            mbl.push_back(mb);
        }
    }

    if (type == LayoutBreakType::NOBREAK && !allNoBreaks) {
        for (MeasureBase* mb : mbl) {
            if (mb->systemLock()) {
                return;
            }
        }
    }

    // toggle the breaks
    Transaction& tx = transactionManager()->currentOrDummyTransaction();
    for (MeasureBase* mb: mbl) {
        bool val = false;
        switch (type) {
        case LayoutBreakType::LINE:
            val = !mb->lineBreak();
            if (val) {
                EditSystemLocks::removeSystemLocksOnAddLayoutBreak(tx, this, type, mb);
                EditPageLocks::removePageLocksOnAddLayoutBreak(tx, this, type, mb);
            }
            mb->undoSetBreak(val, type);
            // remove page break if appropriate
            if (val && mb->pageBreak()) {
                mb->undoSetBreak(false, LayoutBreakType::PAGE);
            }
            break;
        case LayoutBreakType::PAGE:
            val = !mb->pageBreak();
            if (val) {
                EditSystemLocks::removeSystemLocksOnAddLayoutBreak(tx, this, type, mb);
                EditPageLocks::removePageLocksOnAddLayoutBreak(tx, this, type, mb);
            }
            mb->undoSetBreak(val, type);
            // remove line break if appropriate
            if (val && mb->lineBreak()) {
                mb->undoSetBreak(false, LayoutBreakType::LINE);
            }
            break;
        case LayoutBreakType::SECTION:
            val = !mb->sectionBreak();
            if (val) {
                EditSystemLocks::removeSystemLocksOnAddLayoutBreak(tx, this, type, mb);
                EditPageLocks::removePageLocksOnAddLayoutBreak(tx, this, type, mb);
            }
            mb->undoSetBreak(val, type);
            break;
        case LayoutBreakType::NOBREAK:
            mb->undoSetBreak(!allNoBreaks, type);
            // remove other breaks if appropriate
            if (!mb->noBreak()) {
                if (mb->pageBreak()) {
                    mb->undoSetBreak(false, LayoutBreakType::PAGE);
                } else if (mb->lineBreak()) {
                    mb->undoSetBreak(false, LayoutBreakType::LINE);
                } else if (mb->sectionBreak()) {
                    mb->undoSetBreak(false, LayoutBreakType::SECTION);
                }
            }
            break;
        default:
            break;
        }
    }
}

//---------------------------------------------------------
//   cmdToggleMmrest
//---------------------------------------------------------

void Score::cmdToggleMmrest()
{
    bool val = !style().styleB(Sid::createMultiMeasureRests);
    deselectAll();
    undoChangeStyleVal(Sid::createMultiMeasureRests, val);
}

//---------------------------------------------------------
//   cmdToggleHideEmpty
//---------------------------------------------------------

void Score::cmdToggleHideEmpty()
{
    bool val = !style().styleB(Sid::hideEmptyStaves);
    deselectAll();
    undoChangeStyleVal(Sid::hideEmptyStaves, val);
}

void Score::cmdSetHideStaffIfEmptyOverride(staff_idx_t staffIdx, System* system, engraving::AutoOnOff value)
{
    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        undo(new ChangeMStaffHideIfEmpty(engraving::toMeasure(mb), staffIdx, value));
    }
}

//---------------------------------------------------------
//   cmdToggleAutoplace
//---------------------------------------------------------

void Score::cmdToggleAutoplace(bool all)
{
    if (all) {
        bool val = !style().styleB(Sid::autoplaceEnabled);
        undoChangeStyleVal(Sid::autoplaceEnabled, val);
        setLayoutAll();
    } else {
        std::set<EngravingItem*> spanners;
        for (EngravingItem* e : selection().elements()) {
            if (e->isSpannerSegment()) {
                if (EngravingObject* ee = e->propertyDelegate(Pid::AUTOPLACE)) {
                    e = toEngravingItem(ee);
                }
                // spanner segments may each have their own autoplace setting
                // but if they delegate to spanner, only toggle once
                if (e->isSpanner()) {
                    if (muse::contains(spanners, e)) {
                        continue;
                    }
                    spanners.insert(e);
                }
            }
            PropertyFlags pf = e->propertyFlags(Pid::AUTOPLACE);
            if (pf == PropertyFlags::STYLED) {
                pf = PropertyFlags::UNSTYLED;
            }
            e->undoChangeProperty(Pid::AUTOPLACE, !e->getProperty(Pid::AUTOPLACE).toBool(), pf);
        }
    }
}
}
