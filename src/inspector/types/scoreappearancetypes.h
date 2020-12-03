#ifndef SCOREAPPEARANCETYPES_H
#define SCOREAPPEARANCETYPES_H

#include "qobjectdefs.h"

class ScoreAppearanceTypes
{
    Q_GADGET

public:

    enum class OrientationType {
        ORIENTATION_PORTRAIT = 0,
        ORIENTATION_LANDSCAPE
    };

    Q_ENUM(OrientationType)
};

#endif // SCOREAPPEARANCETYPES_H
