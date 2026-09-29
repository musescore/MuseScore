/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <qqmlintegration.h>

#include "engraving/types/types.h"

namespace mu::propertiespanel {
namespace OrnamentTypes {
Q_NAMESPACE;
QML_ELEMENT;

// For ornaments that only accept intervals of second (turns...)
// a simplified view over engraving::OrnamentInterval, which has no single enum equivalent
enum class BasicInterval {
    TYPE_INVALID,
    TYPE_AUTO_DIATONIC,
    TYPE_MAJOR_SECOND,
    TYPE_MINOR_SECOND,
    TYPE_AUGMENTED_SECOND
};
Q_ENUM_NS(BasicInterval)

// For ornaments that can define a custom interval (trills)
enum class IntervalStep {
    STEP_UNISON = int(engraving::IntervalStep::UNISON),
    STEP_SECOND = int(engraving::IntervalStep::SECOND),
    STEP_THIRD = int(engraving::IntervalStep::THIRD),
    STEP_FOURTH = int(engraving::IntervalStep::FOURTH),
    STEP_FIFTH = int(engraving::IntervalStep::FIFTH),
    STEP_SIXTH = int(engraving::IntervalStep::SIXTH),
    STEP_SEVENTH = int(engraving::IntervalStep::SEVENTH),
    STEP_OCTAVE = int(engraving::IntervalStep::OCTAVE),
};
Q_ENUM_NS(IntervalStep)

enum class IntervalType {
    TYPE_AUTO = int(engraving::IntervalType::AUTO),
    TYPE_AUGMENTED = int(engraving::IntervalType::AUGMENTED),
    TYPE_MAJOR = int(engraving::IntervalType::MAJOR),
    TYPE_PERFECT = int(engraving::IntervalType::PERFECT),
    TYPE_MINOR = int(engraving::IntervalType::MINOR),
    TYPE_DIMINISHED = int(engraving::IntervalType::DIMINISHED)
};
Q_ENUM_NS(IntervalType)

enum class OrnamentShowAccidental {
    SHOW_ACCIDENTAL_DEFAULT = int(engraving::OrnamentShowAccidental::DEFAULT),
    SHOW_ACCIDENTAL_ANY_ALTERATION = int(engraving::OrnamentShowAccidental::ANY_ALTERATION),
    SHOW_ACCIDENTAL_ALWAYS = int(engraving::OrnamentShowAccidental::ALWAYS)
};
Q_ENUM_NS(OrnamentShowAccidental)
}
}
