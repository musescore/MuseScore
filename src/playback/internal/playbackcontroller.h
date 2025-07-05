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
#ifndef MU_PLAYBACK_PLAYBACKCONTROLLER_H
#define MU_PLAYBACK_PLAYBACKCONTROLLER_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "notation/notationtypes.h"
#include "engraving/types/types.h"
#include "notation/inotationconfiguration.h"
#include "notation/inotationplayback.h"
#include "audio/iplayer.h"
#include "audio/iplayback.h"
#include "audio/audiotypes.h"
#include "iinteractive.h"
#include "drumsetloader.h"

#include "../iplaybackcontroller.h"
#include "../iplaybackconfiguration.h"
#include "../isoundprofilesrepository.h"

namespace mu::playback {
class PlaybackController : public IPlaybackController, public muse::actions::Actionable, public muse::async::Asyncable
{
    INJECT_STATIC(muse::actions::IActionsDispatcher, dispatcher)
    INJECT_STATIC(context::IGlobalContext, globalContext)
    INJECT_STATIC(IPlaybackConfiguration, configuration)
    INJECT_STATIC(notation::INotationConfiguration, notationConfiguration)
    INJECT_STATIC(muse::audio::IPlayback, playback)
    INJECT_STATIC(ISoundProfilesRepository, profilesRepo)
    INJECT_STATIC(muse::IInteractive, interactive)

public:
    void init();

    bool isPlayAllowed() const override;
    muse::async::Notification isPlayAllowedChanged() const override;

    bool isPlaying() const override;
    muse::async::Notification isPlayingChanged() const override;

    void reset() override;

    muse::async::Channel<muse::audio::secs_t, muse::midi::tick_t> currentPlaybackPositionChanged() const override;

    muse::audio::TrackSequenceId currentTrackSequenceId() const override;
    muse::async::Notification currentTrackSequenceIdChanged() const override;

    const InstrumentTrackIdMap& instrumentTrackIdMap() const override;
    const AuxTrackIdMap& auxTrackIdMap() const override;

    muse::async::Channel<muse::audio::TrackId> trackAdded() const override;
    muse::async::Channel<muse::audio::TrackId> trackRemoved() const override;

    std::string auxChannelName(muse::audio::aux_channel_idx_t index) const override;
    muse::async::Channel<muse::audio::aux_channel_idx_t, std::string> auxChannelNameChanged() const override;

    muse::async::Promise<muse::audio::SoundPresetList> availableSoundPresets(
        const engraving::InstrumentTrackId& instrumentTrackId) const override;

    notation::INotationSoloMuteState::SoloMuteState trackSoloMuteState(const engraving::InstrumentTrackId& trackId) const override;
    void setTrackSoloMuteState(const engraving::InstrumentTrackId& trackId,
                               const notation::INotationSoloMuteState::SoloMuteState& state) override;

    void playElements(const std::vector<const notation::EngravingItem*>& elements, bool isMidi = false) override;
    void playNotes(const notation::NoteValList& notes, const notation::staff_idx_t staffIdx, const notation::Segment* segment) override;
    void playMetronome(int tick) override;

    void seekElement(const notation::EngravingItem* element) override;
    void seekBeat(int measureIndex, int beatIndex) override;

    bool actionChecked(const muse::actions::ActionCode& actionCode) const override;
    muse::async::Channel<muse::actions::ActionCode> actionCheckedChanged() const override;

    QTime totalPlayTime() const override;
    muse::async::Notification totalPlayTimeChanged() const override;

    notation::Tempo currentTempo() const override;
    muse::async::Notification currentTempoChanged() const override;

    notation::MeasureBeat currentBeat() const override;
    muse::audio::secs_t beatToSecs(int measureIndex, int beatIndex) const override;

    double tempoMultiplier() const override;
    void setTempoMultiplier(double multiplier) override;

    muse::Progress loadingProgress() const override;

    void applyProfile(const SoundProfileName& profileName) override;

    void setNotation(notation::INotationPtr notation) override;
    void setIsExportingAudio(bool exporting) override;

    bool canReceiveAction(const muse::actions::ActionCode& code) const override;

private:
    muse::audio::IPlayerPtr currentPlayer() const;

    notation::INotationPlaybackPtr notationPlayback() const;
    notation::INotationPartsPtr masterNotationParts() const;
    notation::INotationSelectionPtr selection() const;
    notation::INotationSelectionRangePtr selectionRange() const;
    notation::INotationInteractionPtr interaction() const;

    uint64_t notationPlaybackKey() const;

    void updateCurrentTempo();

    void seekRawTick(const muse::midi::tick_t tick);
    void seek(const muse::audio::secs_t secs);

    bool isPaused() const;
    bool isLoaded() const;

    bool isLoopEnabled() const;
    bool loopBoundariesSet() const;

