/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "midirecordingcontroller.h"

#include "notation/inotationelements.h"
#include "notation/inotationundostack.h"
#include "midirecordingtiming.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "audio/common/audioutils.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/score.h"
#include "engraving/infrastructure/eidregister.h"
#include "engraving/dom/staff.h"
#include "engraving/types/constants.h"
#include "translation.h"
#include "types/translatablestring.h"

#include "log.h"

using namespace mu::playback;
using namespace mu::iex::midi;
using namespace mu::notation;
using namespace muse;

//! Notes started earlier than this (i.e. during the count-in) are ignored;
//! notes started slightly before the first beat are moved onto it
static constexpr int MAX_ANTICIPATION_TICKS = mu::engraving::Constants::DIVISION / 4;

MidiRecordingController::MidiRecordingController(const muse::modularity::ContextPtr& iocCtx)
    : muse::Contextable(iocCtx)
{
}

void MidiRecordingController::init()
{
    if (!midiInPort()) {
        return;
    }

    midiInPort()->eventReceived().onReceive(this, [this](const muse::midi::tick_t, const muse::midi::Event& event) {
        if (!m_isRecording) {
            return;
        }

        if (event.isChannelVoice20()) {
            for (const muse::midi::Event& midi10event : event.toMIDI10()) {
                onMidiEventReceived(midi10event);
            }
            return;
        }

        onMidiEventReceived(event);
    });
}

void MidiRecordingController::setPlayNoteCallback(PlayNoteCallback callback)
{
    m_playNoteCallback = std::move(callback);
}

bool MidiRecordingController::isRecording() const
{
    return m_isRecording;
}

const MidiRecordingController::Session& MidiRecordingController::session() const
{
    return m_session;
}

bool MidiRecordingController::canWriteRecording() const
{
    return midiRecordingImporter() != nullptr;
}

void MidiRecordingController::start(const Session& session)
{
    m_session = session;
    m_heldNotes.clear();
    m_notes.clear();
    m_previewNotes.clear();

    // Until the playback starts, assume that the music starts now (refined by setMusicStartTime)
    m_anchorTime = Clock::now();
    m_anchorSecs = m_session.startSecs;

    // Used to place the notes played just before the first beat
    m_ticksPerSecAtStart = 0.0;
    if (m_session.notationPlayback) {
        const secs_t oneSecondLater = secs_t(static_cast<double>(m_session.startSecs) + 1.0);
        const midi::tick_t tickOneSecondLater = m_session.notationPlayback->secToPlayedTick(oneSecondLater);
        m_ticksPerSecAtStart = static_cast<double>(tickOneSecondLater) - static_cast<double>(m_session.startPlayedTick);
    }

    m_isRecording = true;
}

void MidiRecordingController::setMusicStartTime(Clock::time_point time)
{
    m_anchorTime = time;
    m_anchorSecs = m_session.startSecs;
}

void MidiRecordingController::onPlaybackPositionChanged(secs_t pos)
{
    if (!m_isRecording) {
        return;
    }

    // During the count-in the position stays at the start: keep the anchor computed from the count-in duration
    if (pos <= m_session.startSecs) {
        return;
    }

    // Re-synchronize the clock with the audio position, to avoid any drift
    m_anchorTime = Clock::now();
    m_anchorSecs = pos;
}

secs_t MidiRecordingController::currentSecs() const
{
    const std::chrono::duration<double> elapsed = Clock::now() - m_anchorTime;
    return secs_t(static_cast<double>(m_anchorSecs) + elapsed.count());
}

secs_t MidiRecordingController::estimateOutputLatency(audio::samples_t driverBufferSize, audio::sample_rate_t sampleRate)
{
    return secs_t(midirecording::estimateOutputLatencySecs(driverBufferSize, sampleRate));
}

