#ifndef MU_INSPECTOR_MARKERTYPES_H
#define MU_INSPECTOR_MARKERTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class MarkerTypes
{
    Q_GADGET

public:
    enum class Type {
        TYPE_SEGNO = 0,
        TYPE_VARSEGNO,
        TYPE_CODA,
        TYPE_VARCODA,
        TYPE_CODETTA,
        TYPE_FINE,
        TYPE_TOCODA,
        TYPE_USER
    };

    Q_ENUM(Type)
};
}

#endif // MU_INSPECTOR_MARKERTYPES_H