    void onNotationChanged();
    void onPartChanged(const engraving::Part* part);

    void onSelectionChanged();
    void seekListSelection();
    void seekRangeSelection();

    void onAudioResourceChanged(const mu::engraving::InstrumentTrackId& trackId, const muse::audio::AudioResourceMeta& oldMeta,
                                const muse::audio::AudioResourceMeta& newMeta);

    bool shouldLoadDrumset(const engraving::InstrumentTrackId& trackId, const muse::audio::AudioResourceMeta& oldMeta,
                           const muse::audio::AudioResourceMeta& newMeta) const;

    void addSoundFlagsIfNeed(const std::vector<engraving::EngravingItem*>& selection);

    void togglePlay();
    void rewind(const muse::actions::ActionData& args);
    void play();
    void pause();
    void stop();
    void resume();

    muse::audio::secs_t playbackStartSecs() const;
    muse::audio::secs_t playbackEndSecs() const;

    muse::audio::secs_t playbackDelay(const muse::secs_t countInDuration) const;

    notation::InstrumentTrackIdSet instrumentTrackIdSetForRangePlayback() const;

    void togglePlayRepeats();
    void togglePlayChordSymbols();
    void toggleAutomaticallyPan();
    void toggleMetronome();
    void toggleCountIn();
    void toggleMidiInput();
    void setMidiUseWrittenPitch(bool useWrittenPitch);
    void toggleLoopPlayback();
    void toggleHearPlaybackWhenEditing();

    void reloadPlaybackCache();

    void openPlaybackSetupDialog();

    void addLoopBoundary(notation::LoopBoundaryType type);
    void addLoopBoundaryToTick(notation::LoopBoundaryType type, int tick);
    void updateLoop();

    void enableLoop();
    void disableLoop();

    void notifyActionCheckedChanged(const muse::actions::ActionCode& actionCode);

    project::IProjectAudioSettingsPtr audioSettings() const;

    void resetCurrentSequence();
    void setupNewCurrentSequence(const muse::audio::TrackSequenceId sequenceId);
    void subscribeOnAudioParamsChanges();
    void setupSequenceTracks();
    void setupSequencePlayer();

    void updateSoloMuteStates();
    void updateAuxMuteStates();

    using TrackAddFinished = std::function<void ()>;

    void addTrack(const engraving::InstrumentTrackId& instrumentTrackId, const TrackAddFinished& onFinished);
    void doAddTrack(const engraving::InstrumentTrackId& instrumentTrackId, const std::string& title, const TrackAddFinished& onFinished);
    void addAuxTrack(muse::audio::aux_channel_idx_t index, const TrackAddFinished& onFinished);

    void setTrackActivity(const engraving::InstrumentTrackId& instrumentTrackId, const bool isActive);
    muse::audio::AudioOutputParams trackOutputParams(const engraving::InstrumentTrackId& instrumentTrackId) const;
    engraving::InstrumentTrackIdSet availableInstrumentTracks() const;
    void removeNonExistingTracks();
    void removeTrack(const engraving::InstrumentTrackId& instrumentTrackId);

    void onTrackNewlyAdded(const engraving::InstrumentTrackId& instrumentTrackId);

    muse::audio::secs_t playedTickToSecs(int tick) const;

    notation::INotationPtr m_notation;
    notation::IMasterNotationPtr m_masterNotation;
    muse::audio::IPlayerPtr m_player;

    muse::async::Notification m_isPlayAllowedChanged;
    muse::async::Notification m_isPlayingChanged;
    muse::async::Notification m_totalPlayTimeChanged;
    muse::async::Notification m_currentTempoChanged;
    muse::async::Channel<muse::audio::secs_t, muse::midi::tick_t> m_currentPlaybackPositionChanged;
    muse::async::Channel<muse::actions::ActionCode> m_actionCheckedChanged;

    muse::audio::TrackSequenceId m_currentSequenceId = -1;

    muse::async::Notification m_currentSequenceIdChanged;
    muse::midi::tick_t m_currentTick = 0;
    notation::Tempo m_currentTempo;

    muse::async::Channel<muse::audio::TrackId> m_trackAdded;
    muse::async::Channel<muse::audio::TrackId> m_trackRemoved;

    muse::async::Channel<muse::audio::aux_channel_idx_t, std::string> m_auxChannelNameChanged;

    InstrumentTrackIdMap m_instrumentTrackIdMap;
    AuxTrackIdMap m_auxTrackIdMap;

    muse::Progress m_loadingProgress;
    size_t m_loadingTrackCount = 0;

    bool m_isExportingAudio = false;
    bool m_isRangeSelection = false;

    DrumsetLoader m_drumsetLoader;

    bool m_measureInputLag = false;
};
}

#endif // MU_PLAYBACK_PLAYBACKCONTROLLER_H
