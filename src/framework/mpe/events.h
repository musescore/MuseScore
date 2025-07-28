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

#ifndef MUSE_MPE_EVENTS_H
#define MUSE_MPE_EVENTS_H

#include <variant>
#include <vector>

#include "async/channel.h"
#include "realfn.h"
#include "global/types/number.h"
#include "types/flags.h"
#include "types/number.h"

#include "mpetypes.h"
#include "playbacksetupdata.h"

namespace muse::mpe {
struct ArrangementContext
{
    timestamp_t nominalTimestamp = 0;
    timestamp_t actualTimestamp = 0;
    duration_t nominalDuration = 0;
    duration_t actualDuration = 0;
    voice_layer_idx_t voiceLayerIndex = 0;
    staff_layer_idx_t staffLayerIndex = 0;
    double bps = 0.0;

    bool hasStart() const
    {
        return actualDuration > 0;
    }

    bool hasEnd() const
    {
        return actualDuration != mpe::INFINITE_DURATION;
    }

    bool operator==(const ArrangementContext& other) const
    {
        return nominalTimestamp == other.nominalTimestamp
               && actualTimestamp == other.actualTimestamp
               && nominalDuration == other.nominalDuration
               && actualDuration == other.actualDuration
               && voiceLayerIndex == other.voiceLayerIndex
               && staffLayerIndex == other.staffLayerIndex
               && muse::is_equal(bps, other.bps);
    }
};

struct PitchContext
{
    pitch_level_t nominalPitchLevel = 0;
    PitchCurve pitchCurve;

    bool operator==(const PitchContext& other) const
    {
        return nominalPitchLevel == other.nominalPitchLevel
               && pitchCurve == other.pitchCurve;
    }
};

struct ExpressionContext
{
    ArticulationMap articulations;
    dynamic_level_t nominalDynamicLevel = MIN_DYNAMIC_LEVEL;
    ExpressionCurve expressionCurve;
    std::optional<float> velocityOverride;

    bool operator==(const ExpressionContext& other) const
    {
        return articulations == other.articulations
               && nominalDynamicLevel == other.nominalDynamicLevel
               && expressionCurve == other.expressionCurve
               && velocityOverride == velocityOverride;
    }
};

struct NoteEvent
{
    NoteEvent() = default;

    explicit NoteEvent(ArrangementContext&& arrangementCtx,
                       PitchContext&& pitchCtx,
                       ExpressionContext&& expressionCtx)
        : m_arrangementCtx(arrangementCtx),
        m_pitchCtx(pitchCtx),
        m_expressionCtx(expressionCtx)
    {
    }

    explicit NoteEvent(const timestamp_t nominalTimestamp,
                       const duration_t nominalDuration,
                       const voice_layer_idx_t voiceIdx,
                       const staff_layer_idx_t staffIdx,
                       const pitch_level_t nominalPitchLevel,
                       const dynamic_level_t nominalDynamicLevel,
                       const ArticulationMap& articulationsApplied,
                       const double bps,
                       const float requiredVelocityFraction = 0.f,
                       const PitchCurve& requiredPitchCurve = {})
    {
        m_arrangementCtx.nominalDuration = nominalDuration;
        m_arrangementCtx.nominalTimestamp = nominalTimestamp;
        m_arrangementCtx.voiceLayerIndex = voiceIdx;
        m_arrangementCtx.staffLayerIndex = staffIdx;
        m_arrangementCtx.bps = bps;

        m_pitchCtx.nominalPitchLevel = nominalPitchLevel;

        m_expressionCtx.articulations = articulationsApplied;
        m_expressionCtx.nominalDynamicLevel = nominalDynamicLevel;

        setUp(requiredVelocityFraction, requiredPitchCurve);
    }

    const ArrangementContext& arrangementCtx() const
    {
        return m_arrangementCtx;
    }

    const PitchContext& pitchCtx() const
    {
        return m_pitchCtx;
    }

    const ExpressionContext& expressionCtx() const
    {
        return m_expressionCtx;
    }

    bool operator==(const NoteEvent& other) const
    {
        return m_arrangementCtx == other.m_arrangementCtx
               && m_pitchCtx == other.m_pitchCtx
               && m_expressionCtx == other.m_expressionCtx;
    }

private:

