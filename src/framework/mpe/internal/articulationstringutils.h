/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#pragma once

#include <QString>
#include <unordered_map>

#include "mpetypes.h"

namespace muse::mpe {
static const std::unordered_map<ArticulationFamily, QString> ARTICULATION_FAMILY_NAMES {
    { ArticulationFamily::Undefined, "Undefined" },
    { ArticulationFamily::Keyboards, "KeyboardsArticulation" },
    { ArticulationFamily::Strings, "StringsArticulation" },
    { ArticulationFamily::Winds, "WindsArticulation" },
    { ArticulationFamily::Percussions, "PercussionsArticulation" },
    { ArticulationFamily::Voices, "VoicesArticulation" },
};

static const std::unordered_map<ArticulationType, QString> ARTICULATION_TYPE_NAMES {
    { ArticulationType::Undefined, "Undefined" },
    { ArticulationType::Standard, "Standard" },
    { ArticulationType::Staccato, "Staccato" },
    { ArticulationType::Staccatissimo, "Staccatissimo" },
    { ArticulationType::Tenuto, "Tenuto" },
    { ArticulationType::Marcato, "Marcato" },
    { ArticulationType::Accent, "Accent" },
    { ArticulationType::SoftAccent, "SoftAccent" },
    { ArticulationType::LaissezVibrer, "LaissezVibrer" },
    { ArticulationType::LetRing, "LetRing" },
    { ArticulationType::Subito, "Subito" },
    { ArticulationType::FadeIn, "FadeIn" },
    { ArticulationType::FadeOut, "FadeOut" },
    { ArticulationType::Harmonic, "Harmonic" },
    { ArticulationType::JazzTone, "JazzTone" },
    { ArticulationType::PalmMute, "PalmMute" },
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
    { ArticulationType::CrossLargeNote, "CrossLargeNote" },
    { ArticulationType::CrossOrnateNote, "CrossOrnateNote" },
    { ArticulationType::CircleNote, "CircleNote" },
    { ArticulationType::CircleCrossNote, "CircleCrossNote" },
    { ArticulationType::CircleDotNote, "CircleDotNote" },
    { ArticulationType::TriangleLeftNote, "TriangleLeftNote" },
    { ArticulationType::TriangleRightNote, "TriangleRightNote" },
    { ArticulationType::TriangleUpNote, "TriangleUpNote" },
    { ArticulationType::TriangleDownNote, "TriangleDownNote" },
    { ArticulationType::TriangleRoundDownNote, "TriangleRoundDownNote" },
    { ArticulationType::DiamondNote, "DiamondNote" },
    { ArticulationType::MoonNote, "MoonNote" },
    { ArticulationType::PlusNote, "PlusNote" },
    { ArticulationType::SlashNote, "SlashNote" },
    { ArticulationType::SquareNote, "SquareNote" },
    { ArticulationType::SlashedBackwardsNote, "SlashedBackwardsNote" },
    { ArticulationType::SlashedForwardsNote, "SlashedForwardsNote" },
    { ArticulationType::Fall, "Fall" },
    { ArticulationType::QuickFall, "QuickFall" },
    { ArticulationType::Doit, "Doit" },
    { ArticulationType::Plop, "Plop" },
    { ArticulationType::Scoop, "Scoop" },
    { ArticulationType::BrassBend, "BrassBend" },
    { ArticulationType::Multibend, "Multibend" },
    { ArticulationType::SlideOutDown, "SlideOutDown" },
    { ArticulationType::SlideOutUp, "SlideOutUp" },
    { ArticulationType::SlideInAbove, "SlideInAbove" },
    { ArticulationType::SlideInBelow, "SlideInBelow" },
    { ArticulationType::Crescendo, "Crescendo" },
    { ArticulationType::Diminuendo, "Diminuendo" },
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
    { ArticulationType::TremoloBuzz, "TremoloBuzz" },
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
    { ArticulationType::LeftHandTapping, "LeftHandTapping" },
    { ArticulationType::RightHandTapping, "RightHandTapping" },
    { ArticulationType::MalletBellOnTable, "MalletBellOnTable" },
    { ArticulationType::MalletBellSuspended, "MalletBellSuspended" },
    { ArticulationType::MalletLift, "MalletLift" },
    { ArticulationType::Pluck, "Pluck" },
    { ArticulationType::PluckLift, "PluckLift" },
    { ArticulationType::Gyro, "Gyro" },
    { ArticulationType::Martellato, "Martellato" },
    { ArticulationType::MartellatoLift, "MartellatoLift" },
    { ArticulationType::HandMartellato, "HandMartellato" },
    { ArticulationType::MutedMartellato, "MutedMartellato" },
    { ArticulationType::ThumbDamp, "ThumbDamp" },
    { ArticulationType::BrushDamp, "BrushDamp" },
    { ArticulationType::Ring, "Ring" },
    { ArticulationType::RingTouch, "RingTouch" },
    { ArticulationType::SingingBell, "SingingBell" },
    { ArticulationType::SingingVibrate, "SingingVibrate" },
};

inline ArticulationFamily articulationFamilyFromString(const QString& str)
{
    return muse::key(ARTICULATION_FAMILY_NAMES, str, ArticulationFamily::Undefined);
}

inline const QString articulationFamilyToString(const ArticulationFamily family)
{
    return muse::value(ARTICULATION_FAMILY_NAMES, family);
}

inline ArticulationType articulationTypeFromString(const QString& str)
{
    return muse::key(ARTICULATION_TYPE_NAMES, str, ArticulationType::Undefined);
}

inline const QString articulationTypeToString(const ArticulationType type)
{
    return muse::value(ARTICULATION_TYPE_NAMES, type);
}
}
