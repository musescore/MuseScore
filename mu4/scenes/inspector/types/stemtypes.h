#ifndef STEMTYPES_H
#define STEMTYPES_H

#include "qobjectdefs.h"

class DirectionTypes
{
    Q_GADGET

public:
    enum VerticalDirection {
        VERTICAL_AUTO,
        VERTICAL_UP,
        VERTICAL_DOWN
    };

    enum HorizontalDirection {
        HORIZONTAL_AUTO,
        HORIZONTAL_LEFT,
        HORIZONTAL_RIGHT
    };

    Q_ENUM(VerticalDirection)
    Q_ENUM(HorizontalDirection)
};

#endif // STEMTYPES_H
