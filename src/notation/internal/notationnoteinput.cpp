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
#include "notationnoteinput.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/input.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/note.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/system.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/utils.h"

#include "mscoreerrorscontroller.h"

#include "log.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::async;

static bool noteInputMethodAvailable(NoteInputMethod method, const Staff* staff, const Fraction& tick)
{
    if (method == NoteInputMethod::BY_DURATION) {
        return staff && !staff->isTabStaff(tick);
    }

    return true;
}

NotationNoteInput::NotationNoteInput(const IGetScore* getScore, INotationInteraction* interaction, INotationUndoStackPtr undoStack
                                     , const modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx), m_getScore(getScore), m_interaction(interaction), m_undoStack(undoStack)
{
    m_interaction->selectionChanged().onNotify(this, [this]() {
        if (!isNoteInputMode()) {
            updateInputState();
        } else if (shouldSetupInputNote()) {
            const NoteInputState& is = state();
            const staff_idx_t prevStaffIdx = mu::engraving::track2staff(is.prevTrack());

            if (prevStaffIdx != is.staffIdx()) {
                setupInputNote();
            }
        }
    });
}

bool NotationNoteInput::isNoteInputMode() const
{
    return score()->inputState().noteEntryMode();
}

const NoteInputState& NotationNoteInput::state() const
{
    return score()->inputState();
}

//! NOTE Copied from `void ScoreView::startNoteEntry()`
void NotationNoteInput::startNoteInput(NoteInputMethod method, bool focusNotation)
{
    TRACEFUNC;

    if (isNoteInputMode()) {
        return;
    }

    EngravingItem* el = resolveNoteInputStartPosition();
    if (!el) {
        return;
    }

    m_interaction->select({ el }, SelectType::SINGLE, 0);

    mu::engraving::InputState& is = score()->inputState();

    Duration d(is.duration());
    if (!d.isValid() || d.isZero() || d.type() == DurationType::V_MEASURE) {
        is.setDuration(Duration(DurationType::V_QUARTER));
    }
    is.setAccidentalType(mu::engraving::AccidentalType::NONE);

    is.setRest(false);
    is.setNoteEntryMode(true);

    const Staff* staff = score()->staff(is.track() / mu::engraving::VOICES);

    if (noteInputMethodAvailable(method, staff, is.tick())) {
        is.setNoteEntryMethod(method);
    } else {
        is.setNoteEntryMethod(NoteInputMethod::BY_NOTE_NAME); // fallback
    }

    if (shouldSetupInputNote()) {
        setupInputNote();
    }

    switch (staff->staffType(is.tick())->group()) {
    case mu::engraving::StaffGroup::STANDARD:
        break;
    case mu::engraving::StaffGroup::TAB: {
        int strg = 0;                           // assume topmost string as current string
        // if entering note entry with a note selected and the note has a string
        // set InputState::_string to note physical string
        if (el->type() == ElementType::NOTE) {
            strg = (static_cast<mu::engraving::Note*>(el))->string();
        }
        is.setString(strg);
        break;
    }
    case mu::engraving::StaffGroup::PERCUSSION:
        break;
    }

    notifyAboutNoteInputStarted(focusNotation);
    notifyAboutStateChanged();

    m_interaction->showItem(el);
}

