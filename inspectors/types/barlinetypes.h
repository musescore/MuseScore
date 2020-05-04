#ifndef BARLINETYPES_H
#define BARLINETYPES_H

#include "qobjectdefs.h"

class BarlineTypes
{
    Q_GADGET

public:
    enum class LineType {
        TYPE_NORMAL = 1,
        TYPE_DOUBLE = 2,
        TYPE_START_REPEAT = 4,
        TYPE_END_REPEAT = 8,
        TYPE_DASHED = 0x10,
        TYPE_FINAL = 0x20,
        TYPE_END_START_REPEAT = 0x40,
        TYPE_DOTTED = 0x80
    };

    enum class SpanPreset {
        PRESET_DEFAULT,
        PRESET_TICK_1,
        PRESET_TICK_2,
        PRESET_SHORT_1,
        PRESET_SHORT_2
    };

    Q_ENUM(LineType)
    Q_ENUM(SpanPreset)
};

#endif // BARLINETYPES_H
