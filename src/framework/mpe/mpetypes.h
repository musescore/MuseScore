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
#include <map>
#include <set>
#include <vector>

#include "log.h"

namespace mu::mpe {
// common
using msecs_t = uint64_t;
using percentage_t = int_fast16_t;
constexpr percentage_t ONE_PERCENT = 100;
constexpr percentage_t HUNDRED_PERCENTS = ONE_PERCENT * 100;
constexpr percentage_t TEN_PERCENTS = ONE_PERCENT * 10;

inline float percentageToFactor(const percentage_t percents)
{
    return percents / static_cast<float>(HUNDRED_PERCENTS);
}

// Arrangement
using timestamp_t = msecs_t;
using duration_t = msecs_t;
using duration_percentage_t = percentage_t;
using voice_layer_idx_t = uint_fast8_t;

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
using PitchCurve = std::map<duration_percentage_t, pitch_level_t>;

constexpr size_t EXPECTED_SIZE = (HUNDRED_PERCENTS / TEN_PERCENTS) + 1;

constexpr octave_t MAX_SUPPORTED_OCTAVE = 12; // 0 - 12
constexpr pitch_level_t MAX_PITCH_LEVEL = HUNDRED_PERCENTS;
constexpr pitch_level_t PITCH_LEVEL_STEP = MAX_PITCH_LEVEL / ((MAX_SUPPORTED_OCTAVE + 1) * static_cast<int>(PitchClass::Last));

inline pitch_level_t pitchLevel(const PitchClass pitchClass, const octave_t octave)
{
    return PITCH_LEVEL_STEP * (static_cast<int>(pitchClass) + 1) * (octave + 1);
}

inline pitch_level_t pitchLevelDiff(const PitchClass fClass, const octave_t fOctave,
                                    const PitchClass sClass, const octave_t sOctave)
{
    return pitchLevel(fClass, fOctave) - pitchLevel(sClass, sOctave);
}

// Expression
enum class ArticulationFamily {
    Undefined = -1,
    KeyboardsArticulation,
    StringsArticulation,
    WindsArticulation,
    PercussionsArticulation
};

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

    VeryShortFermata,
    ShortFermata,
    ShortFermataHenze,
    Fermata,
    LongFermata,
    LongFermataHenze,
    VeryLongFermata,
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
    Glissando,
    Portamento,
    Legato,
    Pedal,
    Arpeggio,
    ArpeggioUp,
    ArpeggioDown,
    ArpeggioBracket,
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

    ShortTrill,
    Trill,
    Mordent,
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

    Appoggiatura,
    Acciaccatura,

    TremoloBar,

    Last
};

