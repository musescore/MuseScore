#ifndef DIRECTION_H
#define DIRECTION_H

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

#endif // DIRECTION_H