EngravingItem* NotationNoteInput::resolveNoteInputStartPosition() const
{
    ChordRest* topLeftChordRest = nullptr;

    if (score()->selection().isNone() && m_getViewRectFunc) {
        // no selection
        // choose page in current view (favor top left quadrant if possible)
        // select first (top/left) chordrest of that page in current view
        // or, CR at last selected position if that is in view
        RectF viewRect = m_getViewRectFunc();
        PointF topLeft = viewRect.topLeft();

        std::vector<PointF> points;
        points.push_back({ topLeft.x() + viewRect.width() * 0.25, topLeft.y() + viewRect.height() * 0.25 });
        points.push_back(topLeft);
        points.push_back(viewRect.bottomLeft());
        points.push_back(viewRect.topRight());
        points.push_back(viewRect.bottomRight());

        Page* page = nullptr;
        for (const PointF& point : points) {
            page = score()->searchPage(point);
            if (page) {
                break;
            }
        }

        if (page) {
            qreal tlY = 0.0;
            Fraction tlTick = Fraction(0, 1);
            RectF pageRect  = page->ldata()->bbox().translated(page->x(), page->y());
            RectF intersect = viewRect & pageRect;
            intersect.translate(-page->x(), -page->y());
            std::vector<EngravingItem*> el = page->items(intersect);

            const ChordRest* lastSelected = score()->selection().currentCR();

            if (lastSelected && lastSelected->voice()) {
                // if last selected CR was not in voice 1,
                // find CR in voice 1 instead
                track_idx_t track = mu::engraving::trackZeroVoice(lastSelected->track());
                const mu::engraving::Segment* s = lastSelected->segment();
                if (s) {
                    lastSelected = s->nextChordRest(track, true);
                }
            }

            for (EngravingItem* e : el) {
                // loop through visible elements
                // looking for the CR in voice 1 with earliest tick and highest staff position
                // but stop if we find the last selected CR
                ElementType et = e->type();
                if (et == ElementType::NOTE || et == ElementType::REST) {
                    if (e->voice()) {
                        continue;
                    }
                    ChordRest* cr;
                    if (et == ElementType::NOTE) {
                        cr = static_cast<ChordRest*>(e->parent());
                        if (!cr) {
                            continue;
                        }
                    } else {
                        cr = static_cast<ChordRest*>(e);
                    }
                    if (cr == lastSelected) {
                        topLeftChordRest = cr;
                        break;
                    }
                    // compare ticks rather than x position
                    // to make sure we favor earlier rather than later systems
                    // even though later system might have note farther to left
                    Fraction crTick = Fraction(0, 1);
                    if (cr->segment()) {
                        crTick = cr->segment()->tick();
                    } else {
                        continue;
                    }
                    // compare staff Y position rather than note Y position
                    // to be sure we do not reject earliest note
                    // just because it is lower in pitch than subsequent notes
                    qreal crY = 0.0;
                    if (cr->measure() && cr->measure()->system()) {
                        crY = cr->measure()->system()->staffYpage(cr->staffIdx());
                    } else {
                        continue;
                    }
                    if (topLeftChordRest) {
                        if (crTick <= tlTick && crY <= tlY) {
                            topLeftChordRest = cr;
                            tlTick = crTick;
                            tlY = crY;
                        }
                    } else {
                        topLeftChordRest = cr;
                        tlTick = crTick;
                        tlY = crY;
                    }
                }
            }
        }
    }

    EngravingItem* el = topLeftChordRest ? topLeftChordRest : score()->selection().element();
    if (!el) {
        el = score()->selection().firstChordRest();
    }

    const mu::engraving::InputState& is = score()->inputState();

    if (!el) {
        if (const mu::engraving::Segment* segment = is.lastSegment()) {
            el = segment->element(is.track());
        }
    }

    if (el == nullptr
        || (el->type() != ElementType::CHORD && el->type() != ElementType::REST && el->type() != ElementType::NOTE)) {
        // if no note/rest is selected, start with voice 0
        engraving::track_idx_t track = is.track() == muse::nidx ? 0 : (is.track() / mu::engraving::VOICES) * mu::engraving::VOICES;
        // try to find an appropriate measure to start in
        Fraction tick = el ? el->tick() : Fraction(0, 1);
        el = score()->searchNote(tick, track);
        if (!el) {
            el = score()->searchNote(Fraction(0, 1), track);
        }
    }

    if (!el) {
        return nullptr;
    }

    if (el->type() == ElementType::CHORD) {
        mu::engraving::Chord* c = static_cast<mu::engraving::Chord*>(el);
        mu::engraving::Note* note = c->selectedNote();
        if (note == 0) {
            note = c->upNote();
        }
        el = note;
    }

    return el;
}

bool NotationNoteInput::shouldSetupInputNote() const
{
    return usingNoteInputMethod(NoteInputMethod::BY_DURATION)
           || usingNoteInputMethod(NoteInputMethod::RHYTHM);
}

