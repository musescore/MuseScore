#ifndef MU_INSPECTOR_SCOREAPPEARANCETYPES_H
#define MU_INSPECTOR_SCOREAPPEARANCETYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
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
}

#endif // MU_INSPECTOR_SCOREAPPEARANCETYPES_H
