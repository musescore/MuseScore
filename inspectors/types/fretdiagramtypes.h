#ifndef FRETDIAGRAMTYPES_H
#define FRETDIAGRAMTYPES_H

#include "qobjectdefs.h"

class FretDiagramTypes
{
    Q_GADGET

public:
    // the difference between the start numbers of two enum types
    // is because of how they're defined in libmscore/fret.h
    // to enable direct cast, we use the same values
    enum class FretDot {
        DOT_NONE = -1,
        DOT_NORMAL = 0,
        DOT_CROSS,
        DOT_SQUARE,
        DOT_TRIANGLE
    };

    enum class FretMarker {
        MARKER_NONE = 0,
        MARKER_CIRCLE,
        MARKER_CROSS
    };

    Q_ENUM(FretDot)
    Q_ENUM(FretMarker)
};

#endif // FRETDIAGRAMTYPES_H
