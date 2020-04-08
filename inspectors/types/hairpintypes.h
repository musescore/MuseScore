#ifndef HAIRPINTYPES_H
#define HAIRPINTYPES_H

#include "qobjectdefs.h"

class HairpinTypes
{
    Q_GADGET

    Q_ENUMS(VelocityEasing)

public:
    enum VelocityEasing {
        VELOCITY_EASING_LINEAR = 0,
        VELOCITY_EASING_EXPONENTIAL,
        VELOCITY_EASING_IN,
        VELOCITY_EASING_OUT,
        VELOCITY_EASING_IN_OUT
    };
};

#endif // HAIRPINTYPES_H
