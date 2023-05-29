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
#include "notationmidiinput.h"

#include <QGuiApplication>

#include "libmscore/masterscore.h"
#include "libmscore/segment.h"
#include "libmscore/tie.h"
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/factory.h"

#include "notationtypes.h"

#include "defer.h"
#include "log.h"

using namespace mu::notation;

static constexpr int PROCESS_INTERVAL = 20;

NotationMidiInput::NotationMidiInput(IGetScore* getScore, INotationInteractionPtr notationInteraction, INotationUndoStackPtr undoStack)
    : m_getScore(getScore), m_notationInteraction(notationInteraction), m_undoStack(undoStack)
{
    QObject::connect(&m_processTimer, &QTimer::timeout, [this]() { doProcessEvents(); });

    m_realtimeTimer.setTimerType(Qt::PreciseTimer);
    QObject::connect(&m_realtimeTimer, &QTimer::timeout, [this]() { doRealtimeAdvance(); });

    m_extendNoteTimer.setTimerType(Qt::PreciseTimer);
    m_extendNoteTimer.setSingleShot(true);
    QObject::connect(&m_extendNoteTimer, &QTimer::timeout, [this]() { doExtendCurrentNote(); });
}

void NotationMidiInput::onMidiEventReceived(const midi::Event& event)
{
    if (event.isChannelVoice20()) {
        auto events = event.toMIDI10();
        for (auto& midi10event : events) {
            onMidiEventReceived(midi10event);
        }

        return;
    }

    if (event.opcode() == midi::Event::Opcode::NoteOn || event.opcode() == midi::Event::Opcode::NoteOff) {
        m_eventsQueue.push_back(event);

        if (!m_processTimer.isActive()) {
            m_processTimer.start(PROCESS_INTERVAL);
        }
    }
}

mu::async::Channel<std::vector<const Note*> > NotationMidiInput::notesReceived() const
{
    return m_notesReceivedChannel;
}

void NotationMidiInput::onRealtimeAdvance()
{
    if (isSoundPreview()) {
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
    if (m_eventsQueue.empty()) {
        m_processTimer.stop();
        return;
    }

    std::vector<const Note*> notesOn;
    std::vector<const Note*> notesOff;

    for (size_t i = 0; i < m_eventsQueue.size(); ++i) {
        const midi::Event& event = m_eventsQueue.at(i);
        bool noteOn = event.opcode() == midi::Event::Opcode::NoteOn;
        bool noteOff = event.opcode() == midi::Event::Opcode::NoteOff;

        Note* note = !isSoundPreview() ? addNoteToScore(event) : makeNote(event);
        if (note) {
            if (noteOff) {
                notesOff.push_back(note);
            } else if (noteOn) {
                notesOn.push_back(note);
            }
        }

        bool chord = i != 0;
        if (!chord && noteOn && !m_realtimeTimer.isActive() && isRealtimeAuto()) {
            m_extendNoteTimer.start(configuration()->delayBetweenNotesInRealTimeModeMilliseconds());
            enableMetronome();
            doRealtimeAdvance();
        }
    }

    if (!notesOn.empty() || !notesOff.empty()) {
        playNoteOn(notesOn);

        if (isSoundPreview()) {
            playNoteOff(notesOff);
        }

        std::vector<const Note*> notes = notesOn;
        notes.insert(notes.end(), notesOff.begin(), notesOff.end());

        m_notesReceivedChannel.send(notes);
    }

    m_eventsQueue.clear();
    m_processTimer.stop();
}

Note* NotationMidiInput::addNoteToScore(const midi::Event& e)
{
    mu::engraving::Score* sc = score();
    if (!sc) {
        return nullptr;
    }

    mu::engraving::MidiInputEvent inputEv;
    inputEv.pitch = e.note();
    inputEv.velocity = e.velocity();

    sc->activeMidiPitches().remove_if([&inputEv](const mu::engraving::MidiInputEvent& val) {
        return inputEv.pitch == val.pitch;
    });

    const mu::engraving::InputState& is = sc->inputState();
    if (!is.noteEntryMode()) {
        return nullptr;
    }

    DEFER {
        m_undoStack->commitChanges();
    };

    m_undoStack->prepareChanges();

    if (e.opcode() == midi::Event::Opcode::NoteOff) {
        if (isRealtime()) {
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
    if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier) {
        mu::engraving::EngravingItem* cr = is.lastSegment()->element(is.track());
        if (cr && cr->isChord()) {
            inputEv.chord = true;
        }
    }

    mu::engraving::Note* note = sc->addMidiPitch(inputEv.pitch, inputEv.chord);

    sc->activeMidiPitches().push_back(inputEv);

    m_notationInteraction->showItem(is.cr());

    return note;
}

Note* NotationMidiInput::makeNote(const midi::Event& e)
{
    mu::engraving::Score* score = this->score();
    if (!score) {
        return nullptr;
    }

    if (score->selection().isNone()) {
        return nullptr;
    }

    const mu::engraving::InputState& inputState = score->inputState();
    if (!inputState.cr()) {
        return nullptr;
    }

    Chord* chord = engraving::Factory::createChord(inputState.lastSegment());
    chord->setParent(inputState.lastSegment());

    Note* note = engraving::Factory::createNote(chord);
    note->setParent(chord);
    note->setStaffIdx(engraving::track2staff(inputState.cr()->track()));

    engraving::NoteVal nval = score->noteVal(e.note());
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
    if (!isRealtime() || isSoundPreview() || (!m_allowRealtimeRests && m_getScore->score()->activeMidiPitches().empty())) {
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
        m_undoStack->prepareChanges();
        m_getScore->score()->realtimeAdvance();
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
    if (isSoundPreview() || m_realtimeTimer.isActive()) {
        return;
    }

    m_allowRealtimeRests = false;
    runRealtime();
    doRealtimeAdvance();
}

NoteInputMethod NotationMidiInput::noteInputMethod() const
{
    return m_notationInteraction->noteInput()->state().method;
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

bool NotationMidiInput::isSoundPreview() const
{
    return !m_notationInteraction->noteInput()->isNoteInputMode();
}

void NotationMidiInput::playNoteOn(const std::vector<const Note*>& notes)
{
    std::vector<const EngravingItem*> notesOnItems;
    for (const Note* note : notes) {
        notesOnItems.push_back(note);
    }

    if (isSoundPreview()) {
        static mpe::duration_t NOTE_ON_LONG_DURATION = configuration()->notePlayDurationMilliseconds() * 10000;
        playbackController()->playElements(notesOnItems, NOTE_ON_LONG_DURATION);
    } else {
        playbackController()->playElements(notesOnItems);
    }
}

void NotationMidiInput::playNoteOff(const std::vector<const Note*>& notes)
{
    std::vector<const EngravingItem*> notesOffItems;
    for (const Note* note : notes) {
        notesOffItems.push_back(note);
    }

    static constexpr mpe::duration_t NOTE_OFF_DURATION = 0;
    playbackController()->playElements(notesOffItems, NOTE_OFF_DURATION);
}
