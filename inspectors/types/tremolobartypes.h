#ifndef TREMOLOBARTYPES_H
#define TREMOLOBARTYPES_H

#include "qobjectdefs.h"

class TremoloBarTypes
{
    Q_GADGET

public:
    enum class TremoloBarType {
        TYPE_DIP = 0,
        TYPE_DIVE,
        TYPE_RELEASE_UP,
        TYPE_INVERTED_DIP,
        TYPE_RETURN,
        TYPE_RELEASE_DOWN,
        TYPE_CUSTOM
    };

    Q_ENUM(TremoloBarType)
};

#endif // TREMOLOBARTYPES_H
