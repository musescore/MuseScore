#ifndef MU_INSPECTOR_TREMOLOTYPES_H
#define MU_INSPECTOR_TREMOLOTYPES_H

#include "qobjectdefs.h"

class TremoloTypes
{
    Q_GADGET

public:
    enum class TremoloStyle {
        STYLE_DEFAULT,
        STYLE_TRADITIONAL,
        STYLE_TRADITIONAL_ALTERNATE
    };

    Q_ENUM(TremoloStyle)
};

#endif // MU_INSPECTOR_TREMOLOTYPES_H
