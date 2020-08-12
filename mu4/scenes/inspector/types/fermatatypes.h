#ifndef FERMATATYPES_H
#define FERMATATYPES_H

#include "qobjectdefs.h"

class FermataTypes
{
    Q_GADGET

public:
    enum class Placement {
        ABOVE,
        BELOW
    };

    Q_ENUM(Placement)
};

#endif // FERMATATYPES_H
