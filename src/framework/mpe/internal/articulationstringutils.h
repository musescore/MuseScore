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

#ifndef MU_MPE_STRINGUTILS_H
#define MU_MPE_STRINGUTILS_H

#include <QString>
#include <unordered_map>

#include "mpetypes.h"

namespace mu::mpe {
static const std::unordered_map<ArticulationFamily, QString> ARTICULATION_FAMILY_NAMES = {
    { ArticulationFamily::Undefined, "Undefined" },
    { ArticulationFamily::Keyboards, "KeyboardsArticulation" },
    { ArticulationFamily::Strings, "StringsArticulation" },
    { ArticulationFamily::Winds, "WindsArticulation" },
    { ArticulationFamily::Percussions, "PercussionsArticulation" },
    { ArticulationFamily::Voices, "VoicesArticulation" }
};

static const std::unordered_map<ArticulationType, QString> ARTICULATION_TYPE_NAMES = {
    { ArticulationType::Undefined, "Undefined" },
    { ArticulationType::Standard, "Standard" },
    { ArticulationType::Staccato, "Staccato" },
    { ArticulationType::Staccatissimo, "Staccatissimo" },
    { ArticulationType::Tenuto, "Tenuto" },
    { ArticulationType::Marcato, "Marcato" },
    { ArticulationType::Accent, "Accent" },
    { ArticulationType::SoftAccent, "SoftAccent" },
    { ArticulationType::LaissezVibrer, "LaissezVibrer" },
    { ArticulationType::Subito, "Subito" },
    { ArticulationType::FadeIn, "FadeIn" },
    { ArticulationType::FadeOut, "FadeOut" },
    { ArticulationType::Harmonic, "Harmonic" },
    { ArticulationType::JazzTone, "JazzTone" },
    { ArticulationType::Mute, "Mute" },
    { ArticulationType::Open, "Open" },
    { ArticulationType::Pizzicato, "Pizzicato" },
    { ArticulationType::SnapPizzicato, "SnapPizzicato" },
    { ArticulationType::RandomPizzicato, "RandomPizzicato" },
    { ArticulationType::UpBow, "UpBow" },
    { ArticulationType::DownBow, "DownBow" },
    { ArticulationType::Detache, "Detache" },
    { ArticulationType::Martele, "Martele" },
    { ArticulationType::Jete, "Jete" },
    { ArticulationType::ColLegno, "ColLegno" },
    { ArticulationType::SulPont, "SulPonticello" },
    { ArticulationType::SulTasto, "SulTasto" },
    { ArticulationType::GhostNote, "GhostNote" },
    { ArticulationType::CrossNote, "CrossNote" },
    { ArticulationType::CircleNote, "CircleNote" },
    { ArticulationType::TriangleNote, "TriangleNote" },
    { ArticulationType::DiamondNote, "DiamondNote" },
    { ArticulationType::Fall, "Fall" },
    { ArticulationType::QuickFall, "QuickFall" },
    { ArticulationType::Doit, "Doit" },
    { ArticulationType::Plop, "Plop" },
    { ArticulationType::Scoop, "Scoop" },
    { ArticulationType::Bend, "Bend" },
    { ArticulationType::SlideOutDown, "SlideOutDown" },
    { ArticulationType::SlideOutUp, "SlideOutUp" },
    { ArticulationType::SlideInAbove, "SlideInAbove" },
    { ArticulationType::SlideInBelow, "SlideInBelow" },
    { ArticulationType::Crescendo, "Crescendo" },
    { ArticulationType::Decrescendo, "Decrescendo" },
    { ArticulationType::DiscreteGlissando, "DiscreteGlissando" },
    { ArticulationType::ContinuousGlissando, "ContinuousGlissando" },
    { ArticulationType::Legato, "Legato" },
    { ArticulationType::Pedal, "Pedal" },
    { ArticulationType::Arpeggio, "Arpeggio" },
    { ArticulationType::ArpeggioUp, "ArpeggioUp" },
    { ArticulationType::ArpeggioDown, "ArpeggioDown" },
    { ArticulationType::ArpeggioStraightUp, "ArpeggioStraightUp" },
    { ArticulationType::ArpeggioStraightDown, "ArpeggioStraightDown" },
    { ArticulationType::Vibrato, "Vibrato" },
    { ArticulationType::WideVibrato, "WideVibrato" },
    { ArticulationType::MoltoVibrato, "MoltoVibrato" },
    { ArticulationType::SenzaVibrato, "SenzaVibrato" },
    { ArticulationType::Tremolo8th, "Tremolo8th" },
    { ArticulationType::Tremolo16th, "Tremolo16th" },
    { ArticulationType::Tremolo32nd, "Tremolo32nd" },
    { ArticulationType::Tremolo64th, "Tremolo64th" },
    { ArticulationType::UpperMordent, "ShortTrill" },
    { ArticulationType::Trill, "Trill" },
    { ArticulationType::TrillBaroque, "TrillBaroque" },
    { ArticulationType::LowerMordent, "Mordent" },
    { ArticulationType::PrallMordent, "PrallMordent" },
    { ArticulationType::UpperMordentBaroque, "UpperMordentBaroque" },
    { ArticulationType::LowerMordentBaroque, "LowerMordentBaroque" },
    { ArticulationType::MordentWithUpperPrefix, "MordentWithUpperPrefix" },
    { ArticulationType::UpMordent, "UpMordent" },
    { ArticulationType::DownMordent, "DownMordent" },
    { ArticulationType::Tremblement, "Tremblement" },
    { ArticulationType::UpPrall, "UpPrall" },
    { ArticulationType::PrallUp, "PrallUp" },
    { ArticulationType::PrallDown, "PrallDown" },
    { ArticulationType::LinePrall, "LinePrall" },
    { ArticulationType::Slide, "Slide" },
    { ArticulationType::Turn, "Turn" },
    { ArticulationType::InvertedTurn, "InvertedTurn" },
    { ArticulationType::PreAppoggiatura, "PreAppoggiatura" },
    { ArticulationType::PostAppoggiatura, "PostAppoggiatura" },
    { ArticulationType::Acciaccatura, "Acciaccatura" },
    { ArticulationType::TremoloBar, "TremoloBar" },
    { ArticulationType::Distortion, "Distortion" },
    { ArticulationType::Overdrive, "Overdrive" },
    { ArticulationType::Slap, "Slap" },
    { ArticulationType::Pop, "Pop" },
};

inline ArticulationFamily articulationFamilyFromString(const QString& str)
{
    auto search = std::find_if(ARTICULATION_FAMILY_NAMES.begin(), ARTICULATION_FAMILY_NAMES.end(), [str](const auto& pair) {
        return pair.second == str;
    });

    if (search == ARTICULATION_FAMILY_NAMES.cend()) {
        return ArticulationFamily::Undefined;
    }

    return search->first;
}

inline QString articulationFamilyToString(const ArticulationFamily family)
{
    auto search = ARTICULATION_FAMILY_NAMES.find(family);

    if (search == ARTICULATION_FAMILY_NAMES.cend()) {
        return QString();
    }

    return search->second;
}

inline ArticulationType articulationTypeFromString(const QString& str)
{
    auto search = std::find_if(ARTICULATION_TYPE_NAMES.begin(), ARTICULATION_TYPE_NAMES.end(), [str](const auto& pair) {
        return pair.second == str;
    });

    if (search == ARTICULATION_TYPE_NAMES.cend()) {
        return ArticulationType::Undefined;
    }

    return search->first;
}

inline QString articulationTypeToString(const ArticulationType type)
{
    auto search = ARTICULATION_TYPE_NAMES.find(type);

    if (search == ARTICULATION_TYPE_NAMES.cend()) {
        return QString();
    }

    return search->second;
}
}

#endif // MU_MPE_STRINGUTILS_H
