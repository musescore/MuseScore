/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
        MODE_AEOLIAN,
        MODE_IONIAN,
        MODE_LOCRIAN
    };

    Q_ENUM(Mode)
};
}

#endif // MU_INSPECTOR_KEYSIGNATURETYPES_H
