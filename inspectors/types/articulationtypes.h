#ifndef ARTICULATIONTYPES_H
#define ARTICULATIONTYPES_H

#include "qobjectdefs.h"

class ArticulationTypes
{
    Q_GADGET

    Q_ENUMS(Style)
public:
    enum Style {
        STYLE_STANDART = 0,
        STYLE_BAROQUE
    };
};

#endif // ARTICULATIONTYPES_H
