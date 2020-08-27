#ifndef TREMOLOTYPES_H
#define TREMOLOTYPES_H

#include "qobjectdefs.h"

class TremoloTypes
{
    Q_GADGET

public:
    enum class TremoloStrokeStyle {
        STYLE_DEFAULT,
        STYLE_ALL_STROKES_ATTACHED,
        STYLE_SINGLE_STROKE_ATTACHED
    };

    Q_ENUM(TremoloStrokeStyle)
};

#endif // TREMOLOTYPES_H
