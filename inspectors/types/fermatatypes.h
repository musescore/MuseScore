#ifndef FERMATATYPES_H
#define FERMATATYPES_H

#include "qobjectdefs.h"

class FermataTypes {

    Q_GADGET

    Q_ENUMS(Placement)
public:
    enum Placement {
        ABOVE,
        BELOW
    };
};

#endif // FERMATATYPES_H
