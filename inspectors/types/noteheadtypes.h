#ifndef NOTEHEADTYPES_H
#define NOTEHEADTYPES_H

#include "qobjectdefs.h"

class NoteHeadTypes
{
    Q_GADGET

    Q_ENUMS(Group)
    Q_ENUMS(HorizontalDirection)
    Q_ENUMS(Type)
    Q_ENUMS(NoteDotPosition)
public:
    enum Group {
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

    enum Type {
        TYPE_AUTO = -1,
        TYPE_WHOLE,
        TYPE_HALF,
        TYPE_QUARTER,
        TYPE_BREVIS
    };

    enum HorizontalDirection {
        DIRECTION_H_AUTO = 0,
        DIRECTION_H_LEFT,
        DIRECTION_H_RIGHT
    };

    enum NoteDotPosition {
        DOT_POSITION_AUTO,
        DOT_POSITION_UP,
        DOT_POSITION_DOWN
    };
};

#endif // NOTEHEADTYPES_H
