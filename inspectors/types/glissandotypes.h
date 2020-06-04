#ifndef GLISSANDOTYPES_H
#define GLISSANDOTYPES_H

#include "qobjectdefs.h"

class GlissandoTypes
{
    Q_GADGET

public:
    enum class Style {
        STYLE_CHROMATIC = 0,
        STYLE_WHITE_KEYS,
        STYLE_BLACK_KEYS,
        STYLE_DIATONIC
    };

    enum class LineType {
        LINE_TYPE_STRAIGHT = 0,
        LINE_TYPE_WAVY
    };

    Q_ENUM(Style)
    Q_ENUM(LineType)
};

#endif // GLISSANDOTYPES_H
