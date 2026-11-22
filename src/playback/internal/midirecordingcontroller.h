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

#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <optional>

#include "async/asyncable.h"
#include "async/notification.h"
#include "types/translatablestring.h"
#include "modularity/ioc.h"

#include "audio/common/audiotypes.h"
#include "interactive/iinteractive.h"
#include "midi/imidiinport.h"
#include "notation/inotation.h"
#include "notation/inotationconfiguration.h"
#include "notation/inotationplayback.h"
#include "importexport/midi/imidirecordingimporter.h"

#include "../iplaybackcontroller.h"

namespace mu::playback {
//! Captures the notes played on the MIDI keyboard during a metronome recording.
//! It only listens: it does not write anything into the score while recording.
class MidiRecordingController : public muse::async::Asyncable, public muse::Contextable
{
    muse::GlobalInject<notation::INotationConfiguration> notationConfiguration;
    muse::GlobalInject<iex::midi::IMidiRecordingImporter> midiRecordingImporter;
    muse::GlobalInject<muse::midi::IMidiInPort> midiInPort;
    muse::ContextInject<muse::IInteractive> interactive = { this };

public:
    using Clock = std::chrono::steady_clock;

    struct Session {
        notation::INotationPtr notation;
        notation::INotationPlaybackPtr notationPlayback;
        engraving::staff_idx_t staffIdx = 0;
        engraving::Fraction startTick;          // raw tick of the first recorded measure
        muse::midi::tick_t startPlayedTick = 0; // the same position, in playback ticks
        muse::secs_t startSecs = 0;             // the same position, in playback seconds

        //! Time between the moment a position is rendered by the audio engine (the reported playback position)
        //! and the moment it is heard; the player plays along with what they hear
        muse::secs_t outputLatency = 0;

        //! Recording on two staves of the same instrument: the notes lower than splitPitch go to lowerStaffIdx
        engraving::staff_idx_t lowerStaffIdx = muse::nidx;
        int splitPitch = 60;

        bool isSplit() const { return lowerStaffIdx != muse::nidx; }
        engraving::staff_idx_t staffForPitch(int pitch) const { return isSplit() && pitch < splitPitch ? lowerStaffIdx : staffIdx; }
    };

    using PreviewNote = MidiRecordingPreviewNote;

    //! Called for each key pressed (velocity > 0) or released (velocity == 0) during the recording
    using PlayNoteCallback = std::function<void (int pitch, int velocity)>;

    explicit MidiRecordingController(const muse::modularity::ContextPtr& iocCtx);

    void init();

    void setPlayNoteCallback(PlayNoteCallback callback);

    bool isRecording() const;
    const Session& session() const;

    void start(const Session& session);

    //! The instant at which the playback reaches the start position (i.e. the end of the count-in)
    void setMusicStartTime(Clock::time_point time);
    void onPlaybackPositionChanged(muse::secs_t pos);

    //! Stops capturing and returns the recorded notes; the notes still held are released now
    iex::midi::RecordedMidiNoteList finish();
    void cancel();

    //! Stops capturing and writes the recorded notes into the score, as a single undoable action
    void finishAndWrite();

    //! The original notes of the last recording are kept, to write them again with the current settings
    bool hasLastRecording(const notation::INotationPtr& notation) const;
    void requantizeLastRecording();
    void clearLastRecording();
    muse::async::Notification lastRecordingChanged() const;

    //! The settings from the preferences
    iex::midi::MidiRecordingOptions currentOptions() const;

    const std::vector<PreviewNote>& previewNotes() const;
    muse::async::Notification previewNotesChanged() const;

    //! See midirecording::estimateOutputLatencySecs
    static muse::secs_t estimateOutputLatency(muse::audio::samples_t driverBufferSize, muse::audio::sample_rate_t sampleRate);

private:
    struct LastRecording {
        Session session;
        iex::midi::RecordedMidiNoteList notes;
        int writtenTicks = 0;
    };

    bool writeNotes(const Session& session, const iex::midi::RecordedMidiNoteList& notes, const iex::midi::MidiRecordingOptions& options,
                    const muse::TranslatableString& actionName, int& writtenTicks);

    struct HeldNote {
        int onTick = 0;
        int velocity = 0;
    };

    void onMidiEventReceived(const muse::midi::Event& event);
    void onNoteOn(int pitch, int velocity);
    void onNoteOff(int pitch);

    muse::secs_t currentSecs() const;
    int secsToRelativeTick(muse::secs_t renderedSecs) const;

    bool m_isRecording = false;
    Session m_session;
    PlayNoteCallback m_playNoteCallback;

    Clock::time_point m_anchorTime;
    muse::secs_t m_anchorSecs = 0;
    double m_ticksPerSecAtStart = 0.0;

    std::map<int, HeldNote> m_heldNotes;
    iex::midi::RecordedMidiNoteList m_notes;

    std::optional<LastRecording> m_lastRecording;
    muse::async::Notification m_lastRecordingChanged;

    std::vector<PreviewNote> m_previewNotes;
    muse::async::Notification m_previewNotesChanged;
};
}
