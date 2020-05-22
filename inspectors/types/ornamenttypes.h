#ifndef ORNAMENTTYPES_H
#define ORNAMENTTYPES_H

#include "qobjectdefs.h"

class OrnamentTypes
{
    Q_GADGET

    Q_ENUMS(Style)
public:
    enum Style {
        STYLE_STANDART = 0,
        STYLE_BAROQUE
    };
};

#endif // ORNAMENTTYPES_H
