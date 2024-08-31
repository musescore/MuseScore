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
#ifndef MU_PLAYBACK_IPLAYBACKCONTROLLER_H
#define MU_PLAYBACK_IPLAYBACKCONTROLLER_H

#include "modularity/imoduleinterface.h"
#include "async/notification.h"
#include "async/channel.h"
#include "async/promise.h"
#include "global/progress.h"
#include "notation/inotation.h"
#include "notation/notationtypes.h"
#include "audio/audiotypes.h"
#include "actions/actiontypes.h"

#include "playbacktypes.h"

namespace mu::playback {
class IPlaybackController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPlaybackController)

public:
    virtual ~IPlaybackController() = default;

    virtual bool isPlayAllowed() const = 0;
    virtual muse::async::Notification isPlayAllowedChanged() const = 0;

    virtual bool isPlaying() const = 0;
    virtual muse::async::Notification isPlayingChanged() const = 0;

    virtual void reset() = 0;

    virtual muse::async::Channel<muse::audio::secs_t, muse::midi::tick_t> currentPlaybackPositionChanged() const = 0;

    virtual muse::audio::TrackSequenceId currentTrackSequenceId() const = 0;
    virtual muse::async::Notification currentTrackSequenceIdChanged() const = 0;

    using InstrumentTrackIdMap = std::unordered_map<engraving::InstrumentTrackId, muse::audio::TrackId>;
    virtual const InstrumentTrackIdMap& instrumentTrackIdMap() const = 0;

    using AuxTrackIdMap = std::map<muse::audio::aux_channel_idx_t, muse::audio::TrackId>;
    virtual const AuxTrackIdMap& auxTrackIdMap() const = 0;

    virtual muse::async::Channel<muse::audio::TrackId> trackAdded() const = 0;
    virtual muse::async::Channel<muse::audio::TrackId> trackRemoved() const = 0;

    virtual std::string auxChannelName(muse::audio::aux_channel_idx_t index) const = 0;
    virtual muse::async::Channel<muse::audio::aux_channel_idx_t, std::string> auxChannelNameChanged() const = 0;

    virtual muse::async::Promise<muse::audio::SoundPresetList> availableSoundPresets(const engraving::InstrumentTrackId& instrumentTrackId)
    const
        = 0;

    virtual notation::INotationSoloMuteState::SoloMuteState trackSoloMuteState(const engraving::InstrumentTrackId& trackId) const = 0;
    virtual void setTrackSoloMuteState(const engraving::InstrumentTrackId& trackId,
                                       const notation::INotationSoloMuteState::SoloMuteState& state) = 0;

    virtual void playElements(const std::vector<const notation::EngravingItem*>& elements) = 0;
    virtual void playMetronome(int tick) = 0;
    virtual void seekElement(const notation::EngravingItem* element) = 0;
    virtual void seekBeat(int measureIndex, int beatIndex) = 0;

    virtual bool actionChecked(const muse::actions::ActionCode& actionCode) const = 0;
    virtual muse::async::Channel<muse::actions::ActionCode> actionCheckedChanged() const = 0;

    virtual QTime totalPlayTime() const = 0;
    virtual muse::async::Notification totalPlayTimeChanged() const = 0;

    virtual notation::Tempo currentTempo() const = 0;
    virtual muse::async::Notification currentTempoChanged() const = 0;

    virtual notation::MeasureBeat currentBeat() const = 0;
    virtual muse::audio::secs_t beatToSecs(int measureIndex, int beatIndex) const = 0;

    virtual double tempoMultiplier() const = 0;
    virtual void setTempoMultiplier(double multiplier) = 0;

    virtual muse::Progress loadingProgress() const = 0;

    virtual void applyProfile(const SoundProfileName& profileName) = 0;

    virtual void setNotation(notation::INotationPtr notation) = 0;
    virtual void setIsExportingAudio(bool exporting) = 0;
};
}

#endif // MU_PLAYBACK_IPLAYBACKCONTROLLER_H
