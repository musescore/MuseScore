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

#ifndef MU_ENGRAVING_DYNAMICUTILS_H
#define MU_ENGRAVING_DYNAMICUTILS_H

#include "mpe/mpetypes.h"

#include "types/types.h"

namespace mu::engraving {
struct DynamicTransition {
    Ms::DynamicType from = Ms::DynamicType::OTHER;
    Ms::DynamicType to = Ms::DynamicType::OTHER;

    bool isValid() const
    {
        return from != Ms::DynamicType::OTHER && to != Ms::DynamicType::OTHER;
    }
};

inline mpe::dynamic_level_t dynamicLevelFromType(const Ms::DynamicType type,
                                                 const mpe::dynamic_level_t defLevel = mpe::dynamicLevelFromType(mpe::DynamicType::Natural))
{
    static const std::unordered_map<Ms::DynamicType, mpe::dynamic_level_t> DYNAMIC_LEVELS = {
        { Ms::DynamicType::PPPPPP, mpe::dynamicLevelFromType(mpe::DynamicType::pppppp) },
        { Ms::DynamicType::PPPPP, mpe::dynamicLevelFromType(mpe::DynamicType::ppppp) },
        { Ms::DynamicType::PPPP, mpe::dynamicLevelFromType(mpe::DynamicType::pppp) },
        { Ms::DynamicType::PPP, mpe::dynamicLevelFromType(mpe::DynamicType::ppp) },
        { Ms::DynamicType::PP, mpe::dynamicLevelFromType(mpe::DynamicType::pp) },
        { Ms::DynamicType::P, mpe::dynamicLevelFromType(mpe::DynamicType::p) },
        { Ms::DynamicType::MP, mpe::dynamicLevelFromType(mpe::DynamicType::mp) },
        { Ms::DynamicType::MF, mpe::dynamicLevelFromType(mpe::DynamicType::mf) },
        { Ms::DynamicType::F, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { Ms::DynamicType::FF, mpe::dynamicLevelFromType(mpe::DynamicType::ff) },
        { Ms::DynamicType::FFF, mpe::dynamicLevelFromType(mpe::DynamicType::fff) },
        { Ms::DynamicType::FFFF, mpe::dynamicLevelFromType(mpe::DynamicType::ffff) },
        { Ms::DynamicType::FFFFF, mpe::dynamicLevelFromType(mpe::DynamicType::fffff) },
        { Ms::DynamicType::FFFFFF, mpe::dynamicLevelFromType(mpe::DynamicType::ffffff) },
        { Ms::DynamicType::SF, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { Ms::DynamicType::SFZ, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { Ms::DynamicType::SFF, mpe::dynamicLevelFromType(mpe::DynamicType::ff) },
        { Ms::DynamicType::SFFZ, mpe::dynamicLevelFromType(mpe::DynamicType::ff) },
        { Ms::DynamicType::RFZ, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { Ms::DynamicType::RF, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { Ms::DynamicType::FZ, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { Ms::DynamicType::N, mpe::dynamicLevelFromType(mpe::DynamicType::ppppppppp) }
    };

    auto search = DYNAMIC_LEVELS.find(type);

    if (search != DYNAMIC_LEVELS.cend()) {
        return search->second;
    }

    return defLevel;
}

inline mpe::dynamic_level_t dynamicLevelRangeByTypes(const Ms::DynamicType dynamicTypeFrom, const Ms::DynamicType dynamicTypeTo,
                                                     const mpe::dynamic_level_t nominalDynamicLevelFrom,
                                                     const mpe::dynamic_level_t nominalDynamicLevelTo, const bool isCrescendo)
{
    mpe::dynamic_level_t dynamicLevelFrom = 0;
    mpe::dynamic_level_t dynamicLevelTo = 0;

    dynamicLevelFrom = dynamicLevelFromType(dynamicTypeFrom, nominalDynamicLevelFrom);

    mpe::dynamic_level_t defaultStep = mpe::DYNAMIC_LEVEL_STEP;
    if (!isCrescendo) {
        defaultStep = -mpe::DYNAMIC_LEVEL_STEP;
    }

    if (nominalDynamicLevelTo == mpe::dynamicLevelFromType(mpe::DynamicType::Natural)) {
        dynamicLevelTo = dynamicLevelFromType(dynamicTypeTo, dynamicLevelFrom + defaultStep);
    } else {
        dynamicLevelTo = dynamicLevelFromType(dynamicTypeTo, nominalDynamicLevelTo);
    }

    return dynamicLevelTo - dynamicLevelFrom;
}

inline bool isOrdinaryDynamicType(const Ms::DynamicType type)
{
    static const std::set<Ms::DynamicType> ORDINARY_DYNAMIC_TYPES = {
        Ms::DynamicType::PPPPPP,
        Ms::DynamicType::PPPPP,
        Ms::DynamicType::PPPP,
        Ms::DynamicType::PPP,
        Ms::DynamicType::PP,
        Ms::DynamicType::P,
        Ms::DynamicType::MP,
        Ms::DynamicType::MF,
        Ms::DynamicType::F,
        Ms::DynamicType::FF,
        Ms::DynamicType::FFF,
        Ms::DynamicType::FFFF,
        Ms::DynamicType::FFFFF,
        Ms::DynamicType::FFFFFF
    };

    return ORDINARY_DYNAMIC_TYPES.find(type) != ORDINARY_DYNAMIC_TYPES.cend();
}

inline bool isSingleNoteDynamicType(const Ms::DynamicType type)
{
    static const std::set<Ms::DynamicType> SINGLE_NOTE_DYNAMIC_TYPES = {
        Ms::DynamicType::SF,
        Ms::DynamicType::SFZ,
        Ms::DynamicType::SFFZ,
        Ms::DynamicType::RFZ,
        Ms::DynamicType::RF
    };

    return SINGLE_NOTE_DYNAMIC_TYPES.find(type) != SINGLE_NOTE_DYNAMIC_TYPES.cend();
}

inline const DynamicTransition& dynamicTransitionFromType(const Ms::DynamicType type)
{
    static const std::unordered_map<Ms::DynamicType, DynamicTransition> DYNAMIC_TRANSITIONS = {
        { Ms::DynamicType::FP, { Ms::DynamicType::F, Ms::DynamicType::P } },
        { Ms::DynamicType::PF, { Ms::DynamicType::P, Ms::DynamicType::F } },
        { Ms::DynamicType::SFP, { Ms::DynamicType::F, Ms::DynamicType::P } },
        { Ms::DynamicType::SFPP, { Ms::DynamicType::F, Ms::DynamicType::PP } }
    };

    auto search = DYNAMIC_TRANSITIONS.find(type);
    if (search != DYNAMIC_TRANSITIONS.cend()) {
        return search->second;
    }

    static DynamicTransition empty;
    return empty;
}

inline mpe::ArticulationType articulationFromPlayTechType(const Ms::PlayingTechniqueType technique)
{
    static const std::unordered_map<Ms::PlayingTechniqueType, mpe::ArticulationType> PLAYING_TECH_TYPES = {
        { Ms::PlayingTechniqueType::Undefined, mpe::ArticulationType::Undefined },
        { Ms::PlayingTechniqueType::Natural, mpe::ArticulationType::Standard },
        { Ms::PlayingTechniqueType::Pizzicato, mpe::ArticulationType::Pizzicato },
        { Ms::PlayingTechniqueType::Open, mpe::ArticulationType::Open },
        { Ms::PlayingTechniqueType::Mute, mpe::ArticulationType::Mute },
        { Ms::PlayingTechniqueType::Tremolo, mpe::ArticulationType::Tremolo64th },
        { Ms::PlayingTechniqueType::Detache, mpe::ArticulationType::Detache },
        { Ms::PlayingTechniqueType::Martele, mpe::ArticulationType::Martele },
        { Ms::PlayingTechniqueType::ColLegno, mpe::ArticulationType::ColLegno },
        { Ms::PlayingTechniqueType::SulPonticello, mpe::ArticulationType::SulPont },
        { Ms::PlayingTechniqueType::SulTasto, mpe::ArticulationType::SulTasto },
        { Ms::PlayingTechniqueType::Distortion, mpe::ArticulationType::Distortion },
        { Ms::PlayingTechniqueType::Overdrive, mpe::ArticulationType::Overdrive }
    };

    auto search = PLAYING_TECH_TYPES.find(technique);

    if (search != PLAYING_TECH_TYPES.cend()) {
        return search->second;
    }

    return mpe::ArticulationType::Undefined;
}
}

#endif // MU_ENGRAVING_DYNAMICUTILS_H
