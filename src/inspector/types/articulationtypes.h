#ifndef MU_INSPECTOR_ARTICULATIONTYPES_H
#define MU_INSPECTOR_ARTICULATIONTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
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
}

#endif // MU_INSPECTOR_ARTICULATIONTYPES_H
