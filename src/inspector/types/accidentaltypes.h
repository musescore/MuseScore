#ifndef ACCIDENTALTYPES_H
#define ACCIDENTALTYPES_H

#include "qobjectdefs.h"

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

#endif // ACCIDENTALTYPES_H
