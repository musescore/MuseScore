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

#include <QList>
#include <qqmlintegration.h>

#include "engraving/dom/textbase.h"
#include "engraving/types/types.h"

namespace mu::propertiespanel {
namespace TextTypes {
Q_NAMESPACE;
QML_ELEMENT;

enum class FontStyle {
    //FONT_STYLE_UNDEFINED = int(engraving::FontStyle::Undefined),
    FONT_STYLE_NORMAL = int(engraving::FontStyle::Normal),
    FONT_STYLE_BOLD = int(engraving::FontStyle::Bold),
    FONT_STYLE_ITALIC = int(engraving::FontStyle::Italic),
    FONT_STYLE_UNDERLINE = int(engraving::FontStyle::Underline),
    FONT_STYLE_STRIKE = int(engraving::FontStyle::Strike)
};
Q_ENUM_NS(FontStyle)

enum class FontHorizontalAlignment {
    FONT_ALIGN_H_LEFT = int(engraving::AlignH::LEFT),
    FONT_ALIGN_H_RIGHT = int(engraving::AlignH::RIGHT),
    FONT_ALIGN_H_CENTER = int(engraving::AlignH::HCENTER),
    FONT_ALIGN_H_JUSTIFY = int(engraving::AlignH::JUSTIFY),
};
Q_ENUM_NS(FontHorizontalAlignment)

enum class FontVerticalAlignment {
    FONT_ALIGN_V_TOP = int(engraving::AlignV::TOP),
    FONT_ALIGN_V_CENTER = int(engraving::AlignV::VCENTER),
    FONT_ALIGN_V_BOTTOM = int(engraving::AlignV::BOTTOM),
    FONT_ALIGN_V_BASELINE = int(engraving::AlignV::BASELINE)
};
Q_ENUM_NS(FontVerticalAlignment)

enum class FrameType {
    FRAME_TYPE_NONE = int(engraving::FrameType::NO_FRAME),
    FRAME_TYPE_SQUARE = int(engraving::FrameType::RECTANGLE),
    FRAME_TYPE_CIRCLE = int(engraving::FrameType::CIRCLE)
};
Q_ENUM_NS(FrameType)

enum class TextSubscriptMode {
    TEXT_SUBSCRIPT_NORMAL = int(engraving::VerticalAlignment::AlignNormal),
    TEXT_SUBSCRIPT_TOP = int(engraving::VerticalAlignment::AlignSuperScript),
    TEXT_SUBSCRIPT_BOTTOM = int(engraving::VerticalAlignment::AlignSubScript)
};
Q_ENUM_NS(TextSubscriptMode)
}

static inline QList<mu::engraving::ElementType> TEXT_ELEMENT_TYPES = {
    mu::engraving::ElementType::TEXT,
    mu::engraving::ElementType::STAFF_TEXT,
    mu::engraving::ElementType::SYSTEM_TEXT,
    mu::engraving::ElementType::TRIPLET_FEEL,
    mu::engraving::ElementType::DYNAMIC,
    mu::engraving::ElementType::EXPRESSION,
    mu::engraving::ElementType::FIGURED_BASS,
    mu::engraving::ElementType::FINGERING,
    mu::engraving::ElementType::HARMONY,
    mu::engraving::ElementType::INSTRUMENT_CHANGE,
    mu::engraving::ElementType::JUMP,
    mu::engraving::ElementType::LYRICS,
    mu::engraving::ElementType::MARKER,
    mu::engraving::ElementType::MEASURE_NUMBER,
    mu::engraving::ElementType::REHEARSAL_MARK,
    mu::engraving::ElementType::STICKING,
    mu::engraving::ElementType::TEMPO_TEXT,
    mu::engraving::ElementType::TUPLET,
    mu::engraving::ElementType::PLAY_COUNT_TEXT,
    mu::engraving::ElementType::PLAYTECH_ANNOTATION,
    mu::engraving::ElementType::CAPO,
    mu::engraving::ElementType::STRING_TUNINGS,
    mu::engraving::ElementType::HARP_DIAGRAM,
    mu::engraving::ElementType::SOUND_FLAG,
    mu::engraving::ElementType::HAMMER_ON_PULL_OFF_TEXT,
};
}
