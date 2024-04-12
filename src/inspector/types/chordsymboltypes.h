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
#ifndef MU_INSPECTOR_CHORDSYMBOLTYPES_H
#define MU_INSPECTOR_CHORDSYMBOLTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class ChordSymbolTypes
{
    Q_GADGET

public:
    enum class VoicingType {
        VOICING_INVALID = -1,
        VOICING_AUTO,
        VOICING_ROOT_ONLY,
        VOICING_CLOSE,
        VOICING_DROP_TWO,
        VOICING_SIX_NOTE,
        VOICING_FOUR_NOTE,
        VOICING_THREE_NOTE
    };

    enum class DurationType {
        DURATION_INVALID = -1,
        DURATION_UNTIL_NEXT_CHORD_SYMBOL,
        DURATION_STOP_AT_MEASURE_END,
        DURATION_SEGMENT_DURATION
    };

    Q_ENUM(VoicingType)
    Q_ENUM(DurationType)
};
}

#endif // MU_INSPECTOR_CHORDSYMBOLTYPES_H
