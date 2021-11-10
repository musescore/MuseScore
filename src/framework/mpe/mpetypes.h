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
#include <vector>

#include "log.h"

namespace mu::mpe {
// common
using msecs_t = uint64_t;
using percentage_t = float; // from 0.0 (0%) to 1.0 (100%)
constexpr percentage_t PERCENTAGE_PRECISION_STEP = 0.1f; // 10%

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

constexpr octave_t MAX_SUPPORTED_OCTAVE = 12; // 0 - 12
constexpr pitch_level_t MAX_PITCH_LEVEL = 1.f;
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
    None,
    Staccato,
    Staccatissimo,
    Tenuto,
    Marcato,
    Accent,
    ShortAccent,

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
    TurnWithSlash,

    Appoggiatura,
    Acciaccatura,

    TremoloBar,
    VolumeSwell,

    Last
};

enum class DynamicType {
    Undefined = -1,
    pppppppp = 0,
    ppppppp = 1,
    pppppp = 2,
    ppppp = 3,
    pppp = 4,
    ppp = 5,
    pp = 6,
    p = 7,
    mp = 8,
    Natural = 9,
    mf = 10,
    f = 11,
    ff = 12,
    fff = 13,
    ffff = 14,
    fffff = 15,
    ffffff = 16,
    fffffff = 17,
    ffffffff = 18,
    Last
};

using dynamic_level_t = percentage_t;
constexpr dynamic_level_t MAX_DYNAMIC_LEVEL = 1.f;
constexpr dynamic_level_t MIN_DYNAMIC_LEVEL = 0.f;
constexpr dynamic_level_t DYNAMIC_LEVEL_STEP = MAX_DYNAMIC_LEVEL / (static_cast<int>(DynamicType::Last) - 1);

inline DynamicType approximateDynamicType(const dynamic_level_t dynamicLevel)
{
    if (dynamicLevel < MIN_DYNAMIC_LEVEL) {
        return DynamicType::pppppppp;
    }

    if (dynamicLevel > MAX_DYNAMIC_LEVEL) {
        return DynamicType::ffffffff;
    }

    return static_cast<DynamicType>(std::roundf(dynamicLevel / DYNAMIC_LEVEL_STEP));
}

inline dynamic_level_t dynamicLevelFromType(const DynamicType type)
{
    if (type == DynamicType::Undefined) {
        return MIN_DYNAMIC_LEVEL;
    }

    return static_cast<int>(type) * DYNAMIC_LEVEL_STEP;
}

struct ArrangementPattern
{
    duration_percentage_t durationFactor = 0.f;
    msecs_t timestampOffset = 0;

    static duration_percentage_t averageDurationFactor(const std::vector<ArrangementPattern>& patterns)
    {
        duration_percentage_t result = 0.f;

        if (patterns.empty()) {
            return result;
        }

        for (const ArrangementPattern& pattern : patterns) {
            result += pattern.durationFactor;
        }

        result /= patterns.size();

        return result;
    }

    static msecs_t averageTimestampOffset(const std::vector<ArrangementPattern>& patterns)
    {
        msecs_t result = 0;

        if (patterns.empty()) {
            return result;
        }

        int meaningTimestampOffsetCount = 0;

        for (const ArrangementPattern& pattern : patterns) {
            if (pattern.timestampOffset > 0) {
                result += pattern.timestampOffset;
                meaningTimestampOffsetCount++;
            }
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

    PitchOffsetMap pitchOffsetMap;

    static PitchOffsetMap averagePitchOffsetMap(const std::vector<PitchPattern>& patterns)
    {
        PitchOffsetMap result;

        if (patterns.empty()) {
            return result;
        }

        constexpr size_t expectedSize = 1.f / PERCENTAGE_PRECISION_STEP;

        for (const PitchPattern& pattern : patterns) {
            IF_ASSERT_FAILED_X(pattern.pitchOffsetMap.size() == expectedSize,
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

    dynamic_level_t maxAmplitudeLevel = 0.f;
    duration_percentage_t amplitudeTimeShift = 0.f;

    DynamicOffsetMap dynamicOffsetMap;

    static dynamic_level_t averageMaxAmplitudeLevel(const std::vector<ExpressionPattern>& patterns)
    {
        dynamic_level_t result = 0.f;

        if (patterns.empty()) {
            return result;
        }

        int meaningAmplitudeOffsetCount = 0;

        for (const ExpressionPattern& pattern : patterns) {
            if (pattern.maxAmplitudeLevel > 0) {
                result += pattern.maxAmplitudeLevel;
                meaningAmplitudeOffsetCount++;
            }
        }

        if (meaningAmplitudeOffsetCount == 0) {
            return 0.f;
        }

        result /= meaningAmplitudeOffsetCount;

        return result;
    }

    static duration_percentage_t averageAmplitudeTimeShift(const std::vector<ExpressionPattern>& patterns)
    {
        duration_percentage_t result = 0.f;

        if (patterns.empty()) {
            return result;
        }

        int meaningAmplitudeTimeShiftCount = 0;

        for (const ExpressionPattern& pattern : patterns) {
            if (pattern.amplitudeTimeShift > 0) {
                result += pattern.amplitudeTimeShift;
                meaningAmplitudeTimeShiftCount++;
            }
        }

        if (meaningAmplitudeTimeShiftCount == 0) {
            return 0.f;
        }

        result /= meaningAmplitudeTimeShiftCount;

        return result;
    }

    static DynamicOffsetMap averageDynamicOffsetMap(const std::vector<ExpressionPattern>& patterns)
    {
        DynamicOffsetMap result;

        if (patterns.empty()) {
            return result;
        }

        constexpr size_t expectedSize = 1.f / PERCENTAGE_PRECISION_STEP;

        for (const ExpressionPattern& pattern : patterns) {
            IF_ASSERT_FAILED_X(pattern.dynamicOffsetMap.size() == expectedSize,
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

struct ArticulationPattern
{
    ArrangementPattern arrangementPattern;
    PitchPattern pitchPattern;
    ExpressionPattern expressionPattern;
};

using ArticulationPatternsScope = std::map<duration_percentage_t, ArticulationPattern>;

struct ArticulationsProfile
{
    std::vector<ArticulationFamily> supportedFamilies;

    ArticulationPatternsScope patterns(const ArticulationType type) const
    {
        auto search = m_patterns.find(type);

        if (search == m_patterns.cend()) {
            return {};
        }

        return search->second;
    }

    void updatePatterns(const ArticulationType type, const ArticulationPatternsScope& scope)
    {
        m_patterns.insert_or_assign(type, scope);
    }

    const std::unordered_map<ArticulationType, ArticulationPatternsScope>& data() const
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
    std::unordered_map<ArticulationType, ArticulationPatternsScope> m_patterns;
};

using ArticulationsProfilePtr = std::shared_ptr<ArticulationsProfile>;

struct ArticulationData {
    explicit ArticulationData(const ArticulationType _type,
                              const ArticulationPatternsScope& _patternsScope,
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
    ArticulationPattern patterns;

    duration_percentage_t occupiedFrom = 0.f;
    duration_percentage_t occupiedTo = 1.f;
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
            return 0.f;
        }

        return max->second;
    }
};
}

#endif // MU_MPE_MPETYPES_H