    template<typename T>
    inline T mult(T v, float f)
    {
        return static_cast<T>(static_cast<float>(v) * f);
    }

    void setUp(const float requiredVelocityFraction, const PitchCurve& requiredPitchCurve)
    {
        calculateActualDuration(m_expressionCtx.articulations);
        calculateActualTimestamp(m_expressionCtx.articulations);

        if (requiredPitchCurve.empty()) {
            calculatePitchCurve(m_expressionCtx.articulations);
        } else {
            m_pitchCtx.pitchCurve = requiredPitchCurve;
        }

        calculateExpressionCurve(m_expressionCtx.articulations, requiredVelocityFraction);
    }

    void calculateActualTimestamp(const ArticulationMap& articulationsApplied)
    {
        m_arrangementCtx.actualTimestamp = m_arrangementCtx.nominalTimestamp;

        if (articulationsApplied.empty()) {
            return;
        }

        timestamp_t timestampOffsetValue = mult(m_arrangementCtx.nominalDuration,
                                                percentageToFactor(articulationsApplied.averageTimestampOffset()));

        m_arrangementCtx.actualTimestamp += timestampOffsetValue;
    }

    void calculateActualDuration(const ArticulationMap& articulationsApplied)
    {
        m_arrangementCtx.actualDuration = m_arrangementCtx.nominalDuration;

        if (articulationsApplied.empty() || m_arrangementCtx.nominalDuration == INFINITE_DURATION) {
            return;
        }

        m_arrangementCtx.actualDuration = mult(m_arrangementCtx.actualDuration,
                                               percentageToFactor(articulationsApplied.averageDurationFactor()));
    }

    void calculatePitchCurve(const ArticulationMap& articulationsApplied)
    {
        m_pitchCtx.pitchCurve = articulationsApplied.averagePitchOffsetMap();

        pitch_level_t averagePitchRange = articulationsApplied.averagePitchRange();
        if (averagePitchRange == 0 || averagePitchRange == PITCH_LEVEL_STEP) {
            return;
        }

        float ratio = static_cast<float>(averagePitchRange) / static_cast<float>(PITCH_LEVEL_STEP);
        float patternUnitRatio = PITCH_LEVEL_STEP / static_cast<float>(ONE_PERCENT);

        for (auto& pair : m_pitchCtx.pitchCurve) {
            pair.second = static_cast<pitch_level_t>(RealRound(static_cast<float>(pair.second) * ratio * patternUnitRatio, 0));
        }
    }

    void calculateExpressionCurve(const ArticulationMap& articulationsApplied, const float requiredVelocityFraction)
    {
        m_expressionCtx.expressionCurve = articulationsApplied.averageDynamicOffsetMap();

        if (!RealIsNull(requiredVelocityFraction)) {
            m_expressionCtx.velocityOverride = requiredVelocityFraction;
        }

        dynamic_level_t articulationDynamicLevel = articulationsApplied.averageMaxAmplitudeLevel();
        dynamic_level_t nominalDynamicLevel = m_expressionCtx.nominalDynamicLevel;

        constexpr dynamic_level_t naturalDynamicLevel = dynamicLevelFromType(DynamicType::Natural);

        float dynamicAmplifyFactor = static_cast<float>(articulationDynamicLevel - naturalDynamicLevel) / DYNAMIC_LEVEL_STEP;

        dynamic_level_t amplificationDiff = mult(std::max(articulationsApplied.averageDynamicRange(), DYNAMIC_LEVEL_STEP),
                                                 dynamicAmplifyFactor);

        dynamic_level_t actualDynamicLevel = nominalDynamicLevel + amplificationDiff;

        if (actualDynamicLevel == articulationDynamicLevel) {
            return;
        }

        float ratio = static_cast<float>(actualDynamicLevel) / static_cast<float>(articulationDynamicLevel);

        for (auto& pair : m_expressionCtx.expressionCurve) {
            pair.second = static_cast<dynamic_level_t>(RealRound(pair.second * ratio, 0));
        }
    }

