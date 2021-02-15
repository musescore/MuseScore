#ifndef MU_INSPECTOR_KEYSIGNATURETYPES_H
#define MU_INSPECTOR_KEYSIGNATURETYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class KeySignatureTypes
{
    Q_GADGET

public:
    enum class Mode {
        MODE_UNKNOWN = -1,
        MODE_NONE,
        MODE_MAJOR,
        MODE_MINOR,
        MODE_DORIAN,
        MODE_PHRYGIAN,
        MODE_LYDIAN,
        MODE_MIXOLYDIAN,
        MODE_IONIAN,
        MODE_LOCRIAN
    };

    Q_ENUM(Mode)
};
}

#endif // MU_INSPECTOR_KEYSIGNATURETYPES_H
