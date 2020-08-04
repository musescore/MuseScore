#ifndef BENDTYPES_H
#define BENDTYPES_H

#include "qobjectdefs.h"

class BendTypes
{
    Q_GADGET

public:
    enum class BendType {
        TYPE_BEND = 0,
        TYPE_BEND_RELEASE,
        TYPE_BEND_RELEASE_BEND,
        TYPE_PREBEND,
        TYPE_PREBEND_RELEASE,
        TYPE_CUSTOM
    };

    Q_ENUM(BendType)
};

#endif // BENDTYPES_H
