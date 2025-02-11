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

/**
 \file
 Handling of several GUI commands.
*/

#include <assert.h>

#include "translation.h"

#include "infrastructure/messagebox.h"
#include "style/style.h"

#include "rw/xmlreader.h"

#include "accidental.h"
#include "articulation.h"
#include "barline.h"
#include "box.h"
#include "chord.h"
#include "clef.h"
#include "drumset.h"
#include "dynamic.h"
#include "factory.h"
#include "glissando.h"
#include "guitarbend.h"
#include "hairpin.h"
#include "harmony.h"
#include "key.h"
#include "laissezvib.h"
#include "linkedobjects.h"
#include "lyrics.h"
#include "masterscore.h"
#include "measure.h"
#include "measurerepeat.h"
#include "mscore.h"
#include "mscoreview.h"
#include "navigate.h"
#include "note.h"
#include "ornament.h"
#include "page.h"
#include "part.h"
#include "pitchspelling.h"
#include "rehearsalmark.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "sig.h"
#include "slur.h"
#include "staff.h"
#include "stafftype.h"
#include "stafftypechange.h"
#include "stem.h"
#include "stringdata.h"
#include "system.h"
#include "spacer.h"
#include "tie.h"
#include "timesig.h"

#include "tuplet.h"
#include "types.h"
#include "undo.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace muse::io;
using namespace mu::engraving;

