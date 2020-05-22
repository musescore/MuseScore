#ifndef ARTICULATIONTYPES_H
#define ARTICULATIONTYPES_H

#include "qobjectdefs.h"

class ArticulationTypes
{
    Q_GADGET

public:
    enum class Placement {
        TYPE_ABOVE_STAFF,
        TYPE_BELOW_STAFF,
        TYPE_CHORD_AUTO,
        TYPE_ABOVE_CHORD,
        TYPE_BELOW_CHORD
    };

    enum class Direction {
        AUTO,
        DOWN,
        UP
    };

    Q_ENUM(Placement)
    Q_ENUM(Direction)
};

#endif // ARTICULATIONTYPES_H
