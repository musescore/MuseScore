#ifndef TEXTTYPES_H
#define TEXTTYPES_H

#include "qobjectdefs.h"

class TextTypes
{
    Q_GADGET

public:
    enum class FontStyle {
        FONT_STYLE_NORMAL = 0,
        FONT_STYLE_BOLD = 1,
        FONT_STYLE_ITALIC = 2,
        FONT_STYLE_UNDERLINE = 4
    };

    enum class FontHorizontalAlignment {
        FONT_ALIGN_H_LEFT = 0,
        FONT_ALIGN_H_RIGHT = 1,
        FONT_ALIGN_H_CENTER = 2,
    };

    enum class FontVerticalAlignment {
        FONT_ALIGN_V_TOP = 0,
        FONT_ALIGN_V_BOTTOM = 4,
        FONT_ALIGN_V_CENTER = 8,
        FONT_ALIGN_V_BASELINE = 16
    };

    enum class FrameType {
        FRAME_TYPE_NONE = 0,
        FRAME_TYPE_SQUARE,
        FRAME_TYPE_CIRCLE
    };

    enum class TextType {
        TEXT_TYPE_DEFAULT,
        TEXT_TYPE_TITLE,
        TEXT_TYPE_SUBTITLE,
        TEXT_TYPE_COMPOSER,
        TEXT_TYPE_POET,
        TEXT_TYPE_LYRICS_ODD,
        TEXT_TYPE_LYRICS_EVEN,
        TEXT_TYPE_FINGERING,
        TEXT_TYPE_LH_GUITAR_FINGERING,
        TEXT_TYPE_RH_GUITAR_FINGERING,
        TEXT_TYPE_STRING_NUMBER,
        TEXT_TYPE_INSTRUMENT_LONG,
        TEXT_TYPE_INSTRUMENT_SHORT,
        TEXT_TYPE_INSTRUMENT_EXCERPT,
        TEXT_TYPE_DYNAMICS,
        TEXT_TYPE_EXPRESSION,
        TEXT_TYPE_TEMPO,
        TEXT_TYPE_METRONOME,
        TEXT_TYPE_MEASURE_NUMBER,
        TEXT_TYPE_TRANSLATOR,
        TEXT_TYPE_TUPLET,
        TEXT_TYPE_SYSTEM,
        TEXT_TYPE_STAFF,
        TEXT_TYPE_HARMONY_A,
        TEXT_TYPE_HARMONY_B,
        TEXT_TYPE_HARMONY_ROMAN,
        TEXT_TYPE_HARMONY_NASHVILLE,
        TEXT_TYPE_REHEARSAL_MARK,
        TEXT_TYPE_REPEAT_LEFT,
        TEXT_TYPE_REPEAT_RIGHT,
        TEXT_TYPE_FRAME,
        TEXT_TYPE_TEXTLINE,
        TEXT_TYPE_GLISSANDO,
        TEXT_TYPE_OTTAVA,
        TEXT_TYPE_VOLTA,
        TEXT_TYPE_PEDAL,
        TEXT_TYPE_LET_RING,
        TEXT_TYPE_PALM_MUTE,
        TEXT_TYPE_HAIRPIN,
        TEXT_TYPE_BEND,
        TEXT_TYPE_HEADER,
        TEXT_TYPE_FOOTER,
        TEXT_TYPE_INSTRUMENT_CHANGE,
        TEXT_TYPE_STICKING
    };

    enum class TextPlacement {
        TEXT_PLACEMENT_ABOVE = 0,
        TEXT_PLACEMENT_BELOW
    };

    enum class TextSubscriptMode {
        TEXT_SUBSCRIPT_NORMAL = 0,
        TEXT_SUBSCRIPT_TOP,
        TEXT_SUBSCRIPT_BOTTOM
    };

    Q_ENUM(FontStyle)
    Q_ENUM(FrameType)
    Q_ENUM(TextType)
    Q_ENUM(TextPlacement)
    Q_ENUM(TextSubscriptMode)

    Q_ENUM(FontHorizontalAlignment)

    Q_ENUM(FontVerticalAlignment)
};

#endif // TEXTTYPES_H
