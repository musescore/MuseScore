#ifndef GLISSANDOTYPES_H
#define GLISSANDOTYPES_H

#include "qobjectdefs.h"

class GlissandoTypes
{
    Q_GADGET

    Q_ENUMS(Style)
    Q_ENUMS(LineType)

public:
    enum Style {
        STYLE_CHROMATIC = 0,
        STYLE_WHITE_KEYS,
        STYLE_BLACK_KEYS,
        STYLE_DIATONIC
    };

    enum LineType {
        LINE_TYPE_STRAIGHT = 0,
        LINE_TYPE_WAVY
    };
};

#endif // GLISSANDOTYPES_H
