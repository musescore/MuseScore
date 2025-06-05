/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_DYNAMICUTILS_H
#define MU_ENGRAVING_DYNAMICUTILS_H

#include "mpe/mpetypes.h"

#include "../types/types.h"

namespace mu::engraving {
constexpr muse::mpe::dynamic_level_t NATURAL_DYNAMIC_LEVEL = muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::Natural);

inline const std::unordered_map<DynamicType, muse::mpe::dynamic_level_t> ORDINARY_DYNAMIC_LEVELS {
    { DynamicType::N, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::ppppppppp) },
    { DynamicType::PPPPPP, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::pppppp) },
    { DynamicType::PPPPP, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::ppppp) },
    { DynamicType::PPPP, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::pppp) },
    { DynamicType::PPP, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::ppp) },
    { DynamicType::PP, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::pp) },
    { DynamicType::P, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::p) },
    { DynamicType::MP, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::mp) },
    { DynamicType::MF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::mf) },
    { DynamicType::F, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::f) },
    { DynamicType::FF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::ff) },
    { DynamicType::FFF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::fff) },
    { DynamicType::FFFF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::ffff) },
    { DynamicType::FFFFF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::fffff) },
    { DynamicType::FFFFFF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::ffffff) }
};

inline bool isOrdinaryDynamicType(const DynamicType type)
{
    return ORDINARY_DYNAMIC_LEVELS.find(type) != ORDINARY_DYNAMIC_LEVELS.cend();
}

inline muse::mpe::dynamic_level_t dynamicLevelFromOrdinaryType(const DynamicType type,
                                                               const muse::mpe::dynamic_level_t defLevel = NATURAL_DYNAMIC_LEVEL)
{
    auto search = ORDINARY_DYNAMIC_LEVELS.find(type);
    if (search != ORDINARY_DYNAMIC_LEVELS.cend()) {
        return search->second;
    }

    return defLevel;
}

inline const std::unordered_map<DynamicType, muse::mpe::dynamic_level_t> SINGLE_NOTE_DYNAMIC_LEVELS {
    { DynamicType::SF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::f) },
    { DynamicType::SFZ, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::f) },
    { DynamicType::SFF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::ff) },
    { DynamicType::SFFZ, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::ff) },
    { DynamicType::SFFF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::fff) },
    { DynamicType::SFFFZ, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::fff) },
    { DynamicType::RFZ, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::f) },
    { DynamicType::RF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::f) }
};

inline bool isSingleNoteDynamicType(const DynamicType type)
{
    return SINGLE_NOTE_DYNAMIC_LEVELS.find(type) != SINGLE_NOTE_DYNAMIC_LEVELS.cend();
}

inline muse::mpe::dynamic_level_t dynamicLevelFromSingleNoteType(const DynamicType type,
                                                                 const muse::mpe::dynamic_level_t defLevel = NATURAL_DYNAMIC_LEVEL)
{
    auto search = SINGLE_NOTE_DYNAMIC_LEVELS.find(type);
    if (search != SINGLE_NOTE_DYNAMIC_LEVELS.cend()) {
        return search->second;
    }

    return defLevel;
}

struct CompoundDynamic {
    DynamicType from = DynamicType::OTHER;
    DynamicType to = DynamicType::OTHER;

    bool isValid() const
    {
        return from != DynamicType::OTHER && to != DynamicType::OTHER;
    }
};

inline const std::unordered_map<DynamicType, CompoundDynamic> COMPOUND_DYNAMICS {
    { DynamicType::FP, { DynamicType::F, DynamicType::P } },
    { DynamicType::PF, { DynamicType::P, DynamicType::F } },
    { DynamicType::SFP, { DynamicType::F, DynamicType::P } },
    { DynamicType::SFPP, { DynamicType::F, DynamicType::PP } }
};

inline bool isCompoundDynamicType(const DynamicType type)
{
    return COMPOUND_DYNAMICS.find(type) != COMPOUND_DYNAMICS.cend();
}

