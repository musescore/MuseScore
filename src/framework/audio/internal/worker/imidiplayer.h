/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/

#ifndef MU_AUDIO_IMIDIPLAYER_H
#define MU_AUDIO_IMIDIPLAYER_H

#include <string>
#include <functional>
#include <memory>

#include "modularity/imoduleexport.h"
#include "midi/miditypes.h"
#include "async/notification.h"
#include "async/channel.h"
#include "iplayer.h"

namespace mu::audio {
class IMIDIPlayer : public IPlayer
{
public:
    virtual ~IMIDIPlayer() = default;

    virtual void loadMIDI(const std::shared_ptr<midi::MidiStream>& stream) = 0;
    virtual async::Channel<midi::tick_t> tickPlayed() const = 0;

    virtual float playbackSpeed() const = 0;
    virtual void setPlaybackSpeed(float speed) = 0;

    virtual void setIsTrackMuted(midi::track_t trackIndex, bool mute) = 0;
    virtual void setTrackVolume(midi::track_t trackIndex, float volume) = 0;
    virtual void setTrackBalance(midi::track_t trackIndex, float balance) = 0;
};
}

#endif // MU_AUDIO_IMIDIPLAYER_H
