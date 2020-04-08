#ifndef BEAMTYPES_H
#define BEAMTYPES_H

#include <QObject>

class BeamTypes
{
    Q_GADGET

    Q_ENUMS(Mode)
    Q_ENUMS(FeatheringMode)
public:

    enum Mode {
        MODE_INVALID = -1,
        MODE_AUTO,
        MODE_BEGIN,
        MODE_MID,
        MODE_NONE,
        MODE_BEGIN32,
        MODE_BEGIN64
    };

    enum FeatheringMode {
        FEATHERING_NONE = 0,
        FEATHERING_LEFT,
        FEATHERING_RIGHT
    };
};

#endif // BEAMTYPES_H
