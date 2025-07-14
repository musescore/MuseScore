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
#include "notationmidiinput.h"

#include <QGuiApplication>

#include "engraving/dom/masterscore.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/score.h"
#include "engraving/dom/note.h"
#include "engraving/dom/factory.h"

#include "notationtypes.h"

#include "defer.h"
#include "log.h"

using namespace mu::notation;

static constexpr int PROCESS_INTERVAL = 20;

NotationMidiInput::NotationMidiInput(IGetScore* getScore, INotationInteractionPtr notationInteraction,
                                     INotationUndoStackPtr undoStack, const muse::modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx), m_getScore(getScore),
    m_notationInteraction(notationInteraction), m_undoStack(undoStack)
{
    QObject::connect(&m_processTimer, &QTimer::timeout, [this]() { doProcessEvents(); });

    m_realtimeTimer.setTimerType(Qt::PreciseTimer);
    QObject::connect(&m_realtimeTimer, &QTimer::timeout, [this]() { doRealtimeAdvance(); });

    m_extendNoteTimer.setTimerType(Qt::PreciseTimer);
    m_extendNoteTimer.setSingleShot(true);
    QObject::connect(&m_extendNoteTimer, &QTimer::timeout, [this]() { doExtendCurrentNote(); });
}

void NotationMidiInput::onMidiEventReceived(const muse::midi::Event& event)
{
    if (event.isChannelVoice20()) {
        auto events = event.toMIDI10();
        for (auto& midi10event : events) {
            onMidiEventReceived(midi10event);
        }

        return;
    }

    if (event.opcode() == muse::midi::Event::Opcode::NoteOn || event.opcode() == muse::midi::Event::Opcode::NoteOff) {
        m_eventsQueue.push_back(event);

        if (!m_processTimer.isActive()) {
            m_processTimer.start(PROCESS_INTERVAL);
        }
    }
}

muse::async::Channel<std::vector<const Note*> > NotationMidiInput::notesReceived() const
{
    return m_notesReceivedChannel;
}

void NotationMidiInput::onRealtimeAdvance()
{
    if (!isNoteInputMode()) {
        return;
    }

    if (isRealtimeManual()) {
        m_allowRealtimeRests = true;
        enableMetronome();
        doRealtimeAdvance();
    } else if (isRealtimeAuto()) {
        if (m_realtimeTimer.isActive()) {
            stopRealtime();
            disableMetronome();
        } else {
            m_allowRealtimeRests = true;
            enableMetronome();
            runRealtime();
        }
    }
}

mu::engraving::Score* NotationMidiInput::score() const
{
    IF_ASSERT_FAILED(m_getScore) {
        return nullptr;
    }

    return m_getScore->score();
}

void NotationMidiInput::doProcessEvents()
{
    DEFER {
        m_eventsQueue.clear();
        m_processTimer.stop();
    };

    if (m_eventsQueue.empty()) {
        return;
    }

    const mu::engraving::Score* sc = score();
    if (!sc || sc->noStaves()) {
        return;
    }

    std::vector<const Note*> notes;

    startNoteInputIfNeed();
    bool isNoteInput = isNoteInputMode();

    if (isNoteInput && isInputByDuration()) {
        addNoteEventsToInputState();
        return;
    }

    for (size_t i = 0; i < m_eventsQueue.size(); ++i) {
        const muse::midi::Event& event = m_eventsQueue.at(i);
        Note* note = isNoteInput ? addNoteToScore(event) : makePreviewNote(event);
        if (note) {
            notes.push_back(note);
        }

        bool chord = i != 0;
        bool noteOn = event.opcode() == muse::midi::Event::Opcode::NoteOn;
        if (!chord && noteOn && !m_realtimeTimer.isActive() && isRealtimeAuto()) {
            m_extendNoteTimer.start(configuration()->delayBetweenNotesInRealTimeModeMilliseconds());
            enableMetronome();
            doRealtimeAdvance();
        }
    }

    if (!notes.empty()) {
        std::vector<const EngravingItem*> notesItems;
        for (const Note* note : notes) {
            notesItems.push_back(note);
        }

        playbackController()->playElements(notesItems, true);
        m_notesReceivedChannel.send(notes);
    }
}