void NotationNoteInput::setupInputNote()
{
    mu::engraving::InputState& is = score()->inputState();
    const EngravingItem* selectedItem = score()->selection().element();

    if (selectedItem && selectedItem->isNote()) {
        is.setNotes({ toNote(selectedItem)->noteVal() });
        return;
    }

    const Fraction tick = is.tick();
    Staff* staff = is.staff();
    NoteVal nval;

    if (staff->isTabStaff(tick)) {
        if (const StringData* stringData = staff->part()->stringData(tick, is.staffIdx())) {
            nval.fret = 0;
            nval.string = is.string();
            nval.pitch = stringData->getPitch(nval.string, nval.fret, staff);
        }
    } else if (staff->isDrumStaff(tick)) {
        if (const Drumset* drumset = is.drumset()) {
            nval.pitch = is.drumNote();

            if (nval.pitch < 0) {
                nval.pitch = drumset->nextPitch(nval.pitch);
            }

            if (drumset->isValid(nval.pitch)) {
                nval.headGroup = drumset->noteHead(nval.pitch);
            }
        }
    } else {
        nval = noteValForLine(staff->middleLine(tick));
    }

    if (nval.pitch > 0) {
        is.setNotes({ nval });
    }
}

NoteVal NotationNoteInput::noteValForLine(int line) const
{
    const mu::engraving::InputState& is = score()->inputState();

    mu::engraving::Position pos;
    pos.segment = is.segment();
    pos.staffIdx = is.staffIdx();
    pos.line = line;

    bool error = false;
    const NoteVal nval = score()->noteValForPosition(pos, is.accidentalType(), error);
    if (error) {
        LOGE() << "Could not find note val for position, staffIdx: " << pos.staffIdx << ", line: " << pos.line;
    }

    return nval;
}

void NotationNoteInput::endNoteInput(bool resetState)
{
    TRACEFUNC;

    if (!isNoteInputMode()) {
        return;
    }

    mu::engraving::InputState& is = score()->inputState();
    is.setNoteEntryMode(false);

    if (is.slur()) {
        const std::vector<mu::engraving::SpannerSegment*>& el = is.slur()->spannerSegments();
        if (!el.empty()) {
            el.front()->setSelected(false);
        }
        is.setSlur(0);
    }

    if (resetState) {
        is.setTrack(muse::nidx);
        is.setString(-1);
        is.setSegment(nullptr);
        is.setNotes({});
    }

    notifyAboutNoteInputEnded();
    updateInputState();
}

Channel</*focusNotation*/ bool> NotationNoteInput::noteInputStarted() const
{
    return m_noteInputStarted;
}

Notification NotationNoteInput::noteInputEnded() const
{
    return m_noteInputEnded;
}

bool NotationNoteInput::usingNoteInputMethod(NoteInputMethod method) const
{
    return score()->usingNoteEntryMethod(method);
}

void NotationNoteInput::setNoteInputMethod(NoteInputMethod method)
{
    TRACEFUNC;

    NoteInputState& is = score()->inputState();
    if (is.usingNoteEntryMethod(method)) {
        return;
    }

    if (!noteInputMethodAvailable(method, is.staff(), is.tick())) {
        return;
    }

    is.setNoteEntryMethod(method);
    if (shouldSetupInputNote()) {
        setupInputNote();
    }

    notifyAboutStateChanged();
}

void NotationNoteInput::addNote(const NoteInputParams& params, NoteAddingMode addingMode)
{
    TRACEFUNC;

    startEdit(TranslatableString("undoableAction", "Enter note"));

    bool addToUpOnCurrentChord = addingMode == NoteAddingMode::CurrentChord;
    bool insertNewChord = addingMode == NoteAddingMode::InsertChord;
    score()->cmdAddPitch(params, addToUpOnCurrentChord, insertNewChord);

    apply();

    if (shouldSetupInputNote()) {
        setupInputNote();
    }

    notifyNoteAddedChanged();
    notifyAboutStateChanged();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();

    m_interaction->showItem(state().cr());
}

