#ifndef STEMTYPES_H
#define STEMTYPES_H

#include "qobjectdefs.h"

class StemTypes {

    Q_GADGET

    Q_ENUMS(Direction)
public:
    enum Direction {
        AUTO,
        UP,
        DOWN
    };
};

#endif // STEMTYPES_H
