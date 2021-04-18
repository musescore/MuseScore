#ifndef MU_INSPECTOR_PEDALTYPES_H
#define MU_INSPECTOR_PEDALTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class PedalTypes
{
    Q_GADGET

public:
    enum class HookType {
        HOOK_TYPE_NONE = 0,
        HOOK_TYPE_RIGHT_ANGLE,
        HOOK_TYPE_ACUTE_ANGLE,
        HOOK_TYPE_STAR
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

    enum class PlacementType : signed char {
        PLACEMENT_TYPE_ABOVE = 0,
        PLACEMENT_TYPE_BELOW
    };

    Q_ENUM(HookType)
    Q_ENUM(LineStyle)
    Q_ENUM(PlacementType)
};
}

#endif // MU_INSPECTOR_PEDALTYPES_H
