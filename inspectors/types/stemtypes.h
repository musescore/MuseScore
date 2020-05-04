#ifndef STEMTYPES_H
#define STEMTYPES_H

#include "qobjectdefs.h"

class StemTypes
{
    Q_GADGET

public:
    enum class Direction {
        AUTO,
        UP,
        DOWN
    };

    Q_ENUM(Direction)
};

#endif // STEMTYPES_H
