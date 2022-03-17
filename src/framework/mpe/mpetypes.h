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

#ifndef MU_MPE_MPETYPES_H
#define MU_MPE_MPETYPES_H

#include <stdint.h>
#include <math.h>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <vector>

#include "log.h"
#include "sharedhashmap.h"
#include "sharedmap.h"

#include "soundid.h"

#ifdef C
#undef C
#endif

namespace mu::mpe {
// common
using msecs_t = int64_t;
using percentage_t = int_fast16_t;
constexpr percentage_t ONE_PERCENT = 100;
constexpr percentage_t HUNDRED_PERCENT = ONE_PERCENT * 100;
constexpr percentage_t TEN_PERCENT = ONE_PERCENT * 10;

constexpr inline float percentageToFactor(const percentage_t percents)
{
    return percents / static_cast<float>(HUNDRED_PERCENT);
}

constexpr inline percentage_t percentageFromFactor(const float factor)
{
    return factor * HUNDRED_PERCENT;
}

// Arrangement
using timestamp_t = msecs_t;
using duration_t = msecs_t;
using duration_percentage_t = percentage_t;
using voice_layer_idx_t = uint_fast8_t;

constexpr inline duration_percentage_t occupiedPercentage(const timestamp_t timestamp,
                                                          const duration_t overallDuration)
{
    return percentageFromFactor(timestamp / static_cast<float>(overallDuration));
}

template<typename T>
struct ValuesCurve : public SharedMap<duration_percentage_t, T>
{
    T maxAmplitudeLevel() const
    {
        const auto& max = std::max_element(this->cbegin(), this->cend(), [](const auto& f, const auto& s) {
            return std::abs(f.second) < std::abs(s.second);
        });

        if (max == this->cend()) {
            return 0;
        }

        return max->second;
    }
};

// Pitch
enum class PitchClass {
    Undefined = -1,
    C = 0,
    C_sharp = 1,
    D = 2,
    D_sharp= 3,
    E = 4,
    F = 5,
    F_sharp = 6,
    G = 7,
    G_sharp = 8,
    A = 9,
    A_sharp = 10,
    B = 11,

    Last
};

using octave_t = uint_fast8_t;
using pitch_level_t = percentage_t;
using PitchCurve = ValuesCurve<pitch_level_t>;

constexpr size_t EXPECTED_SIZE = (HUNDRED_PERCENT / TEN_PERCENT) + 1;

constexpr pitch_level_t PITCH_LEVEL_STEP = 50;
constexpr pitch_level_t MAX_PITCH_LEVEL = HUNDRED_PERCENT;
constexpr octave_t MAX_SUPPORTED_OCTAVE = 17; // 0 - 17

constexpr inline pitch_level_t pitchLevel(const PitchClass pitchClass, const octave_t octave)
{
    return (PITCH_LEVEL_STEP * (static_cast<int>(PitchClass::Last)) * octave) + (PITCH_LEVEL_STEP * static_cast<int>(pitchClass));
}

constexpr inline pitch_level_t pitchLevelDiff(const PitchClass fClass, const octave_t fOctave,
                                              const PitchClass sClass, const octave_t sOctave)
{
    return pitchLevel(fClass, fOctave) - pitchLevel(sClass, sOctave);
}

constexpr inline int pitchStepsCount(const pitch_level_t pitchRange)
{
    return (pitchRange / PITCH_LEVEL_STEP) + 1;
}

// Expression
using ArticulationFamily = mpe::SoundCategory;

enum class ArticulationType {
    Undefined = -1,

    // single note articulations
    Standard,
    Staccato,
    Staccatissimo,
    Tenuto,
    Marcato,
    Accent,
    SoftAccent,

    LaissezVibrer,

    Subito,

    FadeIn,
    FadeOut,

    Harmonic,
    Mute,
    Open,
    Pizzicato,
    SnapPizzicato,
    RandomPizzicato,
    UpBow,
    DownBow,
    Detache,
    Martele,
    Jete,
    ColLegno,
    SulPont,
    SulTasto,

