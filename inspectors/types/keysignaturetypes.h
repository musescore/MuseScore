#ifndef KEYSIGNATURETYPES_H
#define KEYSIGNATURETYPES_H

#include "qobjectdefs.h"

class KeySignatureTypes
{
    Q_GADGET

    Q_ENUMS(Mode)

public:
    enum Mode {
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
};

#endif // KEYSIGNATURETYPES_H
