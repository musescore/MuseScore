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
#ifndef MU_PLAYBACK_IPLAYBACKCONTROLLER_H
#define MU_PLAYBACK_IPLAYBACKCONTROLLER_H

#include "modularity/imoduleexport.h"
#include "async/notification.h"
#include "async/channel.h"
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
    virtual async::Notification isPlayAllowedChanged() const = 0;

    virtual bool isPlaying() const = 0;
    virtual async::Notification isPlayingChanged() const = 0;

    virtual void seek(const midi::tick_t tick) = 0;
    virtual void seek(const audio::msecs_t msecs) = 0;
    virtual void reset() = 0;

    virtual async::Notification playbackPositionChanged() const = 0;
    virtual async::Channel<uint32_t> midiTickPlayed() const = 0;
    virtual float playbackPositionInSeconds() const = 0;

    virtual audio::TrackSequenceId currentTrackSequenceId() const = 0;
    virtual async::Notification currentTrackSequenceIdChanged() const = 0;

    using InstrumentTrackIdMap = std::unordered_map<engraving::InstrumentTrackId, audio::TrackId>;
    virtual const InstrumentTrackIdMap& instrumentTrackIdMap() const = 0;

    virtual async::Channel<audio::TrackId, engraving::InstrumentTrackId> trackAdded() const = 0;
    virtual async::Channel<audio::TrackId, engraving::InstrumentTrackId> trackRemoved() const = 0;

    virtual void playElements(const std::vector<const notation::EngravingItem*>& elements) = 0;
    virtual void playMetronome(int tick) = 0;
    virtual void seekElement(const notation::EngravingItem* element) = 0;

    virtual bool actionChecked(const actions::ActionCode& actionCode) const = 0;
    virtual async::Channel<actions::ActionCode> actionCheckedChanged() const = 0;

    virtual QTime totalPlayTime() const = 0;
    virtual async::Notification totalPlayTimeChanged() const = 0;

    virtual notation::Tempo currentTempo() const = 0;
    virtual async::Notification currentTempoChanged() const = 0;

    virtual notation::MeasureBeat currentBeat() const = 0;
    virtual audio::msecs_t beatToMilliseconds(int measureIndex, int beatIndex) const = 0;

    virtual double tempoMultiplier() const = 0;
    virtual void setTempoMultiplier(double multiplier) = 0;

    virtual framework::Progress loadingProgress() const = 0;

    virtual void applyProfile(const SoundProfileName& profileName) = 0;

    virtual void setExportedNotation(notation::INotationPtr notation) = 0;
};
}

#endif // MU_PLAYBACK_IPLAYBACKCONTROLLER_H