    GhostNote,
    CrossNote,
    CircleNote,
    TriangleNote,
    DiamondNote,

    Fall,
    QuickFall,
    Doit,
    Plop,
    Scoop,
    Bend,
    SlideOutDown,
    SlideOutUp,
    SlideInAbove,
    SlideInBelow,
    VolumeSwell,

    // multi-note articulations
    Crescendo,
    Decrescendo,
    DiscreteGlissando,
    ContinuousGlissando,
    Legato,
    Pedal,
    Arpeggio,
    ArpeggioUp,
    ArpeggioDown,
    ArpeggioStraightUp,
    ArpeggioStraightDown,

    Vibrato,
    WideVibrato,
    MoltoVibrato,
    SenzaVibrato,

    Tremolo8th,
    Tremolo16th,
    Tremolo32nd,
    Tremolo64th,

    Trill,
    TrillBaroque,
    UpperMordent,
    LowerMordent,
    PrallMordent,
    MordentWithUpperPrefix,
    UpMordent,
    DownMordent,
    Tremblement,
    UpPrall,
    PrallUp,
    PrallDown,
    LinePrall,
    Slide,
    Turn,
    InvertedTurn,

    PreAppoggiatura,
    PostAppoggiatura,
    Acciaccatura,

    TremoloBar,
    Distortion,
    Overdrive,
    Slap,
    Pop,

    Last
};

using ArticulationTypeSet = std::unordered_set<ArticulationType>;

inline bool isSingleNoteArticulation(const ArticulationType type)
{
    static std::set<ArticulationType> singleNoteTypes = {
        ArticulationType::Standard, ArticulationType::Staccato, ArticulationType::Staccatissimo,
        ArticulationType::Tenuto, ArticulationType::Marcato, ArticulationType::Accent,
        ArticulationType::SoftAccent, ArticulationType::LaissezVibrer,
        ArticulationType::Subito, ArticulationType::FadeIn, ArticulationType::FadeOut,
        ArticulationType::Harmonic, ArticulationType::Mute, ArticulationType::Open,
        ArticulationType::Pizzicato, ArticulationType::SnapPizzicato, ArticulationType::RandomPizzicato,
        ArticulationType::UpBow, ArticulationType::DownBow, ArticulationType::Detache,
        ArticulationType::Martele, ArticulationType::Jete, ArticulationType::GhostNote,
        ArticulationType::CrossNote, ArticulationType::CircleNote, ArticulationType::TriangleNote,
        ArticulationType::DiamondNote, ArticulationType::Fall, ArticulationType::QuickFall,
        ArticulationType::Doit, ArticulationType::Plop, ArticulationType::Scoop,
        ArticulationType::Bend, ArticulationType::SlideOutDown, ArticulationType::SlideOutUp,
        ArticulationType::SlideInAbove, ArticulationType::SlideInBelow, ArticulationType::VolumeSwell
    };

    return singleNoteTypes.find(type) != singleNoteTypes.cend();
}

inline bool isMultiNoteArticulation(const ArticulationType type)
{
    return !isSingleNoteArticulation(type);
}

using dynamic_level_t = percentage_t;
constexpr dynamic_level_t MAX_DYNAMIC_LEVEL = HUNDRED_PERCENT;
constexpr dynamic_level_t MIN_DYNAMIC_LEVEL = 0;
constexpr dynamic_level_t DYNAMIC_LEVEL_STEP = 5 * ONE_PERCENT;

enum class DynamicType {
    Undefined = -1,
    ppppppppp = MIN_DYNAMIC_LEVEL,
    pppppppp = 5 * ONE_PERCENT,
    ppppppp = 10 * ONE_PERCENT,
    pppppp = 15 * ONE_PERCENT,
    ppppp = 20 * ONE_PERCENT,
    pppp = 25 * ONE_PERCENT,
    ppp = 30 * ONE_PERCENT,
    pp = 35 * ONE_PERCENT,
    p = 40 * ONE_PERCENT,
    mp = 45 * ONE_PERCENT,
    Natural = 50 * ONE_PERCENT,
    mf = 55 * ONE_PERCENT,
    f = 60 * ONE_PERCENT,
    ff = 65 * ONE_PERCENT,
    fff = 70 * ONE_PERCENT,
    ffff = 75 * ONE_PERCENT,
    fffff = 80 * ONE_PERCENT,
    ffffff = 85 * ONE_PERCENT,
    fffffff = 90 * ONE_PERCENT,
    ffffffff = 95 * ONE_PERCENT,
    fffffffff = MAX_DYNAMIC_LEVEL,
    Last
};

using DynamicTypeList = std::vector<DynamicType>;

inline DynamicType approximateDynamicType(const dynamic_level_t dynamicLevel)
{
    if (dynamicLevel < MIN_DYNAMIC_LEVEL) {
        return DynamicType::ppppppppp;
    }

    if (dynamicLevel > MAX_DYNAMIC_LEVEL) {
        return DynamicType::fffffffff;
    }

    return static_cast<DynamicType>(dynamicLevel);
}

constexpr inline dynamic_level_t dynamicLevelFromType(const DynamicType type)
{
    if (type == DynamicType::Undefined) {
        return MIN_DYNAMIC_LEVEL;
    }

    return static_cast<dynamic_level_t>(type);
}

struct ArrangementPattern
{
    ArrangementPattern() = default;

