#ifndef BARTYPES_H
#define BARTYPES_H

#include "qobjectdefs.h"

class BarTypes
{
    Q_GADGET

public:
    enum class BarInsertionType {
        TYPE_PREPEND_TO_SCORE = 0,
        TYPE_APPEND_TO_SCORE,
        TYPE_PREPEND_TO_SELECTION,
        TYPE_APPEND_TO_SELECTION
    };

    Q_ENUM(BarInsertionType)
};

#endif // BARTYPES_H