void NotationMidiInput::startNoteInputIfNeed()
{
    if (isNoteInputMode()) {
        return;
    }

    if (configuration()->startNoteInputAtSelectionWhenPressingMidiKey()) {
        if (m_notationInteraction->selection()->elementsSelected(NOTE_REST_TYPES)) {
            dispatcher()->dispatch("note-input");
        }
    }
}

void NotationMidiInput::addNoteEventsToInputState()
{
    INotationNoteInputPtr noteInput = m_notationInteraction->noteInput();
    const NoteInputState& state = noteInput->state();
    const staff_idx_t staffIdx = state.staffIdx();
    const bool useWrittenPitch = configuration()->midiUseWrittenPitch().val;

    NoteValList notes;
    if (m_holdingNotes) {
        notes = state.notes();
    }

    for (const muse::midi::Event& event : m_eventsQueue) {
        if (event.opcode() == muse::midi::Event::Opcode::NoteOn) {
            notes.push_back(score()->noteVal(event.note(), staffIdx, useWrittenPitch));
            m_holdingNotes = true;
        } else if (event.opcode() == muse::midi::Event::Opcode::NoteOff) {
            m_holdingNotes = false;
        }
    }

    if (!notes.empty()) {
        noteInput->setRestMode(false);

        if (!m_holdingNotes && notes == state.notes()) {
            return;
        }

        noteInput->setInputNotes(notes);

        if (configuration()->isPlayPreviewNotesInInputByDuration()) {
            playbackController()->playNotes(notes, staffIdx, state.segment());
        }
    }
}

Note* NotationMidiInput::addNoteToScore(const muse::midi::Event& e)
{
    mu::engraving::Score* sc = score();

    mu::engraving::MidiInputEvent inputEv;
    inputEv.pitch = e.note();
    inputEv.velocity = e.velocity7();

    sc->activeMidiPitches().remove_if([&inputEv](const mu::engraving::MidiInputEvent& val) {
        return inputEv.pitch == val.pitch;
    });

    const mu::engraving::InputState& is = sc->inputState();
    if (!is.noteEntryMode()) {
        return nullptr;
    }

    if (!is.isValid()) {
        return nullptr;
    }

    DEFER {
        m_undoStack->commitChanges();
    };

    m_undoStack->prepareChanges(muse::TranslatableString("undoableAction", "Enter note"));

    if (e.opcode() == muse::midi::Event::Opcode::NoteOff) {
        if (isRealtime()) {
            if (!is.cr()) {
                return nullptr;
            }

            const Chord* chord = is.cr()->isChord() ? engraving::toChord(is.cr()) : nullptr;
            if (chord) {
                Note* n = chord->findNote(inputEv.pitch);
                if (n) {
                    sc->deleteItem(n->tieBack());
                    sc->deleteItem(n);
                }
            }
        }

        return nullptr;
    }

    if (sc->activeMidiPitches().empty()) {
        inputEv.chord = false;
    } else {
        inputEv.chord = true;
    }

    // holding shift while inputting midi will add the new pitch to the prior existing chord
    if (QGuiApplication::queryKeyboardModifiers() & Qt::ShiftModifier) {
        mu::engraving::EngravingItem* cr = is.lastSegment()->element(is.track());
        if (cr && cr->isChord()) {
            inputEv.chord = true;
        }
    }

    mu::engraving::Note* note = sc->addMidiPitch(inputEv.pitch, inputEv.chord, configuration()->midiUseWrittenPitch().val);

    sc->activeMidiPitches().push_back(inputEv);

    if (is.cr()) {
        m_notationInteraction->showItem(is.cr());
    }

    return note;
}

