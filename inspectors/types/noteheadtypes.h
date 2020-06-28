#ifndef NOTEHEADTYPES_H
#define NOTEHEADTYPES_H

#include "qobjectdefs.h"

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

    enum class HorizontalDirection {
        DIRECTION_H_AUTO = 0,
        DIRECTION_H_LEFT,
        DIRECTION_H_RIGHT
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

    enum class ParenthesesFlags : signed char {
        PARENTHESIS_NONE  = 0x0,
        PARENTHESIS_LEFT  = 0x1,
        PARENTHESIS_RIGHT = 0x2,
        PARENTHESIS_ALL   = PARENTHESIS_RIGHT | PARENTHESIS_LEFT
    };

    Q_ENUM(Group)
    Q_ENUM(HorizontalDirection)
    Q_ENUM(Type)
    Q_ENUM(NoteDotPosition)
    Q_ENUM(SchemeType)
    Q_ENUM(ParenthesesFlags)

};

#endif // NOTEHEADTYPES_H
