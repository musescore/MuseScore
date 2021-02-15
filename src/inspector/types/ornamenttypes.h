#ifndef MU_INSPECTOR_ORNAMENTTYPES_H
#define MU_INSPECTOR_ORNAMENTTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
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
}

#endif // MU_INSPECTOR_ORNAMENTTYPES_H