inline const CompoundDynamic& compoundDynamicFromType(const DynamicType type)
{
    auto search = COMPOUND_DYNAMICS.find(type);
    if (search != COMPOUND_DYNAMICS.cend()) {
        return search->second;
    }

    static CompoundDynamic empty;
    return empty;
}

inline muse::mpe::ArticulationType articulationFromPlayTechType(const PlayingTechniqueType technique)
{
    static const std::unordered_map<PlayingTechniqueType, muse::mpe::ArticulationType> PLAYING_TECH_TYPES = {
        { PlayingTechniqueType::Undefined, muse::mpe::ArticulationType::Undefined },
        { PlayingTechniqueType::Natural, muse::mpe::ArticulationType::Standard },
        { PlayingTechniqueType::Pizzicato, muse::mpe::ArticulationType::Pizzicato },
        { PlayingTechniqueType::Open, muse::mpe::ArticulationType::Open },
        { PlayingTechniqueType::Mute, muse::mpe::ArticulationType::Mute },
        { PlayingTechniqueType::Tremolo, muse::mpe::ArticulationType::Tremolo64th },
        { PlayingTechniqueType::Detache, muse::mpe::ArticulationType::Detache },
        { PlayingTechniqueType::Martele, muse::mpe::ArticulationType::Martele },
        { PlayingTechniqueType::ColLegno, muse::mpe::ArticulationType::ColLegno },
        { PlayingTechniqueType::SulPonticello, muse::mpe::ArticulationType::SulPont },
        { PlayingTechniqueType::SulTasto, muse::mpe::ArticulationType::SulTasto },
        { PlayingTechniqueType::Distortion, muse::mpe::ArticulationType::Distortion },
        { PlayingTechniqueType::Overdrive, muse::mpe::ArticulationType::Overdrive },
        { PlayingTechniqueType::Harmonics, muse::mpe::ArticulationType::Harmonic },
        { PlayingTechniqueType::JazzTone, muse::mpe::ArticulationType::JazzTone },
    };

    auto search = PLAYING_TECH_TYPES.find(technique);

    if (search != PLAYING_TECH_TYPES.cend()) {
        return search->second;
    }

    return muse::mpe::ArticulationType::Undefined;
}

static const muse::mpe::ArticulationTypeSet GRACE_NOTE_ARTICULATION_TYPES {
    muse::mpe::ArticulationType::Acciaccatura,
    muse::mpe::ArticulationType::PostAppoggiatura,
    muse::mpe::ArticulationType::PreAppoggiatura,
};

inline bool isGraceNotePlacedBeforePrincipalNote(const muse::mpe::ArticulationType type)
{
    return type == muse::mpe::ArticulationType::Acciaccatura || type == muse::mpe::ArticulationType::PreAppoggiatura;
}

inline void updateArticulationBoundaries(const muse::mpe::ArticulationType type, const muse::mpe::timestamp_t nominalTimestamp,
                                         const muse::mpe::duration_t nominalDuration,
                                         muse::mpe::ArticulationMap& articulations)
{
    if (articulations.empty()) {
        return;
    }

    const muse::mpe::ArticulationAppliedData& articulationData = articulations.at(type);

    muse::mpe::timestamp_t articulationOccupiedFrom = nominalTimestamp - articulationData.meta.timestamp;
    muse::mpe::timestamp_t articulationOccupiedTo = nominalTimestamp + nominalDuration - articulationData.meta.timestamp;

    articulations.updateOccupiedRange(type,
                                      muse::mpe::occupiedPercentage(articulationOccupiedFrom,
                                                                    articulationData.meta.overallDuration),
                                      muse::mpe::occupiedPercentage(articulationOccupiedTo,
                                                                    articulationData.meta.overallDuration));
}
}

#endif // MU_ENGRAVING_DYNAMICUTILS_H
