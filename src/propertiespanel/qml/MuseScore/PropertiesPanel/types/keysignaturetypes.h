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

#include "engraving/types/types.h"

namespace mu::propertiespanel {
namespace KeySignatureTypes {
Q_NAMESPACE;
QML_ELEMENT;

enum class Mode {
    MODE_UNKNOWN = int(engraving::KeyMode::UNKNOWN),
    MODE_NONE = int(engraving::KeyMode::NONE),
    MODE_MAJOR = int(engraving::KeyMode::MAJOR),
    MODE_MINOR = int(engraving::KeyMode::MINOR),
    MODE_DORIAN = int(engraving::KeyMode::DORIAN),
    MODE_PHRYGIAN = int(engraving::KeyMode::PHRYGIAN),
    MODE_LYDIAN = int(engraving::KeyMode::LYDIAN),
    MODE_MIXOLYDIAN = int(engraving::KeyMode::MIXOLYDIAN),
    MODE_AEOLIAN = int(engraving::KeyMode::AEOLIAN),
    MODE_IONIAN = int(engraving::KeyMode::IONIAN),
    MODE_LOCRIAN = int(engraving::KeyMode::LOCRIAN)
};

Q_ENUM_NS(Mode)
}
}