int MidiRecordingController::secsToRelativeTick(secs_t renderedSecs) const
{
    const double startSecs = static_cast<double>(m_session.startSecs);
    const double startPlayedTick = static_cast<double>(m_session.startPlayedTick);

    const auto secsToPlayedTicks = [this, startSecs, startPlayedTick](double secs) -> double {
        if (secs >= startSecs && m_session.notationPlayback) {
            return static_cast<double>(m_session.notationPlayback->secToPlayedTick(secs_t(secs)));
        }

        // Before the first beat (count-in): extrapolate with the tempo at the start
        return startPlayedTick - (startSecs - secs) * m_ticksPerSecAtStart;
    };

    return midirecording::positionToRelativeTick(static_cast<double>(renderedSecs), startSecs,
                                                 static_cast<double>(m_session.outputLatency), secsToPlayedTicks);
}

void MidiRecordingController::onMidiEventReceived(const muse::midi::Event& event)
{
    using Opcode = muse::midi::Event::Opcode;

    const Opcode opcode = event.opcode();
    if (opcode != Opcode::NoteOn && opcode != Opcode::NoteOff) {
        return;
    }

    const int channel = static_cast<int>(event.channel());
    const int velocity = event.velocity7();

    if (opcode == Opcode::NoteOn && velocity > 0) {
        onNoteOn(channel, event.note(), velocity);
    } else {
        onNoteOff(channel, event.note());
    }
}

bool MidiRecordingController::isPitchHeld(int pitch) const
{
    return std::any_of(m_heldNotes.cbegin(), m_heldNotes.cend(), [pitch](const auto& pair) {
        return pair.first.second == pitch;
    });
}

void MidiRecordingController::onNoteOn(int channel, int pitch, int velocity)
{
    const int tick = secsToRelativeTick(currentSecs());

    // The same key pressed again on the same channel without a release: close the previous note
    const HeldNoteKey key { channel, pitch };
    if (m_heldNotes.count(key)) {
        onNoteOff(channel, pitch);
    }

    m_heldNotes[key] = { tick, velocity };

    // Shown on the score immediately, where the note was played (the notes of the count-in are not shown)
    if (tick >= -MAX_ANTICIPATION_TICKS) {
        PreviewNote preview;
        preview.staffIdx = m_session.staffForPitch(pitch);
        preview.pitch = pitch;
        preview.tick = m_session.startTick.ticks() + std::max(tick, 0);
        m_previewNotes.push_back(preview);
        m_previewNotesChanged.notify();
    }

    if (m_playNoteCallback) {
        m_playNoteCallback(pitch, velocity);
    }
}

void MidiRecordingController::onNoteOff(int channel, int pitch)
{
    auto it = m_heldNotes.find(HeldNoteKey { channel, pitch });
    if (it == m_heldNotes.end()) {
        return;
    }

    const HeldNote held = it->second;
    m_heldNotes.erase(it);

    // The sound is stopped when the last channel holding the key releases it
    if (m_playNoteCallback && !isPitchHeld(pitch)) {
        m_playNoteCallback(pitch, 0);
    }

    const int offTick = secsToRelativeTick(currentSecs());

    if (held.onTick < -MAX_ANTICIPATION_TICKS || offTick <= 0) {
        return; // played during the count-in
    }

    RecordedMidiNote note;
    note.pitch = pitch;
    note.velocity = held.velocity;
    note.onTick = std::max(held.onTick, 0);
    note.offTick = std::max(offTick, note.onTick + 1);

    m_notes.push_back(note);
}

RecordedMidiNoteList MidiRecordingController::finish()
{
    if (!m_isRecording) {
        return {};
    }

    std::vector<HeldNoteKey> stillHeld;
    for (const auto& pair : m_heldNotes) {
        stillHeld.push_back(pair.first);
    }

    for (const HeldNoteKey& key : stillHeld) {
        onNoteOff(key.first, key.second);
    }

    m_isRecording = false;

    RecordedMidiNoteList result = std::move(m_notes);
    m_notes.clear();
    m_heldNotes.clear();

    if (!m_previewNotes.empty()) {
        m_previewNotes.clear();
        m_previewNotesChanged.notify();
    }

    LOGI() << "MIDI recording finished, notes: " << result.size();

    return result;
}