    ArrangementPattern(duration_percentage_t _durationFactor, duration_percentage_t _timestampOffset)
        : durationFactor(_durationFactor), timestampOffset(_timestampOffset)
    {
    }

    duration_percentage_t durationFactor = 0;
    duration_percentage_t timestampOffset = 0;

    bool operator==(const ArrangementPattern& other) const
    {
        return durationFactor == other.durationFactor
               && timestampOffset == other.timestampOffset;
    }
};

using ArrangementPatternList = std::vector<ArrangementPattern>;

struct PitchPattern
{
    using PitchOffsetMap = PitchCurve;

    PitchPattern() = default;

    PitchPattern(size_t size, percentage_t step, pitch_level_t defaultValue)
    {
        for (size_t i = 0; i < size; ++i) {
            pitchOffsetMap.emplace(step * static_cast<int>(i), defaultValue);
        }
    }

    PitchOffsetMap pitchOffsetMap;

    pitch_level_t maxAmplitudeLevel() const
    {
        return pitchOffsetMap.maxAmplitudeLevel();
    }

    bool operator==(const PitchPattern& other) const
    {
        return pitchOffsetMap == other.pitchOffsetMap;
    }
};

using PitchPatternList = std::vector<PitchPattern>;

using ExpressionCurve = ValuesCurve<dynamic_level_t>;

struct ExpressionPattern
{
    using DynamicOffsetMap = ExpressionCurve;

    ExpressionPattern() = default;

    ExpressionPattern(size_t size, percentage_t step, dynamic_level_t defaultValue)
    {
        for (size_t i = 0; i < size; ++i) {
            dynamicOffsetMap.emplace(step * static_cast<int>(i), defaultValue);
        }
    }

    ExpressionCurve dynamicOffsetMap;

    dynamic_level_t maxAmplitudeLevel() const
    {
        return dynamicOffsetMap.maxAmplitudeLevel();
    }

    bool operator==(const ExpressionPattern& other) const
    {
        return dynamicOffsetMap == other.dynamicOffsetMap;
    }
};

using ExpressionPatternList = std::vector<ExpressionPattern>;

struct ArticulationPatternSegment
{
    ArticulationPatternSegment() = default;

