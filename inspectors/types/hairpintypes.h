#ifndef HAIRPINTYPES_H
#define HAIRPINTYPES_H

#include "qobjectdefs.h"

class HairpinTypes
{
    Q_GADGET

public:
    enum class VelocityEasing {
        VELOCITY_EASING_LINEAR = 0,
        VELOCITY_EASING_EXPONENTIAL,
        VELOCITY_EASING_IN,
        VELOCITY_EASING_OUT,
        VELOCITY_EASING_IN_OUT
    };

    enum class LineStyle {
        LINE_STYLE_NONE = 0,
        LINE_STYLE_SOLID,
        LINE_STYLE_DASHED,
        LINE_STYLE_DOTTED,
        LINE_STYLE_DASH_DOT,
        LINE_STYLE_DASH_DOT_DOT,
        LINE_STYLE_CUSTOM
    };

    enum class PlacementType {
        PLACEMENT_TYPE_ABOVE = 0,
        PLACEMENT_TYPE_BELOW
    };

    Q_ENUM(VelocityEasing)
    Q_ENUM(LineStyle)
    Q_ENUM(PlacementType)
};

#endif // HAIRPINTYPES_H
