#ifndef MU_INSPECTOR_FERMATATYPES_H
#define MU_INSPECTOR_FERMATATYPES_H

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

#endif // MU_INSPECTOR_FERMATATYPES_H
