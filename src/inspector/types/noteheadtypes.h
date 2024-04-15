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
#ifndef MU_INSPECTOR_NOTEHEADTYPES_H
#define MU_INSPECTOR_NOTEHEADTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class NoteHeadTypes
{
    Q_GADGET

public:
    enum class Group {
        HEAD_NORMAL = 0,
        HEAD_CROSS,
        HEAD_PLUS,
        HEAD_XCIRCLE,
        HEAD_WITHX,
        HEAD_TRIANGLE_UP,
        HEAD_TRIANGLE_DOWN,
        HEAD_SLASHED1,
        HEAD_SLASHED2,
        HEAD_DIAMOND,
        HEAD_DIAMOND_OLD,
        HEAD_CIRCLED,
        HEAD_CIRCLED_LARGE,
        HEAD_LARGE_ARROW,
        HEAD_BREVIS_ALT,

        HEAD_SLASH,
        HEAD_LARGE_DIAMOND,

        HEAD_SOL,
        HEAD_LA,
        HEAD_FA,
        HEAD_MI,
        HEAD_DO,
        HEAD_RE,
        HEAD_TI
    };

    enum class Type {
        TYPE_AUTO = -1,
        TYPE_WHOLE,
        TYPE_HALF,
        TYPE_QUARTER,
        TYPE_BREVIS
    };

    enum class NoteDotPosition {
        DOT_POSITION_AUTO,
        DOT_POSITION_UP,
        DOT_POSITION_DOWN
    };

    enum class SchemeType {
        SCHEME_AUTO = -1,
        SCHEME_NORMAL,
        SCHEME_PITCHNAME,
        SCHEME_PITCHNAME_GERMAN,
        SCHEME_SOLFEGE,
        SCHEME_SOLFEGE_FIXED,
        SCHEME_SHAPE_NOTE_4,
        SCHEME_SHAPE_NOTE_7_AIKIN,
        SCHEME_SHAPE_NOTE_7_FUNK,
        SCHEME_SHAPE_NOTE_7_WALKER
    };

    Q_ENUM(Group)
    Q_ENUM(Type)
    Q_ENUM(NoteDotPosition)
    Q_ENUM(SchemeType)
};
}

#endif // MU_INSPECTOR_NOTEHEADTYPES_H
