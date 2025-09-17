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

static mu::playback::IPlaybackController::PlayParams makeNoteOnParams(bool infiniteDuration)
{
    mu::playback::IPlaybackController::PlayParams params;
    params.flushSound = !infiniteDuration;

    if (infiniteDuration) {
        params.duration = muse::mpe::INFINITE_DURATION; // play note on only
    }

    return params;
}

static mu::playback::IPlaybackController::PlayParams makeNoteOffParams()
{
    mu::playback::IPlaybackController::PlayParams params;
    params.flushSound = false;
    params.duration = 0; // note off only

    return params;
}

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

    const static std::unordered_set<muse::midi::Event::Opcode> ACCEPTED_OPCODES {
        muse::midi::Event::Opcode::NoteOn,
        muse::midi::Event::Opcode::NoteOff,
        muse::midi::Event::Opcode::ControlChange,
        muse::midi::Event::Opcode::PitchBend,
    };

    if (muse::contains(ACCEPTED_OPCODES, event.opcode())) {
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

    std::vector<const Note*> notesOn;
    std::vector<int> notesOff;
    ControllerEventMap controllers;

    startNoteInputIfNeed();
    bool isNoteInput = isNoteInputMode();
    bool isSoundPreview = !isNoteInput;

    if (isNoteInput && isInputByDuration()) {
        addNoteEventsToInputState();
        return;
    }

    const bool useDurationAndVelocity = isSoundPreview || configuration()->useMidiVelocityAndDurationDuringNoteInput();

    for (size_t i = 0; i < m_eventsQueue.size(); ++i) {
        const muse::midi::Event& event = m_eventsQueue.at(i);
        const muse::midi::Event::Opcode opcode = event.opcode();

        if (opcode == muse::midi::Event::Opcode::ControlChange || opcode == muse::midi::Event::Opcode::PitchBend) {
            controllers[opcode] = event; // keep only last received to prevent spam
            continue;
        }

        Note* note = isNoteInput ? addNoteToScore(event) : makePreviewNote(event);
        if (note) {
            if (useDurationAndVelocity) {
                note->setUserVelocity(event.velocity7());
                m_playingNotes[note->pitch()] = note;
            }
            notesOn.push_back(note);
        }

        const bool chord = i != 0;
        const bool noteOn = opcode == muse::midi::Event::Opcode::NoteOn;
        if (!chord && noteOn && !m_realtimeTimer.isActive() && isRealtimeAuto()) {
            m_extendNoteTimer.start(configuration()->delayBetweenNotesInRealTimeModeMilliseconds());
            enableMetronome();
            doRealtimeAdvance();
        }

        const bool noteOff = opcode == muse::midi::Event::Opcode::NoteOff || event.velocity7() == 0;
        if (useDurationAndVelocity && noteOff) {
            notesOff.push_back(event.note());
        }
    }

    if (!controllers.empty()) {
        triggerControllers(controllers);
    }

    if (!notesOn.empty()) {
        const std::vector<const EngravingItem*> elements(notesOn.begin(), notesOn.end());
        playbackController()->seekElement(notesOn.front(), !useDurationAndVelocity /*flushSound*/);
        playbackController()->playElements(elements, makeNoteOnParams(useDurationAndVelocity), true);
        m_notesReceivedChannel.send(notesOn);
    }

    if (!notesOff.empty()) {
        releasePlayingNotes(notesOff, isSoundPreview);
    }
}

