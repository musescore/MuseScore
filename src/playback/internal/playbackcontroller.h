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
#ifndef MU_PLAYBACK_PLAYBACKCONTROLLER_H
#define MU_PLAYBACK_PLAYBACKCONTROLLER_H

#include <unordered_map>

#include "modularity/ioc.h"
#include "types/retval.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "notation/notationtypes.h"
#include "engraving/types/types.h"
#include "notation/inotationconfiguration.h"
#include "notation/inotationplayback.h"
#include "audio/iplayer.h"
#include "audio/itracks.h"
#include "audio/iaudiooutput.h"
#include "audio/iplayback.h"
#include "audio/audiotypes.h"
#include "iinteractive.h"

#include "../iplaybackcontroller.h"
#include "../iplaybackconfiguration.h"
#include "isoundprofilesrepository.h"

namespace mu::playback {
class PlaybackController : public IPlaybackController, public actions::Actionable, public async::Asyncable
{
    INJECT_STATIC(playback, actions::IActionsDispatcher, dispatcher)
    INJECT_STATIC(playback, context::IGlobalContext, globalContext)
    INJECT_STATIC(playback, IPlaybackConfiguration, configuration)
    INJECT_STATIC(playback, notation::INotationConfiguration, notationConfiguration)
    INJECT_STATIC(playback, audio::IPlayback, playback)
    INJECT_STATIC(playback, ISoundProfilesRepository, profilesRepo)
    INJECT_STATIC(playback, framework::IInteractive, interactive)

public:
    void init();

    bool isPlayAllowed() const override;
    async::Notification isPlayAllowedChanged() const override;

    bool isPlaying() const override;
    async::Notification isPlayingChanged() const override;

    void reset() override;

    void seek(const midi::tick_t tick) override;
    void seek(const audio::msecs_t msecs) override;

    async::Notification playbackPositionChanged() const override;
    async::Channel<uint32_t> midiTickPlayed() const override;
    float playbackPositionInSeconds() const override;

    audio::TrackSequenceId currentTrackSequenceId() const override;
    async::Notification currentTrackSequenceIdChanged() const override;

    const InstrumentTrackIdMap& instrumentTrackIdMap() const override;

    async::Channel<audio::TrackId, engraving::InstrumentTrackId> trackAdded() const override;
    async::Channel<audio::TrackId, engraving::InstrumentTrackId> trackRemoved() const override;

    void playElements(const std::vector<const notation::EngravingItem*>& elements) override;
    void playMetronome(int tick) override;
    void seekElement(const notation::EngravingItem* element) override;

    bool actionChecked(const actions::ActionCode& actionCode) const override;
    async::Channel<actions::ActionCode> actionCheckedChanged() const override;

    QTime totalPlayTime() const override;
    async::Notification totalPlayTimeChanged() const override;

    notation::Tempo currentTempo() const override;
    async::Notification currentTempoChanged() const override;

    notation::MeasureBeat currentBeat() const override;
    audio::msecs_t beatToMilliseconds(int measureIndex, int beatIndex) const override;

    double tempoMultiplier() const override;
    void setTempoMultiplier(double multiplier) override;

    framework::Progress loadingProgress() const override;

    void applyProfile(const SoundProfileName& profileName) override;

    void setExportingAudio(bool isExporting) override;

    bool canReceiveAction(const actions::ActionCode& code) const override;

private:
    notation::INotationPlaybackPtr notationPlayback() const;
    notation::INotationPartsPtr masterNotationParts() const;
    notation::INotationSelectionPtr selection() const;
    notation::INotationSelectionRangePtr selectionRange() const;
    notation::INotationInteractionPtr interaction() const;

    void updateCurrentTempo();

    bool isPaused() const;
    bool isLoaded() const;

    bool isLoopVisible() const;
    bool isPlaybackLooped() const;

    void onNotationChanged();

    void onSelectionChanged();
    void seekListSelection();
    void seekRangeSelection();

    void togglePlay();
    void rewind(const actions::ActionData& args);
    void play();
    void pause();
    void stop();
    void resume();

    audio::msecs_t playbackStartMsecs() const;
    audio::msecs_t playbackEndMsecs() const;

    notation::InstrumentTrackIdSet instrumentTrackIdSetForRangePlayback() const;

    void setCurrentPlaybackStatus(audio::PlaybackStatus status);

    void togglePlayRepeats();
    void togglePlayChordSymbols();
    void toggleAutomaticallyPan();
    void toggleMetronome();
    void toggleMidiInput();
    void toggleCountIn();
    void toggleLoopPlayback();

    void openPlaybackSetupDialog();

    void addLoopBoundary(notation::LoopBoundaryType type);
    void addLoopBoundaryToTick(notation::LoopBoundaryType type, int tick);
    void updateLoop();

    void showLoop();
    void hideLoop();

    void notifyActionCheckedChanged(const actions::ActionCode& actionCode);

    project::IProjectAudioSettingsPtr audioSettings() const;

    void resetCurrentSequence();
    void setupNewCurrentSequence(const audio::TrackSequenceId sequenceId);
    void subscribeOnAudioParamsChanges();
    void setupSequenceTracks();
    void setupSequencePlayer();

    void updateMuteStates();

    void setCurrentPlaybackTime(audio::msecs_t msecs);

    using TrackAddFinished = std::function<void (const engraving::InstrumentTrackId&)>;

    void addTrack(const engraving::InstrumentTrackId& instrumentTrackId, const TrackAddFinished& onFinished);
    void doAddTrack(const engraving::InstrumentTrackId& instrumentTrackId, const std::string& title, const TrackAddFinished& onFinished);

    void setTrackActivity(const engraving::InstrumentTrackId& instrumentTrackId, const bool isActive);
    audio::AudioOutputParams trackOutputParams(const engraving::InstrumentTrackId& instrumentTrackId) const;
    engraving::InstrumentTrackIdSet availableInstrumentTracks() const;
    void removeNonExistingTracks();
    void removeTrack(const engraving::InstrumentTrackId& instrumentTrackId);

    audio::msecs_t tickToMsecs(int tick) const;

    notation::INotationPtr m_notation;
    notation::IMasterNotationPtr m_masterNotation;

    async::Notification m_isPlayAllowedChanged;
    async::Notification m_isPlayingChanged;
    async::Notification m_playbackPositionChanged;
    async::Notification m_totalPlayTimeChanged;
    async::Notification m_currentTempoChanged;
    async::Channel<uint32_t> m_tickPlayed;
    async::Channel<actions::ActionCode> m_actionCheckedChanged;

    audio::TrackSequenceId m_currentSequenceId = -1;
    async::Notification m_currentSequenceIdChanged;
    audio::PlaybackStatus m_currentPlaybackStatus = audio::PlaybackStatus::Stopped;
    audio::msecs_t m_currentPlaybackTimeMsecs = 0;
    midi::tick_t m_currentTick = 0;
    notation::Tempo m_currentTempo;

    async::Channel<audio::TrackId, engraving::InstrumentTrackId> m_trackAdded;
    async::Channel<audio::TrackId, engraving::InstrumentTrackId> m_trackRemoved;

    InstrumentTrackIdMap m_trackIdMap;

    framework::Progress m_loadingProgress;
    std::list<engraving::InstrumentTrackId> m_loadingTracks;

    bool m_isExportingAudio = false;
};
}

#endif // MU_PLAYBACK_PLAYBACKCONTROLLER_H