void MidiRecordingController::cancel()
{
    if (m_playNoteCallback) {
        std::set<int> soundingPitches;
        for (const auto& pair : m_heldNotes) {
            soundingPitches.insert(pair.first.second);
        }

        for (int pitch : soundingPitches) {
            m_playNoteCallback(pitch, 0);
        }
    }

    m_isRecording = false;
    m_notes.clear();
    m_heldNotes.clear();
    m_session = Session();

    if (!m_previewNotes.empty()) {
        m_previewNotes.clear();
        m_previewNotesChanged.notify();
    }
}

MidiRecordingOptions MidiRecordingController::currentOptions() const
{
    MidiRecordingOptions options;
    options.minQuantizationTicks = notationConfiguration()->midiRecordingMinQuantizationTicks();
    options.searchTuplets = notationConfiguration()->midiRecordingSearchTuplets();
    options.legato = notationConfiguration()->midiRecordingLegato();

    return options;
}

bool MidiRecordingController::writeNotes(const Session& session, const RecordedMidiNoteList& notes, const MidiRecordingOptions& options,
                                         const TranslatableString& actionName, int& writtenTicks)
{
    if (!midiRecordingImporter()) {
        LOGE() << "MIDI import module is not available";
        return false;
    }

    mu::engraving::Score* score = session.notation ? session.notation->elements()->msScore() : nullptr;
    INotationUndoStackPtr undoStack = session.notation ? session.notation->undoStack() : nullptr;
    IF_ASSERT_FAILED(score && undoStack) {
        return false;
    }

    undoStack->prepareChanges(actionName);

    const RetVal<int> ret = session.isSplit()
                            ? midiRecordingImporter()->writeRecordedNotesOnTwoStaves(score, session.staffIdx, session.lowerStaffIdx,
                                                                                     session.startTick, notes, session.splitPitch, options)
                            : midiRecordingImporter()->writeRecordedNotes(score, session.staffIdx, session.startTick, notes, options);
    if (!ret.ret) {
        undoStack->rollbackChanges();

        if (ret.ret.code() != static_cast<int>(Ret::Code::Cancel)) {
            LOGE() << "failed to write the MIDI recording: " << ret.ret.toString();
            interactive()->error(muse::trc("playback", "Unable to write the MIDI recording"), ret.ret.text());
        }
        return false;
    }

    undoStack->commitChanges();
    session.notation->notationChanged().send(muse::RectF());

    writtenTicks = ret.val;
    return true;
}

void MidiRecordingController::finishAndWrite()
{
    if (!m_isRecording) {
        return;
    }

    const Session session = m_session;
    const RecordedMidiNoteList notes = finish();
    m_session = Session();

    if (notes.empty() || !session.notation) {
        return;
    }

    int writtenTicks = 0;
    if (!writeNotes(session, notes, currentOptions(), TranslatableString("undoableAction", "Record MIDI"), writtenTicks)) {
        return;
    }

    // The original performance is kept, to write it again with other settings (see requantizeLastRecording)
    LastRecording lastRecording { session, notes, writtenTicks, engraving::ID(), engraving::ID(), engraving::EID::invalid() };

    const mu::engraving::Score* score = session.notation->elements()->msScore();

    if (const mu::engraving::Measure* startMeasure = score ? score->tick2measure(session.startTick) : nullptr) {
        engraving::EID eid = startMeasure->eid();
        if (!eid.isValid()) {
            eid = startMeasure->assignNewEID();
        }
        lastRecording.startMeasureEid = eid;
    }
    if (const mu::engraving::Staff* staff = score ? score->staff(session.staffIdx) : nullptr) {
        lastRecording.staffId = staff->id();
    }

    if (const mu::engraving::Staff* lowerStaff = (score && session.isSplit()) ? score->staff(session.lowerStaffIdx) : nullptr) {
        lastRecording.lowerStaffId = lowerStaff->id();
    }

    m_lastRecording = std::move(lastRecording);
    m_lastRecordingChanged.notify();
}

