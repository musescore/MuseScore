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
#include "notationnoteinput.h"

#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/input.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/system.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/utils.h"

#include "engraving/editing/editnote.h"
#include "engraving/editing/edittie.h"
#include "engraving/editing/noteinput.h"
#include "engraving/editing/transaction/transaction.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::async;

TranslatableString nameOfNoteInputMethod(NoteInputMethod method)
{
    switch (method) {
    case NoteInputMethod::UNKNOWN:          break;
    case NoteInputMethod::BY_NOTE_NAME:     return TranslatableString("noteInputMethod", "Input by note name mode");
    case NoteInputMethod::BY_DURATION:      return TranslatableString("noteInputMethod", "Input by duration mode");
    case NoteInputMethod::REPITCH:          return TranslatableString("noteInputMethod", "Re-pitch existing notes mode");
    case NoteInputMethod::RHYTHM:           return TranslatableString("noteInputMethod", "Rhythm-only input mode");
    case NoteInputMethod::REALTIME_AUTO:    return TranslatableString("noteInputMethod", "Metronome real-time input mode");
    case NoteInputMethod::REALTIME_MANUAL:  return TranslatableString("noteInputMethod", "Pedal real-time input mode");
    case NoteInputMethod::TIMEWISE:         return TranslatableString("noteInputMethod", "Insert mode (grow measures)");
        // No default case. We want a compiler warning if an enum value is not handled here.
    }

    UNREACHABLE;
    return TranslatableString("noteInputMethod", "Unknown note input mode");
}

static bool noteInputMethodAvailable(NoteInputMethod method, const Staff* staff, const Fraction& tick)
{
    if (method == NoteInputMethod::BY_DURATION) {
        return staff && !staff->isTabStaff(tick);
    }

    return true;
}

