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
#include "types/flags.h"

#include "mpetypes.h"
#include "playbacksetupdata.h"

namespace muse::mpe {
struct NoteEvent;
struct RestEvent;
using PlaybackEvent = std::variant<NoteEvent, RestEvent>;
using PlaybackEventList = std::vector<PlaybackEvent>;
using PlaybackEventsMap = std::map<timestamp_t, PlaybackEventList>;

using DynamicLevelMap = std::map<timestamp_t, dynamic_level_t>;
using DynamicLevelLayers = std::map<layer_idx_t, DynamicLevelMap>;

struct PlaybackParam;
using PlaybackParamList = std::vector<PlaybackParam>;
using PlaybackParamMap = std::map<timestamp_t, PlaybackParamList>;
using PlaybackParamLayers = std::map<layer_idx_t, PlaybackParamMap>;

using MainStreamChanges = async::Channel<PlaybackEventsMap, DynamicLevelLayers, PlaybackParamLayers>;
using OffStreamChanges = async::Channel<PlaybackEventsMap, DynamicLevelLayers, PlaybackParamList>;

struct ArrangementContext
{
    timestamp_t nominalTimestamp = 0;
    timestamp_t actualTimestamp = 0;
    duration_t nominalDuration = 0;
    duration_t actualDuration = 0;
    voice_layer_idx_t voiceLayerIndex = 0;
    staff_layer_idx_t staffLayerIndex = 0;
    double bps = 0.0;

    bool operator==(const ArrangementContext& other) const
    {
        return nominalTimestamp == other.nominalTimestamp
               && actualTimestamp == other.actualTimestamp
               && nominalDuration == other.nominalDuration
               && actualDuration == other.actualDuration
               && voiceLayerIndex == other.voiceLayerIndex
               && staffLayerIndex == other.staffLayerIndex
               && bps == other.bps;
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

        if (articulationsApplied.empty()) {
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

struct PlaybackParam {
    enum Type : signed char {
        Undefined = -1,
        SoundPreset,
        PlayingTechnique,
        Syllable,
    };

    enum FlagType : signed char {
        NoFlags = 0,
        IsPersistent,
        HyphenedToNext,
    };

    using Value = String;

    Type type = Undefined;
    Value val;
    Flags<FlagType> flags;

    PlaybackParam(Type t, Value v, const Flags<FlagType>& f = {})
        : type(t), val(std::move(v)), flags(f)
    {
    }

    bool operator==(const PlaybackParam& other) const
    {
        return type == other.type && val == other.val && flags == other.flags;
    }
};

struct PlaybackData {
    PlaybackEventsMap originEvents;
    PlaybackSetupData setupData;
    DynamicLevelLayers dynamics;
    PlaybackParamLayers params;

    MainStreamChanges mainStream;
    OffStreamChanges offStream;

    bool operator==(const PlaybackData& other) const
    {
        return originEvents == other.originEvents
               && setupData == other.setupData
               && dynamics == other.dynamics
               && params == other.params;
    }

    bool isValid() const
    {
        return setupData.isValid();
    }
};
}

#endif // MUSE_MPE_EVENTS_H