void NotationNoteInput::padNote(const Pad& pad)
{
    TRACEFUNC;

    startEdit(TranslatableString("undoableAction", "Pad note"));
    score()->padToggle(pad);
    apply();

    if (pad >= Pad::NOTE00 && pad <= Pad::NOTE1024) {
        const NoteInputState& is = score()->inputState();
        if (!is.rest() && is.usingNoteEntryMethod(NoteInputMethod::BY_DURATION)) {
            score()->toggleAccidental(AccidentalType::NONE);
        }
    }

    notifyAboutStateChanged();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();
}

Ret NotationNoteInput::putNote(const PointF& pos, bool replace, bool insert)
{
    TRACEFUNC;

    startEdit(TranslatableString("undoableAction", "Enter note"));
    Ret ret = score()->putNote(pos, replace, insert);
    apply();

    if (ret) {
        if (shouldSetupInputNote()) {
            setupInputNote();
        }
    }

    notifyNoteAddedChanged();
    notifyAboutStateChanged();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();

    return ret;
}

void NotationNoteInput::removeNote(const PointF& pos)
{
    TRACEFUNC;

    mu::engraving::InputState& inputState = score()->inputState();
    bool restMode = inputState.rest();

    startEdit(TranslatableString("undoableAction", "Delete note"));
    inputState.setRest(!restMode);
    score()->putNote(pos, false, false);
    inputState.setRest(restMode);
    apply();

    notifyAboutStateChanged();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();
}

void NotationNoteInput::setInputNote(const NoteInputParams& params)
{
    TRACEFUNC;

    NoteInputState& is = score()->inputState();
    IF_ASSERT_FAILED(is.isValid()) {
        return;
    }
    const Staff* staff = is.staff();
    const Fraction tick = is.tick();

    NoteVal nval;

    if (staff->isDrumStaff(tick)) {
        const Drumset* drumset = is.drumset();
        if (drumset && drumset->isValid(params.drumPitch)) {
            nval.pitch = params.drumPitch;
            nval.headGroup = drumset->noteHead(params.drumPitch);
        }
    } else {
        nval = noteValForLine(mu::engraving::relStep(params.step, staff->clef(tick)));
    }

    if (nval.pitch > 0) {
        setInputNotes({ nval });
    }
}

void NotationNoteInput::setInputNotes(const NoteValList& notes)
{
    TRACEFUNC;

    NoteInputState& is = score()->inputState();

    if (is.notes() == notes) {
        return;
    }

    if (!notes.empty()) {
        if (const Drumset* drumset = is.drumset()) {
            const int pitch = notes.front().pitch;
            is.setDrumNote(pitch);
            is.setVoice(drumset->voice(pitch));
        }
    }

    is.setNotes(notes);
    notifyAboutStateChanged();
}

void NotationNoteInput::moveInputNotes(bool up, PitchMode mode)
{
    TRACEFUNC;

    mu::engraving::InputState& is = score()->inputState();
    IF_ASSERT_FAILED(is.isValid()) {
        return;
    }

    const Staff* staff = is.staff();
    const Fraction tick = is.tick();

    NoteValList notes;

    for (const NoteVal& val : is.notes()) {
        NoteVal newVal;

        if (staff->isDrumStaff(tick)) {
            if (const Drumset* drumset = is.drumset()) {
                newVal.pitch = up ? drumset->nextPitch(val.pitch) : drumset->prevPitch(val.pitch);

                if (drumset->isValid(newVal.pitch)) {
                    newVal.headGroup = drumset->noteHead(newVal.pitch);
                    notes.push_back(newVal);
                }
            }
            continue;
        }

        switch (mode) {
        case PitchMode::CHROMATIC:
            newVal.pitch = val.pitch + (up ? 1 : -1);
            break;
        case PitchMode::DIATONIC: {
            const int oldLine = mu::engraving::noteValToLine(val, is.staff(), is.tick());
            const int newLine = oldLine + (up ? -1 : 1);
            newVal = noteValForLine(newLine);
        } break;
        case PitchMode::OCTAVE:
            newVal = val;
            newVal.pitch += up ? mu::engraving::PITCH_DELTA_OCTAVE : -mu::engraving::PITCH_DELTA_OCTAVE;
            break;
        }

        newVal.pitch = std::clamp(newVal.pitch, 0, 127);
        notes.push_back(newVal);
    }

    setInputNotes(notes);
}