inline bool isSingleNoteArticulation(const ArticulationType type)
{
    static std::set<ArticulationType> singleNoteTypes = {
        ArticulationType::Standard, ArticulationType::Staccato, ArticulationType::Staccatissimo,
        ArticulationType::Tenuto, ArticulationType::Marcato, ArticulationType::Accent,
        ArticulationType::SoftAccent, ArticulationType::VeryShortFermata, ArticulationType::ShortFermata,
        ArticulationType::ShortFermataHenze, ArticulationType::Fermata, ArticulationType::LongFermata,
        ArticulationType::LongFermataHenze, ArticulationType::VeryLongFermata, ArticulationType::LaissezVibrer,
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
constexpr dynamic_level_t MAX_DYNAMIC_LEVEL = HUNDRED_PERCENTS;
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
};

using ArrangementPatternList = std::vector<ArrangementPattern>;

struct PitchPattern
{
    using PitchOffsetMap = std::map<duration_percentage_t, pitch_level_t>;

    PitchPattern() = default;

    PitchPattern(size_t size, percentage_t step, pitch_level_t defaultValue)
    {
        for (size_t i = 0; i < size; ++i) {
            pitchOffsetMap.emplace(step * static_cast<int>(i), defaultValue);
        }
    }

    PitchOffsetMap pitchOffsetMap;
};

using PitchPatternList = std::vector<PitchPattern>;

struct ExpressionPattern
{
    using DynamicOffsetMap = std::map<duration_percentage_t, dynamic_level_t>;

    ExpressionPattern() = default;

    ExpressionPattern(size_t size, percentage_t step, dynamic_level_t defaultValue)
    {
        for (size_t i = 0; i < size; ++i) {
            dynamicOffsetMap.emplace(step * static_cast<int>(i), defaultValue);
        }
    }

    DynamicOffsetMap dynamicOffsetMap;

    dynamic_level_t maxAmplitudeLevel() const
    {
        auto maxElement = std::max_element(dynamicOffsetMap.begin(), dynamicOffsetMap.end(), [](const auto& f, const auto& s) {
            return f.second < s.second;
        });

        if (maxElement == dynamicOffsetMap.cend()) {
            return 0;
        }

        return maxElement->second;
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
};

using ArticulationPattern = std::map<duration_percentage_t, ArticulationPatternSegment>;

struct ArticulationsProfile
{
    std::vector<ArticulationFamily> supportedFamilies;

    ArticulationPattern pattern(const ArticulationType type) const
    {
        auto search = m_patterns.find(type);

        if (search == m_patterns.cend()) {
            return {};
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

    const std::unordered_map<ArticulationType, ArticulationPattern>& data() const
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

private:
    std::unordered_map<ArticulationType, ArticulationPattern> m_patterns;
};

using ArticulationsProfilePtr = std::shared_ptr<ArticulationsProfile>;

struct ArticulationData {
    explicit ArticulationData(const ArticulationType _type,
                              const ArticulationPattern& _pattern,
                              const duration_percentage_t _occupiedFrom,
                              const duration_percentage_t _occupiedTo,
                              const pitch_level_t overallPitchRange = 0,
                              const dynamic_level_t overallDynamicRange = 0)
        : type(_type),
        occupiedFrom(_occupiedFrom),
        occupiedTo(_occupiedTo)
    {
        if (_pattern.empty()) {
            return;
        }

        float occupiedFactor = percentageToFactor(occupiedTo);
        pitchChangesRange = overallPitchRange * occupiedFactor;
        dynamicChangesRange = overallDynamicRange * occupiedFactor;

        const auto& lower =_pattern.lower_bound(occupiedFrom);

        if (lower != _pattern.cend()) {
            patternSegment = lower->second;
        } else {
            patternSegment = _pattern.begin()->second;
        }
    }

    ArticulationType type = ArticulationType::Undefined;
    ArticulationPatternSegment patternSegment;

    duration_percentage_t occupiedFrom = 0;
    duration_percentage_t occupiedTo = HUNDRED_PERCENTS;

    pitch_level_t pitchChangesRange = 0;
    dynamic_level_t dynamicChangesRange = 0;
};

struct ArticulationMap : public std::map<ArticulationType, ArticulationData>
{
    ArrangementPatternList arrangementPatterns() const
    {
        ArrangementPatternList patterns;
        patterns.reserve(size());

        for (const auto& pair : *this) {
            patterns.emplace_back(pair.second.patternSegment.arrangementPattern);
        }

        return patterns;
    }

    PitchPatternList pitchPatterns() const
    {
        PitchPatternList patterns;
        patterns.reserve(size());

        for (const auto& pair : *this) {
            patterns.emplace_back(pair.second.patternSegment.pitchPattern);
        }

        return patterns;
    }

    ExpressionPatternList expressionPatterns() const
    {
        ExpressionPatternList patterns;
        patterns.reserve(size());

        for (const auto& pair : *this) {
            patterns.emplace_back(pair.second.patternSegment.expressionPattern);
        }

        return patterns;
    }

    duration_percentage_t averageDurationFactor() const
    {
        duration_percentage_t result = 0;

        if (empty()) {
            return HUNDRED_PERCENTS;
        }

        for (const auto& pair : *this) {
            result += pair.second.patternSegment.arrangementPattern.durationFactor;
        }

        result /= static_cast<int>(size());

        return result;
    }

    duration_percentage_t averageTimestampOffset() const
    {
        duration_percentage_t result = 0;

        if (empty()) {
            return result;
        }

        int meaningTimestampOffsetCount = 0;

        for (const auto& pair : *this) {
            const ArticulationPatternSegment& segment = pair.second.patternSegment;
            if (segment.arrangementPattern.timestampOffset == 0) {
                continue;
            }

            result += segment.arrangementPattern.timestampOffset;
            meaningTimestampOffsetCount++;
        }

        if (meaningTimestampOffsetCount == 0) {
            return 0;
        }

        result /= meaningTimestampOffsetCount;

        return result;
    }

    PitchPattern::PitchOffsetMap averagePitchOffsetMap() const
    {
        PitchPattern::PitchOffsetMap result;

        if (empty()) {
            return result;
        }

        for (const auto& pair : *this) {
            const ArticulationPatternSegment& segment = pair.second.patternSegment;
            IF_ASSERT_FAILED_X(segment.pitchPattern.pitchOffsetMap.size() == EXPECTED_SIZE,
                               "Pitch pattern doesn't suit expected size. There is probably something wrong in Articulation Profile") {
                break;
            }

            for (const auto& pair2 : segment.pitchPattern.pitchOffsetMap) {
                result[pair2.first] += pair2.second;
            }
        }

        for (auto& pair : result) {
            pair.second /= static_cast<int>(size());
        }

        return result;
    }

    pitch_level_t averagePitchRange() const
    {
        pitch_level_t result = 0;

        if (empty()) {
            return result;
        }

        int meaningPitchRangeCount = 0;

        for (const auto& pair : *this) {
            if (pair.second.pitchChangesRange > 0) {
                result += pair.second.pitchChangesRange;
                meaningPitchRangeCount++;
            }
        }

        if (meaningPitchRangeCount == 0) {
            return 0;
        }

        result /= meaningPitchRangeCount;

        return result;
    }

    dynamic_level_t averageMaxAmplitudeLevel() const
    {
        dynamic_level_t result = 0;

        if (empty()) {
            return result;
        }

        int meaningAmplitudeOffsetCount = 0;

        for (const auto& pair : *this) {
            const ArticulationPatternSegment& segment = pair.second.patternSegment;
            dynamic_level_t maxLevel = segment.expressionPattern.maxAmplitudeLevel();
            if (maxLevel > 0) {
                result += maxLevel;
                meaningAmplitudeOffsetCount++;
            }
        }

        if (meaningAmplitudeOffsetCount == 0) {
            return 0;
        }

        result /= meaningAmplitudeOffsetCount;

        return result;
    }

    dynamic_level_t averageDynamicRange() const
    {
        dynamic_level_t result = 0;

        if (empty()) {
            return result;
        }

        int meaningDynamicRangeCount = 0;

        for (const auto& pair : *this) {
            if (pair.second.dynamicChangesRange > 0) {
                result += pair.second.dynamicChangesRange;
                meaningDynamicRangeCount++;
            }
        }

        if (meaningDynamicRangeCount == 0) {
            return 0;
        }

        result /= meaningDynamicRangeCount;

        return result;
    }

    ExpressionPattern::DynamicOffsetMap averageDynamicOffsetMap() const
    {
        ExpressionPattern::DynamicOffsetMap result;

        if (empty()) {
            return result;
        }

        for (const auto& pair : *this) {
            const ArticulationPatternSegment& segment = pair.second.patternSegment;
            IF_ASSERT_FAILED_X(segment.expressionPattern.dynamicOffsetMap.size() == EXPECTED_SIZE,
                               "Expression pattern doesn't suit expected size. There is probably something wrong in Articulation Profile") {
                break;
            }

            for (const auto& pair2 : segment.expressionPattern.dynamicOffsetMap) {
                result[pair2.first] += pair2.second;
            }
        }

        for (auto& pair : result) {
            pair.second /= static_cast<int>(size());
        }

        return result;
    }
};

struct ExpressionCurve : public std::map<duration_percentage_t, dynamic_level_t>
{
    dynamic_level_t maxAmplitudeLevel() const
    {
        const auto& max = std::max_element(cbegin(), cend(), [](const auto& f, const auto& s) {
            return f.second < s.second;
        });

        if (max == cend()) {
            return 0;
        }

        return max->second;
    }
};
}

#endif // MU_MPE_MPETYPES_H
