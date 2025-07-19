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

#ifndef MUSE_AUDIO_AUDIOTYPES_H
#define MUSE_AUDIO_AUDIOTYPES_H

#include <variant>
#include <set>
#include <string>

#include "global/types/number.h"
#include "global/types/secs.h"
#include "global/types/ratio.h"
#include "global/types/string.h"
#include "global/realfn.h"
#include "global/async/channel.h"
#include "global/io/iodevice.h"
#include "global/progress.h"

#include "mpe/events.h"

namespace muse::audio {
using msecs_t = int64_t;
using secs_t = muse::secs_t;

inline secs_t milisecsToSecs(msecs_t ms) { return secs_t(ms / 1000.0); }
inline secs_t microsecsToSecs(msecs_t us) { return secs_t(us / 1000000.0); }

inline msecs_t secsToMilisecs(secs_t s) { return msecs_t(s * 1000.0); }
inline msecs_t secsToMicrosecs(secs_t s) { return msecs_t(s * 1000000.0); }

using samples_t = uint64_t;
using sample_rate_t = uint64_t;
using audioch_t = uint8_t;
using volume_db_t = db_t;
using volume_dbfs_t = db_t;
using gain_t = float;
using balance_t = float;

using TrackSequenceId = int32_t;
using TrackSequenceIdList = std::vector<TrackSequenceId>;

using TrackId = int32_t;
using TrackIdList = std::vector<TrackId>;
using TrackName = std::string;

using aux_channel_idx_t = uint8_t;

using PlaybackData = std::variant<mpe::PlaybackData, io::IODevice*>;
using PlaybackSetupData = mpe::PlaybackSetupData;

static constexpr TrackId INVALID_TRACK_ID = -1;

static constexpr char DEFAULT_DEVICE_ID[] = "default";

#ifdef Q_OS_WIN
static constexpr size_t MINIMUM_BUFFER_SIZE = 256;
#else
static constexpr size_t MINIMUM_BUFFER_SIZE = 128;
#endif

static constexpr size_t MAXIMUM_BUFFER_SIZE = 4096;

enum class SoundTrackType {
    Undefined = -1,
    MP3,
    OGG,
    FLAC,
    WAV
};

struct SoundTrackFormat {
    SoundTrackType type = SoundTrackType::Undefined;
    sample_rate_t sampleRate = 0;
    samples_t samplesPerChannel = 0;
    audioch_t audioChannelsNumber = 0;
    int bitRate = 0;

    bool operator==(const SoundTrackFormat& other) const
    {
        return type == other.type
               && sampleRate == other.sampleRate
               && audioChannelsNumber == other.audioChannelsNumber
               && samplesPerChannel == other.samplesPerChannel
               && bitRate == other.bitRate;
    }

    bool isValid() const
    {
        return type != SoundTrackType::Undefined
               && sampleRate != 0
               && samplesPerChannel != 0
               && audioChannelsNumber != 0;
    }
};

using AudioSourceName = std::string;
using AudioResourceId = std::string;
using AudioResourceIdList = std::vector<AudioResourceId>;
using AudioResourceVendor = std::string;
using AudioResourceAttributes = std::map<String, String>;
using AudioUnitConfig = std::map<std::string, std::string>;

static const String PLAYBACK_SETUP_DATA_ATTRIBUTE("playbackSetupData");
static const String CATEGORIES_ATTRIBUTE("categories");

enum class AudioResourceType {
    Undefined = -1,
    FluidSoundfont,
    VstPlugin,
    MusePlugin,
    MuseSamplerSoundPack,
    Lv2Plugin,
    AudioUnit,
};

struct AudioResourceMeta {
    AudioResourceId id;
    AudioResourceType type = AudioResourceType::Undefined;
    AudioResourceVendor vendor;
    AudioResourceAttributes attributes;

    const String& attributeVal(const String& key) const
    {
        auto search = attributes.find(key);
        if (search != attributes.cend()) {
            return search->second;
        }

        static String empty;
        return empty;
    }

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
               && hasNativeEditorSupport == other.hasNativeEditorSupport
               && attributes == other.attributes;
    }

    bool operator!=(const AudioResourceMeta& other) const
    {
        return !(*this == other);
    }

    bool operator<(const AudioResourceMeta& other) const
    {
        return id < other.id
               || vendor < other.vendor;
    }
};

using AudioResourceMetaList = std::vector<AudioResourceMeta>;
using AudioResourceMetaSet = std::set<AudioResourceMeta>;

static const AudioResourceId MUSE_REVERB_ID("Muse Reverb");