void NotationNoteInput::setAccidental(AccidentalType accidentalType)
{
    TRACEFUNC;

    score()->toggleAccidental(accidentalType);

    notifyAboutStateChanged();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();
}

void NotationNoteInput::setArticulation(SymbolId articulationSymbolId)
{
    TRACEFUNC;

    mu::engraving::InputState& inputState = score()->inputState();

    std::set<SymbolId> articulations = mu::engraving::updateArticulations(
        inputState.articulationIds(), articulationSymbolId, mu::engraving::ArticulationsUpdateMode::Remove);
    inputState.setArticulationIds(articulations);

    notifyAboutStateChanged();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();
}

void NotationNoteInput::setDrumNote(int note)
{
    TRACEFUNC;

    mu::engraving::InputState& is = score()->inputState();
    if (is.drumNote() == note) {
        return;
    }

    is.setDrumNote(note);
    if (shouldSetupInputNote()) {
        setupInputNote();
    }

    notifyAboutStateChanged();
}

void NotationNoteInput::setCurrentVoice(voice_idx_t voiceIndex)
{
    TRACEFUNC;

    if (!isVoiceIndexValid(voiceIndex)) {
        return;
    }

    mu::engraving::InputState& inputState = score()->inputState();

    // TODO: Inserting notes to a new voice in the middle of a tuplet is not yet supported. In this case
    // we'll move the input to the start of the tuplet...
    if (const Segment* prevSeg = inputState.segment()) {
        const ChordRest* prevCr = prevSeg->cr(inputState.track());
        //! NOTE: if there's an existing ChordRest at the new voiceIndex, we don't need to move the cursor
        if (prevCr && prevCr->topTuplet() && !prevSeg->cr(voiceIndex)) {
            Segment* newSeg = score()->tick2segment(prevCr->topTuplet()->tick());
            if (newSeg) {
                inputState.setSegment(newSeg);
            }
        }
    }

    inputState.setVoice(voiceIndex);
    notifyAboutStateChanged();
}

void NotationNoteInput::setCurrentTrack(track_idx_t trackIndex)
{
    TRACEFUNC;

    score()->inputState().setTrack(trackIndex);
    notifyAboutStateChanged();
}

void NotationNoteInput::addTuplet(const TupletOptions& options)
{
    TRACEFUNC;

    const mu::engraving::InputState& inputState = score()->inputState();

    startEdit(TranslatableString("undoableAction", "Add tuplet"));
    score()->expandVoice();
    mu::engraving::ChordRest* chordRest = inputState.cr();
    if (chordRest) {
        Fraction ratio = options.ratio;
        if (options.autoBaseLen) {
            ratio.setDenominator(mu::engraving::Tuplet::computeTupletDenominator(ratio.numerator(), inputState.ticks()));
        }
        score()->changeCRlen(chordRest, inputState.duration());
        score()->addTuplet(chordRest, ratio, options.numberType, options.bracketType);
    }
    apply();

    notifyAboutStateChanged();
}

muse::RectF NotationNoteInput::cursorRect() const
{
    TRACEFUNC;

    if (!isNoteInputMode()) {
        return {};
    }

    const mu::engraving::InputState& inputState = score()->inputState();
    const mu::engraving::Segment* segment = inputState.segment();
    if (!segment) {
        return {};
    }

    mu::engraving::System* system = segment->measure()->system();
    if (!system) {
        return {};
    }

    mu::engraving::track_idx_t track = inputState.track() == muse::nidx ? 0 : inputState.track();
    mu::engraving::staff_idx_t staffIdx = track / mu::engraving::VOICES;

    const Staff* staff = score()->staff(staffIdx);
    if (!staff) {
        return {};
    }

    constexpr int sideMargin = 4;
    constexpr int skylineMargin = 20;

    RectF segmentContentRect = segment->contentRect();
    double x = segmentContentRect.translated(segment->pagePos()).x() - sideMargin;
    double y = system->staffYpage(staffIdx) + system->page()->pos().y();
    double w = segmentContentRect.width() + 2 * sideMargin;
    double h = 0.0;

    const mu::engraving::StaffType* staffType = staff->staffType(inputState.tick());
    double spatium = score()->style().spatium();
    double lineDist = staffType->lineDistance().val() * spatium;
    int lines = staffType->lines();
    double yOffset = staffType ? staffType->yoffset().val() * spatium : 0.0;
    int inputStateStringsCount = inputState.string();

    y += yOffset;

    int instrumentStringsCount = static_cast<int>(staff->part()->instrument()->stringData()->strings());
    if (staff->isTabStaff(inputState.tick()) && inputStateStringsCount >= 0 && inputStateStringsCount <= instrumentStringsCount) {
        h = lineDist;
        y += staffType->physStringToYOffset(inputStateStringsCount) * spatium;
        y -= (staffType->onLines() ? lineDist * 0.5 : lineDist);
    } else {
        h = (lines - 1) * lineDist + 2 * skylineMargin;
        y -= skylineMargin;
    }

    RectF result = RectF(x, y, w, h);

    if (configuration()->canvasOrientation().val == muse::Orientation::Horizontal) {
        result.translate(system->page()->pos());
    }

    return result;
}