Note* NotationMidiInput::makePreviewNote(const muse::midi::Event& e)
{
    if (e.opcode() == muse::midi::Event::Opcode::NoteOff || e.velocity7() == 0) {
        return nullptr;
    }

    mu::engraving::Score* score = this->score();
    const mu::engraving::InputState& inputState = score->inputState();
    Segment* seg = inputState.lastSegment() ? inputState.lastSegment() : score->dummy()->segment();

    Chord* chord = engraving::Factory::createChord(seg);
    chord->setParent(seg);

    const ChordRest* cr = inputState.cr();
    const mu::engraving::staff_idx_t staffIdx = cr ? engraving::track2staff(cr->track()) : 0;

    Note* note = engraving::Factory::createNote(chord);
    note->setParent(chord);
    note->setStaffIdx(staffIdx);

    engraving::NoteVal nval = score->noteVal(e.note(), staffIdx, configuration()->midiUseWrittenPitch().val);
    note->setNval(nval);

    return note;
}

void NotationMidiInput::enableMetronome()
{
    bool metronomeEnabled = configuration()->isMetronomeEnabled();
    if (metronomeEnabled) {
        return;
    }

    dispatcher()->dispatch("metronome");
    m_shouldDisableMetronome = true;
}

void NotationMidiInput::disableMetronome()
{
    if (!m_shouldDisableMetronome) {
        return;
    }

    dispatcher()->dispatch("metronome");

    m_shouldDisableMetronome = false;
}

void NotationMidiInput::runRealtime()
{
    m_realtimeTimer.start(configuration()->delayBetweenNotesInRealTimeModeMilliseconds());
}

void NotationMidiInput::stopRealtime()
{
    m_realtimeTimer.stop();
}

void NotationMidiInput::doRealtimeAdvance()
{
    if (!isRealtime() || !isNoteInputMode() || (!m_allowRealtimeRests && m_getScore->score()->activeMidiPitches().empty())) {
        if (m_realtimeTimer.isActive()) {
            stopRealtime();
        }

        disableMetronome();

        m_allowRealtimeRests = true;
        return;
    }

    const mu::engraving::InputState& is = m_getScore->score()->inputState();
    playbackController()->playMetronome(is.tick().ticks());

    QTimer::singleShot(100, Qt::PreciseTimer, [this]() {
        m_undoStack->prepareChanges(muse::TranslatableString("undoableAction", "Realtime advance"));
        m_getScore->score()->realtimeAdvance(configuration()->midiUseWrittenPitch().val);
        m_undoStack->commitChanges();
    });

    if (isRealtimeManual()) {
        int metronomeDuration = 500;
        QTimer::singleShot(metronomeDuration, Qt::PreciseTimer, [this]() {
            disableMetronome();
        });
    }
}

void NotationMidiInput::doExtendCurrentNote()
{
    if (!isNoteInputMode() || m_realtimeTimer.isActive()) {
        return;
    }

    m_allowRealtimeRests = false;
    runRealtime();
    doRealtimeAdvance();
}

NoteInputMethod NotationMidiInput::noteInputMethod() const
{
    return m_notationInteraction->noteInput()->state().noteEntryMethod();
}

bool NotationMidiInput::isRealtime() const
{
    return isRealtimeAuto() || isRealtimeManual();
}

bool NotationMidiInput::isRealtimeAuto() const
{
    return noteInputMethod() == NoteInputMethod::REALTIME_AUTO;
}

bool NotationMidiInput::isRealtimeManual() const
{
    return noteInputMethod() == NoteInputMethod::REALTIME_MANUAL;
}

bool NotationMidiInput::isInputByDuration() const
{
    return noteInputMethod() == NoteInputMethod::BY_DURATION;
}

bool NotationMidiInput::isNoteInputMode() const
{
    return m_notationInteraction->noteInput()->isNoteInputMode();
}