std::optional<MidiRecordingController::Session> MidiRecordingController::resolveLastRecordingSession() const
{
    if (!m_lastRecording.has_value() || !m_lastRecording->session.notation) {
        return std::nullopt;
    }

    const mu::engraving::Score* score = m_lastRecording->session.notation->elements()->msScore();
    if (!score) {
        return std::nullopt;
    }

    const auto findStaff = [score](const engraving::ID& id) -> const mu::engraving::Staff* {
        if (!id.isValid()) {
            return nullptr;
        }

        for (const mu::engraving::Staff* staff : score->staves()) {
            if (staff->id() == id) {
                return staff;
            }
        }

        return nullptr;
    };

    Session session = m_lastRecording->session;

    // The start measure, wherever it is now; a deleted measure is kept alive by the undo stack, so it must
    // also be one of the current measures of the score
    const mu::engraving::EID& measureEid = m_lastRecording->startMeasureEid;
    const mu::engraving::EngravingObject* measureObject = measureEid.isValid()
                                                          ? score->masterScore()->eidRegister()->itemFromEID(measureEid)
                                                          : nullptr;
    const mu::engraving::Measure* startMeasure = nullptr;
    for (const mu::engraving::Measure* m = score->firstMeasure(); m && measureObject; m = m->nextMeasure()) {
        if (m == measureObject) {
            startMeasure = m;
            break;
        }
    }

    if (!startMeasure) {
        return std::nullopt;
    }
    session.startTick = startMeasure->tick();

    const mu::engraving::Staff* staff = findStaff(m_lastRecording->staffId);
    if (!staff) {
        return std::nullopt;
    }
    session.staffIdx = staff->idx();

    if (m_lastRecording->session.isSplit()) {
        const mu::engraving::Staff* lowerStaff = findStaff(m_lastRecording->lowerStaffId);
        if (!lowerStaff || lowerStaff->part() != staff->part()) {
            return std::nullopt;
        }
        session.lowerStaffIdx = lowerStaff->idx();
    }

    return session;
}

bool MidiRecordingController::hasLastRecording(const INotationPtr& notation) const
{
    return m_lastRecording.has_value() && notation && m_lastRecording->session.notation == notation
           && resolveLastRecordingSession().has_value();
}

void MidiRecordingController::requantizeLastRecording()
{
    if (m_isRecording || !m_lastRecording.has_value()) {
        return;
    }

    MidiRecordingOptions options = currentOptions();

    // Replace at least what the previous take wrote, in case the new one is shorter
    options.minReplacedTicks = m_lastRecording->writtenTicks;

    // The recorded staves may have been moved, or removed, since the recording
    const std::optional<Session> session = resolveLastRecordingSession();
    if (!session.has_value()) {
        LOGW() << "the staves of the last MIDI recording are no longer available";
        return;
    }

    int writtenTicks = 0;
    if (!writeNotes(session.value(), m_lastRecording->notes, options,
                    TranslatableString("undoableAction", "Re-quantize MIDI recording"), writtenTicks)) {
        return;
    }

    m_lastRecording->writtenTicks = std::max(m_lastRecording->writtenTicks, writtenTicks);
}

void MidiRecordingController::clearLastRecording()
{
    if (!m_lastRecording.has_value()) {
        return;
    }

    m_lastRecording.reset();
    m_lastRecordingChanged.notify();
}

muse::async::Notification MidiRecordingController::lastRecordingChanged() const
{
    return m_lastRecordingChanged;
}

const std::vector<MidiRecordingController::PreviewNote>& MidiRecordingController::previewNotes() const
{
    return m_previewNotes;
}

muse::async::Notification MidiRecordingController::previewNotesChanged() const
{
    return m_previewNotesChanged;
}
