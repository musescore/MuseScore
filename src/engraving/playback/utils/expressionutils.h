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
struct DynamicTransition {
    DynamicType from = DynamicType::OTHER;
    DynamicType to = DynamicType::OTHER;

    bool isValid() const
    {
        return from != DynamicType::OTHER && to != DynamicType::OTHER;
    }
};

inline muse::mpe::dynamic_level_t dynamicLevelFromType(const DynamicType type,
                                                       const muse::mpe::dynamic_level_t defLevel = muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::Natural))
{
    static const std::unordered_map<DynamicType, muse::mpe::dynamic_level_t> DYNAMIC_LEVELS = {
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
        { DynamicType::FFFFFF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::ffffff) },
        { DynamicType::SF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::f) },
        { DynamicType::SFZ, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::f) },
        { DynamicType::SFF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::ff) },
        { DynamicType::SFFZ, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::ff) },
        { DynamicType::SFFF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::fff) },
        { DynamicType::SFFFZ, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::fff) },
        { DynamicType::RFZ, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::f) },
        { DynamicType::RF, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::f) },
        { DynamicType::FZ, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::f) },
        { DynamicType::N, muse::mpe::dynamicLevelFromType(muse::mpe::DynamicType::ppppppppp) }
    };

    auto search = DYNAMIC_LEVELS.find(type);

    if (search != DYNAMIC_LEVELS.cend()) {
        return search->second;
    }

    return defLevel;
}

inline bool isOrdinaryDynamicType(const DynamicType type)
{
    static const std::unordered_set<DynamicType> ORDINARY_DYNAMIC_TYPES = {
        DynamicType::N,
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
    static const std::unordered_set<DynamicType> SINGLE_NOTE_DYNAMIC_TYPES = {
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
}

#endif // MU_ENGRAVING_DYNAMICUTILS_H
