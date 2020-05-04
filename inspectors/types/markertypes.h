#ifndef MARKERTYPES_H
#define MARKERTYPES_H

#include "qobjectdefs.h"

class MarkerTypes
{
    Q_GADGET

    Q_ENUMS(Type)

public:
    enum Type {
        TYPE_SEGNO = 0,
        TYPE_VARSEGNO,
        TYPE_CODA,
        TYPE_VARCODA,
        TYPE_CODETTA,
        TYPE_FINE,
        TYPE_TOCODA,
        TYPE_USER
    };
};

#endif // MARKERTYPES_H
