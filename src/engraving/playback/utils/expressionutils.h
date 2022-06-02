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
    mu::engraving::DynamicType from = mu::engraving::DynamicType::OTHER;
    mu::engraving::DynamicType to = mu::engraving::DynamicType::OTHER;

    bool isValid() const
    {
        return from != mu::engraving::DynamicType::OTHER && to != mu::engraving::DynamicType::OTHER;
    }
};

inline mpe::dynamic_level_t dynamicLevelFromType(const mu::engraving::DynamicType type,
                                                 const mpe::dynamic_level_t defLevel = mpe::dynamicLevelFromType(mpe::DynamicType::Natural))
{
    static const std::unordered_map<mu::engraving::DynamicType, mpe::dynamic_level_t> DYNAMIC_LEVELS = {
        { mu::engraving::DynamicType::PPPPPP, mpe::dynamicLevelFromType(mpe::DynamicType::pppppp) },
        { mu::engraving::DynamicType::PPPPP, mpe::dynamicLevelFromType(mpe::DynamicType::ppppp) },
        { mu::engraving::DynamicType::PPPP, mpe::dynamicLevelFromType(mpe::DynamicType::pppp) },
        { mu::engraving::DynamicType::PPP, mpe::dynamicLevelFromType(mpe::DynamicType::ppp) },
        { mu::engraving::DynamicType::PP, mpe::dynamicLevelFromType(mpe::DynamicType::pp) },
        { mu::engraving::DynamicType::P, mpe::dynamicLevelFromType(mpe::DynamicType::p) },
        { mu::engraving::DynamicType::MP, mpe::dynamicLevelFromType(mpe::DynamicType::mp) },
        { mu::engraving::DynamicType::MF, mpe::dynamicLevelFromType(mpe::DynamicType::mf) },
        { mu::engraving::DynamicType::F, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { mu::engraving::DynamicType::FF, mpe::dynamicLevelFromType(mpe::DynamicType::ff) },
        { mu::engraving::DynamicType::FFF, mpe::dynamicLevelFromType(mpe::DynamicType::fff) },
        { mu::engraving::DynamicType::FFFF, mpe::dynamicLevelFromType(mpe::DynamicType::ffff) },
        { mu::engraving::DynamicType::FFFFF, mpe::dynamicLevelFromType(mpe::DynamicType::fffff) },
        { mu::engraving::DynamicType::FFFFFF, mpe::dynamicLevelFromType(mpe::DynamicType::ffffff) },
        { mu::engraving::DynamicType::SF, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { mu::engraving::DynamicType::SFZ, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { mu::engraving::DynamicType::SFF, mpe::dynamicLevelFromType(mpe::DynamicType::ff) },
        { mu::engraving::DynamicType::SFFZ, mpe::dynamicLevelFromType(mpe::DynamicType::ff) },
        { mu::engraving::DynamicType::RFZ, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { mu::engraving::DynamicType::RF, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { mu::engraving::DynamicType::FZ, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { mu::engraving::DynamicType::N, mpe::dynamicLevelFromType(mpe::DynamicType::ppppppppp) }
    };

    auto search = DYNAMIC_LEVELS.find(type);

    if (search != DYNAMIC_LEVELS.cend()) {
        return search->second;
    }

    return defLevel;
}

inline mpe::dynamic_level_t dynamicLevelRangeByTypes(const mu::engraving::DynamicType dynamicTypeFrom,
                                                     const mu::engraving::DynamicType dynamicTypeTo,
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

inline bool isOrdinaryDynamicType(const mu::engraving::DynamicType type)
{
    static const std::set<mu::engraving::DynamicType> ORDINARY_DYNAMIC_TYPES = {
        mu::engraving::DynamicType::PPPPPP,
        mu::engraving::DynamicType::PPPPP,
        mu::engraving::DynamicType::PPPP,
        mu::engraving::DynamicType::PPP,
        mu::engraving::DynamicType::PP,
        mu::engraving::DynamicType::P,
        mu::engraving::DynamicType::MP,
        mu::engraving::DynamicType::MF,
        mu::engraving::DynamicType::F,
        mu::engraving::DynamicType::FF,
        mu::engraving::DynamicType::FFF,
        mu::engraving::DynamicType::FFFF,
        mu::engraving::DynamicType::FFFFF,
        mu::engraving::DynamicType::FFFFFF
    };

    return ORDINARY_DYNAMIC_TYPES.find(type) != ORDINARY_DYNAMIC_TYPES.cend();
}

inline bool isSingleNoteDynamicType(const mu::engraving::DynamicType type)
{
    static const std::set<mu::engraving::DynamicType> SINGLE_NOTE_DYNAMIC_TYPES = {
        mu::engraving::DynamicType::SF,
        mu::engraving::DynamicType::SFZ,
        mu::engraving::DynamicType::SFFZ,
        mu::engraving::DynamicType::RFZ,
        mu::engraving::DynamicType::RF
    };

    return SINGLE_NOTE_DYNAMIC_TYPES.find(type) != SINGLE_NOTE_DYNAMIC_TYPES.cend();
}

inline const DynamicTransition& dynamicTransitionFromType(const mu::engraving::DynamicType type)
{
    static const std::unordered_map<mu::engraving::DynamicType, DynamicTransition> DYNAMIC_TRANSITIONS = {
        { mu::engraving::DynamicType::FP, { mu::engraving::DynamicType::F, mu::engraving::DynamicType::P } },
        { mu::engraving::DynamicType::PF, { mu::engraving::DynamicType::P, mu::engraving::DynamicType::F } },
        { mu::engraving::DynamicType::SFP, { mu::engraving::DynamicType::F, mu::engraving::DynamicType::P } },
        { mu::engraving::DynamicType::SFPP, { mu::engraving::DynamicType::F, mu::engraving::DynamicType::PP } }
    };

    auto search = DYNAMIC_TRANSITIONS.find(type);
    if (search != DYNAMIC_TRANSITIONS.cend()) {
        return search->second;
    }

    static DynamicTransition empty;
    return empty;
}

inline mpe::ArticulationType articulationFromPlayTechType(const mu::engraving::PlayingTechniqueType technique)
{
    static const std::unordered_map<mu::engraving::PlayingTechniqueType, mpe::ArticulationType> PLAYING_TECH_TYPES = {
        { mu::engraving::PlayingTechniqueType::Undefined, mpe::ArticulationType::Undefined },
        { mu::engraving::PlayingTechniqueType::Natural, mpe::ArticulationType::Standard },
        { mu::engraving::PlayingTechniqueType::Pizzicato, mpe::ArticulationType::Pizzicato },
        { mu::engraving::PlayingTechniqueType::Open, mpe::ArticulationType::Open },
        { mu::engraving::PlayingTechniqueType::Mute, mpe::ArticulationType::Mute },
        { mu::engraving::PlayingTechniqueType::Tremolo, mpe::ArticulationType::Tremolo64th },
        { mu::engraving::PlayingTechniqueType::Detache, mpe::ArticulationType::Detache },
        { mu::engraving::PlayingTechniqueType::Martele, mpe::ArticulationType::Martele },
        { mu::engraving::PlayingTechniqueType::ColLegno, mpe::ArticulationType::ColLegno },
        { mu::engraving::PlayingTechniqueType::SulPonticello, mpe::ArticulationType::SulPont },
        { mu::engraving::PlayingTechniqueType::SulTasto, mpe::ArticulationType::SulTasto },
        { mu::engraving::PlayingTechniqueType::Distortion, mpe::ArticulationType::Distortion },
        { mu::engraving::PlayingTechniqueType::Overdrive, mpe::ArticulationType::Overdrive }
    };

    auto search = PLAYING_TECH_TYPES.find(technique);

    if (search != PLAYING_TECH_TYPES.cend()) {
        return search->second;
    }

    return mpe::ArticulationType::Undefined;
}
}

#endif // MU_ENGRAVING_DYNAMICUTILS_H
