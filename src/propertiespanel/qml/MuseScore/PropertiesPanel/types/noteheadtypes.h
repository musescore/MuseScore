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
namespace NoteHeadTypes {
Q_NAMESPACE;
QML_NAMED_ELEMENT(NoteHead);

enum class Group {
    HEAD_NORMAL = int(engraving::NoteHeadGroup::HEAD_NORMAL),
    HEAD_CROSS = int(engraving::NoteHeadGroup::HEAD_CROSS),
    HEAD_PLUS = int(engraving::NoteHeadGroup::HEAD_PLUS),
    HEAD_XCIRCLE = int(engraving::NoteHeadGroup::HEAD_XCIRCLE),
    HEAD_WITHX = int(engraving::NoteHeadGroup::HEAD_WITHX),
    HEAD_TRIANGLE_UP = int(engraving::NoteHeadGroup::HEAD_TRIANGLE_UP),
    HEAD_TRIANGLE_DOWN = int(engraving::NoteHeadGroup::HEAD_TRIANGLE_DOWN),
    HEAD_SLASHED1 = int(engraving::NoteHeadGroup::HEAD_SLASHED1),
    HEAD_SLASHED2 = int(engraving::NoteHeadGroup::HEAD_SLASHED2),
    HEAD_DIAMOND = int(engraving::NoteHeadGroup::HEAD_DIAMOND),
    HEAD_DIAMOND_OLD = int(engraving::NoteHeadGroup::HEAD_DIAMOND_OLD),
    HEAD_CIRCLED = int(engraving::NoteHeadGroup::HEAD_CIRCLED),
    HEAD_CIRCLED_LARGE = int(engraving::NoteHeadGroup::HEAD_CIRCLED_LARGE),
    HEAD_LARGE_ARROW = int(engraving::NoteHeadGroup::HEAD_LARGE_ARROW),
    HEAD_BREVIS_ALT = int(engraving::NoteHeadGroup::HEAD_BREVIS_ALT),

    HEAD_SLASH = int(engraving::NoteHeadGroup::HEAD_SLASH),
    HEAD_LARGE_DIAMOND = int(engraving::NoteHeadGroup::HEAD_LARGE_DIAMOND),

    HEAD_SOL = int(engraving::NoteHeadGroup::HEAD_SOL),
    HEAD_LA = int(engraving::NoteHeadGroup::HEAD_LA),
    HEAD_FA = int(engraving::NoteHeadGroup::HEAD_FA),
    HEAD_MI = int(engraving::NoteHeadGroup::HEAD_MI),
    HEAD_DO = int(engraving::NoteHeadGroup::HEAD_DO),
    HEAD_RE = int(engraving::NoteHeadGroup::HEAD_RE),
    HEAD_TI = int(engraving::NoteHeadGroup::HEAD_TI),
};
Q_ENUM_NS(Group)

enum class Type {
    TYPE_AUTO = int(engraving::NoteHeadType::HEAD_AUTO),
    TYPE_WHOLE = int(engraving::NoteHeadType::HEAD_WHOLE),
    TYPE_HALF = int(engraving::NoteHeadType::HEAD_HALF),
    TYPE_QUARTER = int(engraving::NoteHeadType::HEAD_QUARTER),
    TYPE_BREVIS = int(engraving::NoteHeadType::HEAD_BREVIS)
};
Q_ENUM_NS(Type)

enum class NoteDotPosition {
    DOT_POSITION_AUTO = int(engraving::DirectionV::AUTO),
    DOT_POSITION_UP = int(engraving::DirectionV::UP),
    DOT_POSITION_DOWN = int(engraving::DirectionV::DOWN)
};
Q_ENUM_NS(NoteDotPosition)

enum class SchemeType {
    SCHEME_AUTO = int(engraving::NoteHeadScheme::HEAD_AUTO),
    SCHEME_NORMAL = int(engraving::NoteHeadScheme::HEAD_NORMAL),
    SCHEME_PITCHNAME = int(engraving::NoteHeadScheme::HEAD_PITCHNAME),
    SCHEME_PITCHNAME_NO_ACCIDENTALS = int(engraving::NoteHeadScheme::HEAD_PITCHNAME_NO_ACCIDENTALS),
    SCHEME_PITCHNAME_GERMAN = int(engraving::NoteHeadScheme::HEAD_PITCHNAME_GERMAN),
    SCHEME_PITCHNAME_GERMAN_NO_ACCIDENTALS = int(engraving::NoteHeadScheme::HEAD_PITCHNAME_GERMAN_NO_ACCIDENTALS),
    SCHEME_SOLFEGE = int(engraving::NoteHeadScheme::HEAD_SOLFEGE),
    SCHEME_SOLFEGE_FIXED = int(engraving::NoteHeadScheme::HEAD_SOLFEGE_FIXED),
    SCHEME_SHAPE_NOTE_4 = int(engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_4),
    SCHEME_SHAPE_NOTE_7_AIKIN = int(engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN),
    SCHEME_SHAPE_NOTE_7_FUNK = int(engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK),
    SCHEME_SHAPE_NOTE_7_WALKER = int(engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER)
};
Q_ENUM_NS(SchemeType)

enum class ParenthesesType {
    PAREN_NONE = int(engraving::ParenthesesMode::NONE),
    PAREN_LEFT = int(engraving::ParenthesesMode::LEFT),
    PAREN_RIGHT = int(engraving::ParenthesesMode::RIGHT),
    PAREN_BOTH = int(engraving::ParenthesesMode::BOTH)
};
Q_ENUM_NS(ParenthesesType)
}
}
