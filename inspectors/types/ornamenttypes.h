#ifndef ORNAMENTTYPES_H
#define ORNAMENTTYPES_H

#include "qobjectdefs.h"

class OrnamentTypes
{
    Q_GADGET

public:
    enum class Style {
        STYLE_STANDARD = 0,
        STYLE_BAROQUE
    };

    Q_ENUM(Style)
};

#endif // ORNAMENTTYPES_H
