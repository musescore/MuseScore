#ifndef MU_INSPECTOR_BEAMTYPES_H
#define MU_INSPECTOR_BEAMTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class BeamTypes
{
    Q_GADGET

public:
    enum class Mode {
        MODE_INVALID = -1,
        MODE_AUTO,
        MODE_BEGIN,
        MODE_MID,
        MODE_END,
        MODE_NONE,
        MODE_BEGIN32,
        MODE_BEGIN64
    };

    enum class FeatheringMode {
        FEATHERING_NONE = 0,
        FEATHERING_LEFT,
        FEATHERING_RIGHT
    };

    Q_ENUM(Mode)
    Q_ENUM(FeatheringMode)
};
}

#endif // MU_INSPECTOR_BEAMTYPES_H
