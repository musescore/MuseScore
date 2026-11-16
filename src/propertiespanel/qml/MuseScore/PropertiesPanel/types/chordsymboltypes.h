/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <qqmlintegration.h>

#include "engraving/dom/realizedharmony.h"

namespace mu::propertiespanel {
namespace ChordSymbolTypes {
Q_NAMESPACE;
QML_ELEMENT;

enum class VoicingType {
    VOICING_INVALID = int(engraving::Voicing::INVALID),
    VOICING_AUTO = int(engraving::Voicing::AUTO),
    VOICING_ROOT_ONLY = int(engraving::Voicing::ROOT_ONLY),
    VOICING_CLOSE = int(engraving::Voicing::CLOSE),
    VOICING_DROP_TWO = int(engraving::Voicing::DROP_2),
    VOICING_SIX_NOTE = int(engraving::Voicing::SIX_NOTE),
    VOICING_FOUR_NOTE = int(engraving::Voicing::FOUR_NOTE),
    VOICING_THREE_NOTE = int(engraving::Voicing::THREE_NOTE)
};
Q_ENUM_NS(VoicingType)

enum class DurationType {
    DURATION_INVALID = int(engraving::HDuration::INVALID),
    DURATION_UNTIL_NEXT_CHORD_SYMBOL = int(engraving::HDuration::UNTIL_NEXT_CHORD_SYMBOL),
    DURATION_STOP_AT_MEASURE_END = int(engraving::HDuration::STOP_AT_MEASURE_END),
    DURATION_SEGMENT_DURATION = int(engraving::HDuration::SEGMENT_DURATION)
};

Q_ENUM_NS(DurationType)
}
}