void NotationMidiInput::startNoteInputIfNeed()
{
    if (isNoteInputMode()) {
        return;
    }

    const auto containsNoteOn = [this]() -> bool {
        return std::any_of(m_eventsQueue.begin(), m_eventsQueue.end(), [](const muse::midi::Event& e) {
            return e.opcode() == muse::midi::Event::Opcode::NoteOn;
        });
    };

    if (configuration()->startNoteInputAtSelectedNoteRestWhenPressingMidiKey()) {
        if (m_notationInteraction->selection()->elementsSelected(NOTE_REST_TYPES) && containsNoteOn()) {
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
    const bool playPreviewNotes = configuration()->isPlayPreviewNotesInInputByDuration();
    const bool useVelocityAndDuration = playPreviewNotes && configuration()->useMidiVelocityAndDurationDuringNoteInput();

    NoteValList notesOn;
    NoteValList notesOff;

    if (m_holdingNotesInInputByDuration) {
        notesOn = state.notes();
    }

    ControllerEventMap controllers;

    for (const muse::midi::Event& event : m_eventsQueue) {
        const muse::midi::Event::Opcode opcode = event.opcode();

        if (opcode == muse::midi::Event::Opcode::NoteOn) {
            NoteVal nval = score()->noteVal(event.note(), staffIdx, useWrittenPitch);
            nval.velocityOverride = event.velocity7();
            notesOn.push_back(nval);
            m_holdingNotesInInputByDuration = true;
        } else if (opcode == muse::midi::Event::Opcode::NoteOff) {
            if (useVelocityAndDuration) {
                notesOff.push_back(score()->noteVal(event.note(), staffIdx, useWrittenPitch));
            }
            m_holdingNotesInInputByDuration = false;
        } else if (opcode == muse::midi::Event::Opcode::ControlChange || opcode == muse::midi::Event::Opcode::PitchBend) {
            if (playPreviewNotes) {
                controllers[opcode] = event; // keep only last received to prevent spam
            }
        }
    }

    if (!controllers.empty()) {
        triggerControllers(controllers);
    }

    if (!notesOff.empty()) {
        playbackController()->playNotes(notesOff, staffIdx, state.segment(), makeNoteOffParams());
    }

    if (!notesOn.empty() && notesOn != state.notes()) {
        noteInput->setRestMode(false);
        noteInput->setInputNotes(notesOn);

        if (playPreviewNotes) {
            playbackController()->playNotes(notesOn, staffIdx, state.segment(), makeNoteOnParams(useVelocityAndDuration));
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

void NotationMidiInput::triggerControllers(const ControllerEventMap& events)
{
    muse::mpe::ControllerChangeEventList controllers;

    static const std::unordered_map<int, muse::mpe::ControllerChangeEvent::Type> MIDI_CC_TO_EVENT_TYPE {
        { muse::midi::MODWHEEL_CONTROLLER, muse::mpe::ControllerChangeEvent::Modulation },
        { muse::midi::SUSTAIN_PEDAL_CONTROLLER, muse::mpe::ControllerChangeEvent::SustainPedalOnOff },
    };

    const mu::engraving::InputState& is = score()->inputState();

    for (const auto& pair : events) {
        const muse::midi::Event& e = pair.second;
        muse::mpe::ControllerChangeEvent cc;

        if (pair.first == muse::midi::Event::Opcode::PitchBend) {
            cc.type = muse::mpe::ControllerChangeEvent::PitchBend;
            cc.val = static_cast<float>(e.pitchBend14()) / 16383.f;
        } else {
            cc.type = muse::value(MIDI_CC_TO_EVENT_TYPE, e.index(), muse::mpe::ControllerChangeEvent::Undefined);
            cc.val = static_cast<float>(e.data()) / 127.f;
        }

        if (cc.type != muse::mpe::ControllerChangeEvent::Undefined) {
            cc.layerIdx = static_cast<muse::mpe::layer_idx_t>(is.track());
            controllers.push_back(cc);
        }
    }

    playbackController()->triggerControllers(controllers, is.staffIdx(), is.tick().ticks());
}

void NotationMidiInput::releasePlayingNotes(const std::vector<int>& pitches, bool deleteNotes)
{
    std::vector<const EngravingItem*> notes;

    const staff_idx_t staffIdx = score()->inputState().staffIdx();
    const bool useWrittenPitch = configuration()->midiUseWrittenPitch().val;

    for (int pitch : pitches) {
        const NoteVal nval = score()->noteVal(pitch, staffIdx, useWrittenPitch);

        auto it = m_playingNotes.find(nval.pitch);
        if (it == m_playingNotes.end()) {
            continue;
        }

        notes.push_back(it->second);
        it->second->setUserVelocity(0);
        m_playingNotes.erase(it);
    }

    playbackController()->playElements(notes, makeNoteOffParams(), true /*isMidi*/);

    if (deleteNotes) {
        muse::DeleteAll(notes);
    }
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
