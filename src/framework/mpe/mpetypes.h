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
constexpr percentage_t SINGLE_PERCENT = 100;
constexpr percentage_t HUNDRED_PERCENTS = SINGLE_PERCENT * 100;
constexpr percentage_t TEN_PERCENTS = SINGLE_PERCENT * 10;

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

    // multi-note articulations
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
    TurnWithSlash,

    Appoggiatura,
    Acciaccatura,

    TremoloBar,
    VolumeSwell,

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
        ArticulationType::SlideInAbove, ArticulationType::SlideInBelow
    };

    return singleNoteTypes.find(type) != singleNoteTypes.cend();
}

inline bool isMultiNoteArticulation(const ArticulationType type)
{
    return !isSingleNoteArticulation(type);
}

enum class DynamicType {
    Undefined = -1,
    pppppppp = 0,
    ppppppp = 10 * SINGLE_PERCENT,
    pppppp = 15 * SINGLE_PERCENT,
    ppppp = 20 * SINGLE_PERCENT,
    pppp = 25 * SINGLE_PERCENT,
    ppp = 30 * SINGLE_PERCENT,
    pp = 35 * SINGLE_PERCENT,
    p = 40 * SINGLE_PERCENT,
    mp = 45 * SINGLE_PERCENT,
    Natural = 50 * SINGLE_PERCENT,
    mf = 55 * SINGLE_PERCENT,
    f = 60 * SINGLE_PERCENT,
    ff = 65 * SINGLE_PERCENT,
    fff = 70 * SINGLE_PERCENT,
    ffff = 75 * SINGLE_PERCENT,
    fffff = 80 * SINGLE_PERCENT,
    ffffff = 90 * SINGLE_PERCENT,
    fffffff = 100 * SINGLE_PERCENT,
    Last
};

using dynamic_level_t = percentage_t;
constexpr dynamic_level_t MAX_DYNAMIC_LEVEL = HUNDRED_PERCENTS;
constexpr dynamic_level_t MIN_DYNAMIC_LEVEL = 0;

inline DynamicType approximateDynamicType(const dynamic_level_t dynamicLevel)
{
    if (dynamicLevel < MIN_DYNAMIC_LEVEL) {
        return DynamicType::pppppppp;
    }

    if (dynamicLevel > MAX_DYNAMIC_LEVEL) {
        return DynamicType::fffffff;
    }

    return static_cast<DynamicType>(dynamicLevel);
}

inline dynamic_level_t dynamicLevelFromType(const DynamicType type)
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

    static duration_percentage_t averageDurationFactor(const std::vector<ArrangementPattern>& patterns)
    {
        duration_percentage_t result = 0;

        if (patterns.empty()) {
            return HUNDRED_PERCENTS;
        }

        for (const ArrangementPattern& pattern : patterns) {
            result += pattern.durationFactor;
        }

        result /= patterns.size();

        return result;
    }

    static duration_percentage_t averageTimestampOffset(const std::vector<ArrangementPattern>& patterns)
    {
        duration_percentage_t result = 0;

        if (patterns.empty()) {
            return result;
        }

        int meaningTimestampOffsetCount = 0;

        for (const ArrangementPattern& pattern : patterns) {
            if (pattern.timestampOffset == 0) {
                continue;
            }

            result += pattern.timestampOffset;
            meaningTimestampOffsetCount++;
        }

        if (meaningTimestampOffsetCount == 0) {
            return 0;
        }

        result /= meaningTimestampOffsetCount;

        return result;
    }
};

using ArrangementPatternList = std::vector<ArrangementPattern>;

struct PitchPattern
{
    using PitchOffsetMap = std::map<duration_percentage_t, pitch_level_t>;

    PitchPattern() = default;

    PitchPattern(size_t size, percentage_t step, pitch_level_t defaultValue)
    {
        for (size_t i = 0; i < size; ++i) {
            pitchOffsetMap.emplace(step * i, defaultValue);
        }
    }

    PitchOffsetMap pitchOffsetMap;

