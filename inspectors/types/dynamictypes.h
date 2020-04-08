#ifndef DYNAMICTYPES_H
#define DYNAMICTYPES_H

#include "qobjectdefs.h"

class DynamicTypes
{
    Q_GADGET

    Q_ENUMS(Scope)
    Q_ENUMS(VelocityChangeSpeed)
public:
    enum Scope {
        SCOPE_STAFF = 0,
        SCOPE_SINGLE_INSTRUMENT,
        SCOPE_ALL_INSTRUMENTS
    };

    enum VelocityChangeSpeed {
        VELOCITY_CHANGE_SPEED_SLOW = 0,
        VELOCITY_CHANGE_SPEED_NORMAL,
        VELOCITY_CHANGE_SPEED_FAST
    };
};

#endif // DYNAMICTYPES_H
