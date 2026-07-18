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

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "rcommand/commandtypes.h"
#include "rcommand/icommanddispatcher.h"
#include "rcommand/commandable.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "engraving/types/types.h"
#include "notation/inotationconfiguration.h"
#include "notation/inotationplayback.h"
#include "audio/main/iplayer.h"
#include "audio/main/iplayback.h"
#include "audio/common/audiotypes.h"
#include "interactive/iinteractive.h"
#include "tours/itoursservice.h"

#include "drumsetloader.h"

#include "../iplaybackcontroller.h"
#include "../iplaybackconfiguration.h"
#include "../isoundprofilesrepository.h"
#include "project/iprojectvideosettings.h"

namespace mu::playback {
class OnlineSoundsController;
class PlaybackController : public IPlaybackController, public muse::actions::Actionable, public muse::async::Asyncable,
    public muse::rcommand::Commandable, public muse::Contextable
{
    muse::GlobalInject<IPlaybackConfiguration> configuration;
    muse::GlobalInject<notation::INotationConfiguration> notationConfiguration;
    muse::ContextInject<ISoundProfilesRepository> profilesRepo = { this };
    muse::ContextInject<muse::audio::IPlayback> playback = { this };
    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::ContextInject<muse::rcommand::ICommandDispatcher> commandsDispatcher = { this };
    muse::ContextInject<context::IGlobalContext> globalContext = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<muse::tours::IToursService> tours = { this };

public:
    PlaybackController(const muse::modularity::ContextPtr& iocCtx);
    ~PlaybackController() override;

    void init();

    bool isPlaybackInited() const override;
    muse::async::Channel<bool> playbackInitedChanged() const override;

    bool isPlayAllowed() const override;
    muse::async::Channel<bool> isPlayAllowedChanged() const override;

    bool isPlaying() const override;
    muse::async::Channel<bool> isPlayingChanged() const override;

    bool isLoopEnabled() const override;
    muse::async::Channel<bool> loopEnabledChanged() const override;

    const InstrumentTrackIdMap& instrumentTrackIdMap() const override;
    const AuxTrackIdMap& auxTrackIdMap() const override;

    muse::async::Channel<muse::audio::TrackId> trackAdded() const override;
    muse::async::Channel<muse::audio::TrackId> trackRemoved() const override;

    std::string auxChannelName(muse::audio::aux_channel_idx_t index) const override;
    muse::async::Channel<muse::audio::aux_channel_idx_t, std::string> auxChannelNameChanged() const override;

    muse::async::Promise<muse::audio::SoundPresetList> availableSoundPresets(
        const engraving::InstrumentTrackId& instrumentTrackId) const override;

    const SoloMuteState& trackSoloMuteState(const engraving::InstrumentTrackId& trackId) const override;
    void setTrackSoloMuteState(const engraving::InstrumentTrackId& trackId, const SoloMuteState& state) override;

    void playElements(const std::vector<const engraving::EngravingItem*>& elements,
                      const PlayParams& params = PlayParams(), bool isMidi = false) override;
    void playNotes(const engraving::NoteValList& notes, engraving::staff_idx_t staffIdx, const engraving::Segment* segment,
                   const PlayParams& params = PlayParams()) override;
    void playMetronome(int tick) override;

    void triggerControllers(const muse::mpe::ControllerChangeEventList& list, engraving::staff_idx_t staffIdx, int tick) override;

    void seekElement(const engraving::EngravingItem* element, bool flushSound = true) override;
    void seekBeat(int measureIndex, int beatIndex, bool flushSound = true) override;

    bool actionChecked(const muse::actions::ActionCode& actionCode) const override;
    muse::async::Channel<muse::actions::ActionCode> actionCheckedChanged() const override;

    muse::secs_t totalPlayTime() const override;
    muse::async::Notification totalPlayTimeChanged() const override;

    const notation::Tempo& currentTempo() const override;
    muse::async::Notification currentTempoChanged() const override;

    engraving::MeasureBeat currentBeat() const override;
    muse::audio::secs_t beatToSecs(int measureIndex, int beatIndex) const override;

    double tempoMultiplier() const override;
    void setTempoMultiplier(double multiplier) override;

    muse::Progress loadingProgress() const override;

    void applyProfile(const SoundProfileName& profileName) override;

    void setNotation(notation::INotationPtr notation) override;
    void setMasterNotation(notation::IMasterNotationPtr masterNotation);

    void setIsExportingAudio(bool exporting) override;

    bool canReceiveAction(const muse::actions::ActionCode& code) const override;

    const std::map<muse::audio::TrackId, muse::audio::AudioResourceMeta>& onlineSounds() const override;
    muse::async::Notification onlineSoundsChanged() const override;
    muse::Progress onlineSoundsProcessingProgress() const override;

private:
    muse::audio::IPlayerPtr currentPlayer() const;

    notation::INotationPlaybackPtr notationPlayback() const;
    notation::INotationPartsPtr masterNotationParts() const;
    notation::INotationSelectionPtr selection() const;
    notation::INotationSelectionRangePtr selectionRange() const;
    notation::INotationInteractionPtr interaction() const;

    uint64_t notationPlaybackKey() const;

    void updateCurrentTempo();

    void seekRawTick(const muse::midi::tick_t tick, const bool flushSound = true);
    void seek(const muse::audio::secs_t secs, const bool flushSound = true);

    bool isPaused() const;
    bool isLoaded() const;

    bool loopBoundariesSet() const;

    void onNotationChanged();
    void onPartChanged(const engraving::Part* part);
    void onPlaybackStatusChanged();

    void onSelectionChanged();
    void seekRangeSelection();

    void onAudioResourceChanged(const muse::audio::TrackId trackId, const mu::engraving::InstrumentTrackId& instrumentTrackId,
                                const muse::audio::AudioResourceMeta& oldMeta, const muse::audio::AudioResourceMeta& newMeta);

    bool shouldLoadDrumset(const engraving::InstrumentTrackId& instrumentTrackId, const muse::audio::AudioResourceMeta& oldMeta,
                           const muse::audio::AudioResourceMeta& newMeta) const;

    void addSoundFlagsIfNeed(const std::vector<engraving::EngravingItem*>& selection);

    muse::Ret togglePlay();
    muse::Ret play(bool showErrors = true);
    muse::Ret playFromSelection(bool showErrors = true);
    muse::Ret pause(bool select = false);
    muse::Ret stop();
    muse::rcommand::Response rewind(const muse::rcommand::Request& request);

    void doRewind(muse::secs_t newPosition);
    void doPlay();
    void doPause(bool select = false);
    void doStop();
    void doResume();

    muse::audio::secs_t playbackStartSecs() const;

    engraving::InstrumentTrackIdSet instrumentTrackIdSetForRangePlayback() const;

    muse::Ret togglePlayRepeats();
    muse::Ret togglePlayChordSymbols();
    muse::Ret toggleAutomaticallyPan();
    muse::Ret toggleMetronome();
    muse::Ret toggleCountIn();
    muse::Ret toggleMidiInput();
    muse::Ret setMidiUseWrittenPitch(bool useWrittenPitch);
    muse::Ret toggleHearPlaybackWhenEditing();

    muse::Ret reloadPlaybackCache();

    muse::Ret showPlaybackSetup();

    muse::Ret toggleLoopPlayback();
    muse::Ret addLoopBoundary(engraving::LoopBoundaryType type);
    void addLoopBoundaryToTick(engraving::LoopBoundaryType type, int tick);
    void updateLoop();

    void enableLoop();
    void disableLoop();

    void notifyActionCheckedChanged(const muse::actions::ActionCode& actionCode);

    project::IProjectAudioSettingsPtr audioSettings() const;
    project::IProjectVideoSettingsPtr videoSettings() const;
    void updateMasterControlParams();

    void resetPlayback();
    void setupPlayback();
    void subscribeOnAudioParamsChanges();
    void setupTracks();
    void setupPlayer();

    void updateSoloMuteStates();
    void updateAuxMuteStates();

    using TrackAddFinished = std::function<void ()>;

    void addTrack(const engraving::InstrumentTrackId& instrumentTrackId, const TrackAddFinished& onFinished);
    void doAddTrack(const engraving::InstrumentTrackId& instrumentTrackId, const std::string& title, const TrackAddFinished& onFinished);
    void addAuxTrack(muse::audio::aux_channel_idx_t index, const TrackAddFinished& onFinished);

    void setTrackActivity(const engraving::InstrumentTrackId& instrumentTrackId, const bool isActive);
    project::AudioOutputParams trackOutputParams(const engraving::InstrumentTrackId& instrumentTrackId) const;
    void removeTrack(const engraving::InstrumentTrackId& instrumentTrackId);

    void onTrackNewlyAdded(const engraving::InstrumentTrackId& instrumentTrackId);

    muse::audio::secs_t playedTickToSecs(int tick) const;

    notation::INotationPtr m_notation;
    notation::IMasterNotationPtr m_masterNotation;
    muse::audio::IPlayerPtr m_player;
    bool m_isPlaybackInited = false;
    muse::async::Channel<bool> m_playbackInited;

    muse::async::Channel<bool> m_isPlayAllowedChanged;
    muse::async::Channel<bool> m_isPlayingChanged;
    muse::async::Channel<bool> m_loopEnabledChanged;
    muse::async::Notification m_totalPlayTimeChanged;
    muse::async::Notification m_currentTempoChanged;
    muse::async::Channel<muse::actions::ActionCode> m_actionCheckedChanged;

    muse::midi::tick_t m_currentTick = 0;
    notation::Tempo m_currentTempo;

    muse::async::Channel<muse::audio::TrackId> m_trackAdded;
    muse::async::Channel<muse::audio::TrackId> m_trackRemoved;

    muse::async::Channel<muse::audio::aux_channel_idx_t, std::string> m_auxChannelNameChanged;

    muse::async::Asyncable m_seqAsyncReceiver; //! HACK - see PlaybackController::setupTracks

    InstrumentTrackIdMap m_instrumentTrackIdMap;
    AuxTrackIdMap m_auxTrackIdMap;

    muse::Progress m_loadingProgress;
    size_t m_loadingTrackCount = 0;

    bool m_isExportingAudio = false;
    bool m_isRangeSelection = false;

    DrumsetLoader m_drumsetLoader;
    std::unique_ptr<OnlineSoundsController> m_onlineSoundsController;

    bool m_measureInputLag = false;
};
}
