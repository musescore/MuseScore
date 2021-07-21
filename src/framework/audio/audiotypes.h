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

#ifndef MU_AUDIO_AUDIOTYPES_H
#define MU_AUDIO_AUDIOTYPES_H

#include <variant>
#include <memory>
#include <string>

#include "midi/miditypes.h"
#include "io/path.h"
#include "io/device.h"
#include "async/channel.h"

namespace mu::audio {
using msecs_t = uint64_t;
using samples_t = uint64_t;
using audioch_t = uint8_t;
using volume_db_t = float;
using volume_dbfs_t = float;
using gain_t = float;
using balance_t = float;

using TrackSequenceId = int32_t;
using TrackSequenceIdList = std::vector<TrackSequenceId>;

using TrackId = int32_t;
using TrackIdList = std::vector<TrackId>;
using TrackName = std::string;

using MixerChannelId = int32_t;

using AudioSourceName = std::string;
using AudioResourceName = std::string;

using FxProcessorId = std::string;
using FxProcessorIdList =  std::vector<FxProcessorId>;

enum class AudioFxType {
    Undefined = -1,
    Vst
};

struct AudioFxParams {
    AudioFxType type = AudioFxType::Undefined;
    AudioResourceName resourceName;
    bool active = false;

    bool operator ==(const AudioFxParams& other) const
    {
        return type == other.type
               && resourceName == other.resourceName
               && active == other.active;
    }
};

struct AudioOutputParams {
    AudioFxParams fxParams;
    volume_db_t volume = 1.f;
    balance_t balance = 0.f;
    bool isMuted = false;

    bool operator ==(const AudioOutputParams& other) const
    {
        return fxParams == other.fxParams
               && volume == other.volume
               && balance == other.balance
               && isMuted == other.isMuted;
    }
};

enum class AudioSourceType {
    Undefined = -1,
    Fluid,
    Vsti,
    MuseSampler
};

struct AudioSourceParams {
    AudioSourceType type = AudioSourceType::Undefined;
    AudioResourceName resourceName;

    bool operator ==(const AudioSourceParams& other) const
    {
        return type == other.type
               && resourceName == other.resourceName;
    }
};

using AudioInputParams = AudioSourceParams;

struct AudioParams {
    AudioInputParams in;
    AudioOutputParams out;
};

struct AudioSignalChanges {
    async::Channel<audioch_t, float> amplitudeChanges;
    async::Channel<audioch_t, volume_dbfs_t> pressureChanges;
};

using PlaybackData = std::variant<midi::MidiData, io::Device*>;

enum class PlaybackStatus {
    Stopped = 0,
    Paused,
    Running
};
}

#endif // MU_AUDIO_AUDIOTYPES_H
