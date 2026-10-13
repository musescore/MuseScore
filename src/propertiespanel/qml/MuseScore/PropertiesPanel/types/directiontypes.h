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
namespace DirectionTypes {
Q_NAMESPACE;
QML_ELEMENT;

enum VerticalDirection {
    VERTICAL_AUTO = int(engraving::DirectionV::AUTO),
    VERTICAL_UP = int(engraving::DirectionV::UP),
    VERTICAL_DOWN = int(engraving::DirectionV::DOWN)
};
Q_ENUM_NS(VerticalDirection)

enum HorizontalDirection {
    HORIZONTAL_AUTO = int(engraving::DirectionH::AUTO),
    HORIZONTAL_LEFT = int(engraving::DirectionH::LEFT),
    HORIZONTAL_RIGHT = int(engraving::DirectionH::RIGHT)
};
Q_ENUM_NS(HorizontalDirection)

enum CenterBetweenStaves {
    CENTER_STAVES_AUTO = int(engraving::AutoOnOff::AUTO),
    CENTER_STAVES_ON = int(engraving::AutoOnOff::ON),
    CENTER_STAVES_OFF = int(engraving::AutoOnOff::OFF)
};
Q_ENUM_NS(CenterBetweenStaves)
}
}
