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

#include "engraving/dom/fret.h"

namespace mu::propertiespanel {
namespace FretDiagramTypes {
Q_NAMESPACE;
QML_ELEMENT;

// engraving::FretDotType has no "none" entry, so DOT_NONE has no engraving counterpart
enum class FretDot {
    DOT_NONE = -1,
    DOT_NORMAL = int(engraving::FretDotType::NORMAL),
    DOT_CROSS = int(engraving::FretDotType::CROSS),
    DOT_SQUARE = int(engraving::FretDotType::SQUARE),
    DOT_TRIANGLE = int(engraving::FretDotType::TRIANGLE)
};
Q_ENUM_NS(FretDot)

enum class FretMarker {
    MARKER_NONE = int(engraving::FretMarkerType::NONE),
    MARKER_CIRCLE = int(engraving::FretMarkerType::CIRCLE),
    MARKER_CROSS = int(engraving::FretMarkerType::CROSS)
};
Q_ENUM_NS(FretMarker)

enum class Orientation {
    ORIENTATION_VERTICAL = int(engraving::Orientation::VERTICAL),
    ORIENTATION_HORIZONTAL = int(engraving::Orientation::HORIZONTAL)
};
Q_ENUM_NS(Orientation)
}
}