    ArticulationPatternSegment(ArrangementPattern&& arrangement, PitchPattern&& pitch, ExpressionPattern&& expression)
        : arrangementPattern(arrangement), pitchPattern(pitch), expressionPattern(expression)
    {}

    ArrangementPattern arrangementPattern;
    PitchPattern pitchPattern;
    ExpressionPattern expressionPattern;

    bool operator==(const ArticulationPatternSegment& other) const
    {
        return arrangementPattern == other.arrangementPattern
               && pitchPattern == other.pitchPattern
               && expressionPattern == other.expressionPattern;
    }
};

using ArticulationPattern = SharedMap<duration_percentage_t, ArticulationPatternSegment>;

struct ArticulationsProfile
{
    std::vector<ArticulationFamily> supportedFamilies;

    const ArticulationPattern& pattern(const ArticulationType type) const
    {
        auto search = m_patterns.find(type);

        if (search == m_patterns.cend()) {
            static ArticulationPattern emptyPattern;
            return emptyPattern;
        }

        return search->second;
    }

    void setPattern(const ArticulationType type, const ArticulationPattern& scope)
    {
        m_patterns.insert_or_assign(type, scope);
    }

    void removePattern(const ArticulationType type)
    {
        m_patterns.erase(type);
    }

    const SharedHashMap<ArticulationType, ArticulationPattern>& data() const
    {
        return m_patterns;
    }

    bool contains(const ArticulationType type) const
    {
        return m_patterns.find(type) != m_patterns.cend();
    }

    bool isValid() const
    {
        return !m_patterns.empty();
    }

    bool operator==(const ArticulationsProfile& other) const
    {
        return m_patterns == other.m_patterns;
    }

private:
    SharedHashMap<ArticulationType, ArticulationPattern> m_patterns;
};

using ArticulationsProfilePtr = std::shared_ptr<ArticulationsProfile>;

struct ArticulationMeta
{
    ArticulationMeta() = default;

    ArticulationMeta(const ArticulationType _type)
        : type(_type)
    {}

    ArticulationMeta(const ArticulationType _type,
                     const ArticulationPattern& _pattern,
                     const timestamp_t _timestamp,
                     const duration_t _duration,
                     const pitch_level_t overallPitchRange = 0,
                     const dynamic_level_t overallDynamicRange = 0)
        : type(_type),
        pattern(_pattern),
        timestamp(_timestamp),
        overallDuration(_duration),
        overallPitchChangesRange(overallPitchRange),
        overallDynamicChangesRange(overallDynamicRange)
    {}

    ArticulationType type = ArticulationType::Undefined;
    ArticulationPattern pattern;

    timestamp_t timestamp = 0;
    duration_t overallDuration = 0;
    pitch_level_t overallPitchChangesRange = 0;
    dynamic_level_t overallDynamicChangesRange = 0;

    bool operator==(const ArticulationMeta& other) const
    {
        return type == other.type
               && pattern == other.pattern
               && timestamp == other.timestamp
               && overallDuration == other.overallDuration
               && overallPitchChangesRange == other.overallPitchChangesRange
               && overallDynamicChangesRange == other.overallDynamicChangesRange;
    }
};

using ArticulationMetaMap = SharedHashMap<ArticulationType, ArticulationMeta>;

struct ArticulationAppliedData {
    ArticulationAppliedData() = default;

    explicit ArticulationAppliedData(ArticulationMeta&& _meta,
                                     const duration_percentage_t _occupiedFrom,
                                     const duration_percentage_t _occupiedTo)
        : meta(std::move(_meta))
    {
        updateOccupiedRange(_occupiedFrom, _occupiedTo);
    }

    explicit ArticulationAppliedData(const ArticulationMeta& _meta,
                                     const duration_percentage_t _occupiedFrom,
                                     const duration_percentage_t _occupiedTo)
        : meta(_meta)
    {
        updateOccupiedRange(_occupiedFrom, _occupiedTo);
    }