    static PitchOffsetMap averagePitchOffsetMap(const std::vector<PitchPattern>& patterns)
    {
        PitchOffsetMap result;

        if (patterns.empty()) {
            return result;
        }

        for (const PitchPattern& pattern : patterns) {
            IF_ASSERT_FAILED_X(pattern.pitchOffsetMap.size() == EXPECTED_SIZE,
                               "Pitch pattern doesn't suit expected size. There is probably something wrong in Articulation Profile") {
                break;
            }

            for (const auto& pair : pattern.pitchOffsetMap) {
                result[pair.first] += pair.second;
            }
        }

        for (auto& pair : result) {
            pair.second /= patterns.size();
        }

        return result;
    }
};

using PitchPatternList = std::vector<PitchPattern>;

struct ExpressionPattern
{
    using DynamicOffsetMap = std::map<duration_percentage_t, dynamic_level_t>;

    ExpressionPattern() = default;

    ExpressionPattern(size_t size, percentage_t step, dynamic_level_t defaultValue)
    {
        for (size_t i = 0; i < size; ++i) {
            dynamicOffsetMap.emplace(step * i, defaultValue);
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

    static dynamic_level_t averageMaxAmplitudeLevel(const std::vector<ExpressionPattern>& patterns)
    {
        dynamic_level_t result = 0;

        if (patterns.empty()) {
            return result;
        }

        int meaningAmplitudeOffsetCount = 0;

        for (const ExpressionPattern& pattern : patterns) {
            dynamic_level_t maxLevel = pattern.maxAmplitudeLevel();
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

    static DynamicOffsetMap averageDynamicOffsetMap(const std::vector<ExpressionPattern>& patterns)
    {
        DynamicOffsetMap result;

        if (patterns.empty()) {
            return result;
        }

        for (const ExpressionPattern& pattern : patterns) {
            IF_ASSERT_FAILED_X(pattern.dynamicOffsetMap.size() == EXPECTED_SIZE,
                               "Expression pattern doesn't suit expected size. There is probably something wrong in Articulation Profile") {
                break;
            }

            for (const auto& pair : pattern.dynamicOffsetMap) {
                result[pair.first] += pair.second;
            }
        }

        for (auto& pair : result) {
            pair.second /= patterns.size();
        }

        return result;
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

    void updatePatterns(const ArticulationType type, const ArticulationPattern& scope)
    {
        m_patterns.insert_or_assign(type, scope);
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
                              const ArticulationPattern& _patternsScope,
                              const duration_percentage_t _occupiedFrom,
                              const duration_percentage_t _occupiedTo)
        : type(_type),
        occupiedFrom(_occupiedFrom),
        occupiedTo(_occupiedTo)
    {
        if (_patternsScope.empty()) {
            return;
        }

        const auto& lower =_patternsScope.lower_bound(occupiedFrom);

        if (lower != _patternsScope.cend()) {
            patterns = lower->second;
        } else {
            patterns = _patternsScope.begin()->second;
        }
    }

    ArticulationType type = ArticulationType::Undefined;
    ArticulationPatternSegment patterns;

    duration_percentage_t occupiedFrom = 0;
    duration_percentage_t occupiedTo = HUNDRED_PERCENTS;
};

struct ArticulationMap : public std::map<ArticulationType, ArticulationData>
{
    ArrangementPatternList arrangementPatterns() const
    {
        ArrangementPatternList patterns;
        patterns.reserve(size());

        for (const auto& pair : *this) {
            patterns.emplace_back(pair.second.patterns.arrangementPattern);
        }

        return patterns;
    }

    PitchPatternList pitchPatterns() const
    {
        PitchPatternList patterns;
        patterns.reserve(size());

        for (const auto& pair : *this) {
            patterns.emplace_back(pair.second.patterns.pitchPattern);
        }

        return patterns;
    }

    ExpressionPatternList expressionPatterns() const
    {
        ExpressionPatternList patterns;
        patterns.reserve(size());

        for (const auto& pair : *this) {
            patterns.emplace_back(pair.second.patterns.expressionPattern);
        }

        return patterns;
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