void NotationNoteInput::addSlur(mu::engraving::Slur* slur)
{
    TRACEFUNC;

    mu::engraving::InputState& inputState = score()->inputState();
    inputState.setSlur(slur);

    if (slur) {
        std::vector<mu::engraving::SpannerSegment*> slurSpannerSegments = slur->spannerSegments();
        if (!slurSpannerSegments.empty()) {
            slurSpannerSegments.front()->setSelected(true);
        }
    }

    notifyAboutStateChanged();
}

void NotationNoteInput::resetSlur()
{
    TRACEFUNC;

    mu::engraving::InputState& inputState = score()->inputState();
    mu::engraving::Slur* slur = inputState.slur();
    if (!slur) {
        return;
    }

    score()->deselect(slur);

    addSlur(nullptr);
}

void NotationNoteInput::addTie()
{
    TRACEFUNC;

    // Calls `startEdit` internally
    score()->cmdAddTie();

    notifyAboutStateChanged();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();
}

void NotationNoteInput::addLaissezVib()
{
    TRACEFUNC;

    // Calls `startEdit` internally
    score()->cmdToggleLaissezVib();

    notifyAboutStateChanged();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();
}

Notification NotationNoteInput::noteAdded() const
{
    return m_noteAdded;
}

Notification NotationNoteInput::stateChanged() const
{
    return m_stateChanged;
}

void NotationNoteInput::setGetViewRectFunc(const std::function<RectF()>& func)
{
    m_getViewRectFunc = func;
}

mu::engraving::Score* NotationNoteInput::score() const
{
    return m_getScore->score();
}

void NotationNoteInput::startEdit(const muse::TranslatableString& actionName)
{
    m_undoStack->prepareChanges(actionName);
}

void NotationNoteInput::apply()
{
    m_undoStack->commitChanges();

    if (mu::engraving::ChordRest* chordRest = score()->inputState().cr()) {
        m_interaction->showItem(chordRest);
    }
}

void NotationNoteInput::updateInputState()
{
    TRACEFUNC;

    score()->inputState().update(score()->selection());

    notifyAboutStateChanged();
}

void NotationNoteInput::notifyAboutStateChanged()
{
    m_stateChanged.notify();
}

void NotationNoteInput::notifyNoteAddedChanged()
{
    m_noteAdded.notify();
}

void NotationNoteInput::notifyAboutNoteInputStarted(bool focusNotation)
{
    m_noteInputStarted.send(focusNotation);
}

void NotationNoteInput::notifyAboutNoteInputEnded()
{
    m_noteInputEnded.notify();
}

void NotationNoteInput::doubleNoteInputDuration()
{
    TRACEFUNC;

    startEdit(TranslatableString("undoableAction", "Double note input duration"));
    score()->cmdPadNoteIncreaseTAB();
    apply();

    notifyAboutStateChanged();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();
}

void NotationNoteInput::halveNoteInputDuration()
{
    TRACEFUNC;

    startEdit(TranslatableString("undoableAction", "Halve note input duration"));
    score()->cmdPadNoteDecreaseTAB();
    apply();

    notifyAboutStateChanged();

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();
}
