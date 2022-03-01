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

#include "realfn.h"
#include "midi/miditypes.h"
#include "mpe/events.h"
#include "io/path.h"
#include "io/device.h"
#include "async/channel.h"

namespace mu::audio {
using msecs_t = int64_t;
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
using AudioUnitConfig = std::map<std::string, std::string>;

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
    AudioUnitConfig configuration;
    bool active = false;

    bool operator ==(const AudioFxParams& other) const
    {
        return resourceMeta == other.resourceMeta
               && active == other.active
               && chainOrder == other.chainOrder
               && categories == other.categories
               && configuration == other.configuration;
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
    volume_db_t volume = 0.f;
    balance_t balance = 0.f;
    bool muted = false;
    bool solo = false;

    bool operator ==(const AudioOutputParams& other) const
    {
        return fxChain == other.fxChain
               && volume == other.volume
               && balance == other.balance
               && muted == other.muted
               && solo == other.solo;
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
    AudioUnitConfig configuration;

    bool isValid() const
    {
        return type() != AudioSourceType::Undefined
               && resourceMeta.isValid();
    }

    bool operator ==(const AudioSourceParams& other) const
    {
        return type() == other.type()
               && resourceMeta == other.resourceMeta
               && configuration == other.configuration;
    }
};

using AudioInputParams = AudioSourceParams;

struct AudioParams {
    AudioInputParams in;
    AudioOutputParams out;
};

struct AudioSignalVal {
    float amplitude = 0.f;
    volume_dbfs_t pressure = 0.f;
};

using AudioSignalChanges = async::Channel<audioch_t, AudioSignalVal>;

struct AudioSignalsNotifier {
    void updateSignalValues(const audioch_t audioChNumber, const float newAmplitude, const volume_dbfs_t newPressure)
    {
        AudioSignalVal& signalVal = m_signalValuesMap[audioChNumber];

        volume_dbfs_t validatedPressure = std::max(newPressure, MINIMUM_OPERABLE_DBFS_LEVEL);

        if (RealIsEqual(signalVal.amplitude, newAmplitude)
            && RealIsEqual(signalVal.pressure, validatedPressure)) {
            return;
        }

        if (std::abs(signalVal.amplitude - newAmplitude) < AMPLITUDE_MINIMAL_VALUABLE_DIFF
            && std::abs(signalVal.pressure - newPressure) < PRESSURE_MINIMAL_VALUABLE_DIFF) {
            return;
        }

        signalVal.amplitude = newAmplitude;
        signalVal.pressure = validatedPressure;

        audioSignalChanges.send(audioChNumber, signalVal);
    }

    AudioSignalChanges audioSignalChanges;

private:
    static constexpr float AMPLITUDE_MINIMAL_VALUABLE_DIFF = 0.01f;
    static constexpr volume_dbfs_t PRESSURE_MINIMAL_VALUABLE_DIFF = 1.f;
    static constexpr volume_dbfs_t MINIMUM_OPERABLE_DBFS_LEVEL = -100.f;

    std::map<audioch_t, AudioSignalVal> m_signalValuesMap;
};

using PlaybackData = std::variant<mpe::PlaybackData, io::Device*>;

enum class PlaybackStatus {
    Stopped = 0,
    Paused,
    Running
};
}

#endif // MU_AUDIO_AUDIOTYPES_H
