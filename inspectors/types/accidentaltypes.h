#ifndef ACCIDENTALTYPES_H
#define ACCIDENTALTYPES_H

#include "qobjectdefs.h"

class AccidentalTypes
{
    Q_GADGET

    Q_ENUMS(BracketType)
public:
    enum BracketType {
        BRACKET_TYPE_NONE = 0,
        BRACKET_TYPE_PARENTHESIS,
        BRACKET_TYPE_SQUARE
    };
};

#endif // ACCIDENTALTYPES_H