NotationNoteInput::NotationNoteInput(const IGetScore* getScore, INotationInteraction* interaction, INotationUndoStackPtr undoStack
                                     , const modularity::ContextPtr& iocCtx)
    : muse::Contextable(iocCtx), m_getScore(getScore), m_interaction(interaction), m_undoStack(undoStack)
{
    m_interaction->selectionChanged().onNotify(this, [this]() {
        if (!isNoteInputMode()) {
            updateInputState();
        } else if (shouldSetupInputNote()) {
            const NoteInputState& is = state();
            const staff_idx_t prevStaffIdx = track2staff(is.prevTrack());

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

    if (Part* part = el->part(); part && part->isSharedPart()) {
        return;
    }

    m_interaction->select({ el }, SelectType::SINGLE, 0);

    InputState& is = score()->inputState();

    Duration d(is.duration());
    if (!d.isValid() || d.isZero() || d.type() == DurationType::V_MEASURE) {
        is.setDuration(Duration(DurationType::V_QUARTER));
    }
    is.setAccidentalType(AccidentalType::NONE);

    is.setRest(false);
    is.setNoteEntryMode(true);

    const Staff* staff = score()->staff(is.track() / VOICES);

    if (noteInputMethodAvailable(method, staff, is.tick())) {
        is.setNoteEntryMethod(method);
    } else {
        is.setNoteEntryMethod(NoteInputMethod::BY_NOTE_NAME); // fallback
    }

    if (shouldSetupInputNote()) {
        setupInputNote();
    }

    switch (staff->staffType(is.tick())->group()) {
    case StaffGroup::STANDARD:
        break;
    case StaffGroup::TAB: {
        int strg = 0;                           // assume topmost string as current string
        // if entering note entry with a note selected and the note has a string
        // set InputState::_string to note physical string
        if (el->isNote()) {
            strg = (toNote(el))->string();
        }
        is.setString(strg);
        break;
    }
    case StaffGroup::PERCUSSION:
        break;
    }

    notifyAboutNoteInputStarted(focusNotation);
    notifyAboutStateChanged();

    m_interaction->showItem(el);
    accessibilityController()->announce(nameOfNoteInputMethod(method).translated());
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
                track_idx_t track = trackZeroVoice(lastSelected->track());
                const Segment* s = lastSelected->segment();
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
                        cr = toChordRest(e->parent());
                        if (!cr) {
                            continue;
                        }
                    } else {
                        cr = toChordRest(e);
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

    const InputState& is = score()->inputState();

    if (!el) {
        if (const Segment* segment = is.lastSegment()) {
            el = segment->element(is.track());
        }
    }

    if (!el || (!el->isChordRest() && !el->isNote())) {
        // if no note/rest is selected, start with voice 0
        engraving::track_idx_t track = is.track() == muse::nidx ? 0 : (is.track() / VOICES) * VOICES;
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

    if (el->isChord()) {
        Chord* c = toChord(el);
        Note* note = c->selectedNote();
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
    InputState& is = score()->inputState();
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
    const InputState& is = score()->inputState();

    Position pos;
    pos.segment = is.segment();
    pos.staffIdx = is.staffIdx();
    pos.line = line;

    bool error = false;
    const NoteVal nval = NoteInput::noteValForPosition(score(), pos, is.accidentalType(), error);
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

    InputState& is = score()->inputState();
    is.setNoteEntryMode(false);

    if (is.slur()) {
        const std::vector<SpannerSegment*>& el = is.slur()->spannerSegments();
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
    accessibilityController()->announce(TranslatableString("noteInputMethod", "Normal mode").translated());
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

    m_interaction->hideShadowNote();

    bool addToUpOnCurrentChord = addingMode == NoteAddingMode::CurrentChord;
    bool insertNewChord = addingMode == NoteAddingMode::InsertChord;

    if (addToUpOnCurrentChord) {
        startEdit(TranslatableString("undoableAction", "Add note to chord"));
    } else if (insertNewChord) {
        startEdit(TranslatableString("undoableAction", "Insert note"));
    } else {
        startEdit(TranslatableString("undoableAction", "Enter note"));
    }
    NoteInput::addPitch(score()->transactionManager()->currentOrDummyTransaction(), score(), params, addToUpOnCurrentChord,
                        insertNewChord);

    apply();

    if (shouldSetupInputNote()) {
        setupInputNote();
    }

    notifyNoteAddedChanged();
    notifyAboutStateChanged();

    m_interaction->checkAndShowError();
    m_interaction->showItem(state().cr());
}

void NotationNoteInput::applyNoteValueChange(const muse::TranslatableString& actionName,
                                             const std::function<void(mu::engraving::Transaction&)>& change)
{
    TRACEFUNC;

    m_interaction->hideShadowNote();

    startEdit(actionName);
    change(score()->transactionManager()->currentOrDummyTransaction());
    apply();

    notifyAboutStateChanged();

    m_interaction->checkAndShowError();
}

void NotationNoteInput::setDuration(DurationType duration)
{
    applyNoteValueChange(muse::TranslatableString("undoableAction", "Set duration"), [this, duration](mu::engraving::Transaction& tx) {
        NoteInput::setDuration(tx, score(), duration);
    });
}

void NotationNoteInput::toggleRest()
{
    applyNoteValueChange(muse::TranslatableString("undoableAction", "Toggle rest"), [this](mu::engraving::Transaction& tx) {
        NoteInput::toggleRest(tx, score());
    });
}

void NotationNoteInput::toggleDots(int dots)
{
    applyNoteValueChange(muse::TranslatableString("undoableAction", "Toggle dots"), [this, dots](mu::engraving::Transaction& tx) {
        NoteInput::toggleDots(tx, score(), dots);
    });
}

Ret NotationNoteInput::putNote(const PointF& pos, bool replace, bool insert)
{
    TRACEFUNC;

    startEdit(TranslatableString("undoableAction", "Enter note"));
    Ret ret = NoteInput::putNote(score()->transactionManager()->currentOrDummyTransaction(), score(), pos, replace, insert);
    apply();

    if (ret) {
        if (shouldSetupInputNote()) {
            setupInputNote();
        }
    }

    notifyNoteAddedChanged();
    notifyAboutStateChanged();

    m_interaction->checkAndShowError();

    return ret;
}

void NotationNoteInput::removeNote(const PointF& pos)
{
    TRACEFUNC;

    InputState& inputState = score()->inputState();
    bool restMode = inputState.rest();

    startEdit(TranslatableString("undoableAction", "Delete note"));
    inputState.setRest(!restMode);
    NoteInput::putNote(score()->transactionManager()->currentOrDummyTransaction(), score(), pos, false, false);
    inputState.setRest(restMode);
    apply();

    notifyAboutStateChanged();

    m_interaction->checkAndShowError();
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
        nval = noteValForLine(relStep(params.step, staff->clef(tick)));
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

    InputState& is = score()->inputState();
    IF_ASSERT_FAILED(is.isValid()) {
        return;
    }

    is.setAccidentalType(AccidentalType::NONE);

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
            const int oldLine = noteValToLine(val, staff, tick);
            const int newLine = oldLine + (up ? -1 : 1);
            newVal = noteValForLine(newLine);
        } break;
        case PitchMode::OCTAVE:
            newVal = val;
            newVal.pitch += up ? PITCH_DELTA_OCTAVE : -PITCH_DELTA_OCTAVE;
            break;
        }

        newVal.pitch = clampPitch(newVal.pitch);
        notes.push_back(newVal);
    }

    setInputNotes(notes);
}

void NotationNoteInput::setRestMode(bool rest)
{
    InputState& is = score()->inputState();
    if (is.rest() == rest) {
        return;
    }

    is.setRest(rest);
    notifyAboutStateChanged();
}

void NotationNoteInput::setAccidental(AccidentalType accidentalType)
{
    TRACEFUNC;

    EditNote::toggleAccidental(score(), accidentalType);

    notifyAboutStateChanged();

    m_interaction->checkAndShowError();
}

void NotationNoteInput::setArticulation(SymbolId articulationSymbolId)
{
    TRACEFUNC;

    InputState& inputState = score()->inputState();

    std::set<SymbolId> articulations = updateArticulations(
        inputState.articulationIds(), articulationSymbolId, ArticulationsUpdateMode::Remove);
    inputState.setArticulationIds(articulations);

    notifyAboutStateChanged();

    m_interaction->checkAndShowError();
}

void NotationNoteInput::setDrumNote(int note)
{
    TRACEFUNC;

    InputState& is = score()->inputState();
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

    InputState& inputState = score()->inputState();
    if (!isVoiceIndexValid(voiceIndex) || voiceIndex == inputState.voice()) {
        return;
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

    const InputState& inputState = score()->inputState();

    startEdit(TranslatableString("undoableAction", "Add tuplet"));
    score()->expandVoice();
    ChordRest* chordRest = inputState.cr();
    if (chordRest) {
        Fraction ratio = options.ratio;
        if (options.autoBaseLen) {
            ratio.setDenominator(Tuplet::computeTupletDenominator(ratio.numerator(), inputState.ticks()));
        }
        score()->changeCRlen(chordRest, inputState.duration());
        score()->addTuplet(chordRest, ratio, options.numberType, options.bracketType);
    }
    apply();

    notifyAboutStateChanged();
}

static muse::RectF segmentContentRect(const Segment* segment, track_idx_t track)
{
    RectF result;

    const EngravingItem* el = segment->element(track);
    if (!el) {
        return result;
    }

    if (el->isChord()) {
        const Chord* chord = toChord(el);
        for (const Note* note: chord->notes()) {
            result.unite(note->ldata()->bbox().translated(note->pos()));
        }

        if (Hook* hook = chord->hook()) {
            result.unite(hook->ldata()->bbox().translated(hook->pos()));
        }
    } else if (el->isRest() && !toRest(el)->isFullMeasureRest()) {
        result.unite(el->ldata()->bbox());
    }

    return result;
}

muse::RectF NotationNoteInput::cursorRect() const
{
    TRACEFUNC;

    if (!isNoteInputMode()) {
        return {};
    }

    const InputState& inputState = score()->inputState();
    const Segment* segment = inputState.segment();
    if (!segment) {
        return {};
    }

    const System* system = segment->measure()->system();
    if (!system) {
        return {};
    }

    const track_idx_t track = inputState.track() == muse::nidx ? 0 : inputState.track();
    const staff_idx_t staffIdx = track2staff(track);

    const Staff* staff = score()->staff(staffIdx);
    if (!staff) {
        return {};
    }

    const double globalSpatium = score()->style().spatium();

    const StaffType* staffType = staff->staffType(inputState.tick());
    const bool isTabStaff = staffType->isTabStaff();
    const double localSpatium = staffType->spatium();

    const double defaultWidth = score()->noteHeadWidth() * localSpatium / globalSpatium;

    const RectF segmentContentRect = ::segmentContentRect(segment, track);
    double x = segmentContentRect.x() + segment->pagePos().x();

    const double barNoteDist = score()->style().styleAbsolute(Sid::barNoteDistance);
    if (segment->x() < barNoteDist) {
        x += barNoteDist - segment->x();
    }

    double y = system->staffCanvasYpage(staffIdx);
    double w = segmentContentRect.width() > 0 ? segmentContentRect.width() : defaultWidth;
    double h = 0.0;

    if (inputState.beyondScore()) {
        const Measure* lastMeasure = score()->lastMeasure();
        x = lastMeasure->pageBoundingRect().right() + globalSpatium;
        w = defaultWidth;
    }

    double sideMargin = 0.0;

    if (isTabStaff) {
        const double minWidth = 2.0 * globalSpatium;
        sideMargin = std::max(0.3 * globalSpatium, 0.5 * (minWidth - w));
    } else {
        sideMargin = 0.5 * globalSpatium;
    }

    // Don't extend further to the left than the center between the current and previous segment
    const engraving::Segment* prevSeg = segment->prev1WithElemsOnTrack(track, engraving::SegmentType::ChordRest);
    if (prevSeg && prevSeg->measure() == segment->measure()) {
        const RectF prevSegContentRect = ::segmentContentRect(prevSeg, track);
        if (prevSegContentRect.width() > 0) {
            const double centerBetweenPrevSegRightAndCurrSegLeft = (prevSeg->pagePos().x() + prevSegContentRect.right() + x) * 0.5;
            sideMargin = std::min(sideMargin, x - centerBetweenPrevSegRightAndCurrSegLeft);
        }
    }

    x -= sideMargin;
    w += 2 * sideMargin;

    const double yOffset = staffType->yoffset().val() * localSpatium;
    y += yOffset;

    const int inputStateStringsCount = inputState.string();
    const int instrumentStringsCount = static_cast<int>(staff->part()->instrument()->stringData()->strings());

    const double lineDist = staffType->lineDistance().val() * localSpatium;

    if (isTabStaff && inputStateStringsCount >= 0 && inputStateStringsCount <= instrumentStringsCount) {
        h = lineDist;
        y += staffType->physStringToYOffset(inputStateStringsCount).toAbsolute(localSpatium);
        y -= (staffType->onLines() ? lineDist * 0.5 : lineDist);
    } else {
        const double skylineMargin = 0.75 * localSpatium;
        const int lines = staffType->lines();

        h = (lines - 1) * lineDist + 2 * skylineMargin;
        y -= skylineMargin;
    }

    RectF result { x, y, w, h };

    if (configuration()->canvasOrientation().val == muse::Orientation::Horizontal) {
        result.translate(system->page()->pos());
    }

    return result;
}

void NotationNoteInput::addSlur(Slur* slur)
{
    TRACEFUNC;

    InputState& inputState = score()->inputState();
    inputState.setSlur(slur);

    if (slur) {
        std::vector<SpannerSegment*> slurSpannerSegments = slur->spannerSegments();
        if (!slurSpannerSegments.empty()) {
            slurSpannerSegments.front()->setSelected(true);
        }
    }

    notifyAboutStateChanged();
}

void NotationNoteInput::resetSlur()
{
    TRACEFUNC;

    InputState& inputState = score()->inputState();
    Slur* slur = inputState.slur();
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
    EditTie::cmdAddTie(score());

    notifyAboutStateChanged();
    m_interaction->checkAndShowError();
}

void NotationNoteInput::addLaissezVib()
{
    TRACEFUNC;

    // Calls `startEdit` internally
    EditTie::cmdToggleLaissezVib(score());

    notifyAboutStateChanged();
    m_interaction->checkAndShowError();
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

Score* NotationNoteInput::score() const
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

    if (ChordRest* chordRest = score()->inputState().cr()) {
        m_interaction->showItem(chordRest);
    }
}

void NotationNoteInput::updateInputState()
{
    TRACEFUNC;

    NoteInputState& is = score()->inputState();
    is.update(score()->selection());

    if (!configuration()->addAccidentalDotsArticulationsToNextNoteEntered()) {
        is.setAccidentalType(AccidentalType::NONE);
        is.setDots(0);
        is.setArticulationIds({});
    }

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
    NoteInput::increaseDuration(score()->transactionManager()->currentOrDummyTransaction(), score());
    apply();

    notifyAboutStateChanged();
    m_interaction->checkAndShowError();
}

void NotationNoteInput::halveNoteInputDuration()
{
    TRACEFUNC;

    startEdit(TranslatableString("undoableAction", "Halve note input duration"));
    NoteInput::decreaseDuration(score()->transactionManager()->currentOrDummyTransaction(), score());
    apply();

    notifyAboutStateChanged();
    m_interaction->checkAndShowError();
}