    void updateOccupiedRange(const duration_percentage_t from, const duration_percentage_t to)
    {
        occupiedFrom = from;
        occupiedTo = to;

        if (meta.pattern.empty()) {
            return;
        }

        if (occupiedFrom == 0 && occupiedTo == HUNDRED_PERCENT) {
            appliedPatternSegment = meta.pattern.cbegin()->second;
            occupiedPitchChangesRange = meta.overallPitchChangesRange;
            occupiedDynamicChangesRange = meta.overallDynamicChangesRange;
            return;
        }

        float occupiedFactor = percentageToFactor(occupiedTo);
        occupiedPitchChangesRange = meta.overallPitchChangesRange * occupiedFactor;
        occupiedDynamicChangesRange = meta.overallDynamicChangesRange * occupiedFactor;

        const auto& lower = meta.pattern.lower_bound(occupiedFrom);

        if (lower != meta.pattern.cend()) {
            appliedPatternSegment = lower->second;
        } else {
            appliedPatternSegment = meta.pattern.cbegin()->second;
        }
    }

    ArticulationMeta meta;

    ArticulationPatternSegment appliedPatternSegment;

    duration_percentage_t occupiedFrom = 0;
    duration_percentage_t occupiedTo = HUNDRED_PERCENT;

    pitch_level_t occupiedPitchChangesRange = 0;
    dynamic_level_t occupiedDynamicChangesRange = 0;

    bool operator==(const ArticulationAppliedData& other) const
    {
        return meta == other.meta
               && appliedPatternSegment == other.appliedPatternSegment
               && occupiedFrom == other.occupiedFrom
               && occupiedTo == other.occupiedTo
               && occupiedPitchChangesRange == other.occupiedPitchChangesRange
               && occupiedDynamicChangesRange == other.occupiedDynamicChangesRange;
    }
};

struct ArticulationMap : public SharedHashMap<ArticulationType, ArticulationAppliedData>
{
    void updateOccupiedRange(const ArticulationType type, const duration_percentage_t occupiedFrom, const duration_percentage_t occupiedTo)
    {
        if (!contains(type)) {
            return;
        }

        ArticulationAppliedData& data = at(type);
        data.updateOccupiedRange(occupiedFrom, occupiedTo);

        if (occupiedFrom == 0 && occupiedTo == HUNDRED_PERCENT) {
            return;
        }

        preCalculateAverageData();
    }

    duration_percentage_t averageDurationFactor() const
    {
        return m_averageDurationFactor;
    }

    duration_percentage_t averageTimestampOffset() const
    {
        return m_averageTimestampOffset;
    }

    const PitchPattern::PitchOffsetMap& averagePitchOffsetMap() const
    {
        return m_averagePitchOffsetMap;
    }

    pitch_level_t averagePitchRange() const
    {
        return m_averagePitchRange;
    }

    dynamic_level_t averageMaxAmplitudeLevel() const
    {
        return m_averageMaxAmplitudeLevel;
    }

    dynamic_level_t averageDynamicRange() const
    {
        return m_averageDynamicRange;
    }

    const ExpressionPattern::DynamicOffsetMap& averageDynamicOffsetMap() const
    {
        return m_averageDynamicOffsetMap;
    }

    void preCalculateAverageData()
    {
        if (empty()) {
            return;
        }

        resetData();

        for (auto it = cbegin(); it != cend(); ++it) {
            sumUpData(it);
        }

        calculateAverage();
    }

private:
    void sumUpOffsets(const ArticulationPatternSegment& segment)
    {
        auto averageDynamicOffsetIt = m_averageDynamicOffsetMap.begin();
        auto averagePitchOffsetIt = m_averagePitchOffsetMap.begin();
        auto segmentDynamicOffsetIt = segment.expressionPattern.dynamicOffsetMap.cbegin();
        auto segmentPitchOffsetIt = segment.pitchPattern.pitchOffsetMap.cbegin();

        while (segmentDynamicOffsetIt != segment.expressionPattern.dynamicOffsetMap.cend()
               && segmentPitchOffsetIt != segment.pitchPattern.pitchOffsetMap.cend()) {
            averageDynamicOffsetIt->second += segmentDynamicOffsetIt->second;
            averagePitchOffsetIt->second += segmentPitchOffsetIt->second;

            ++averageDynamicOffsetIt;
            ++averagePitchOffsetIt;
            ++segmentDynamicOffsetIt;
            ++segmentPitchOffsetIt;
        }
    }

