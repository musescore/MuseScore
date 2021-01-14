#ifndef MU_INSPECTOR_ACCIDENTALTYPES_H
#define MU_INSPECTOR_ACCIDENTALTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class AccidentalTypes
{
    Q_GADGET

public:
    enum class BracketType {
        BRACKET_TYPE_NONE = 0,
        BRACKET_TYPE_PARENTHESIS,
        BRACKET_TYPE_SQUARE,
        BRACKET_TYPE_ROUND,
    };

    Q_ENUM(BracketType)
};
}

#endif // MU_INSPECTOR_ACCIDENTALTYPES_H
