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
#ifndef MU_INSPECTOR_TEXTTYPES_H
#define MU_INSPECTOR_TEXTTYPES_H

#include "qobjectdefs.h"

#include "engraving/types/types.h"

namespace mu::inspector {
class TextTypes
{
    Q_GADGET

public:
    enum class FontStyle {
        //FONT_STYLE_UNDEFINED = -1,
        FONT_STYLE_NORMAL = 0,
        FONT_STYLE_BOLD = 1 << 0,
        FONT_STYLE_ITALIC = 1 << 1,
        FONT_STYLE_UNDERLINE = 1 << 2,
        FONT_STYLE_STRIKE = 1 << 3
    };

    enum class FontHorizontalAlignment {
        FONT_ALIGN_H_LEFT = 0,
        FONT_ALIGN_H_RIGHT = 1,
        FONT_ALIGN_H_CENTER = 2,
    };

    enum class FontVerticalAlignment {
        FONT_ALIGN_V_TOP = 0,
        FONT_ALIGN_V_CENTER = 1,
        FONT_ALIGN_V_BOTTOM = 2,
        FONT_ALIGN_V_BASELINE = 3
    };

    enum class FrameType {
        FRAME_TYPE_NONE = 0,
        FRAME_TYPE_SQUARE,
        FRAME_TYPE_CIRCLE
    };
    // must match mu::engraving::Tid
    enum class TextType {
        TEXT_TYPE_DEFAULT,
        TEXT_TYPE_TITLE,
        TEXT_TYPE_SUBTITLE,
        TEXT_TYPE_COMPOSER,
        TEXT_TYPE_POET,
        TEXT_TYPE_TRANSLATOR,
        TEXT_TYPE_FRAME,
        TEXT_TYPE_INSTRUMENT_EXCERPT,
        TEXT_TYPE_INSTRUMENT_LONG,
        TEXT_TYPE_INSTRUMENT_SHORT,
        TEXT_TYPE_INSTRUMENT_CHANGE,
        TEXT_TYPE_HEADER,
        TEXT_TYPE_FOOTER,
        TEXT_TYPE_COPYRIGHT,
        TEXT_TYPE_PAGE_NUMBER,
        TEXT_TYPE_MEASURE_NUMBER,
        TEXT_TYPE_MMREST_RANGE,
        TEXT_TYPE_TEMPO,
        TEXT_TYPE_TEMPO_CHANGE,
        TEXT_TYPE_METRONOME,
        TEXT_TYPE_REPEAT_LEFT,       // align to start of measure
        TEXT_TYPE_REPEAT_RIGHT,      // align to end of measure
        TEXT_TYPE_REHEARSAL_MARK,
        TEXT_TYPE_SYSTEM,
        TEXT_TYPE_STAFF,
        TEXT_TYPE_EXPRESSION,
        TEXT_TYPE_DYNAMICS,
        TEXT_TYPE_HAIRPIN,
        TEXT_TYPE_LYRICS_ODD,
        TEXT_TYPE_LYRICS_EVEN,
        TEXT_TYPE_HARMONY_A,
        TEXT_TYPE_HARMONY_B,
        TEXT_TYPE_HARMONY_ROMAN,
        TEXT_TYPE_HARMONY_NASHVILLE,
        TEXT_TYPE_TUPLET,
        TEXT_TYPE_STICKING,
        TEXT_TYPE_FINGERING,
        TEXT_TYPE_LH_GUITAR_FINGERING,
        TEXT_TYPE_RH_GUITAR_FINGERING,
        TEXT_TYPE_STRING_NUMBER,
        TEXT_TYPE_HARP_PEDAL_DIAGRAM,
        TEXT_TYPE_HARP_PEDAL_TEXT_DIAGRAM,
        TEXT_TYPE_TEXTLINE,
        TEXT_TYPE_VOLTA,
        TEXT_TYPE_OTTAVA,
        TEXT_TYPE_GLISSANDO,
        TEXT_TYPE_PEDAL,
        TEXT_TYPE_BEND,
        TEXT_TYPE_LET_RING,
        TEXT_TYPE_PALM_MUTE,
        TEXT_TYPE_USER1,
        TEXT_TYPE_USER2,
        TEXT_TYPE_USER3,
        TEXT_TYPE_USER4,
        TEXT_TYPE_USER5,
        TEXT_TYPE_USER6,
        TEXT_TYPE_USER7,
        TEXT_TYPE_USER8,
        TEXT_TYPE_USER9,
        TEXT_TYPE_USER10,
        TEXT_TYPE_USER11,
        TEXT_TYPE_USER12
    };

    enum class TextSubscriptMode {
        TEXT_SUBSCRIPT_NORMAL = 0,
        TEXT_SUBSCRIPT_TOP,
        TEXT_SUBSCRIPT_BOTTOM
    };

    Q_ENUM(FontStyle)
    Q_ENUM(FrameType)
    Q_ENUM(TextType)
    Q_ENUM(TextSubscriptMode)

    Q_ENUM(FontHorizontalAlignment)

    Q_ENUM(FontVerticalAlignment)
};

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
    mu::engraving::ElementType::PLAYTECH_ANNOTATION,
    mu::engraving::ElementType::CAPO,
    mu::engraving::ElementType::STRING_TUNINGS,
    mu::engraving::ElementType::HARP_DIAGRAM,
    mu::engraving::ElementType::SOUND_FLAG,
    mu::engraving::ElementType::HAMMER_ON_PULL_OFF_TEXT
};
}

#endif // MU_INSPECTOR_TEXTTYPES_H
