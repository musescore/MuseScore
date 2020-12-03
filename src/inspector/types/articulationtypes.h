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

    enum class Style {
        STYLE_STANDART = 0,
        STYLE_BAROQUE
    };

    Q_ENUM(Placement)
    Q_ENUM(Direction)
    Q_ENUM(Style)
};

#endif // ARTICULATIONTYPES_H