enum class AudioFxType {
    Undefined = -1,
    VstFx,
    MuseFx,
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
        case AudioResourceType::MusePlugin: return AudioFxType::MuseFx;
        case AudioResourceType::AudioUnit:
        case AudioResourceType::Lv2Plugin:
        case AudioResourceType::FluidSoundfont:
        case AudioResourceType::MuseSamplerSoundPack:
        case AudioResourceType::Undefined: break;
        }

        return AudioFxType::Undefined;
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

    bool operator !=(const AudioFxParams& other) const
    {
        return !(*this == other);
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

struct AuxSendParams {
    gain_t signalAmount = 0.f; // [0; 1]
    bool active = false;

    bool operator ==(const AuxSendParams& other) const
    {
        return RealIsEqual(signalAmount, other.signalAmount) && active == other.active;
    }
};

using AuxSendsParams = std::vector<AuxSendParams>;

struct AudioOutputParams {
    AudioFxChain fxChain;
    volume_db_t volume = 0.f;
    balance_t balance = 0.f;
    AuxSendsParams auxSends;
    bool solo = false;
    bool muted = false;
    bool forceMute = false;

    bool operator ==(const AudioOutputParams& other) const
    {
        return fxChain == other.fxChain
               && RealIsEqual(volume, other.volume)
               && RealIsEqual(balance, other.balance)
               && auxSends == other.auxSends
               && solo == other.solo
               && muted == other.muted
               && forceMute == other.forceMute;
    }
};

enum class AudioSourceType {
    Undefined = -1,
    Fluid,
    Vsti,
    MuseSampler
};

inline AudioSourceType sourceTypeFromResourceType(AudioResourceType type)
{
    switch (type) {
    case AudioResourceType::FluidSoundfont: return AudioSourceType::Fluid;
    case AudioResourceType::VstPlugin: return AudioSourceType::Vsti;
    case AudioResourceType::MuseSamplerSoundPack: return AudioSourceType::MuseSampler;
    case AudioResourceType::AudioUnit:
    case AudioResourceType::Lv2Plugin:
    case AudioResourceType::MusePlugin:
    case AudioResourceType::Undefined: break;
    }

    return AudioSourceType::Undefined;
}

struct AudioSourceParams {
    AudioResourceMeta resourceMeta;
    AudioUnitConfig configuration;

    AudioSourceType type() const
    {
        return sourceTypeFromResourceType(resourceMeta.type);
    }

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

using AudioSignalValuesMap = std::map<audioch_t, AudioSignalVal>;
using AudioSignalChanges = async::Channel<AudioSignalValuesMap>;

static constexpr volume_dbfs_t MINIMUM_OPERABLE_DBFS_LEVEL = volume_dbfs_t::make(-100.f);
struct AudioSignalsNotifier {
    void updateSignalValues(const audioch_t audioChNumber, const float newAmplitude)
    {
        volume_dbfs_t newPressure = (newAmplitude > 0.f) ? volume_dbfs_t(muse::linear_to_db(newAmplitude)) : MINIMUM_OPERABLE_DBFS_LEVEL;
        newPressure = std::max(newPressure, MINIMUM_OPERABLE_DBFS_LEVEL);

        AudioSignalVal& signalVal = m_signalValuesMap[audioChNumber];

        if (muse::is_equal(signalVal.pressure, newPressure)) {
            return;
        }

        if (std::abs(signalVal.pressure - newPressure) < PRESSURE_MINIMAL_VALUABLE_DIFF) {
            return;
        }

        signalVal.amplitude = newAmplitude;
        signalVal.pressure = newPressure;

        m_needNotifyAboutChanges = true;
    }

    void notifyAboutChanges()
    {
        if (m_needNotifyAboutChanges) {
            audioSignalChanges.send(m_signalValuesMap);
            m_needNotifyAboutChanges = false;
        }
    }

    AudioSignalChanges audioSignalChanges;

private:
    static constexpr volume_dbfs_t PRESSURE_MINIMAL_VALUABLE_DIFF = volume_dbfs_t::make(2.5f);

    AudioSignalValuesMap m_signalValuesMap;
    bool m_needNotifyAboutChanges = false;
};

enum class PlaybackStatus {
    Stopped = 0,
    Paused,
    Running
};

using AudioDeviceID = std::string;
struct AudioDevice {
    AudioDeviceID id;
    std::string name;

    bool operator==(const AudioDevice& other) const
    {
        return id == other.id;
    }
};

using AudioDeviceList = std::vector<AudioDevice>;

using SoundPresetAttributes = std::map<String, String>;
static const String PLAYING_TECHNIQUES_ATTRIBUTE(u"playing_techniques");

struct SoundPreset
{
    String code;
    String name;
    bool isDefault = false;
    SoundPresetAttributes attributes;

    bool operator==(const SoundPreset& other) const
    {
        return code == other.code && name == other.name && isDefault == other.isDefault && attributes == other.attributes;
    }

    bool isValid() const
    {
        return !code.empty();
    }
};

using SoundPresetList = std::vector<SoundPreset>;

enum class RenderMode {
    Undefined = -1,
    RealTimeMode,
    IdleMode,
    OfflineMode
};

struct InputProcessingProgress {
    struct ChunkInfo {
        secs_t start = 0.0;
        secs_t end = 0.0;

        bool operator==(const ChunkInfo& c) const
        {
            return start == c.start && end == c.end;
        }
    };

    using ChunkInfoList = std::vector<ChunkInfo>;

    async::Channel<ChunkInfoList> chunksBeingProcessedChannel;
    Progress progress;
};
}

#endif // MUSE_AUDIO_AUDIOTYPES_H
