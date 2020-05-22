#ifndef ARTICULATIONTYPES_H
#define ARTICULATIONTYPES_H

#include "qobjectdefs.h"

class ArticulationTypes
{
    Q_GADGET

    Q_ENUMS(Placement)
    Q_ENUMS(Direction)

public:
    enum Placement {
        TYPE_ABOVE_STAFF,
        TYPE_BELOW_STAFF,
        TYPE_CHORD_AUTO,
        TYPE_ABOVE_CHORD,
        TYPE_BELOW_CHORD
    };

    enum Direction {
        AUTO,
        DOWN,
        UP
    };
};

#endif // ARTICULATIONTYPES_H
