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
#include <set>
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

using AudioSourceName = std::string;
using AudioResourceId = std::string;
using AudioResourceIdList = std::vector<AudioResourceId>;
using AudioResourceVendor = std::string;

enum class AudioResourceType {
    Undefined = -1,
    FluidSoundfont,
    VstPlugin
};

struct AudioResourceMeta {
    AudioResourceId id;
    AudioResourceType type = AudioResourceType::Undefined;
    AudioResourceVendor vendor;

    bool hasNativeEditorSupport = false;

    bool isValid() const
    {
        return !id.empty()
               && !vendor.empty()
               && type != AudioResourceType::Undefined;
    }

    bool operator==(const AudioResourceMeta& other) const
    {
        return id == other.id
               && vendor == other.vendor
               && type == other.type
               && hasNativeEditorSupport == other.hasNativeEditorSupport;
    }

    bool operator<(const AudioResourceMeta& other) const
    {
        return id < other.id
               && type < other.type
               && vendor < other.vendor;
    }
};

using AudioResourceMetaList = std::vector<AudioResourceMeta>;

enum class AudioFxType {
    Undefined = -1,
    VstFx
};

enum class AudioFxCategory {
    Undefined = -1,
    FxEqualizer,
    FxAnalyzer,
    FxDelay,
    FxDistortion,
    FxDynamics,
    FxFilter,
    FxGenerator,
    FxMastering,
    FxModulation,
    FxPitchShift,
    FxRestoration,
    FxReverb,
    FxSurround,
    FxTools
};

using AudioFxCategories = std::set<AudioFxCategory>;

using AudioFxChainOrder = int8_t;

struct AudioFxParams {
    AudioFxType type() const
    {
        switch (resourceMeta.type) {
        case AudioResourceType::VstPlugin: return AudioFxType::VstFx;
        default: return AudioFxType::Undefined;
        }
    }

    AudioFxCategories categories;
    AudioFxChainOrder chainOrder = -1;
    AudioResourceMeta resourceMeta;
    bool active = false;

    bool operator ==(const AudioFxParams& other) const
    {
        return resourceMeta == other.resourceMeta
               && active == other.active
               && chainOrder == other.chainOrder
               && categories == other.categories;
    }

    bool operator<(const AudioFxParams& other) const
    {
        return resourceMeta < other.resourceMeta
               && chainOrder < other.chainOrder;
    }

    bool isValid() const
    {
        return resourceMeta.isValid();
    }
};

using AudioFxChain = std::map<AudioFxChainOrder, AudioFxParams>;

struct AudioOutputParams {
    AudioFxChain fxChain;
    volume_db_t volume = 1.f;
    balance_t balance = 0.f;
    bool muted = false;

    bool operator ==(const AudioOutputParams& other) const
    {
        return fxChain == other.fxChain
               && volume == other.volume
               && balance == other.balance
               && muted == other.muted;
    }
};

enum class AudioSourceType {
    Undefined = -1,
    Fluid,
    Vsti
};

struct AudioSourceParams {
    AudioSourceType type() const
    {
        switch (resourceMeta.type) {
        case AudioResourceType::FluidSoundfont: return AudioSourceType::Fluid;
        case AudioResourceType::VstPlugin: return AudioSourceType::Vsti;
        default: return AudioSourceType::Undefined;
        }
    }

    AudioResourceMeta resourceMeta;

    bool isValid() const
    {
        return type() != AudioSourceType::Undefined
               && resourceMeta.isValid();
    }

    bool operator ==(const AudioSourceParams& other) const
    {
        return type() == other.type()
               && resourceMeta == other.resourceMeta;
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
