#ifndef DYNAMICTYPES_H
#define DYNAMICTYPES_H

#include "qobjectdefs.h"

class DynamicTypes
{
    Q_GADGET

public:
    enum class Scope {
        SCOPE_STAFF = 0,
        SCOPE_SINGLE_INSTRUMENT,
        SCOPE_ALL_INSTRUMENTS
    };

    enum class VelocityChangeSpeed {
        VELOCITY_CHANGE_SPEED_SLOW = 0,
        VELOCITY_CHANGE_SPEED_NORMAL,
        VELOCITY_CHANGE_SPEED_FAST
    };

    Q_ENUM(Scope)
    Q_ENUM(VelocityChangeSpeed)
};

#endif // DYNAMICTYPES_H
