#ifndef KEYSIGNATURETYPES_H
#define KEYSIGNATURETYPES_H

#include "qobjectdefs.h"

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

#endif // KEYSIGNATURETYPES_H
