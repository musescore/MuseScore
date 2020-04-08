#ifndef GLISSANDOTYPES_H
#define GLISSANDOTYPES_H

#include "qobjectdefs.h"

class GlissandoTypes
{
    Q_GADGET

    Q_ENUMS(Style)

public:
    enum Style {
        STYLE_CHROMATIC = 0,
        STYLE_WHITE_KEYS,
        STYLE_BLACK_KEYS,
        STYLE_DIATONIC
    };
};

#endif // GLISSANDOTYPES_H
