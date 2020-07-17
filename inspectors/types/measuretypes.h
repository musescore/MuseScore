#ifndef BARTYPES_H
#define BARTYPES_H

#include "qobjectdefs.h"

class MeasureTypes
{
    Q_GADGET

public:
    enum class MeasureInsertionType {
        TYPE_PREPEND_TO_SCORE = 0,
        TYPE_APPEND_TO_SCORE,
        TYPE_PREPEND_TO_SELECTION,
        TYPE_APPEND_TO_SELECTION
    };

    Q_ENUM(MeasureInsertionType)
};

#endif // BARTYPES_H
