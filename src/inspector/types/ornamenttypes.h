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
#ifndef MU_INSPECTOR_ORNAMENTTYPES_H
#define MU_INSPECTOR_ORNAMENTTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class OrnamentTypes
{
    Q_GADGET

public:
    enum class Style {
        STYLE_STANDARD = 0,
        STYLE_BAROQUE
    };

    // For ornaments that only accept intervals of second (turns...)
    enum class BasicInterval {
        TYPE_INVALID,
        TYPE_AUTO_DIATONIC,
        TYPE_MAJOR_SECOND,
        TYPE_MINOR_SECOND,
        TYPE_AUGMENTED_SECOND
    };

    // For ornaments that can define a custom interval (trills)
    enum class IntervalStep {
        STEP_UNISON = 0,
        STEP_SECOND,
        STEP_THIRD,
        STEP_FOURTH,
        STEP_FIFTH,
        STEP_SIXTH,
        STEP_SEVENTH,
        STEP_OCTAVE,
    };

    enum class IntervalType {
        TYPE_AUTO = 0,
        TYPE_AUGMENTED,
        TYPE_MAJOR,
        TYPE_PERFECT,
        TYPE_MINOR,
        TYPE_DIMINISHED
    };

    enum class OrnamentShowAccidental {
        SHOW_ACCIDENTAL_DEFAULT,
        SHOW_ACCIDENTAL_ANY_ALTERATION,
        SHOW_ACCIDENTAL_ALWAYS
    };

    Q_ENUM(Style)
    Q_ENUM(BasicInterval)
    Q_ENUM(IntervalStep)
    Q_ENUM(IntervalType)
    Q_ENUM(OrnamentShowAccidental)
};
}

#endif // MU_INSPECTOR_ORNAMENTTYPES_H