    void resetData()
    {
        m_averageDurationFactor = 0;
        m_averageTimestampOffset = 0;
        m_averageMaxAmplitudeLevel = 0;
        m_averageDynamicRange = 0;
        m_averagePitchRange = 0;

        for (size_t i = 0; i < EXPECTED_SIZE; ++i) {
            m_averagePitchOffsetMap.insert_or_assign(static_cast<int>(i) * TEN_PERCENT, 0);
            m_averageDynamicOffsetMap.insert_or_assign(static_cast<int>(i) * TEN_PERCENT, 0);
        }
    }

    void sumUpData(const const_iterator segmentIt)
    {
        const ArticulationPatternSegment& segment = segmentIt->second.appliedPatternSegment;

        m_averageDurationFactor += segment.arrangementPattern.durationFactor;
        m_averageTimestampOffset += segment.arrangementPattern.timestampOffset;
        m_averageMaxAmplitudeLevel += segment.expressionPattern.maxAmplitudeLevel();
        m_averagePitchRange += segmentIt->second.occupiedPitchChangesRange;
        m_averageDynamicRange += segmentIt->second.occupiedDynamicChangesRange;

        sumUpOffsets(segment);
    }

    void calculateAverage()
    {
        int count = static_cast<int>(size());

        if (count == 1) {
            return;
        }

        int dynamicChangesCount = 0;
        int pitchChangesCount = 0;
        int timestampChangesCount = 0;

        for (auto it = cbegin(); it != cend(); ++it) {
            dynamic_level_t amplitudeDynamicLevel = it->second.appliedPatternSegment.expressionPattern.maxAmplitudeLevel();
            dynamic_level_t dynamicLevelOffset = std::abs(amplitudeDynamicLevel - dynamicLevelFromType(DynamicType::Natural));

            if (dynamicLevelOffset != 0) {
                dynamicChangesCount++;
            }

            if (it->second.appliedPatternSegment.pitchPattern.maxAmplitudeLevel() != 0) {
                pitchChangesCount++;
            }

            if (it->second.appliedPatternSegment.arrangementPattern.timestampOffset != 0) {
                timestampChangesCount++;
            }
        }

        m_averageDurationFactor /= count;

        if (timestampChangesCount > 0) {
            m_averageTimestampOffset /= timestampChangesCount;
        }

        if (dynamicChangesCount > 0) {
            m_averageMaxAmplitudeLevel /= dynamicChangesCount;
            m_averageDynamicRange /= dynamicChangesCount;

            for (auto& pair : m_averageDynamicOffsetMap) {
                pair.second /= dynamicChangesCount;
            }
        }

        if (pitchChangesCount > 0) {
            m_averagePitchRange /= pitchChangesCount;

            for (auto& pair : m_averagePitchOffsetMap) {
                pair.second /= pitchChangesCount;
            }
        }
    }

    duration_percentage_t m_averageDurationFactor = HUNDRED_PERCENT;
    duration_percentage_t m_averageTimestampOffset = 0;
    pitch_level_t m_averagePitchRange = 0;
    dynamic_level_t m_averageMaxAmplitudeLevel = 0;
    dynamic_level_t m_averageDynamicRange = 0;
    PitchPattern::PitchOffsetMap m_averagePitchOffsetMap;
    ExpressionPattern::DynamicOffsetMap m_averageDynamicOffsetMap;
};
}

#endif // MU_MPE_MPETYPES_H