namespace mu::engraving {
static UndoMacro::ChangesInfo changesInfo(const UndoStack* stack, bool undo = false)
{
    IF_ASSERT_FAILED(stack) {
        static UndoMacro::ChangesInfo empty;
        return empty;
    }

    const UndoMacro* actualMacro = stack->activeCommand();

    if (!actualMacro) {
        actualMacro = stack->last();
    }

    if (!actualMacro) {
        static UndoMacro::ChangesInfo empty;
        return empty;
    }

    return actualMacro->changesInfo(undo);
}

static ScoreChangesRange buildChangesRange(const CmdState& cmdState, const UndoMacro::ChangesInfo& changes)
{
    int startTick = cmdState.startTick().ticks();
    int endTick = cmdState.endTick().ticks();

    for (const auto& pair : changes.changedItems) {
        int tick = pair.first->tick().ticks();

        if (startTick > tick) {
            startTick = tick;
        }

        if (endTick < tick) {
            endTick = tick;
        }
    }

    return { startTick, endTick,
             cmdState.startStaff(), cmdState.endStaff(),
             std::move(changes.changedItems),
             std::move(changes.changedObjectTypes),
             std::move(changes.changedPropertyIdSet),
             std::move(changes.changedStyleIdSet) };
}

//---------------------------------------------------------
//    For use with Score::scanElements.
//    Reset positions and autoplacement for the given
//    element.
//---------------------------------------------------------

static void resetElementPosition(void*, EngravingItem* e)
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

static void resetTextProperties(void*, EngravingItem* e)
{
    if (e->generated() || !e->isTextBase()) {
        return;
    }

    static const std::vector<Pid> TEXT_STYLE_TO_RESET {
        Pid::FONT_FACE,
        Pid::FONT_SIZE,
        Pid::FONT_STYLE,
        Pid::SIZE_SPATIUM_DEPENDENT,
        Pid::FRAME_TYPE,
        Pid::TEXT_LINE_SPACING,
        Pid::FRAME_FG_COLOR,
        Pid::FRAME_BG_COLOR,
        Pid::FRAME_WIDTH,
        Pid::FRAME_PADDING,
        Pid::FRAME_ROUND,
        Pid::ALIGN
    };

    for (Pid pid : TEXT_STYLE_TO_RESET) {
        // TODO: use undoResetProperty: https://github.com/musescore/MuseScore/issues/16516
        // But for now, we'll use resetPropety since undoResetProperty leads to various problems
        e->resetProperty(pid);
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
//   startCmd
///   Start a GUI command by clearing the redraw area
///   and starting a user-visible undo.
//---------------------------------------------------------

void Score::startCmd(const TranslatableString& actionName)
{
    if (undoStack()->isLocked()) {
        return;
    }

    if (MScore::debugMode) {
        LOGD("===startCmd()");
    }

    if (undoStack()->hasActiveCommand()) {
        LOGD("Score::startCmd(): cmd already active");
        return;
    }

    MScore::setError(MsError::MS_NO_ERROR);

    cmdState().reset();

    // Start collecting low-level undo operations for a
    // user-visible undo action.
    undoStack()->beginMacro(this, actionName);
}

//---------------------------------------------------------
//   undoRedo
//---------------------------------------------------------

void Score::undoRedo(bool undo, EditData* ed)
{
    if (readOnly()) {
        return;
    }

    //! NOTE: the order of operations is very important here
    //! 1. for the undo operation, the list of changed elements is available before undo()
    //! 2. for the redo operation, the list of changed elements will be available after redo()
    UndoMacro::ChangesInfo changes;

    cmdState().reset();
    if (undo) {
        changes = changesInfo(undoStack(), undo);
        undoStack()->undo(ed);
    } else {
        undoStack()->redo(ed);
        changes = changesInfo(undoStack());
    }

    update(false);
    masterScore()->setPlaylistDirty();    // TODO: flag all individual operations
    updateSelection();

    ScoreChangesRange range = buildChangesRange(cmdState(), changes);
    changesChannel().send(range);
}

//---------------------------------------------------------
//   endCmd
///   End a GUI command by (if \a undo) ending a user-visible undo
///   and (always) updating the redraw area.
//---------------------------------------------------------

void Score::endCmd(bool rollback, bool layoutAllParts)
{
    if (undoStack()->isLocked()) {
        return;
    }

    if (!undoStack()->hasActiveCommand()) {
        LOGW() << "no command active";
        update();
        return;
    }

    if (readOnly() || MScore::_error != MsError::MS_NO_ERROR) {
        rollback = true;
    }

    if (rollback) {
        undoStack()->activeCommand()->unwind();
    }

    update(false, layoutAllParts);

    ScoreChangesRange range;
    if (!rollback) {
        range = buildChangesRange(cmdState(), changesInfo(undoStack()));
    }

    LOGD() << "Undo stack current macro child count: " << undoStack()->activeCommand()->childCount();

    const bool isCurrentCommandEmpty = undoStack()->activeCommand()->empty(); // nothing to undo?
    undoStack()->endMacro(isCurrentCommandEmpty);

    if (dirty()) {
        masterScore()->setPlaylistDirty(); // TODO: flag individual operations
    }

    cmdState().reset();

    if (!isCurrentCommandEmpty && !rollback) {
        changesChannel().send(range);
    }
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

void Score::update(bool resetCmdState, bool layoutAllParts)
{
    if (m_updatesLocked) {
        return;
    }

    TRACEFUNC;

    bool updateAll = false;
    {
        MasterScore* ms = masterScore();
        CmdState& cs = ms->cmdState();
        ms->deletePostponed();

        if (cs.layoutRange()) {
            for (Score* s : ms->scoreList()) {
                if (s != this && !s->isOpen() && ms->scoreList().size() > 1 && !layoutAllParts) {
                    continue;
                }
                s->doLayoutRange(cs.startTick(), cs.endTick());
            }
            updateAll = true;
        }
    }

    if (m_needSetUpTempoMap) {
        setUpTempoMap();
        m_needSetUpTempoMap = false;
    }

    MasterScore* ms = masterScore();
    CmdState& cs = ms->cmdState();
    if (updateAll || cs.updateAll()) {
        for (Score* s : scoreList()) {
            for (MuseScoreView* v : s->m_viewer) {
                v->updateAll();
            }
        }
    } else if (cs.updateRange()) {
        // updateRange updates only current score
        double d = style().spatium() * .5;
        m_updateState.refresh.adjust(-d, -d, 2 * d, 2 * d);
        for (MuseScoreView* v : m_viewer) {
            v->dataChanged(m_updateState.refresh);
        }
        m_updateState.refresh = RectF();
    }
    if (playlistDirty()) {
        masterScore()->setPlaylistClean();
    }
    if (resetCmdState) {
        cs.reset();
    }

    for (Score* score : ms->scoreList()) {
        Selection& sel = score->selection();
        if (sel.isRange() && !sel.isLocked()) {
            sel.updateSelectedElements();
        }
    }
}

void Score::lockUpdates(bool locked)
{
    m_updatesLocked = locked;
}

//---------------------------------------------------------
//   deletePostponed
//---------------------------------------------------------

void Score::deletePostponed()
{
    for (EngravingObject* e : m_updateState.deleteList) {
        if (e->isSystem()) {
            System* s = toSystem(e);
            std::list<SpannerSegment*> spanners = s->spannerSegments();
            for (SpannerSegment* ss : spanners) {
                if (ss->system() == s) {
                    ss->setSystem(0);
                }
            }
        }
    }
    muse::DeleteAll(m_updateState.deleteList);
    m_updateState.deleteList.clear();
}

//---------------------------------------------------------
//   cmdAddSpanner
//   drop VOLTA, OTTAVA, TRILL, PEDAL, DYNAMIC
//        HAIRPIN, LET_RING, VIBRATO and TEXTLINE
//---------------------------------------------------------

void Score::cmdAddSpanner(Spanner* spanner, const PointF& pos, bool systemStavesOnly)
{
    staff_idx_t staffIdx = spanner->staffIdx();
    Segment* segment;
    MeasureBase* mb = pos2measure(pos, &staffIdx, 0, &segment, 0);
    if (systemStavesOnly) {
        staffIdx = 0;
    }
    // ignore if we do not have a measure
    if (mb == 0 || mb->type() != ElementType::MEASURE) {
        LOGD("cmdAddSpanner: cannot put object here");
        delete spanner;
        return;
    }

    // all spanners live in voice 0 (except slurs/ties)
    track_idx_t track = staffIdx == muse::nidx ? muse::nidx : staffIdx * VOICES;

    spanner->setTrack(track);
    spanner->setTrack2(track);

    if (spanner->anchor() == Spanner::Anchor::SEGMENT) {
        spanner->setTick(segment->tick());
        Fraction lastTick = lastMeasure()->tick() + lastMeasure()->ticks();
        Fraction tick2 = std::min(segment->measure()->tick() + segment->measure()->ticks(), lastTick);
        spanner->setTick2(tick2);
    } else {      // Anchor::MEASURE, Anchor::CHORD, Anchor::NOTE
        Measure* m = toMeasure(mb);
        spanner->setTick(m->tick());
        spanner->setTick2(m->endTick());
    }
    spanner->eraseSpannerSegments();

    bool ctrlModifier = isSystemTextLine(spanner) && !systemStavesOnly;
    undoAddElement(spanner, true /*addToLinkedStaves*/, ctrlModifier);
}

//---------------------------------------------------------
//   cmdAddSpanner
//    used when applying a spanner to a selection
//---------------------------------------------------------

void Score::cmdAddSpanner(Spanner* spanner, staff_idx_t staffIdx, Segment* startSegment, Segment* endSegment, bool ctrlModifier)
{
    track_idx_t track = staffIdx * VOICES;
    spanner->setTrack(track);
    spanner->setTrack2(track);
    for (auto ss : spanner->spannerSegments()) {
        ss->setTrack(track);
    }

    bool isMeasureAnchor = spanner->anchor() == Spanner::Anchor::MEASURE;
    Fraction tick1 = isMeasureAnchor ? startSegment->measure()->tick() : startSegment->tick();
    spanner->setTick(tick1);

    Fraction tick2;
    if (!endSegment) {
        tick2 = lastSegment()->tick();
    } else if (endSegment == startSegment) {
        tick2 = startSegment->measure()->last()->tick();
    } else {
        tick2 = endSegment->tick();
    }
    if (isMeasureAnchor) {
        Measure* endMeasure = tick2measureMM(tick2);
        if (endMeasure->tick() != tick2) {
            tick2 = endMeasure->endTick();
        }
    }

    spanner->setTick2(tick2);
    undoAddElement(spanner, true, ctrlModifier);
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
        Fraction tick = cr->tick() + cr->actualTicks();
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
    bool shouldSelectFirstNote = selIsSingle && _nl[0]->tieFor();

    std::sort(_nl.begin(), _nl.end(), [](const Note* a, const Note* b) -> bool {
        return a->tick() < b->tick();
    });

    for (auto n : _nl) {
        if (std::find(tmpnl.begin(), tmpnl.end(), n) != tmpnl.end()) {
            continue;
        }
        tmpnl.push_back(n);
        if (n->tieFor() && n->tieFor()->endNote() && std::find(tmpnl.begin(), tmpnl.end(), n->tieFor()->endNote()) == tmpnl.end()) {
            Note* currNote = n->tieFor()->endNote();
            do {
                tmpnl.push_back(currNote);
                currNote = currNote->tieFor() ? currNote->tieFor()->endNote() : nullptr;
            }while (currNote);
        }
        if (selIsList && n->selected()) {
            deselect(n);
        }
    }

    Note* prevTied = nullptr;
    std::vector<EngravingItem*> notesToSelect;
    for (Note* on : tmpnl) {
        Chord* chord = on->chord();
        int valTmp = val < 0 ? val + 1 : val - 1;

        int npitch;
        int ntpc1;
        int ntpc2;
        bool accidental = m_is.noteEntryMode() && m_is.accidentalType() != AccidentalType::NONE;
        bool forceAccidental = false;
        if (std::abs(valTmp) != 7 || accidental) {
            int line      = on->line() - valTmp;
            Fraction tick = chord->tick();
            Staff* estaff = staff(chord->vStaffIdx());
            ClefType clef = estaff->clef(tick);
            Key key       = estaff->key(tick);
            int ntpc;
            if (accidental) {
                AccidentalVal acci = Accidental::subtype2value(m_is.accidentalType());
                int step = absStep(line, clef);
                int octave = step / 7;
                npitch = step2pitch(step) + octave * 12 + int(acci);
                forceAccidental = (npitch == line2pitch(line, clef, key));
                ntpc = step2tpc(step % 7, acci);
            } else {
                npitch = line2pitch(line, clef, key);
                ntpc = pitch2tpc(npitch, key, Prefer::NEAREST);
            }

            Interval v = estaff->transpose(tick);
            if (v.isZero()) {
                ntpc1 = ntpc2 = ntpc;
            } else {
                if (style().styleB(Sid::concertPitch)) {
                    v.flip();
                    ntpc1 = ntpc;
                    ntpc2 = mu::engraving::transposeTpc(ntpc, v, true);
                } else {
                    npitch += v.chromatic;
                    ntpc2 = ntpc;
                    ntpc1 = mu::engraving::transposeTpc(ntpc, v, true);
                }
            }
        } else {   //special case for octave
            Interval interval(7, 12);
            if (val < 0) {
                interval.flip();
            }
            transposeInterval(on->pitch(), on->tpc(), &npitch, &ntpc1, interval, false);
            ntpc1 = on->tpc1();
            ntpc2 = on->tpc2();
        }
        if (npitch < 0 || npitch > 127) {
            notesToSelect.push_back(dynamic_cast<EngravingItem*>(on));
            continue;
        }

        Note* note = Factory::createNote(chord);
        note->setParent(chord);
        note->setTrack(chord->track());
        note->setPitch(npitch, ntpc1, ntpc2);
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
            notesToSelect.push_back(dynamic_cast<EngravingItem*>(note));
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
    if (m_is.cr() == toChordRest(_nl[0]->chord()) && selIsSingle) {
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

    note->setPitch(pitch);
    // find corresponding note within chord and use its tpc information
    for (Note* n : ch->notes()) {
        if (n->pitch() == pitch) {
            note->setTpc1(n->tpc1());
            note->setTpc2(n->tpc2());
            note->setString(n->string());
            note->setFret(n->fret());
            break;
        }
    }
    // note with same pitch not found, derive tpc from pitch / key
    if (!tpcIsValid(note->tpc1()) || !tpcIsValid(note->tpc2())) {
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
    if (note->isPreBendStart()) {
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

    if (note->bendBack() && (type == GuitarBendType::PRE_BEND || type == GuitarBendType::GRACE_NOTE_BEND)) {
        return nullptr;
    }

    if (note->bendFor() && (type == GuitarBendType::BEND || type == GuitarBendType::SLIGHT_BEND)) {
        return nullptr;
    }

    Chord* chord = note->chord();

    if (type == GuitarBendType::BEND) {
        for (Spanner* sp : note->spannerFor()) {
            if (sp->isGuitarBend() || sp->isGlissando()) {
                return nullptr;
            }
        }

        if (!endNote) {
            endNote = SLine::guessFinalNote(note);
        }

        bool suitableEndNote = endNote;
        if (endNote) {
            for (Spanner* sp : endNote->spannerBack()) {
                if (sp->isTie() || sp->isGlissando() || sp->isGuitarBend()) {
                    suitableEndNote = false;
                    break;
                }
            }
        }

        if (!suitableEndNote) {
            endNote = GuitarBend::createEndNote(note);
        }

        if (!endNote) {
            return nullptr;
        }
    }

    GuitarBend* bend = new GuitarBend(score()->dummy()->note());
    bend->setAnchor(Spanner::Anchor::NOTE);
    bend->setTick(chord->tick());
    bend->setTrack(chord->track());

    if (type == GuitarBendType::BEND) {
        bend->setType(chord->isGrace() ? GuitarBendType::GRACE_NOTE_BEND : type);
        bend->setStartElement(note);
        bend->setTick2(endNote->tick());
        bend->setTrack2(endNote->track());
        bend->setEndElement(endNote);
        bend->setParent(note);
        GuitarBend::fixNotesFrettingForStandardBend(note, endNote);
    } else {
        bend->setType(type);
        bend->setTick2(chord->tick());
        bend->setTrack2(chord->track());

        if (type == GuitarBendType::PRE_BEND || type == GuitarBendType::GRACE_NOTE_BEND) {
            const GraceNotesGroup& gracesBefore = chord->graceNotesBefore();

            // Create grace note
            Note* graceNote = gracesBefore.empty()
                              ? setGraceNote(chord, note->pitch(), NoteType::APPOGGIATURA, Constants::DIVISION / 2)
                              : addNote(gracesBefore.back(), note->noteVal());
            graceNote->transposeDiatonic(-1, true, false);
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
        } else if (type == GuitarBendType::SLIGHT_BEND) {
            bend->setParent(note);
            bend->setStartElement(note);
            // Slight bends don't end on another note
            bend->setEndElement(note);
        }
    }

    if (bend->type() == GuitarBendType::GRACE_NOTE_BEND) {
        bend->setEndTimeFactor(GuitarBend::GRACE_NOTE_BEND_DEFAULT_END_TIME_FACTOR);
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
//    return segment of last created note/rest
//---------------------------------------------------------

Segment* Score::setNoteRest(Segment* segment, track_idx_t track, NoteVal nval, Fraction sd, DirectionV stemDirection,
                            bool forceAccidental, const std::set<SymId>& articulationIds, bool rhythmic, InputState* externalInputState)
{
    assert(segment->segmentType() == SegmentType::ChordRest);
    InputState& is = externalInputState ? (*externalInputState) : m_is;

    bool isRest   = nval.pitch == -1;
    Fraction tick = segment->tick();
    EngravingItem* nr   = nullptr;
    Tie* tie      = nullptr;
    ChordRest* cr = toChordRest(segment->element(track));
    Tuplet* tuplet = cr && cr->tuplet() ? cr->tuplet() : nullptr;
    Measure* measure = nullptr;
    bool targetIsRest = cr && cr->isRest();
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
        std::vector<TDuration> dl;
        if (rhythmic) {
            dl = toRhythmicDurationList(dd, isRest, segment->rtick(), sigmap()->timesig(tick).nominal(), measure, 1);
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
                ncr->setTicks(d.isMeasure() ? measure->ticks() : d.fraction());
            } else {
                nr = note = Factory::createNote(this->dummy()->chord());

                if (tie) {
                    tie->setEndNote(note);
                    tie->setTick2(tie->endNote()->tick());
                    note->setTieBack(tie);
                    addTie = tie;
                }
                Chord* chord = Factory::createChord(this->dummy()->segment());
                chord->setTrack(track);
                chord->setDurationType(d);
                chord->setTicks(d.fraction());
                chord->setStemDirection(stemDirection);
                chord->add(note);
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

        Segment* nseg = tick2segment(tick, false, SegmentType::ChordRest);
        if (!nseg) {
            LOGD("reached end of score");
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
        if (is.slur() && nr->type() == ElementType::NOTE) {
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
//    gap does not exceed measure or scope of tuplet
//
//    return size of actual gap
//---------------------------------------------------------

Fraction Score::makeGap(Segment* segment, track_idx_t track, const Fraction& _sd, Tuplet* tuplet, bool keepChord)
{
    assert(_sd.numerator());

    Measure* measure = segment->measure();
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
        if (!cr) {
            if (seg->tick() < nextTick) {
                continue;
            }
            Segment* seg1 = seg->next(SegmentType::ChordRest);
            Fraction tick2 = seg1 ? seg1->tick() : seg->measure()->tick() + seg->measure()->ticks();
            Fraction td(tick2 - seg->tick());
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
        Fraction timeStretch = cr->staff()->timeStretch(cr->tick());
        nextTick += actualTicks(td, tuplet, timeStretch);
        if (sd < td) {
            //
            // we removed too much
            //
            accumulated = _sd;
            Fraction rd = td - sd;
            Fraction tick = cr->tick() + actualTicks(sd, tuplet, timeStretch);

            std::vector<TDuration> dList;
            if (tuplet || staff(track / VOICES)->isLocalTimeSignature(tick)) {
                dList = toDurationList(rd, false);
                std::reverse(dList.begin(), dList.end());
            } else {
                dList = toRhythmicDurationList(rd, true, tick - measure->tick(), sigmap()->timesig(tick).nominal(), measure, 0);
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
//      Fraction ticks = measure->tick() + measure->ticks() - segment->tick();
//      Fraction td = Fraction::fromTicks(ticks);
// NEEDS REVIEW !!
// once the statement below is removed, these two lines do nothing
//      if (td > sd)
//            td = sd;
// ???  accumulated should already contain the total value of the created gap: line 749, 811 or 838
//      this line creates a double-sized gap if the needed gap crosses a measure boundary
//      by adding again the duration already added in line 838
//      accumulated += td;

    const Fraction t1 = firstSegmentEnd;
    const Fraction t2 = firstSegment->tick() + accumulated;
    if (t1 < t2) {
        Segment* s1 = tick2rightSegment(t1);
        Segment* s2 = tick2rightSegment(t2);
        typedef SelectionFilterType Sel;
        // chord symbols can exist without chord/rest so they should not be removed
        constexpr Sel filter = static_cast<Sel>(int(Sel::ALL) & ~int(Sel::CHORD_SYMBOL));
        deleteAnnotationsFromRange(s1, s2, track, track + 1, filter);
        deleteSlursFromRange(t1, t2, track, track + 1, filter);
    }

    return accumulated;
}

//---------------------------------------------------------
//   makeGap1
//    make time gap for each voice
//    starting at tick+voiceOffset[voice] by removing/shortening
//    chord/rest
//    - cr is top level (not part of a tuplet)
//    - do not stop at measure end
//---------------------------------------------------------

bool Score::makeGap1(const Fraction& baseTick, staff_idx_t staffIdx, const Fraction& len, int voiceOffset[VOICES])
{
    Measure* m = tick2measure(baseTick);
    if (!m) {
        LOGD() << "No measure to paste at tick " << baseTick.toString();
        return false;
    }

    Segment* seg = m->undoGetSegment(SegmentType::ChordRest, baseTick);
    track_idx_t strack = staffIdx * VOICES;
    for (track_idx_t track = strack; track < strack + VOICES; track++) {
        if (voiceOffset[track - strack] == -1) {
            continue;
        }
        Fraction tick = baseTick + Fraction::fromTicks(voiceOffset[track - strack]);
        Measure* m   = tick2measure(tick);
        if ((track % VOICES) && !m->hasVoices(staffIdx)) {
            continue;
        }

        Fraction newLen = len - Fraction::fromTicks(voiceOffset[track - strack]);
        assert(newLen.numerator() != 0);

        if (newLen > Fraction(0, 1)) {
            const Fraction endTick = tick + newLen;
            typedef SelectionFilterType Sel;
            // chord symbols can exist without chord/rest so they should not be removed
            constexpr Sel filter = static_cast<Sel>(int(Sel::ALL) & ~int(Sel::CHORD_SYMBOL));
            deleteAnnotationsFromRange(tick2rightSegment(tick), tick2rightSegment(endTick), track, track + 1, filter);
            deleteOrShortenOutSpannersFromRange(tick, endTick, track, track + 1, filter);
        }

        seg = m->undoGetSegment(SegmentType::ChordRest, tick);
        bool result = makeGapVoice(seg, track, newLen, tick);
        if (track == strack && !result) {   // makeGap failed for first voice
            return false;
        }
    }
    return true;
}

bool Score::makeGapVoice(Segment* seg, track_idx_t track, Fraction len, const Fraction& tick)
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
                return makeGapVoice(seg, track, len, tick);
            }
            if (seg1->element(track)) {
                break;
            }
            seg1 = seg1->prev(SegmentType::ChordRest);
        }
        ChordRest* cr1 = toChordRest(seg1->element(track));
        Fraction srcF = cr1->ticks();
        Fraction dstF = tick - cr1->tick();
        std::vector<TDuration> dList = toDurationList(dstF, true);
        size_t n = dList.size();
        undoChangeChordRestLen(cr1, TDuration(dList[0]));
        if (n > 1) {
            Fraction crtick = cr1->tick() + cr1->actualTicks();
            Measure* measure = tick2measure(crtick);
            if (cr1->type() == ElementType::CHORD) {
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
        Fraction l = makeGap(cr->segment(), cr->track(), len, 0);
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
            options.createEmptyMeasures = false;
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
//    gap - gap len
//---------------------------------------------------------

std::list<Fraction> Score::splitGapToMeasureBoundaries(ChordRest* cr, Fraction gap)
{
    std::list<Fraction> flist;

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
        m = m->nextMeasure();
        if (m == 0) {
            return flist;
        }
        s = m->first(SegmentType::ChordRest);
    }
    return flist;
}

//---------------------------------------------------------
//   changeCRlen
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
        undoChangeChordRestLen(cr, dList[0]);
        Fraction tick2 = cr->tick();
        for (unsigned i = 1; i < dList.size(); ++i) {
            tick2 += actualTicks(dList[i - 1].ticks(), tuplet, timeStretch);
            TDuration d = dList[i];
            setRest(tick2, track, d.fraction(), (d.dots() > 0), tuplet);
        }
        if (fillWithRest) {
            setRest(cr->tick() + cr->actualTicks(), track, srcF - dstF, false, tuplet);
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
    std::list<Fraction> flist = splitGapToMeasureBoundaries(cr, dstF);
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
        const Measure* m1 = m->nextMeasure();
        if (m1 == 0) {
            break;
        }
        s = m1->first(SegmentType::ChordRest);
        cr1 = toChordRest(s->element(track));
    }
    connectTies();

    if (elementToSelect) {
        if (containsElement(elementToSelect)) {
            select(elementToSelect, SelectType::SINGLE, 0);
        }
    }
}

//---------------------------------------------------------
//   upDownChromatic
//---------------------------------------------------------

static void upDownChromatic(bool up, int pitch, Note* n, Key key, int tpc1, int tpc2, int& newPitch, int& newTpc1, int& newTpc2)
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
///   Increment/decrement pitch of note by one or by an octave.
//---------------------------------------------------------

void Score::upDown(bool up, UpDownMode mode)
{
    std::list<Note*> el = selection().uniqueNotes();

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
            case UpDownMode::OCTAVE:                            // move same note to next string, if possible
            {
                const StaffType* stt = staff->staffType(tick);
                string = stt->physStringToVisual(string);
                string += (up ? -1 : 1);
                if (string < 0 || string >= static_cast<int>(stringData->strings())) {
                    return;                                 // no next string to move to
                }
                string = stt->visualStringToPhys(string);
                fret = stringData->fret(pitch + pitchOffset, string, staff);
                if (fret == -1) {                            // can't have that note on that string
                    return;
                }
                // newPitch and newTpc remain unchanged
            }
            break;

            case UpDownMode::DIATONIC:                          // increase / decrease the pitch,
                // letting the algorithm to choose fret & string
                upDownChromatic(up, pitch, oNote, key, tpc1, tpc2, newPitch, newTpc1, newTpc2);
                break;

            case UpDownMode::CHROMATIC:                         // increase / decrease the fret
            {                                               // without changing the string
                // compute new fret
                if (!stringData->frets()) {
                    LOGD("upDown tab chromatic: no frets?");
                    return;
                }
                fret += (up ? 1 : -1);
                if (fret < 0 || fret > stringData->frets()) {
                    LOGD("upDown tab in-string: out of fret range");
                    return;
                }
                // update pitch and tpc's and check it matches stringData
                upDownChromatic(up, pitch, oNote, key, tpc1, tpc2, newPitch, newTpc1, newTpc2);
                if (newPitch + pitchOffset != stringData->getPitch(string, fret, staff) && !oNote->bendBack()) {
                    // oh-oh: something went very wrong!
                    LOGD("upDown tab in-string: pitch mismatch");
                    return;
                }
                // store the fretting change before undoChangePitch() chooses
                // a fretting of its own liking!
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
                // newTpc remains unchanged
                break;

            case UpDownMode::CHROMATIC:
                upDownChromatic(up, pitch, oNote, key, tpc1, tpc2, newPitch, newTpc1, newTpc2);
                break;

            case UpDownMode::DIATONIC:
            {
                Note* firstTiedNote = oNote->firstTiedNote();
                int newLine = firstTiedNote->line() + (up ? -1 : 1);
                Staff* vStaff = score()->staff(firstTiedNote->chord()->vStaffIdx());
                Key vKey = vStaff->key(tick);
                Key cKey = vStaff->concertKey(tick);
                Interval interval = vStaff->part()->instrument(tick)->transpose();

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
                    if (!firstTiedNote->concertPitch()) {
                        newPitch += interval.chromatic;
                    } else {
                        interval.flip();
                        vKey = transposeKey(cKey, interval, vStaff->part()->preferSharpFlat());
                    }
                    newTpc1 = pitch2tpc(newPitch, cKey, Prefer::NEAREST);
                    newTpc2 = pitch2tpc(newPitch - firstTiedNote->transposition(), vKey, Prefer::NEAREST);
                }
            }
            break;
            }
            break;
        }

        if ((oNote->pitch() != newPitch) || (oNote->tpc1() != newTpc1) || oNote->tpc2() != newTpc2) {
            // remove accidental if present to make sure
            // user added accidentals are removed here
            // unless it's an octave change
            // in this case courtesy accidentals are preserved
            // because they're now harder to be re-entered due to the revised note-input workflow
            if (mode != UpDownMode::OCTAVE) {
                auto l = oNote->linkList();
                for (EngravingObject* e : l) {
                    Note* ln = toNote(e);
                    if (ln->accidental()) {
                        doUndoRemoveElement(ln->accidental());
                    }
                }
            }
            undoChangePitch(oNote, newPitch, newTpc1, newTpc2);
        }
        // store fret change only if undoChangePitch has not been called,
        // as undoChangePitch() already manages fret changes, if necessary
        else if (staff->staffType(tick)->group() == StaffGroup::TAB) {
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

        // play new note with velocity 80 for 0.3 sec:
        setPlayNote(true);
    }
    setSelectionChanged(true);
}

//---------------------------------------------------------
//   upDownDelta
///   Add the delta to the pitch of note.
//---------------------------------------------------------

void Score::upDownDelta(int pitchDelta)
{
    while (pitchDelta > 0) {
        upDown(true, UpDownMode::CHROMATIC);
        pitchDelta--;
    }

    while (pitchDelta < 0) {
        upDown(false, UpDownMode::CHROMATIC);
        pitchDelta++;
    }
}

//---------------------------------------------------------
//   toggleArticulation
///   Toggle attribute \a attr for all selected notes/rests.
///
///   Called from padToggle() to add note prefix/accent.
//---------------------------------------------------------

void Score::toggleArticulation(SymId attr)
{
    std::set<Chord*> set;
    for (EngravingItem* el : selection().elements()) {
        if (el->isNote() || el->isChord()) {
            Chord* cr = 0;
            // apply articulation on a given chord only once
            if (el->isNote()) {
                cr = toNote(el)->chord();
                if (muse::contains(set, cr)) {
                    continue;
                }
            }
            Articulation* na = Factory::createArticulation(this->dummy()->chord());
            na->setSymId(attr);
            if (!toggleArticulation(el, na)) {
                delete na;
            }

            if (cr) {
                set.insert(cr);
            }
        }
    }
}

//---------------------------------------------------------
//   toggleOrnament
///   Toggle attribute \a attr for all selected notes/rests.
///
///   Like toggleArticulation, but for ornaments.
//---------------------------------------------------------

void Score::toggleOrnament(SymId attr)
{
    std::set<Chord*> set;
    for (EngravingItem* el : selection().elements()) {
        if (el->isNote() || el->isChord()) {
            Chord* cr = 0;
            // apply articulation on a given chord only once
            if (el->isNote()) {
                cr = toNote(el)->chord();
                if (muse::contains(set, cr)) {
                    continue;
                }
            }
            Ornament* na = Factory::createOrnament(this->dummy()->chord());
            na->setSymId(attr);
            if (!toggleArticulation(el, na)) {
                delete na;
            }

            if (cr) {
                set.insert(cr);
            }
        }
    }
}

void Score::toggleAccidental(AccidentalType at)
{
    if (m_is.accidentalType() == at) {
        at = AccidentalType::NONE;
    }

    if (noteEntryMode()) {
        m_is.setAccidentalType(at);
        m_is.setRest(false);

        if (usingNoteEntryMethod(NoteEntryMethod::BY_DURATION)) {
            applyAccidentalToInputNotes();
        }
    } else {
        if (selection().isNone()) {
            m_is.setAccidentalType(at);
            m_is.setDuration(DurationType::V_QUARTER);
            m_is.setRest(false);
        } else {
            changeAccidental(at);
        }
    }
}

void Score::applyAccidentalToInputNotes()
{
    const AccidentalVal acc = Accidental::subtype2value(m_is.accidentalType());
    const bool concertPitch = style().styleB(Sid::concertPitch);
    NoteValList notes = m_is.notes();

    for (NoteVal& nval : notes) {
        const int oldPitch = nval.pitch;
        const int step = mu::engraving::pitch2step(oldPitch);
        const int newTpc = mu::engraving::step2tpc(step, acc);

        nval.pitch += static_cast<int>(acc);

        if (concertPitch) {
            nval.tpc1 = newTpc;
        } else {
            nval.tpc2 = newTpc;
        }
    }

    m_is.setNotes(notes);
}

//---------------------------------------------------------
//   changeAccidental
///   Change accidental to subtype \a idx for all selected
///   notes.
//---------------------------------------------------------

void Score::changeAccidental(AccidentalType idx)
{
    for (EngravingItem* item : selection().elements()) {
        Accidental* accidental = 0;
        Note* note = 0;
        switch (item->type()) {
        case ElementType::ACCIDENTAL:
            accidental = toAccidental(item);

            if (accidental->accidentalType() == idx) {
                changeAccidental(accidental->note(), AccidentalType::NONE);
            } else {
                changeAccidental(accidental->note(), idx);
            }

            break;
        case ElementType::NOTE:
            note = toNote(item);
            changeAccidental(note, idx);
            break;
        default:
            break;
        }
    }
}

//---------------------------------------------------------
//   changeAccidental2
//---------------------------------------------------------

static void changeAccidental2(Note* n, int pitch, int tpc)
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
    score->undoChangePitch(n, pitch, tpc1, tpc2);
}

//---------------------------------------------------------
//   changeAccidental
///   Change accidental to subtype \accidental for
///   note \a note.
//---------------------------------------------------------

void Score::changeAccidental(Note* note, AccidentalType accidental)
{
    Chord* chord = note->chord();
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
    Staff* estaff = staff(chord->staffIdx() + chord->staffMove());
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
                undoRemoveElement(a);
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
    setPlayNote(true);
    setSelectionChanged(true);
}

//---------------------------------------------------------
//   toggleArticulation
//---------------------------------------------------------

bool Score::toggleArticulation(EngravingItem* el, Articulation* a)
{
    Chord* c;
    if (el->isNote()) {
        c = toNote(el)->chord();
    } else if (el->isChord()) {
        c = toChord(el);
    } else {
        return false;
    }
    Articulation* oa = c->hasArticulation(a);
    if (oa) {
        undoRemoveElement(oa);
        return false;
    }

    if (!a->isDouble()) {
        a->setParent(c);
        a->setTrack(c->track());
        undoAddElement(a);
        return true;
    }

    // Split the new articulation into "sub-components", only add the unique ones (not present in the chord)...
    std::set<SymId> newSubComponentIds = splitArticulations({ a->symId() });
    for (const SymId& id : newSubComponentIds) {
        Articulation* articCopy = a->clone();
        articCopy->setSymId(id);

        if (!c->hasArticulation(articCopy)) {
            articCopy->setParent(c);
            articCopy->setTrack(c->track());
            undoAddElement(articCopy);
            continue;
        }
        delete articCopy;
    }
    return true;
}

//---------------------------------------------------------
//   resetUserStretch
//---------------------------------------------------------

void Score::resetUserStretch()
{
    Measure* m1 = nullptr;
    Measure* m2 = nullptr;
    // retrieve span of selection
    Segment* s1 = m_selection.startSegment();
    Segment* s2 = m_selection.endSegment();
    // if either segment is not returned by the selection
    // (for instance, no selection) fall back to first/last measure
    if (!s1) {
        m1 = firstMeasureMM();
    } else {
        m1 = s1->measure();
    }
    if (!s2) {
        m2 = lastMeasureMM();
    } else {
        m2 = s2->measure();
    }
    if (!m1 || !m2) {               // should not happen!
        return;
    }

    for (Measure* m = m1; m; m = m->nextMeasureMM()) {
        m->undoChangeProperty(Pid::USER_STRETCH, 1.0);
        if (m == m2) {
            break;
        }
    }
}

//---------------------------------------------------------
//   moveUp
//---------------------------------------------------------

void Score::moveUp(ChordRest* cr)
{
    Staff* staff  = cr->staff();
    Part* part    = staff->part();
    staff_idx_t rstaff    = staff->rstaff();
    int staffMove = cr->staffMove();

    if ((staffMove == -1) || (static_cast<int>(rstaff) + staffMove <= 0)) {
        return;
    }

    const std::vector<Staff*>& staves = part->staves();
    // we know that staffMove+rstaff-1 index exists due to the previous condition.
    if (staff->staffType(cr->tick())->group() != StaffGroup::STANDARD
        || staves.at(rstaff + staffMove - 1)->staffType(cr->tick())->group() != StaffGroup::STANDARD) {
        LOGD("User attempted to move a note from/to a staff which does not use standard notation - ignoring.");
    } else {
        // move the chord up a staff
        undo(new ChangeChordStaffMove(cr, staffMove - 1));
    }
}

//---------------------------------------------------------
//   moveDown
//---------------------------------------------------------

void Score::moveDown(ChordRest* cr)
{
    Staff* staff  = cr->staff();
    Part* part    = staff->part();
    staff_idx_t rstaff = staff->rstaff();
    int staffMove = cr->staffMove();
    // calculate the number of staves available so that we know whether there is another staff to move down to
    size_t rstaves = part->nstaves();

    if ((staffMove == 1) || (rstaff + staffMove >= rstaves - 1)) {
        LOGD("moveDown staffMove==%d  rstaff %zu rstaves %zu", staffMove, rstaff, rstaves);
        return;
    }

    const std::vector<Staff*>& staves = part->staves();
    // we know that staffMove+rstaff+1 index exists due to the previous condition.
    if (staff->staffType(cr->tick())->group() != StaffGroup::STANDARD
        || staves.at(staffMove + rstaff + 1)->staffType(cr->tick())->group() != StaffGroup::STANDARD) {
        LOGD("User attempted to move a note from/to a staff which does not use standard notation - ignoring.");
    } else {
        // move the chord down a staff
        undo(new ChangeChordStaffMove(cr, staffMove + 1));
    }
}

//---------------------------------------------------------
//   cmdAddStretch
//---------------------------------------------------------

void Score::cmdAddStretch(double val)
{
    if (!selection().isRange()) {
        return;
    }
    Fraction startTick = selection().tickStart();
    Fraction endTick   = selection().tickEnd();
    for (Measure* m = firstMeasureMM(); m; m = m->nextMeasureMM()) {
        if (m->tick() < startTick) {
            continue;
        }
        if (m->tick() >= endTick) {
            break;
        }
        double stretch = m->userStretch();
        stretch += val;
        if (stretch < 0) {
            stretch = 0;
        }
        m->undoChangeProperty(Pid::USER_STRETCH, stretch);
    }
}

void Score::cmdResetToDefaultLayout()
{
    TRACEFUNC;

    StyleIdSet dontResetTheseStyles {
        Sid::lyricsPlacement,
        Sid::repeatBarTips,
        Sid::startBarlineSingle,
        Sid::startBarlineMultiple,
        Sid::dividerLeft,
        Sid::dividerRightY,
        Sid::useStraightNoteFlags,
        Sid::mrNumberSeries,
        Sid::mrNumberEveryXMeasures,
        Sid::mrNumberSeriesWithParentheses,
        Sid::oneMeasureRepeatShow1,
        Sid::fourMeasureRepeatShowExtenders,
        Sid::useWideBeams,
        Sid::hairpinPlacement,
        Sid::pedalPlacement,
        Sid::trillPlacement,
        Sid::vibratoPlacement,
        Sid::harmonyPlacement,
        Sid::romanNumeralPlacement,
        Sid::nashvilleNumberPlacement,
        Sid::harmonyVoiceLiteral,
        Sid::harmonyVoicing,
        Sid::harmonyDuration,
        Sid::capoPosition,
        Sid::fretNumPos,
        Sid::fretPlacement,
        Sid::fretStrings,
        Sid::fretFrets,
        Sid::fretNut,
        Sid::fretOrientation,
        Sid::showPageNumber,
        Sid::showPageNumberOne,
        Sid::pageNumberOddEven,
        Sid::showMeasureNumber,
        Sid::showMeasureNumberOne,
        Sid::measureNumberInterval,
        Sid::measureNumberSystem,
        Sid::measureNumberAllStaves,
        Sid::genClef,
        Sid::hideTabClefAfterFirst,
        Sid::genKeysig,
        Sid::genCourtesyTimesig,
        Sid::genCourtesyKeysig,
        Sid::genCourtesyClef,
        Sid::swingRatio,
        Sid::swingUnit,
        Sid::useStandardNoteNames,
        Sid::useGermanNoteNames,
        Sid::useFullGermanNoteNames,
        Sid::useSolfeggioNoteNames,
        Sid::useFrenchNoteNames,
        Sid::automaticCapitalization,
        Sid::lowerCaseMinorChords,
        Sid::lowerCaseBassNotes,
        Sid::allCapsNoteNames,
        Sid::chordStyle,
        Sid::chordsXmlFile,
        Sid::chordDescriptionFile,
        Sid::concertPitch,
        Sid::createMultiMeasureRests,
        Sid::minEmptyMeasures,
        Sid::hideEmptyStaves,
        Sid::dontHideStavesInFirstSystem,
        Sid::enableIndentationOnFirstSystem,
        Sid::firstSystemIndentationValue,
        Sid::hideInstrumentNameIfOneInstrument,
        Sid::gateTime,
        Sid::tenutoGateTime,
        Sid::staccatoGateTime,
        Sid::slurGateTime,
        Sid::sectionPause,
        Sid::showHeader,
        Sid::headerFirstPage,
        Sid::headerOddEven,
        Sid::evenHeaderL,
        Sid::evenHeaderC,
        Sid::evenHeaderR,
        Sid::oddHeaderL,
        Sid::oddHeaderC,
        Sid::oddHeaderR,
        Sid::showFooter,
        Sid::footerFirstPage,
        Sid::footerOddEven,
        Sid::evenFooterL,
        Sid::evenFooterC,
        Sid::evenFooterR,
        Sid::oddFooterL,
        Sid::oddFooterC,
        Sid::oddFooterR,
        Sid::tupletOutOfStaff,
        Sid::tupletDirection,
        Sid::tupletBracketType,
        Sid::dynamicsPlacement,
        Sid::textLinePlacement,
        Sid::harpPedalDiagramPlacement,
        Sid::harpPedalTextDiagramPlacement,
        Sid::expressionPlacement,
        Sid::tupletNumberType,
        Sid::mmRestShowMeasureNumberRange,
        Sid::mmRestRangeBracketType,
        Sid::mmRestRangeVPlacement,
        Sid::staffTextPlacement,
        Sid::repeatLeftPlacement,
        Sid::repeatRightPlacement,
        Sid::instrumentChangePlacement,
        Sid::stickingPlacement,
        Sid::letRingPlacement,
        Sid::palmMutePlacement,
        Sid::pageWidth,
        Sid::pageHeight,
        Sid::pagePrintableWidth,
        Sid::pageEvenTopMargin,
        Sid::pageEvenBottomMargin,
        Sid::pageEvenLeftMargin,
        Sid::pageOddTopMargin,
        Sid::pageOddBottomMargin,
        Sid::pageOddLeftMargin,
        Sid::pageTwosided,
        Sid::spatium,
        Sid::concertPitch,
        Sid::createMultiMeasureRests
    };

    auto resetPositionAndTextProperties = [](void* ptr, EngravingItem* e) {
        resetElementPosition(ptr, e);
        resetTextProperties(ptr, e);
    };

    cmdResetMeasuresLayout();
    scanElements(nullptr, resetPositionAndTextProperties);
    cmdResetAllStyles(dontResetTheseStyles);
    undoRemoveAllLocks();
}

//---------------------------------------------------------
//   cmdResetBeamMode
//---------------------------------------------------------

void Score::cmdResetBeamMode()
{
    bool noSelection = selection().isNone();
    if (noSelection) {
        cmdSelectAll();
    } else if (!selection().isRange()) {
        LOGD("no system or staff selected");
        return;
    }

    Fraction endTick = selection().tickEnd();

    for (Segment* seg = selection().firstChordRestSegment(); seg && seg->tick() < endTick; seg = seg->next1(SegmentType::ChordRest)) {
        for (track_idx_t track = selection().staffStart() * VOICES; track < selection().staffEnd() * VOICES; ++track) {
            ChordRest* cr = toChordRest(seg->element(track));
            if (!cr) {
                continue;
            }
            if (cr->type() == ElementType::CHORD) {
                if (cr->beamMode() != BeamMode::AUTO) {
                    cr->undoChangeProperty(Pid::BEAM_MODE, BeamMode::AUTO);
                }
            } else if (cr->type() == ElementType::REST) {
                if (cr->beamMode() != BeamMode::NONE) {
                    cr->undoChangeProperty(Pid::BEAM_MODE, BeamMode::NONE);
                }
            }
        }
    }
    if (noSelection) {
        deselectAll();
    }
}

void Score::cmdResetTextStyleOverrides()
{
    TRACEFUNC;

    scanElements(nullptr, resetTextProperties);
}

void Score::cmdResetAllStyles(const StyleIdSet& exceptTheseOnes)
{
    TRACEFUNC;

    int beginIdx = int(Sid::NOSTYLE) + 1;
    int endIdx = int(Sid::STYLES);

    StyleIdSet stylesToReset;

    for (int idx = beginIdx; idx < endIdx; idx++) {
        Sid styleId = Sid(idx);
        if (!muse::contains(exceptTheseOnes, styleId)) {
            stylesToReset.insert(styleId);
        }
    }

    score()->resetStyleValues(stylesToReset);
}

// Removes system/page breaks and spacers
void Score::cmdResetMeasuresLayout()
{
    TRACEFUNC;

    std::vector<EngravingItem*> itemsToRemove;

    for (MeasureBase* mb = first(); mb; mb = mb->next()) {
        if (mb->isMeasure()) {
            mb->undoResetProperty(Pid::USER_STRETCH);

            for (const MStaff* staff : toMeasure(mb)->mstaves()) {
                if (Spacer* spacer = staff->vspacerDown()) {
                    itemsToRemove.push_back(spacer);
                }

                if (Spacer* spacer = staff->vspacerUp()) {
                    itemsToRemove.push_back(spacer);
                }
            }
        }

        if (!mb->lineBreak() && !mb->pageBreak()) {
            continue;
        }

        for (EngravingItem* item : mb->el()) {
            if (item->isLayoutBreak()) {
                itemsToRemove.push_back(item);
            }
        }
    }

    for (EngravingItem* item : itemsToRemove) {
        undoRemoveElement(item);
    }
}

//---------------------------------------------------------
//   cmdResetNoteAndRestGroupings
//---------------------------------------------------------

void Score::cmdResetNoteAndRestGroupings()
{
    bool noSelection = selection().isNone();
    if (noSelection) {
        cmdSelectAll();
    } else if (!selection().isRange()) {
        LOGD("no system or staff selected");
        return;
    }

    // save selection values because selection changes during grouping
    Fraction sTick = selection().tickStart();
    Fraction eTick = selection().tickEnd();
    staff_idx_t sStaff = selection().staffStart();
    staff_idx_t eStaff = selection().staffEnd();

    for (staff_idx_t staff = sStaff; staff < eStaff; staff++) {
        track_idx_t sTrack = staff * VOICES;
        track_idx_t eTrack = sTrack + VOICES;
        for (track_idx_t track = sTrack; track < eTrack; track++) {
            if (selectionFilter().canSelectVoice(track)) {
                regroupNotesAndRests(sTick, eTick, track);
            }
        }
    }

    if (noSelection) {
        deselectAll();
        return;
    }

    // Reset selection to original selection
    selection().setRangeTicks(sTick, eTick, sStaff, eStaff);
    selection().updateSelectedElements();
}

static void resetBeamOffSet(void*, EngravingItem* e)
{
    // Reset completely cross staff beams from MU1&2
    if (e->isBeam() && toBeam(e)->fullCross()) {
        e->reset();
    }
}

//---------------------------------------------------------
//   cmdResetAllPositions
//---------------------------------------------------------
void Score::cmdResetAllPositions(bool undoable)
{
    TRACEFUNC;

    if (undoable) {
        startCmd(TranslatableString("undoableAction", "Reset all positions"));
    }
    resetAutoplace();
    if (undoable) {
        endCmd();
    }
}

void Score::resetAutoplace()
{
    TRACEFUNC;

    scanElements(nullptr, resetElementPosition);
}

void Score::resetCrossBeams()
{
    TRACEFUNC;

    scanElements(nullptr, resetBeamOffSet);
}

//---------------------------------------------------------
//   move
//    move current selection
//---------------------------------------------------------

EngravingItem* Score::move(const String& cmd)
{
    ChordRest* cr { nullptr };
    Box* box { nullptr };
    if (noteEntryMode()) {
        // if selection exists and is grace note, use it
        // otherwise use chord/rest at input position
        // also use it if we are moving to next chord
        // to catch up with the cursor and not move the selection by 2 positions
        cr = selection().cr();
        if (cr && (cr->isGrace() || cmd == u"next-chord" || cmd == u"prev-chord")) {
        } else {
            cr = inputState().cr();
        }
    } else if (selection().activeCR()) {
        cr = selection().activeCR();
    } else {
        cr = selection().lastChordRest();
    }

    // no chord/rest found? look for another type of element,
    // but commands [empty-trailing-measure] and [top-staff] don't
    // necessarily need an active selection for appropriate functioning
    if (!cr && cmd != u"empty-trailing-measure" && cmd != u"top-staff") {
        if (selection().elements().empty()) {
            return 0;
        }
        // retrieve last element of section list
        EngravingItem* el = selection().elements().back();
        EngravingItem* trg = nullptr;

        // get parent of element and process accordingly:
        // trg is the element to select on "next-chord" cmd
        // cr is the ChordRest to move from on other cmd's
        track_idx_t track = el->track();                // keep note of element track
        if (!el->isBox()) {
            el = el->parentItem();
        }
        // element with no parent (eg, a newly-added line) - no way to find context
        if (!el) {
            return 0;
        }
        switch (el->type()) {
        case ElementType::NOTE:                     // a note is a valid target
            trg = el;
            cr  = toNote(el)->chord();
            break;
        case ElementType::CHORD:                    // a chord or a rest are valid targets
        case ElementType::REST:
        case ElementType::MMREST:
            trg = el;
            cr  = toChordRest(trg);
            break;
        case ElementType::SEGMENT: {                // from segment go to top chordrest in segment
            Segment* seg  = toSegment(el);
            // if segment is not chord/rest or grace, move to next chord/rest or grace segment
            if (!seg->isChordRest()) {
                seg = seg->next1(SegmentType::ChordRest);
                if (!seg) {                 // if none found, return failure
                    return 0;
                }
            }
            // segment for sure contains chords/rests,
            size_t size = seg->elist().size();
            // if segment has a chord/rest in original element track, use it
            if (track < size && seg->element(track)) {
                trg  = seg->element(track);
                cr = toChordRest(trg);
                break;
            }
            // if not, get topmost chord/rest
            for (size_t i = 0; i < size; i++) {
                if (seg->element(i)) {
                    trg  = seg->element(i);
                    cr = toChordRest(trg);
                    break;
                }
            }
            break;
        }
        case ElementType::HBOX:           // fallthrough
        case ElementType::VBOX:           // fallthrough
        case ElementType::TBOX:
            box = toBox(el);
            break;
        default:                                // on anything else, return failure
            return 0;
        }

        // if something found and command is forward, the element found is the destination
        if (trg && cmd == u"next-chord") {
            // if chord, go to topmost note
            if (trg->type() == ElementType::CHORD) {
                trg = toChord(trg)->upNote();
            }
            setPlayNote(true);
            select(trg, SelectType::SINGLE, 0);
            return trg;
        }
        // if no chordrest and no box (frame) found, do nothing
        if (!cr && !box) {
            return 0;
        }
        // if some chordrest found, continue with default processing
    }

    EngravingItem* el = nullptr;
    Segment* ois = noteEntryMode() ? m_is.segment() : nullptr;
    Measure* oim = ois ? ois->measure() : nullptr;

    if (cmd == u"next-chord" && cr) {
        // note input cursor
        if (noteEntryMode()) {
            m_is.moveToNextInputPos();
        }

        // selection "cursor"
        // find next chordrest, which might be a grace note
        // this may override note input cursor
        el = nextChordRest(cr);

        // Skip gap rests if we're not in note entry mode...
        while (!noteEntryMode() && el && el->isRest() && toRest(el)->isGap()) {
            el = nextChordRest(toChordRest(el));
        }
        if (el && noteEntryMode()) {
            // do not use if not in original or new measure (don't skip measures)
            Measure* m = toChordRest(el)->measure();
            Segment* nis = m_is.segment();
            Measure* nim = nis ? nis->measure() : nullptr;
            if (m != oim && m != nim) {
                el = cr;
            }
            // do not use if new input segment is current cr
            // this means input cursor just caught up to current selection
            else if (cr && nis == cr->segment()) {
                el = cr;
            }
        } else if (!el) {
            el = cr;
        }
    } else if (cmd == u"prev-chord" && cr) {
        // note input cursor
        if (noteEntryMode() && m_is.segment()) {
            Measure* m = m_is.segment()->measure();
            Segment* s = m_is.segment()->prev1(SegmentType::ChordRest);
            track_idx_t track = m_is.track();
            for (; s; s = s->prev1(SegmentType::ChordRest)) {
                if (s->element(track) || (s->measure() != m && s->rtick().isZero())) {
                    if (s->element(track)) {
                        if (s->element(track)->isRest() && toRest(s->element(track))->isGap()) {
                            continue;
                        }
                    }
                    break;
                }
            }
            m_is.moveInputPos(s);
        }

        // selection "cursor"
        // find previous chordrest, which might be a grace note
        // this may override note input cursor
        el = prevChordRest(cr);

        // Skip gap rests if we're not in note entry mode...
        while (!noteEntryMode() && el && el->isRest() && toRest(el)->isGap()) {
            el = prevChordRest(toChordRest(el));
        }
        if (el && noteEntryMode()) {
            // do not use if not in original or new measure (don't skip measures)
            Measure* m = toChordRest(el)->measure();
            Segment* nis = m_is.segment();
            Measure* nim = nis ? nis->measure() : nullptr;
            if (m != oim && m != nim) {
                el = cr;
            }
            // do not use if new input segment is current cr
            // this means input cursor just caught up to current selection
            else if (cr && nis == cr->segment()) {
                el = cr;
            }
        } else if (!el) {
            el = cr;
        }
    } else if (cmd == u"next-measure") {
        if (box && box->nextMeasure() && box->nextMeasure()->first()) {
            el = box->nextMeasure()->first()->nextChordRest(0, false);
        }
        if (cr) {
            el = nextMeasure(cr);
        }
        if (el && noteEntryMode()) {
            m_is.moveInputPos(el);
        }
    } else if (cmd == u"prev-measure") {
        if (box && box->prevMeasure() && box->prevMeasure()->first()) {
            el = box->prevMeasure()->first()->nextChordRest(0, false);
        }
        if (cr) {
            el = prevMeasure(cr);
        }
        if (el && noteEntryMode()) {
            m_is.moveInputPos(el);
        }
    } else if (cmd == u"next-system" && cr) {
        el = cmdNextPrevSystem(cr, true);
        if (noteEntryMode()) {
            m_is.moveInputPos(el);
        }
    } else if (cmd == u"prev-system" && cr) {
        el = cmdNextPrevSystem(cr, false);
        if (noteEntryMode()) {
            m_is.moveInputPos(el);
        }
    } else if (cmd == u"next-frame") {
        auto measureBase = cr ? cr->measure()->findMeasureBase() : box->findMeasureBase();
        el = measureBase ? cmdNextPrevFrame(measureBase, true) : nullptr;
    } else if (cmd == u"prev-frame") {
        auto measureBase = cr ? cr->measure()->findMeasureBase() : box->findMeasureBase();
        el = measureBase ? cmdNextPrevFrame(measureBase, false) : nullptr;
    } else if (cmd == u"next-section") {
        if (!(el = box)) {
            el = cr;
        }
        el = cmdNextPrevSection(el, true);
    } else if (cmd == u"prev-section") {
        if (!(el = box)) {
            el = cr;
        }
        el = cmdNextPrevSection(el, false);
    } else if (cmd == u"next-track" && cr) {
        el = nextTrack(cr);
        if (noteEntryMode()) {
            m_is.moveInputPos(el);
        }
    } else if (cmd == u"prev-track" && cr) {
        el = prevTrack(cr);
        if (noteEntryMode()) {
            m_is.moveInputPos(el);
        }
    } else if (cmd == u"top-staff") {
        el = cr ? cmdTopStaff(cr) : cmdTopStaff();
        if (noteEntryMode()) {
            m_is.moveInputPos(el);
        }
    } else if (cmd == u"empty-trailing-measure") {
        const Measure* ftm = nullptr;
        if (!cr) {
            ftm = firstTrailingMeasure() ? firstTrailingMeasure() : lastMeasure();
        } else {
            ftm = firstTrailingMeasure(&cr) ? firstTrailingMeasure(&cr) : lastMeasure();
        }
        if (ftm) {
            if (style().styleB(Sid::createMultiMeasureRests) && ftm->hasMMRest()) {
                ftm = ftm->coveringMMRestOrThis();
            }
            el = !cr ? ftm->first()->nextChordRest(0, false) : ftm->first()->nextChordRest(trackZeroVoice(cr->track()), false);
        }
        // Note: Due to the nature of this command as being preparatory for input,
        // Note-Entry is activated from within ScoreView::cmd()
        m_is.moveInputPos(el);
    }

    if (el) {
        if (el->type() == ElementType::CHORD) {
            el = toChord(el)->upNote();             // originally downNote
        }
        setPlayNote(true);
        if (noteEntryMode()) {
            // if cursor moved into a gap, selection cannot follow
            // only select & play el if it was not already selected (does not normally happen)
            if (m_is.cr() || !el->selected()) {
                select(el, SelectType::SINGLE, 0);
            } else {
                setPlayNote(false);
            }
            for (MuseScoreView* view : m_viewer) {
                view->moveCursor();
            }
        } else {
            select(el, SelectType::SINGLE, 0);
        }
    }
    return el;
}

//---------------------------------------------------------
//   selectMove
//---------------------------------------------------------

EngravingItem* Score::selectMove(const String& cmd)
{
    ChordRest* cr;
    if (selection().activeCR()) {
        cr = selection().activeCR();
    } else {
        cr = selection().lastChordRest();
    }
    if (!cr && noteEntryMode()) {
        cr = inputState().cr();
    }
    if (!cr) {
        return 0;
    }

    ChordRest* el = nullptr;
    ChordRestNavigateOptions options;
    options.skipGrace = true;
    options.skipMeasureRepeatRests = false;
    if (cmd == u"select-next-chord") {
        el = nextChordRest(cr, options);
    } else if (cmd == u"select-prev-chord") {
        el = prevChordRest(cr, options);
    } else if (cmd == u"select-next-measure") {
        el = nextMeasure(cr, true, true);
    } else if (cmd == u"select-prev-measure") {
        el = prevMeasure(cr, true);
    } else if (cmd == u"select-begin-line") {
        Measure* measure = cr->segment()->measure()->system()->firstMeasure();
        if (!measure) {
            return 0;
        }
        el = measure->first()->nextChordRest(cr->track());
    } else if (cmd == u"select-end-line") {
        Measure* measure = cr->segment()->measure()->system()->lastMeasure();
        if (!measure) {
            return 0;
        }
        el = measure->last()->nextChordRest(cr->track(), true);
    } else if (cmd == u"select-begin-score") {
        Measure* measure = firstMeasureMM();
        if (!measure) {
            return 0;
        }
        el = measure->first()->nextChordRest(cr->track());
    } else if (cmd == u"select-end-score") {
        Measure* measure = lastMeasureMM();
        if (!measure) {
            return 0;
        }
        el = measure->last()->nextChordRest(cr->track(), true);
    } else if (cmd == u"select-staff-above") {
        el = upStaff(cr);
    } else if (cmd == u"select-staff-below") {
        el = downStaff(cr);
    }
    if (el) {
        select(el, SelectType::RANGE, el->staffIdx());
    }
    return el;
}

//---------------------------------------------------------
//   cmdMirrorNoteHead
//---------------------------------------------------------

void Score::cmdMirrorNoteHead()
{
    const std::vector<EngravingItem*>& el = selection().elements();
    for (EngravingItem* e : el) {
        if (e->isNote()) {
            Note* note = toNote(e);
            if (note->staff() && note->staff()->isTabStaff(note->chord()->tick())) {
                e->undoChangeProperty(Pid::DEAD, !note->deadNote());
            } else {
                DirectionH d = note->userMirror();
                if (d == DirectionH::AUTO) {
                    d = note->chord()->up() ? DirectionH::RIGHT : DirectionH::LEFT;
                } else {
                    d = d == DirectionH::LEFT ? DirectionH::RIGHT : DirectionH::LEFT;
                }
                undoChangeUserMirror(note, d);
            }
        } else if (e->isHairpinSegment()) {
            Hairpin* h = toHairpinSegment(e)->hairpin();
            HairpinType st = h->hairpinType();
            switch (st) {
            case HairpinType::CRESC_HAIRPIN:
                st = HairpinType::DECRESC_HAIRPIN;
                break;
            case HairpinType::DECRESC_HAIRPIN:
                st = HairpinType::CRESC_HAIRPIN;
                break;
            case HairpinType::CRESC_LINE:
                st = HairpinType::DECRESC_LINE;
                break;
            case HairpinType::DECRESC_LINE:
                st = HairpinType::CRESC_LINE;
                break;
            case HairpinType::INVALID:
                break;
            }
            h->undoChangeProperty(Pid::HAIRPIN_TYPE, int(st));
        }
    }
}

//---------------------------------------------------------
//   cmdIncDecDuration
//     When stepDotted is false and nSteps is 1 or -1, will halve or double the duration
//     When stepDotted is true, will step by nearest dotted or undotted note
//---------------------------------------------------------

void Score::cmdIncDecDuration(int nSteps, bool stepDotted)
{
    EngravingItem* el = selection().element();
    if (el == 0) {
        return;
    }
    if (el->isNote()) {
        el = el->parentItem();
    }
    if (!el->isChordRest()) {
        return;
    }

    ChordRest* cr = toChordRest(el);

    // if measure rest is selected as input, then the correct initialDuration will be the
    // duration of the measure's time signature, else is just the input state's duration
    TDuration initialDuration;
    if (cr->durationType() == DurationType::V_MEASURE) {
        initialDuration = TDuration(cr->measure()->timesig(), true);

        if (initialDuration.fraction() < cr->measure()->timesig() && nSteps > 0) {
            // Duration already shortened by truncation; shorten one step less
            --nSteps;
        }
    } else {
        initialDuration = m_is.duration();
    }
    TDuration d = (nSteps != 0) ? initialDuration.shiftRetainDots(nSteps, stepDotted) : initialDuration;
    if (!d.isValid()) {
        return;
    }
    if (cr->isChord() && (toChord(cr)->noteType() != NoteType::NORMAL)) {
        //
        // handle appoggiatura and acciaccatura
        //
        undoChangeChordRestLen(cr, d);
    } else {
        changeCRlen(cr, d);
    }
    m_is.setDuration(d);
    nextInputPos(cr, false);
}

//---------------------------------------------------------
//   cmdAddBracket
//---------------------------------------------------------

void Score::cmdAddBracket()
{
    for (EngravingItem* el : selection().elements()) {
        if (el->type() == ElementType::ACCIDENTAL) {
            Accidental* acc = toAccidental(el);
            acc->undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::BRACKET));
        }
    }
}

//---------------------------------------------------------
//   cmdAddParentheses
//---------------------------------------------------------

void Score::cmdAddParentheses()
{
    for (EngravingItem* el : selection().elements()) {
        cmdAddParentheses(el);
    }
}

void Score::cmdAddParentheses(EngravingItem* el)
{
    if (el->type() == ElementType::NOTE) {
        Note* n = toNote(el);
        n->undoChangeProperty(Pid::HEAD_HAS_PARENTHESES, !n->headHasParentheses());
    } else if (el->type() == ElementType::ACCIDENTAL) {
        Accidental* acc = toAccidental(el);
        acc->undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::PARENTHESIS));
    } else if (el->type() == ElementType::HARMONY) {
        Harmony* h = toHarmony(el);
        h->setLeftParen(true);
        h->setRightParen(true);
        h->render();
    } else if (el->type() == ElementType::TIMESIG) {
        TimeSig* ts = toTimeSig(el);
        ts->setLargeParentheses(true);
    }
}

//---------------------------------------------------------
//   cmdAddBraces
//---------------------------------------------------------

void Score::cmdAddBraces()
{
    for (EngravingItem* el : selection().elements()) {
        if (el->type() == ElementType::ACCIDENTAL) {
            Accidental* acc = toAccidental(el);
            acc->undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::BRACE));
        }
    }
}

//---------------------------------------------------------
//   cmdMoveRest
//---------------------------------------------------------

void Score::cmdMoveRest(Rest* rest, DirectionV dir)
{
    PointF pos(rest->offset());
    if (dir == DirectionV::UP) {
        pos.ry() -= style().spatium();
    } else if (dir == DirectionV::DOWN) {
        pos.ry() += style().spatium();
    }
    rest->undoChangeProperty(Pid::OFFSET, pos);
}

//---------------------------------------------------------
//   cmdMoveLyrics
//---------------------------------------------------------

void Score::cmdMoveLyrics(Lyrics* lyrics, DirectionV dir)
{
    int verse = lyrics->no() + (dir == DirectionV::UP ? -1 : 1);
    if (verse < 0) {
        return;
    }
    lyrics->undoChangeProperty(Pid::VERSE, verse);
}

//---------------------------------------------------------
//   realtimeAdvance
//---------------------------------------------------------

void Score::realtimeAdvance(bool allowTransposition)
{
    InputState& is = inputState();
    if (!is.noteEntryMode()) {
        return;
    }

    Fraction ticks2measureEnd = is.segment()->measure()->ticks() - is.segment()->rtick();
    if (!is.cr() || (is.cr()->ticks() != is.duration().fraction() && is.duration() < ticks2measureEnd)) {
        setNoteRest(is.segment(), is.track(), NoteVal(), is.duration().fraction(), DirectionV::AUTO);
    }

    ChordRest* prevCR = toChordRest(is.cr());
    if (inputState().endOfScore()) {
        appendMeasures(1);
    }

    is.moveToNextInputPos();

    std::list<MidiInputEvent>& midiPitches = activeMidiPitches();
    if (midiPitches.empty()) {
        setNoteRest(is.segment(), is.track(), NoteVal(), is.duration().fraction(), DirectionV::AUTO);
    } else {
        Chord* prevChord = prevCR->isChord() ? toChord(prevCR) : 0;
        bool partOfChord = false;
        for (const MidiInputEvent& ev : midiPitches) {
            addTiedMidiPitch(ev.pitch, partOfChord, prevChord, allowTransposition);
            partOfChord = true;
        }
    }

    if (prevCR->measure() != is.segment()->measure()) {
        // just advanced across barline. Now simplify tied notes.
        score()->regroupNotesAndRests(prevCR->measure()->tick(), is.segment()->measure()->tick(), is.track());
    }

    return;
}

bool Score::canInsertClef(ClefType type) const
{
    if (type == ClefType::INVALID) {
        return false;
    }

    const Staff* staff = this->staff(m_is.track() / VOICES);
    const ChordRest* cr = m_is.cr();

    return staff && cr;
}

void Score::cmdInsertClef(ClefType type)
{
    undoChangeClef(staff(m_is.track() / VOICES), m_is.cr(), type);
}

//---------------------------------------------------------
//   cmdInsertClef
//    insert clef before cr
//---------------------------------------------------------

void Score::cmdInsertClef(Clef* clef, ChordRest* cr)
{
    undoChangeClef(cr->staff(), cr, clef->clefType());
    delete clef;
}

//---------------------------------------------------------
//   cmdAddGrace
///   adds grace note of specified type to selected notes
//---------------------------------------------------------

void Score::cmdAddGrace(NoteType graceType, int duration)
{
    const std::vector<EngravingItem*> copyOfElements = selection().elements();
    for (EngravingItem* e : copyOfElements) {
        if (e->type() == ElementType::NOTE) {
            Note* n = toNote(e);
            Note* graceNote = setGraceNote(n->chord(), n->pitch(), graceType, duration);
            select(graceNote, SelectType::SINGLE, 0);
        }
    }
}

//---------------------------------------------------------
//   cmdAddMeasureRepeat
//---------------------------------------------------------

void Score::cmdAddMeasureRepeat(Measure* firstMeasure, int numMeasures, staff_idx_t staffIdx)
{
    //
    // make measures into group
    //
    if (!makeMeasureRepeatGroup(firstMeasure, numMeasures, staffIdx)) {
        return;
    }

    //
    // add MeasureRepeat element
    //
    int measureWithElementNo;
    if (numMeasures % 2) {
        // odd number, element anchored to center measure of group
        measureWithElementNo = numMeasures / 2 + 1;
    } else {
        // even number, element anchored to last measure in first half of group
        measureWithElementNo = numMeasures / 2;
    }
    Measure* measureWithElement = firstMeasure;
    for (int i = 1; i < measureWithElementNo; ++i) {
        measureWithElement = measureWithElement->nextMeasure();
    }
    // MeasureRepeat element will be positioned appropriately (in center of measure / on following barline)
    // when stretchMeasure() is called on measureWithElement
    MeasureRepeat* mr = addMeasureRepeat(measureWithElement->tick(), staff2track(staffIdx), numMeasures);
    select(mr, SelectType::SINGLE, 0);
}

//---------------------------------------------------------
//   makeMeasureRepeatGroup
///   clear measures, apply noBreak, set measureRepeatCount
///   returns false if these measures won't work or user aborted
//---------------------------------------------------------

bool Score::makeMeasureRepeatGroup(Measure* firstMeasure, int numMeasures, staff_idx_t staffIdx)
{
    //
    // check that sufficient measures exist, with equal durations
    //
    std::vector<Measure*> measures;
    Measure* measure = firstMeasure;
    for (int i = 1; i <= numMeasures; ++i) {
        if (!measure || measure->ticks() != firstMeasure->ticks()) {
            MScore::setError(MsError::INSUFFICIENT_MEASURES);
            return false;
        }
        measures.push_back(measure);
        measure = measure->nextMeasure();
    }

    //
    // warn user if things will have to be deleted to make room for measure repeat
    //
    bool empty = true;
    for (auto m : measures) {
        if (m != measures.back()) {
            if (m->endBarLineType() != BarLineType::NORMAL) {
                empty = false;
                break;
            }
        }
        for (auto seg = m->first(); seg && empty; seg = seg->next()) {
            if (seg->segmentType() & SegmentType::ChordRest) {
                track_idx_t strack = staffIdx * VOICES;
                track_idx_t etrack = strack + VOICES;
                for (track_idx_t track = strack; track < etrack; ++track) {
                    EngravingItem* e = seg->element(track);
                    if (e && !e->generated() && !e->isRest()) {
                        empty = false;
                        break;
                    }
                }
            }
        }
    }

    if (!empty) {
        auto b = MessageBox(iocContext()).warning(muse::trc("engraving", "Current contents of measures will be replaced"),
                                                  muse::trc("engraving", "Continue with inserting measure repeat?"));
        if (b == MessageBox::Button::Cancel) {
            return false;
        }
    }

    //
    // group measures and clear current contents
    //

    deselectAll();
    int i = 1;
    for (auto m : measures) {
        select(m, SelectType::RANGE, staffIdx);
        if (m->isMeasureRepeatGroup(staffIdx)) {
            deleteItem(m->measureRepeatElement(staffIdx)); // reset measures related to an earlier MeasureRepeat
        }
        undoChangeMeasureRepeatCount(m, i++, staffIdx);
        if (m != measures.front()) {
            m->undoChangeProperty(Pid::REPEAT_START, false);
        }
        if (m != measures.back()) {
            m->undoSetNoBreak(true);
            Segment* seg = m->findSegmentR(SegmentType::EndBarLine, m->ticks());
            BarLine* endBarLine = toBarLine(seg->element(staff2track(staffIdx)));
            deleteItem(endBarLine); // also takes care of Pid::REPEAT_END
        }
    }
    cmdDeleteSelection();
    return true;
}

//---------------------------------------------------------
//   cmdExplode
///   explodes contents of top selected staff into subsequent staves
//---------------------------------------------------------

void Score::cmdExplode()
{
    size_t srcStaff  = selection().staffStart();
    size_t lastStaff = selection().staffEnd();
    size_t srcTrack  = srcStaff * VOICES;

    // reset selection to top staff only
    // force complete measures
    Segment* startSegment = selection().startSegment();
    Segment* endSegment   = selection().endSegment();
    Measure* startMeasure = startSegment->measure();
    Measure* endMeasure   = nullptr;
    if (!endSegment) {
        endMeasure = lastMeasure();
    } else if (endSegment->tick() == endSegment->measure()->tick()) {
        endMeasure = endSegment->measure()->prevMeasure() ? endSegment->measure()->prevMeasure() : firstMeasure();
    } else {
        endMeasure = endSegment->measure();
    }

    Fraction lTick = endMeasure->endTick();
    bool voice = false;

    for (Measure* m = startMeasure; m && m->tick() != lTick; m = m->nextMeasure()) {
        if (m->hasVoices(srcStaff)) {
            voice = true;
            break;
        }
    }
    if (!voice) {
        // force complete measures
        deselectAll();
        select(startMeasure, SelectType::RANGE, srcStaff);
        select(endMeasure, SelectType::RANGE, srcStaff);
        startSegment = selection().startSegment();
        endSegment = selection().endSegment();
        if (srcStaff == lastStaff - 1) {
            // only one staff was selected up front - determine number of staves
            // loop through all chords looking for maximum number of notes
            int n = 0;
            for (Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
                EngravingItem* e = s->element(srcTrack);
                if (e && e->type() == ElementType::CHORD) {
                    Chord* c = toChord(e);
                    n = std::max(n, int(c->notes().size()));
                    for (Chord* graceChord : c->graceNotes()) {
                        n = std::max(n, int(graceChord->notes().size()));
                    }
                }
            }
            lastStaff = std::min(nstaves(), srcStaff + n);
        }

        const muse::ByteArray mimeData(selection().mimeData());
        // copy to all destination staves
        Segment* firstCRSegment = startMeasure->tick2segment(startMeasure->tick());
        for (size_t i = 1; srcStaff + i < lastStaff; ++i) {
            track_idx_t track = (srcStaff + i) * VOICES;
            ChordRest* cr = toChordRest(firstCRSegment->element(track));
            if (cr) {
                XmlReader e(mimeData);
                pasteStaff(e, cr->segment(), cr->staffIdx());
            }
        }

        auto doExplode = [this](Chord* c, size_t lastStaff, size_t srcStaff, size_t i) -> void
        {
            std::vector<Note*> notes = c->notes();
            size_t nnotes = notes.size();
            // keep note "i" from top, which is backwards from nnotes - 1
            // reuse notes if there are more instruments than notes
            size_t stavesPerNote = std::max((lastStaff - srcStaff) / nnotes, static_cast<size_t>(1));
            size_t keepIndex = static_cast<size_t>(std::max(static_cast<int>(nnotes) - 1 - static_cast<int>(i / stavesPerNote), 0));
            Note* keepNote = c->notes()[keepIndex];
            for (Note* n : notes) {
                if (n != keepNote) {
                    undoRemoveElement(n);
                }
            }
        };

        // loop through each staff removing all but one note from each chord
        for (size_t i = 0; srcStaff + i < lastStaff; ++i) {
            track_idx_t track = (srcStaff + i) * VOICES;
            for (Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
                EngravingItem* e = s->element(track);
                if (e && e->type() == ElementType::CHORD) {
                    Chord* c = toChord(e); //chord, laststaff, srcstaff
                    doExplode(c, lastStaff, srcStaff, i);
                    for (Chord* graceChord : c->graceNotes()) {
                        doExplode(graceChord, lastStaff, srcStaff, i);
                    }
                }
            }
        }
    } else {
        track_idx_t sTracks[VOICES];
        track_idx_t dTracks[VOICES];
        if (srcStaff == lastStaff - 1) {
            lastStaff = std::min(nstaves(), srcStaff + VOICES);
        }

        for (voice_idx_t i = 0; i < VOICES; i++) {
            sTracks[i] = muse::nidx;
            dTracks[i] = muse::nidx;
        }
        int full = 0;

        for (Segment* seg = startSegment; seg && seg->tick() < lTick; seg = seg->next1()) {
            for (track_idx_t i = srcTrack; i < srcTrack + VOICES && full != VOICES; i++) {
                bool t = true;
                for (voice_idx_t j = 0; j < VOICES; j++) {
                    if (i == sTracks[j]) {
                        t = false;
                        break;
                    }
                }

                if (!seg->measure()->hasVoice(i) || seg->measure()->isOnlyRests(i) || !t) {
                    continue;
                }
                sTracks[full] = i;

                for (size_t j = srcTrack + full * VOICES; j < lastStaff * VOICES; j++) {
                    if (i == j) {
                        dTracks[full] = j;
                        break;
                    }
                    for (Measure* m = seg->measure(); m && m->tick() < lTick; m = m->nextMeasure()) {
                        if (!m->hasVoice(j) || (m->hasVoice(j) && m->isOnlyRests(j))) {
                            dTracks[full] = j;
                        } else {
                            dTracks[full] = muse::nidx;
                            break;
                        }
                    }
                    if (dTracks[full] != muse::nidx) {
                        break;
                    }
                }
                full++;
            }
        }

        for (track_idx_t i = srcTrack, j = 0; i < lastStaff * VOICES && j < VOICES; i += VOICES, j++) {
            track_idx_t strack = sTracks[j % VOICES];
            track_idx_t dtrack = dTracks[j % VOICES];
            if (strack != muse::nidx && strack != dtrack && dtrack != muse::nidx) {
                undo(new CloneVoice(startSegment, lTick, startSegment, strack, dtrack, muse::nidx, false));
            }
        }
    }

    // select exploded region
    deselectAll();
    select(startMeasure, SelectType::RANGE, srcStaff);
    select(endMeasure, SelectType::RANGE, lastStaff - 1);
}

//---------------------------------------------------------
//   cmdImplode
///   implodes contents of selected staves into top staff
///   for single staff, merge voices
//---------------------------------------------------------

void Score::cmdImplode()
{
    staff_idx_t dstStaff   = selection().staffStart();
    staff_idx_t endStaff   = selection().staffEnd();
    track_idx_t dstTrack   = dstStaff * VOICES;
    track_idx_t startTrack = dstStaff * VOICES;
    track_idx_t endTrack   = endStaff * VOICES;
    Segment* startSegment = selection().startSegment();
    Segment* endSegment = selection().endSegment();
    Measure* startMeasure = startSegment->measure();
    Measure* endMeasure = endSegment ? endSegment->measure() : lastMeasure();
    Fraction startTick       = startSegment->tick();
    Fraction endTick         = endSegment ? endSegment->tick() : lastMeasure()->endTick();
    assert(startMeasure && endMeasure);

    // if single staff selected, combine voices
    // otherwise combine staves
    if (dstStaff == endStaff - 1) {
        // loop through segments adding notes to chord on top staff
        for (Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
            if (!s->isChordRestType()) {
                continue;
            }
            EngravingItem* dst = s->element(dstTrack);
            if (dst && dst->isChord()) {
                Chord* dstChord = toChord(dst);
                // see if we are tying in to this chord
                Chord* tied = 0;
                for (Note* n : dstChord->notes()) {
                    if (n->tieBackNonPartial()) {
                        tied = n->tieBack()->startNote()->chord();
                        break;
                    }
                }
                // loop through each subsequent staff (or track within staff)
                // looking for notes to add
                for (track_idx_t srcTrack = startTrack + 1; srcTrack < endTrack; srcTrack++) {
                    EngravingItem* src = s->element(srcTrack);
                    if (src && src->isChord()) {
                        Chord* srcChord = toChord(src);
                        // when combining voices, skip if not same duration
                        if (srcChord->ticks() != dstChord->ticks()) {
                            continue;
                        }
                        // add notes
                        for (Note* n : srcChord->notes()) {
                            NoteVal nv(n->pitch());
                            nv.tpc1 = n->tpc1();
                            // skip duplicates
                            if (dstChord->findNote(nv.pitch)) {
                                continue;
                            }
                            bool forceAccidental = n->accidental() && n->accidental()->role() == AccidentalRole::USER;
                            Note* nn = addNote(dstChord, nv, forceAccidental);
                            // add tie to this note if original chord was tied
                            if (tied) {
                                // find note to tie to
                                for (Note* tn : tied->notes()) {
                                    if (nn->pitch() == tn->pitch() && nn->tpc() == tn->tpc() && !tn->tieFor()) {
                                        // found note to tie
                                        Tie* tie = Factory::createTie(this->dummy());
                                        tie->setStartNote(tn);
                                        tie->setEndNote(nn);
                                        tie->setTick(tie->startNote()->tick());
                                        tie->setTick2(tie->endNote()->tick());
                                        tie->setTrack(tn->track());
                                        undoAddElement(tie);
                                    }
                                }
                            }
                        }
                    }
                    // delete chordrest from source track if possible
                    if (src && src->voice()) {
                        undoRemoveElement(src);
                    }
                }
            }
            // TODO - use first voice that actually has a note and implode remaining voices on it?
            // see https://musescore.org/en/node/174111
            else if (dst) {
                // destination track has something, but it isn't a chord
                // remove rests from other voices if in "voice mode"
                for (voice_idx_t i = 1; i < VOICES; ++i) {
                    EngravingItem* e = s->element(dstTrack + i);
                    if (e && e->isRest()) {
                        undoRemoveElement(e);
                    }
                }
            }
        }
        // delete orphaned spanners (TODO: figure out solution to reconnect orphaned spanners to their cloned notes)
        checkSpanner(startTick, endTick);
    } else {
        track_idx_t tracks[VOICES];
        for (voice_idx_t i = 0; i < VOICES; i++) {
            tracks[i] = muse::nidx;
        }
        voice_idx_t full = 0;

        // identify tracks to combine, storing the source track numbers in tracks[]
        // first four non-empty tracks to win
        for (track_idx_t track = startTrack; track < endTrack && full < VOICES; ++track) {
            Measure* m = startMeasure;
            do {
                if (m->hasVoice(track) && !m->isOnlyRests(track)) {
                    tracks[full++] = track;
                    break;
                }
            } while ((m != endMeasure) && (m = m->nextMeasure()));
        }

        // clone source tracks into destination
        for (track_idx_t i = dstTrack; i < dstTrack + VOICES; i++) {
            track_idx_t strack = tracks[i % VOICES];
            if (strack != muse::nidx && strack != i) {
                undo(new CloneVoice(startSegment, endTick, startSegment, strack, i, i, false));
            }
        }
    }

    // select destination staff only
    deselectAll();
    select(startMeasure, SelectType::RANGE, dstStaff);
    select(endMeasure, SelectType::RANGE, dstStaff);
}

//---------------------------------------------------------
//   cmdSlashFill
///   fills selected region with slashes
//---------------------------------------------------------

void Score::cmdSlashFill()
{
    staff_idx_t startStaff = selection().staffStart();
    staff_idx_t endStaff = selection().staffEnd();
    Segment* startSegment = selection().startSegment();
    if (!startSegment) { // empty score?
        return;
    }

    Segment* endSegment = selection().endSegment();

    // operate on measures underlying mmrests
    if (startSegment && startSegment->measure() && startSegment->measure()->isMMRest()) {
        startSegment = startSegment->measure()->mmRestFirst()->first();
    }
    if (endSegment && endSegment->measure() && endSegment->measure()->isMMRest()) {
        endSegment = endSegment->measure()->mmRestLast()->last();
    }

    Fraction endTick = endSegment ? endSegment->tick() : lastSegment()->tick() + Fraction::fromTicks(1);
    Chord* firstSlash = 0;
    Chord* lastSlash = 0;

    // loop through staves in selection
    for (staff_idx_t staffIdx = startStaff; staffIdx < endStaff; ++staffIdx) {
        track_idx_t track = staffIdx * VOICES;
        voice_idx_t voice = muse::nidx;
        // loop through segments adding slashes on each beat
        for (Segment* s = startSegment; s && s->tick() < endTick; s = s->next1()) {
            if (s->segmentType() != SegmentType::ChordRest) {
                continue;
            }
            // determine beat type based on time signature
            int d = s->measure()->timesig().denominator();
            int n = (d > 4 && s->measure()->timesig().numerator() % 3 == 0) ? 3 : 1;
            Fraction f(n, d);
            // skip over any leading segments before next (first) beat
            if (s->rtick().ticks() % f.ticks()) {
                continue;
            }
            // determine voice to use - first available voice for this measure / staff
            if (voice == muse::nidx || s->rtick().isZero()) {
                bool needGap[VOICES];
                for (voice = 0; voice < VOICES; ++voice) {
                    needGap[voice] = false;
                    ChordRest* cr = toChordRest(s->element(track + voice));
                    // no chordrest == treat as ordinary rest for purpose of determining availability of voice
                    // but also, we will need to make a gap for this voice if we do end up choosing it
                    if (!cr) {
                        needGap[voice] = true;
                    }
                    // chord == keep looking for an available voice
                    else if (cr->type() == ElementType::CHORD) {
                        continue;
                    }
                    // full measure rest == OK to use voice
                    else if (cr->durationType() == DurationType::V_MEASURE) {
                        break;
                    }
                    // no chordrest or ordinary rest == OK to use voice
                    // if there are nothing but rests for duration of measure / selection
                    bool ok = true;
                    for (Segment* ns = s->next(SegmentType::ChordRest); ns && ns != endSegment; ns = ns->next(SegmentType::ChordRest)) {
                        ChordRest* ncr = toChordRest(ns->element(track + voice));
                        if (ncr && ncr->type() == ElementType::CHORD) {
                            ok = false;
                            break;
                        }
                    }
                    if (ok) {
                        break;
                    }
                }
                // no available voices, just use voice 0
                if (voice == VOICES) {
                    voice = 0;
                }
                // no cr was found in segment for this voice, so make gap
                if (needGap[voice]) {
                    makeGapVoice(s, track + voice, f, s->tick());
                }
            }
            // construct note
            int line = 0;
            bool error = false;
            NoteVal nv;
            if (staff(staffIdx)->staffType(s->tick())->group() == StaffGroup::TAB) {
                line = staff(staffIdx)->lines(s->tick()) / 2;
            } else {
                line = staff(staffIdx)->middleLine(s->tick());             // staff(staffIdx)->lines() - 1;
            }
            if (staff(staffIdx)->staffType(s->tick())->group() == StaffGroup::PERCUSSION) {
                nv.pitch = 0;
                nv.headGroup = NoteHeadGroup::HEAD_SLASH;
            } else {
                Position p;
                p.segment = s;
                p.staffIdx = staffIdx;
                p.line = line;
                p.fret = INVALID_FRET_INDEX;
                m_is.setRest(false);             // needed for tab
                nv = noteValForPosition(p, AccidentalType::NONE, error);
            }
            if (error) {
                continue;
            }
            // insert & turn into slash
            s = setNoteRest(s, track + voice, nv, f);
            Chord* c = toChord(s->element(track + voice));
            if (c) {
                if (c->links()) {
                    for (EngravingObject* e : *c->links()) {
                        Chord* lc = toChord(e);
                        lc->setSlash(true, true);
                    }
                } else {
                    c->setSlash(true, true);
                }
            }
            lastSlash = c;
            if (!firstSlash) {
                firstSlash = c;
            }
        }
    }

    // re-select the slashes
    deselectAll();
    if (firstSlash && lastSlash) {
        select(firstSlash, SelectType::RANGE);
        select(lastSlash, SelectType::RANGE);
    }
}

//---------------------------------------------------------
//   cmdSlashRhythm
///   converts rhythms in selected region to slashes
//---------------------------------------------------------

void Score::cmdSlashRhythm()
{
    std::set<Chord*> chords;
    // loop through all notes in selection
    for (EngravingItem* e : selection().elements()) {
        if (e->voice() >= 2 && e->isRest()) {
            Rest* r = toRest(e);
            if (r->links()) {
                for (EngravingObject* se : *r->links()) {
                    Rest* lr = toRest(se);
                    lr->setAccent(!lr->accent());
                }
            } else {
                r->setAccent(!r->accent());
            }
            continue;
        } else if (e->isNote()) {
            Note* n = toNote(e);
            if (n->noteType() != NoteType::NORMAL) {
                continue;
            }
            Chord* c = n->chord();
            // check for duplicates (chords with multiple notes)
            if (muse::contains(chords, c)) {
                continue;
            }
            chords.insert(c);
            // toggle slash setting
            c->setSlash(!c->slash(), false);
        }
    }
}

//---------------------------------------------------------
//   setChord
//    return segment of last created chord
//---------------------------------------------------------
static Segment* setChord(Score* score, Segment* segment, track_idx_t track, const Chord* chordTemplate, Fraction dur)
{
    assert(segment->segmentType() == SegmentType::ChordRest);

    Fraction tick = segment->tick();
    Chord* nr     = nullptr;   //current added chord used so we can select the last added chord and so we can apply ties
    std::vector<Tie*> tie(chordTemplate->notes().size());   //keep pointer to a tie for each note in the chord in case we need to tie notes
    ChordRest* cr = toChordRest(segment->element(track));   //chord rest under the segment for the specified track

    bool addTie = false;

    Measure* measure = nullptr;
    //keep creating chords and tieing them until we created the full duration asked for (dur)
    for (;;) {
        if (track % VOICES) {
            score->expandVoice(segment, track);
        }

        Tuplet* t = cr ? cr->tuplet() : 0;
        Fraction tDur = segment->ticks();
        Segment* seg = segment->next();

        //we need to get a correct subduration so that makeGap can function properly
        //since makeGap() takes "normal" duration rather than actual length
        while (seg) {
            if (seg->segmentType() == SegmentType::ChordRest) {
                //design choice made to keep multiple notes across a tuplet as tied single notes rather than combining them
                //since it's arguably more readable, but the other code is still here (commented)
                ChordRest* testCr = toChordRest(seg->element(track));

                //code here allows us to combine tuplet realization together which I have opted not to do for readability (of the music)
                //if (!!t ^ (testCr && testCr->tuplet())) //stop if we started with a tuplet and reach something that's not a tuplet,
                //      break;                          //or start with not a tuplet and reach a tuplet

                if (testCr && testCr->tuplet()) {       //stop on tuplet
                    break;
                }
                tDur += seg->ticks();
            }
            if (tDur >= dur) {       //do not go further than the duration asked for
                tDur = dur;
                break;
            }
            seg = seg->next();       //iterate only across measure (hence usage of next() rather than next1())
        }
        if (t) {
            tDur *= t->ratio();       //scale by tuplet ratio to get "normal" length rather than actual length when dealing with tuplets
        }
        // the returned gap ends at the measure boundary or at tuplet end
        Fraction dd = score->makeGap(segment, track, tDur, t);

        if (dd.isZero()) {
            LOGD("cannot get gap at %d type: %d/%d", tick.ticks(), dur.numerator(),
                 dur.denominator());
            break;
        }

        measure = segment->measure();
        std::vector<TDuration> dl = toDurationList(dd, true);
        size_t n = dl.size();
        //add chord, tieing when necessary within measure
        for (size_t i = 0; i < n; ++i) {
            const TDuration& d = dl[i];

            //create new chord from template and add it
            Chord* chord = Factory::copyChord(*chordTemplate);
            nr = chord;

            chord->setTrack(track);
            chord->setDurationType(d);
            chord->setTicks(d.fraction());
            chord->setTuplet(t);
            score->undoAddCR(chord, measure, tick);
            //if there is something to tie, complete tie backwards
            //and add the tie to score
            const std::vector<Note*> notes = chord->notes();
            if (addTie) {
                for (size_t j = 0; j < notes.size(); ++j) {
                    tie[j]->setEndNote(notes[j]);
                    notes[j]->setTieBack(tie[j]);
                    score->undoAddElement(tie[j]);
                }
                addTie = false;
            }
            //if we're not the last element in the duration list,
            //set tie forward
            if (i + 1 < n) {
                for (size_t j = 0; j < notes.size(); ++j) {
                    tie[j] = Factory::createTie(score->dummy());
                    tie[j]->setStartNote(notes[j]);
                    tie[j]->setTick(tie[j]->startNote()->tick());
                    tie[j]->setTrack(track);
                    notes[j]->setTieFor(tie[j]);
                    addTie = true;
                }
            }
            score->setPlayChord(true);
            segment = chord->segment();
            tick += chord->actualTicks();
        }

        //subtract the duration already realized and move on
        if (t) {
            dur -= dd / t->ratio();
        } else {
            dur -= dd;
        }
        //we are done when there is no duration left to realize
        if (dur.isZero()) {
            break;
        }

        //go to next segment unless we are at the score (which means we will just be done there)
        Segment* nseg = score->tick2segment(tick, false, SegmentType::ChordRest);
        if (!nseg) {
            LOGD("reached end of score");
            break;
        }

        //it is possible that the next measure's ticks have not been computed yet. compute them now
        if (nseg->ticks().isZero()) {
            nseg->measure()->computeTicks();
        }

        segment = nseg;

        cr = toChordRest(segment->element(track));

        if (!cr) {
            if (track % VOICES) {
                cr = score->addRest(segment, track, TDuration(DurationType::V_MEASURE), 0);
            } else {
                LOGD("no rest in voice 0");
                break;
            }
        }
        //
        //  Note does not fit on current measure, create Tie to
        //  next part of note
        std::vector<Note*> notes = nr->notes();
        for (size_t i = 0; i < notes.size(); ++i) {
            tie[i] = Factory::createTie(score->dummy());
            tie[i]->setStartNote(notes[i]);
            tie[i]->setTick(tie[i]->startNote()->tick());
            tie[i]->setTrack(notes[i]->track());
            notes[i]->setTieFor(tie[i]);
        }
    }
    if (!tie.empty()) {
        score->connectTies();
    }
    if (nr) {
        score->select(nr, SelectType::SINGLE, 0);
    }
    return segment;
}

//---------------------------------------------------------
//   cmdRealizeChordSymbols
///   Realize selected chord symbols into notes on the staff.
///
///   If a voicing and duration type are specified, the
///   harmony voicing settings will be overridden by the
///   passed parameters. Otherwise, the settings set on the
///   harmony object will be used.
//---------------------------------------------------------

void Score::cmdRealizeChordSymbols(bool literal, Voicing voicing, HDuration durationType)
{
    // Create copy, because setChord selects newly created chord and thus
    // modifies selection().elements() while we're iterating over it
    const std::vector<EngravingItem*> elist = selection().elements();

    for (EngravingItem* e : elist) {
        if (!e->isHarmony()) {
            continue;
        }
        Harmony* h = toHarmony(e);
        if (!h->isRealizable()) {
            continue;
        }
        const RealizedHarmony& r = h->getRealizedHarmony();
        Segment* seg = h->explicitParent()->isSegment() ? toSegment(h->explicitParent()) : toSegment(h->explicitParent()->explicitParent());
        Fraction tick = seg->tick();
        Fraction duration = r.getActualDuration(tick.ticks(), durationType);
        bool concertPitch = style().styleB(Sid::concertPitch);

        Chord* chord = Factory::createChord(this->dummy()->segment());     //chord template
        chord->setTrack(h->track());     //set track so notes have a track to sit on

        //create chord from notes
        RealizedHarmony::PitchMap notes;
        if (voicing == Voicing::INVALID || durationType == HDuration::INVALID) {
            notes = r.notes();       //no override, just use notes from realize harmony
        } else {
            //generate notes list based on overridden settings
            int offset = 0;
            Interval interval = h->staff()->part()->instrument(h->tick())->transpose();
            if (!concertPitch) {
                offset = interval.chromatic;
            }
            notes = r.generateNotes(h->rootTpc(), h->baseTpc(),
                                    literal, voicing, offset);
        }

        for (const auto& p : notes) {
            Note* note = Factory::createNote(chord);
            NoteVal nval;
            nval.pitch = p.first;
            if (concertPitch) {
                nval.tpc1 = p.second;
            } else {
                nval.tpc2 = p.second;
            }
            chord->add(note);       //add note first to set track and such
            note->setNval(nval, tick);
        }

        setChord(this, seg, h->track(), chord, duration);     //add chord using template
        delete chord;
    }
}

//---------------------------------------------------------
//   cmdResequenceRehearsalMarks
///   resequences rehearsal marks within a range selection
///   or, if nothing is selected, the entire score
//---------------------------------------------------------

void Score::cmdResequenceRehearsalMarks()
{
    bool noSelection = !selection().isRange();

    if (noSelection) {
        cmdSelectAll();
    } else if (!selection().isRange()) {
        return;
    }

    RehearsalMark* last = nullptr;
    for (Segment* s = selection().startSegment(); s && s != selection().endSegment(); s = s->next1()) {
        for (EngravingItem* e : s->annotations()) {
            if (e->type() == ElementType::REHEARSAL_MARK) {
                RehearsalMark* rm = toRehearsalMark(e);
                if (rm->isTopSystemObject()) {
                    if (last) {
                        String rmText = nextRehearsalMarkText(last, rm);
                        for (EngravingObject* le : rm->linkList()) {
                            le->undoChangeProperty(Pid::TEXT, rmText);
                        }
                    }
                    last = rm;
                }
            }
        }
    }

    if (noSelection) {
        deselectAll();
    }
}

void Score::addRemoveSystemLocks(int interval, bool lock)
{
    bool mmrests = style().styleB(Sid::createMultiMeasureRests);

    MeasureBase* startMeasure = selection().startMeasureBase();
    MeasureBase* endMeasure = selection().endMeasureBase();
    if (!endMeasure) {
        endMeasure = mmrests ? lastMeasureMM() : lastMeasure();
    }

    if (!startMeasure || !endMeasure) {
        return;
    }

    if (lock) {
        for (const System* system : m_systems) {
            if (system->last()->isBefore(startMeasure)) {
                continue;
            }
            if (system->first()->isAfter(endMeasure)) {
                break;
            }
            if (!system->isLocked()) {
                undoAddSystemLock(new SystemLock(system->first(), system->last()));
            }
        }
        return;
    }

    std::vector<const SystemLock*> currentLocks = m_systemLocks.locksContainedInRange(startMeasure, endMeasure);
    for (const SystemLock* l : currentLocks) {
        undoRemoveSystemLock(l);
    }

    if (interval == 0) {
        return;
    }

    int count = 0;
    MeasureBase* lockStart = nullptr;
    for (MeasureBase* mb = startMeasure; mb; mb = mmrests ? mb->nextMM() : mb->next()) {
        if (count == 0) {
            lockStart = mb;
        }
        count++;
        if (count == interval || mb == endMeasure) {
            undoAddSystemLock(new SystemLock(lockStart, mb));
            lockStart = nullptr;
            count = 0;
        }
        if (mb == endMeasure) {
            break;
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
//   cmdPitchUp
//---------------------------------------------------------

void Score::cmdPitchUp()
{
    EngravingItem* el = selection().element();
    if (el && el->isLyrics()) {
        cmdMoveLyrics(toLyrics(el), DirectionV::UP);
    } else if (el && (el->isArticulationFamily() || el->isTextBase())) {
        el->undoChangeProperty(Pid::OFFSET, el->offset() + PointF(0.0, -MScore::nudgeStep * el->spatium()), PropertyFlags::UNSTYLED);
    } else if (el && el->isRest()) {
        cmdMoveRest(toRest(el), DirectionV::UP);
    } else {
        upDown(true, UpDownMode::CHROMATIC);
    }
}

//---------------------------------------------------------
//   cmdPitchDown
//---------------------------------------------------------

void Score::cmdPitchDown()
{
    EngravingItem* el = selection().element();
    if (el && el->isLyrics()) {
        cmdMoveLyrics(toLyrics(el), DirectionV::DOWN);
    } else if (el && (el->isArticulationFamily() || el->isTextBase())) {
        el->undoChangeProperty(Pid::OFFSET, PropertyValue::fromValue(el->offset() + PointF(0.0, MScore::nudgeStep * el->spatium())),
                               PropertyFlags::UNSTYLED);
    } else if (el && el->isRest()) {
        cmdMoveRest(toRest(el), DirectionV::DOWN);
    } else {
        upDown(false, UpDownMode::CHROMATIC);
    }
}

//---------------------------------------------------------
//   cmdPitchUpOctave
//---------------------------------------------------------

void Score::cmdPitchUpOctave()
{
    EngravingItem* el = selection().element();
    if (el && (el->isArticulationFamily() || el->isTextBase())) {
        el->undoChangeProperty(Pid::OFFSET,
                               PropertyValue::fromValue(el->offset() + PointF(0.0, -MScore::nudgeStep10 * el->spatium())),
                               PropertyFlags::UNSTYLED);
    } else {
        upDown(true, UpDownMode::OCTAVE);
    }
}

//---------------------------------------------------------
//   cmdPitchDownOctave
//---------------------------------------------------------

void Score::cmdPitchDownOctave()
{
    EngravingItem* el = selection().element();
    if (el && (el->isArticulationFamily() || el->isTextBase())) {
        el->undoChangeProperty(Pid::OFFSET, el->offset() + PointF(0.0, MScore::nudgeStep10 * el->spatium()), PropertyFlags::UNSTYLED);
    } else {
        upDown(false, UpDownMode::OCTAVE);
    }
}

void Score::cmdPadNoteIncreaseTAB()
{
    switch (m_is.duration().type()) {
// cycle back from longest to shortest?
//          case TDuration::V_LONG:
//                padToggle(Pad::NOTE128);
//                break;
    case DurationType::V_BREVE:
        padToggle(Pad::NOTE00);
        break;
    case DurationType::V_WHOLE:
        padToggle(Pad::NOTE0);
        break;
    case DurationType::V_HALF:
        padToggle(Pad::NOTE1);
        break;
    case DurationType::V_QUARTER:
        padToggle(Pad::NOTE2);
        break;
    case DurationType::V_EIGHTH:
        padToggle(Pad::NOTE4);
        break;
    case DurationType::V_16TH:
        padToggle(Pad::NOTE8);
        break;
    case DurationType::V_32ND:
        padToggle(Pad::NOTE16);
        break;
    case DurationType::V_64TH:
        padToggle(Pad::NOTE32);
        break;
    case DurationType::V_128TH:
        padToggle(Pad::NOTE64);
        break;
    default:
        break;
    }
}

void Score::cmdPadNoteDecreaseTAB()
{
    switch (m_is.duration().type()) {
    case DurationType::V_LONG:
        padToggle(Pad::NOTE0);
        break;
    case DurationType::V_BREVE:
        padToggle(Pad::NOTE1);
        break;
    case DurationType::V_WHOLE:
        padToggle(Pad::NOTE2);
        break;
    case DurationType::V_HALF:
        padToggle(Pad::NOTE4);
        break;
    case DurationType::V_QUARTER:
        padToggle(Pad::NOTE8);
        break;
    case DurationType::V_EIGHTH:
        padToggle(Pad::NOTE16);
        break;
    case DurationType::V_16TH:
        padToggle(Pad::NOTE32);
        break;
    case DurationType::V_32ND:
        padToggle(Pad::NOTE64);
        break;
    case DurationType::V_64TH:
        padToggle(Pad::NOTE128);
        break;
    case DurationType::V_128TH:
        padToggle(Pad::NOTE256);
        break;
    case DurationType::V_256TH:
        padToggle(Pad::NOTE512);
        break;
    case DurationType::V_512TH:
        padToggle(Pad::NOTE1024);
        break;
// cycle back from shortest to longest?
//          case DurationType::V_1024TH:
//                padToggle(Pad::NOTE00);
//                break;
    default:
        break;
    }
}

//---------------------------------------------------------
//   cmdToggleLayoutBreak
//---------------------------------------------------------

void Score::cmdToggleLayoutBreak(LayoutBreakType type)
{
    // find measure(s)
    std::list<MeasureBase*> mbl;
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
                    mbl.push_back(m);
                    if (m == endMeasure) {
                        mbl.pop_back();
                        break;
                    }
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
                mb = toMeasureBase(el);
                break;
            default: {
                // find measure
                Measure* measure = toMeasure(el->findMeasure());
                // for start repeat, attach break to previous measure
                if (measure && el->isBarLine()) {
                    BarLine* bl = toBarLine(el);
                    if (bl->barLineType() == BarLineType::START_REPEAT) {
                        measure = measure->prevMeasure();
                    }
                }
                // if measure is mmrest, then propagate to last original measure
                if (measure) {
                    mb = measure->isMMRest() ? measure->mmRestLast() : measure;
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
    for (MeasureBase* mb: mbl) {
        bool val = false;
        switch (type) {
        case LayoutBreakType::LINE:
            val = !mb->lineBreak();
            if (val) {
                removeSystemLocksOnAddLayoutBreak(type, mb);
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
                removeSystemLocksOnAddLayoutBreak(type, mb);
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
                removeSystemLocksOnAddLayoutBreak(type, mb);
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

void Score::cmdMoveMeasureToPrevSystem()
{
    bool mmrests = style().styleB(Sid::createMultiMeasureRests);

    MeasureBase* refMeasure = m_selection.endMeasureBase();
    if (!refMeasure) {
        return;
    }

    const System* prevSystem = refMeasure->prevNonVBoxSystem();
    if (!prevSystem) {
        return;
    }

    MeasureBase* prevSystemFirstMeas = prevSystem->first();

    const SystemLock* prevSystemLock = m_systemLocks.lockStartingAt(prevSystemFirstMeas);
    if (prevSystemLock) {
        undoRemoveSystemLock(prevSystemLock);
    }

    const System* curSystem = refMeasure->system();
    const SystemLock* curSystemLock = m_systemLocks.lockStartingAt(curSystem->first());
    if (curSystemLock) {
        undoRemoveSystemLock(curSystemLock);
        if (curSystemLock->endMB() != refMeasure) {
            MeasureBase* nextMB = mmrests ? refMeasure->nextMM() : refMeasure->next();
            SystemLock* newLockOnCurSystem = new SystemLock(nextMB, curSystemLock->endMB());
            undoAddSystemLock(newLockOnCurSystem);
        }
    }

    SystemLock* sysLock = new SystemLock(prevSystemFirstMeas, refMeasure);
    undoAddSystemLock(sysLock);
}

void Score::cmdMoveMeasureToNextSystem()
{
    bool mmrests = style().styleB(Sid::createMultiMeasureRests);

    MeasureBase* refMeasure = m_selection.startMeasureBase();
    if (!refMeasure) {
        return;
    }

    const System* curSystem = refMeasure->system();
    MeasureBase* startMeas = curSystem->first();
    bool refMeasureIsStartOfSystem = refMeasure == startMeas;

    const SystemLock* curLock = m_systemLocks.lockStartingAt(startMeas);
    if (curLock) {
        undoRemoveSystemLock(curLock);
    }

    if (!refMeasureIsStartOfSystem) {
        MeasureBase* prevMeas = mmrests ? refMeasure->prevMM() : refMeasure->prev();
        SystemLock* sysLock = new SystemLock(startMeas, prevMeas);
        undoAddSystemLock(sysLock);
    }

    const System* nextSystem = refMeasure->nextNonVBoxSystem();
    if (!nextSystem) {
        return;
    }

    const SystemLock* nextSysLock = m_systemLocks.lockStartingAt(nextSystem->first());
    if (nextSysLock) {
        undoRemoveSystemLock(nextSysLock);
    }

    if (nextSysLock || refMeasureIsStartOfSystem) {
        SystemLock* newNextSysLock = new SystemLock(refMeasure, nextSystem->last());
        undoAddSystemLock(newNextSysLock);
    }
}

void Score::cmdToggleSystemLock()
{
    toggleSystemLock(m_selection.selectedSystems());
}

void Score::cmdApplyLockToSelection()
{
    MeasureBase* first = nullptr;
    MeasureBase* last = nullptr;

    if (selection().isRange()) {
        first = selection().startMeasureBase();
        last = selection().endMeasureBase();
    } else {
        for (EngravingItem* el : selection().elements()) {
            if (el->isSystemLockIndicator()) {
                const SystemLock* lock = toSystemLockIndicator(el)->systemLock();
                first = lock->startMB();
                last = lock->endMB();
                break;
            }
            MeasureBase* mb = el->findMeasureBase();
            if (!mb) {
                continue;
            }
            if (!first || mb->isBefore(first)) {
                first = mb;
            }
            if (!last || mb->isAfter(last)) {
                last = mb;
            }
        }
    }

    if (!first || !last) {
        return;
    }

    const SystemLock* lockOnLast = systemLocks()->lockContaining(last);
    if (lockOnLast && lockOnLast->endMB() == last) {
        undoRemoveSystemLock(lockOnLast);
    } else if (first != last) {
        makeIntoSystem(first, last);
    } else {
        makeIntoSystem(first->system()->first(), last);
    }
}

void Score::cmdToggleScoreLock()
{
    bool unlockAll = true;
    for (const System* system : m_systems) {
        const MeasureBase* first = system->first();
        if (!(first->isMeasure() || first->isHBox())) {
            continue;
        }
        if (!system->isLocked()) {
            unlockAll = false;
            break;
        }
    }

    for (System* system : m_systems) {
        MeasureBase* startMeas = system->first();
        if (!(startMeas->isMeasure() || startMeas->isHBox())) {
            continue;
        }
        const SystemLock* currentLock = m_systemLocks.lockStartingAt(startMeas);
        if (currentLock && unlockAll) {
            undoRemoveSystemLock(currentLock);
            continue;
        } else if (!currentLock && !unlockAll) {
            SystemLock* newSystemLock = new SystemLock(startMeas, system->last());
            undoAddSystemLock(newSystemLock);
        }
    }
}

void Score::cmdMakeIntoSystem()
{
    MeasureBase* firstSelected = m_selection.startMeasureBase();
    MeasureBase* lastSelected = m_selection.endMeasureBase();
    if (!(firstSelected && lastSelected)) {
        return;
    }

    makeIntoSystem(firstSelected, lastSelected);
}

void Score::cmdAddStaffTypeChange(Measure* measure, staff_idx_t staffIdx, StaffTypeChange* stc)
{
    if (!measure) {
        return;
    }

    if (measure->isMMRest()) {
        measure = measure->mmRestFirst();
    }

    stc->setParent(measure);
    stc->setTrack(staffIdx * VOICES);
    score()->undoAddElement(stc);
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

//---------------------------------------------------------
//   cmdSetVisible
//---------------------------------------------------------

void Score::cmdSetVisible()
{
    for (EngravingItem* e : selection().elements()) {
        undo(new ChangeProperty(e, Pid::VISIBLE, true));
    }
}

//---------------------------------------------------------
//   cmdUnsetVisible
//---------------------------------------------------------

void Score::cmdUnsetVisible()
{
    for (EngravingItem* e : selection().elements()) {
        undo(new ChangeProperty(e, Pid::VISIBLE, false));
    }
}

bool Score::resolveNoteInputParams(int note, bool addFlag, NoteInputParams& out) const
{
    const InputState& is = inputState();

    //! NOTE: Drumset params should be defined explicitly (see NotationViewInputController::tryPercussionShortcut)
    if (!is.isValid() || is.drumset()) {
        return false;
    }

    int octave = 4;

    static const int tab[] = { 0, 2, 4, 5, 7, 9, 11 };

    // if adding notes, add above the upNote of the current chord
    EngravingItem* el = selection().element();
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
            Staff* staff = Score::staff(is.track() / VOICES);
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
                        ClefType ctb = staff->clef(clef->tick() - Fraction::fromTicks(1));
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
//   cmdAddPitch
///   insert note or add note to chord
//---------------------------------------------------------
void Score::cmdAddPitch(const EditData& ed, const NoteInputParams& params, bool addFlag, bool insert)
{
    InputState& is = inputState();
    if (!is.isValid()) {
        LOGD("cannot enter notes here (no chord rest at current position)");
        return;
    }

    is.setRest(false);

    const Drumset* ds = is.drumset();
    if (ds) {
        is.setDrumNote(params.drumPitch);
        is.setVoice(ds->voice(params.drumPitch));

        if (is.segment()) {
            Segment* seg = is.segment();
            while (seg) {
                if (seg->element(is.track())) {
                    break;
                }
                seg = seg->prev(SegmentType::ChordRest);
            }
            if (seg) {
                is.setSegment(seg);
            } else {
                is.setSegment(is.segment()->measure()->first(SegmentType::ChordRest));
            }
        }
    }

    cmdAddPitch(params.step, addFlag, insert);

    ed.view()->adjustCanvasPosition(is.cr());
}

void Score::cmdAddPitch(int step, bool addFlag, bool insert)
{
    insert = insert || inputState().usingNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    Position pos;
    if (addFlag) {
        EngravingItem* el = selection().element();
        if (el && el->isNote()) {
            Note* selectedNote = toNote(el);
            Chord* chord  = selectedNote->chord();
            Segment* seg  = chord->segment();
            pos.segment   = seg;
            pos.staffIdx  = chord->vStaffIdx();
            ClefType clef = staff(pos.staffIdx)->clef(seg->tick());
            pos.line      = relStep(step, clef);
            bool error;
            NoteVal nval = noteValForPosition(pos, m_is.accidentalType(), error);
            if (error) {
                return;
            }
            bool forceAccidental = false;
            if (m_is.accidentalType() != AccidentalType::NONE) {
                NoteVal nval2 = noteValForPosition(pos, AccidentalType::NONE, error);
                forceAccidental = (nval.pitch == nval2.pitch);
            }
            addNote(chord, nval, forceAccidental, m_is.articulationIds());
            m_is.setAccidentalType(AccidentalType::NONE);
            return;
        }
    }

    pos.segment   = inputState().segment();
    pos.staffIdx  = inputState().track() / VOICES;
    ClefType clef = staff(pos.staffIdx)->clef(pos.segment->tick());
    pos.line      = relStep(step, clef);

    if (inputState().usingNoteEntryMethod(NoteEntryMethod::REPITCH)) {
        repitchNote(pos, !addFlag);
    } else {
        if (insert) {
            insertChordByInsertingTime(pos);
        } else {
            putNote(pos, !addFlag);
        }
    }
    m_is.setAccidentalType(AccidentalType::NONE);
}

void Score::cmdToggleVisible()
{
    bool allVisible = true;

    for (EngravingItem* item : selection().elements()) {
        if (!item->visible()) {
            allVisible = false;
            break;
        }
    }

    bool newVisible = !allVisible;

    for (EngravingItem* item : selection().elements()) {
        undoChangeVisible(item, newVisible);
    }
}

//---------------------------------------------------------
//   cmdAddFret
///   insert note with given fret on current string
//---------------------------------------------------------

void Score::cmdAddFret(int fret)
{
    InputState& is = inputState();
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
    pos.line      = staff(pos.staffIdx)->staffType(is.tick())->physStringToVisual(is.string());
    pos.fret      = fret;
    putNote(pos, false);
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
                if (EngravingItem* ee = e->propertyDelegate(Pid::AUTOPLACE)) {
                    e = ee;
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
