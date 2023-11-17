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
    DynamicType from = DynamicType::OTHER;
    DynamicType to = DynamicType::OTHER;

    bool isValid() const
    {
        return from != DynamicType::OTHER && to != DynamicType::OTHER;
    }
};

inline mpe::dynamic_level_t dynamicLevelFromType(const DynamicType type,
                                                 const mpe::dynamic_level_t defLevel = mpe::dynamicLevelFromType(mpe::DynamicType::Natural))
{
    static const std::unordered_map<DynamicType, mpe::dynamic_level_t> DYNAMIC_LEVELS = {
        { DynamicType::PPPPPP, mpe::dynamicLevelFromType(mpe::DynamicType::pppppp) },
        { DynamicType::PPPPP, mpe::dynamicLevelFromType(mpe::DynamicType::ppppp) },
        { DynamicType::PPPP, mpe::dynamicLevelFromType(mpe::DynamicType::pppp) },
        { DynamicType::PPP, mpe::dynamicLevelFromType(mpe::DynamicType::ppp) },
        { DynamicType::PP, mpe::dynamicLevelFromType(mpe::DynamicType::pp) },
        { DynamicType::P, mpe::dynamicLevelFromType(mpe::DynamicType::p) },
        { DynamicType::MP, mpe::dynamicLevelFromType(mpe::DynamicType::mp) },
        { DynamicType::MF, mpe::dynamicLevelFromType(mpe::DynamicType::mf) },
        { DynamicType::F, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { DynamicType::FF, mpe::dynamicLevelFromType(mpe::DynamicType::ff) },
        { DynamicType::FFF, mpe::dynamicLevelFromType(mpe::DynamicType::fff) },
        { DynamicType::FFFF, mpe::dynamicLevelFromType(mpe::DynamicType::ffff) },
        { DynamicType::FFFFF, mpe::dynamicLevelFromType(mpe::DynamicType::fffff) },
        { DynamicType::FFFFFF, mpe::dynamicLevelFromType(mpe::DynamicType::ffffff) },
        { DynamicType::SF, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { DynamicType::SFZ, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { DynamicType::SFF, mpe::dynamicLevelFromType(mpe::DynamicType::ff) },
        { DynamicType::SFFZ, mpe::dynamicLevelFromType(mpe::DynamicType::ff) },
        { DynamicType::RFZ, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { DynamicType::RF, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { DynamicType::FZ, mpe::dynamicLevelFromType(mpe::DynamicType::f) },
        { DynamicType::N, mpe::dynamicLevelFromType(mpe::DynamicType::ppppppppp) }
    };

    auto search = DYNAMIC_LEVELS.find(type);

    if (search != DYNAMIC_LEVELS.cend()) {
        return search->second;
    }

    return defLevel;
}

inline bool isOrdinaryDynamicType(const DynamicType type)
{
    static const std::set<DynamicType> ORDINARY_DYNAMIC_TYPES = {
        DynamicType::PPPPPP,
        DynamicType::PPPPP,
        DynamicType::PPPP,
        DynamicType::PPP,
        DynamicType::PP,
        DynamicType::P,
        DynamicType::MP,
        DynamicType::MF,
        DynamicType::F,
        DynamicType::FF,
        DynamicType::FFF,
        DynamicType::FFFF,
        DynamicType::FFFFF,
        DynamicType::FFFFFF
    };

    return ORDINARY_DYNAMIC_TYPES.find(type) != ORDINARY_DYNAMIC_TYPES.cend();
}

inline bool isSingleNoteDynamicType(const DynamicType type)
{
    static const std::set<DynamicType> SINGLE_NOTE_DYNAMIC_TYPES = {
        DynamicType::SF,
        DynamicType::SFZ,
        DynamicType::SFFZ,
        DynamicType::RFZ,
        DynamicType::RF
    };

    return SINGLE_NOTE_DYNAMIC_TYPES.find(type) != SINGLE_NOTE_DYNAMIC_TYPES.cend();
}

inline const DynamicTransition& dynamicTransitionFromType(const DynamicType type)
{
    static const std::unordered_map<DynamicType, DynamicTransition> DYNAMIC_TRANSITIONS = {
        { DynamicType::FP, { DynamicType::F, DynamicType::P } },
        { DynamicType::PF, { DynamicType::P, DynamicType::F } },
        { DynamicType::SFP, { DynamicType::F, DynamicType::P } },
        { DynamicType::SFPP, { DynamicType::F, DynamicType::PP } }
    };

    auto search = DYNAMIC_TRANSITIONS.find(type);
    if (search != DYNAMIC_TRANSITIONS.cend()) {
        return search->second;
    }

    static DynamicTransition empty;
    return empty;
}

inline mpe::ArticulationType articulationFromPlayTechType(const PlayingTechniqueType technique)
{
    static const std::unordered_map<PlayingTechniqueType, mpe::ArticulationType> PLAYING_TECH_TYPES = {
        { PlayingTechniqueType::Undefined, mpe::ArticulationType::Undefined },
        { PlayingTechniqueType::Natural, mpe::ArticulationType::Standard },
        { PlayingTechniqueType::Pizzicato, mpe::ArticulationType::Pizzicato },
        { PlayingTechniqueType::Open, mpe::ArticulationType::Open },
        { PlayingTechniqueType::Mute, mpe::ArticulationType::Mute },
        { PlayingTechniqueType::Tremolo, mpe::ArticulationType::Tremolo64th },
        { PlayingTechniqueType::Detache, mpe::ArticulationType::Detache },
        { PlayingTechniqueType::Martele, mpe::ArticulationType::Martele },
        { PlayingTechniqueType::ColLegno, mpe::ArticulationType::ColLegno },
        { PlayingTechniqueType::SulPonticello, mpe::ArticulationType::SulPont },
        { PlayingTechniqueType::SulTasto, mpe::ArticulationType::SulTasto },
        { PlayingTechniqueType::Distortion, mpe::ArticulationType::Distortion },
        { PlayingTechniqueType::Overdrive, mpe::ArticulationType::Overdrive },
        { PlayingTechniqueType::Harmonics, mpe::ArticulationType::Harmonic },
        { PlayingTechniqueType::JazzTone, mpe::ArticulationType::JazzTone },
    };

    auto search = PLAYING_TECH_TYPES.find(technique);

    if (search != PLAYING_TECH_TYPES.cend()) {
        return search->second;
    }

    return mpe::ArticulationType::Undefined;
}
}

#endif // MU_ENGRAVING_DYNAMICUTILS_H
