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

#include <QObject>
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

enum class TextType {
    TEXT_TYPE_DEFAULT = int(engraving::TextStyleType::DEFAULT),
    TEXT_TYPE_TITLE = int(engraving::TextStyleType::TITLE),
    TEXT_TYPE_SUBTITLE = int(engraving::TextStyleType::SUBTITLE),
    TEXT_TYPE_COMPOSER = int(engraving::TextStyleType::COMPOSER),
    TEXT_TYPE_POET = int(engraving::TextStyleType::LYRICIST),
    TEXT_TYPE_TRANSLATOR = int(engraving::TextStyleType::TRANSLATOR),
    TEXT_TYPE_FRAME = int(engraving::TextStyleType::FRAME),
    TEXT_TYPE_INSTRUMENT_EXCERPT = int(engraving::TextStyleType::INSTRUMENT_EXCERPT),
    TEXT_TYPE_INSTRUMENT_LONG = int(engraving::TextStyleType::INSTRUMENT_LONG),
    TEXT_TYPE_INSTRUMENT_SHORT = int(engraving::TextStyleType::INSTRUMENT_SHORT),
    TEXT_TYPE_INSTRUMENT_CHANGE = int(engraving::TextStyleType::INSTRUMENT_CHANGE),
    TEXT_TYPE_HEADER = int(engraving::TextStyleType::HEADER),
    TEXT_TYPE_FOOTER = int(engraving::TextStyleType::FOOTER),
    TEXT_TYPE_COPYRIGHT = int(engraving::TextStyleType::COPYRIGHT),
    TEXT_TYPE_PAGE_NUMBER = int(engraving::TextStyleType::PAGE_NUMBER),
    TEXT_TYPE_MEASURE_NUMBER = int(engraving::TextStyleType::MEASURE_NUMBER),
    TEXT_TYPE_MMREST_RANGE = int(engraving::TextStyleType::MMREST_RANGE),
    TEXT_TYPE_TEMPO = int(engraving::TextStyleType::TEMPO),
    TEXT_TYPE_TEMPO_CHANGE = int(engraving::TextStyleType::TEMPO_CHANGE),
    TEXT_TYPE_METRONOME = int(engraving::TextStyleType::METRONOME),
    TEXT_TYPE_REPEAT_LEFT = int(engraving::TextStyleType::REPEAT_LEFT),       // align to start of measure
    TEXT_TYPE_REPEAT_RIGHT = int(engraving::TextStyleType::REPEAT_RIGHT),      // align to end of measure
    TEXT_TYPE_REHEARSAL_MARK = int(engraving::TextStyleType::REHEARSAL_MARK),
    TEXT_TYPE_SYSTEM = int(engraving::TextStyleType::SYSTEM),
    TEXT_TYPE_STAFF = int(engraving::TextStyleType::STAFF),
    TEXT_TYPE_EXPRESSION = int(engraving::TextStyleType::EXPRESSION),
    TEXT_TYPE_DYNAMICS = int(engraving::TextStyleType::DYNAMICS),
    TEXT_TYPE_HAIRPIN = int(engraving::TextStyleType::HAIRPIN),
    TEXT_TYPE_LYRICS_ODD = int(engraving::TextStyleType::LYRICS_ODD),
    TEXT_TYPE_LYRICS_EVEN = int(engraving::TextStyleType::LYRICS_EVEN),
    TEXT_TYPE_HARMONY_A = int(engraving::TextStyleType::HARMONY_A),
    TEXT_TYPE_HARMONY_B = int(engraving::TextStyleType::HARMONY_B),
    TEXT_TYPE_HARMONY_ROMAN = int(engraving::TextStyleType::HARMONY_ROMAN),
    TEXT_TYPE_HARMONY_NASHVILLE = int(engraving::TextStyleType::HARMONY_NASHVILLE),
    TEXT_TYPE_TUPLET = int(engraving::TextStyleType::TUPLET),
    TEXT_TYPE_STICKING = int(engraving::TextStyleType::STICKING),
    TEXT_TYPE_FINGERING = int(engraving::TextStyleType::FINGERING),
    TEXT_TYPE_LH_GUITAR_FINGERING = int(engraving::TextStyleType::LH_GUITAR_FINGERING),
    TEXT_TYPE_RH_GUITAR_FINGERING = int(engraving::TextStyleType::RH_GUITAR_FINGERING),
    TEXT_TYPE_STRING_NUMBER = int(engraving::TextStyleType::STRING_NUMBER),
    TEXT_TYPE_HARP_PEDAL_DIAGRAM = int(engraving::TextStyleType::HARP_PEDAL_DIAGRAM),
    TEXT_TYPE_HARP_PEDAL_TEXT_DIAGRAM = int(engraving::TextStyleType::HARP_PEDAL_TEXT_DIAGRAM),
    TEXT_TYPE_TEXTLINE = int(engraving::TextStyleType::TEXTLINE),
    TEXT_TYPE_VOLTA = int(engraving::TextStyleType::VOLTA),
    TEXT_TYPE_OTTAVA = int(engraving::TextStyleType::OTTAVA),
    TEXT_TYPE_GLISSANDO = int(engraving::TextStyleType::GLISSANDO),
    TEXT_TYPE_PEDAL = int(engraving::TextStyleType::PEDAL),
    TEXT_TYPE_BEND = int(engraving::TextStyleType::BEND),
    TEXT_TYPE_LET_RING = int(engraving::TextStyleType::LET_RING),
    TEXT_TYPE_PALM_MUTE = int(engraving::TextStyleType::PALM_MUTE),
    TEXT_TYPE_USER1 = int(engraving::TextStyleType::USER1),
    TEXT_TYPE_USER2 = int(engraving::TextStyleType::USER2),
    TEXT_TYPE_USER3 = int(engraving::TextStyleType::USER3),
    TEXT_TYPE_USER4 = int(engraving::TextStyleType::USER4),
    TEXT_TYPE_USER5 = int(engraving::TextStyleType::USER5),
    TEXT_TYPE_USER6 = int(engraving::TextStyleType::USER6),
    TEXT_TYPE_USER7 = int(engraving::TextStyleType::USER7),
    TEXT_TYPE_USER8 = int(engraving::TextStyleType::USER8),
    TEXT_TYPE_USER9 = int(engraving::TextStyleType::USER9),
    TEXT_TYPE_USER10 = int(engraving::TextStyleType::USER10),
    TEXT_TYPE_USER11 = int(engraving::TextStyleType::USER11),
    TEXT_TYPE_USER12 = int(engraving::TextStyleType::USER12)
};
Q_ENUM_NS(TextType)

enum class TextSubscriptMode {
    TEXT_SUBSCRIPT_NORMAL = int(engraving::VerticalAlignment::AlignNormal),
    TEXT_SUBSCRIPT_TOP = int(engraving::VerticalAlignment::AlignSuperScript),
    TEXT_SUBSCRIPT_BOTTOM = int(engraving::VerticalAlignment::AlignSubScript)
};
Q_ENUM_NS(TextSubscriptMode)
}

static const QList<mu::engraving::ElementType> TEXT_ELEMENT_TYPES = {
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