    ArrangementContext m_arrangementCtx;
    PitchContext m_pitchCtx;
    ExpressionContext m_expressionCtx;
};

struct RestEvent
{
    RestEvent() = default;

    explicit RestEvent(ArrangementContext&& arrangement)
        : m_arrangementCtx(arrangement) {}

    explicit RestEvent(const timestamp_t nominalTimestamp,
                       const duration_t nominalDuration,
                       const voice_layer_idx_t voiceIdx)
    {
        m_arrangementCtx.nominalTimestamp = nominalTimestamp;
        m_arrangementCtx.actualTimestamp = nominalTimestamp;
        m_arrangementCtx.nominalDuration = nominalDuration;
        m_arrangementCtx.actualDuration = nominalDuration;
        m_arrangementCtx.voiceLayerIndex = voiceIdx;
    }

    const ArrangementContext& arrangementCtx() const
    {
        return m_arrangementCtx;
    }

    bool operator==(const RestEvent& other) const
    {
        return m_arrangementCtx == other.m_arrangementCtx;
    }

private:
    ArrangementContext m_arrangementCtx;
};

struct ControllerChangeEvent {
    enum Type : signed char {
        Undefined = -1,
        Modulation,
        SustainPedalOnOff,
        PitchBend,
    };

    using Value = muse::number_t<float>;

    Type type = Undefined;
    Value val; // [0;1]
    layer_idx_t layerIdx = 0;

    bool operator==(const ControllerChangeEvent& e) const
    {
        return type == e.type && val == e.val && layerIdx == e.layerIdx;
    }
};

using ControllerChangeEventList = std::vector<ControllerChangeEvent>;

struct TextArticulationEvent {
    enum FlagType : unsigned char {
        NoFlags = 0,
        StartsAtPlaybackPosition,
    };

    String text;
    layer_idx_t layerIdx = 0;
    Flags<FlagType> flags;

    bool operator==(const TextArticulationEvent& e) const
    {
        return text == e.text && layerIdx == e.layerIdx && flags == e.flags;
    }
};

using TextArticulationEventList = std::vector<TextArticulationEvent>;

struct SoundPresetChangeEvent {
    String code;
    layer_idx_t layerIdx = 0;

    bool operator==(const SoundPresetChangeEvent& e) const
    {
        return code == e.code && layerIdx == e.layerIdx;
    }
};

using SoundPresetChangeEventList = std::vector<SoundPresetChangeEvent>;

struct SyllableEvent {
    enum FlagType : unsigned char {
        NoFlags = 0,
        StartsAtPlaybackPosition,
        HyphenedToNext,
    };

    String text;
    layer_idx_t layerIdx = 0;
    Flags<FlagType> flags;

    bool operator==(const SyllableEvent& e) const
    {
        return text == e.text && layerIdx == e.layerIdx && flags == e.flags;
    }
};

using SyllableEventList = std::vector<SyllableEvent>;

using PlaybackEvent = std::variant<std::monostate, NoteEvent,
                                   RestEvent,
                                   TextArticulationEvent,
                                   SoundPresetChangeEvent,
                                   SyllableEvent,
                                   ControllerChangeEvent>;

using PlaybackEventList = std::vector<PlaybackEvent>;
using PlaybackEventsMap = std::map<timestamp_t, PlaybackEventList>;

using DynamicLevelMap = std::map<timestamp_t, dynamic_level_t>;
using DynamicLevelLayers = std::map<layer_idx_t, DynamicLevelMap>;

using MainStreamChanges = async::Channel<PlaybackEventsMap, DynamicLevelLayers>;
using OffStreamChanges = async::Channel<PlaybackEventsMap, DynamicLevelLayers, bool /*flushOffstream*/>;

struct PlaybackData {
    PlaybackEventsMap originEvents;
    PlaybackSetupData setupData;
    DynamicLevelLayers dynamics;

    MainStreamChanges mainStream;
    OffStreamChanges offStream;

    bool operator==(const PlaybackData& other) const
    {
        return originEvents == other.originEvents
               && setupData == other.setupData
               && dynamics == other.dynamics;
    }

    bool isValid() const
    {
        return setupData.isValid();
    }
};
}

#endif // MUSE_MPE_EVENTS_H
