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

#pragma once

#include <qqmlintegration.h>

namespace mu::inspector {
namespace FretDiagramTypes {
Q_NAMESPACE;
QML_ELEMENT;

// the difference between the start numbers of two enum types
// is because of how they're defined in engraving/dom/fret.h
// to enable direct cast, we use the same values
enum class FretDot {
    DOT_NONE = -1,
    DOT_NORMAL = 0,
    DOT_CROSS,
    DOT_SQUARE,
    DOT_TRIANGLE
};
Q_ENUM_NS(FretDot)

enum class FretMarker {
    MARKER_NONE = 0,
    MARKER_CIRCLE,
    MARKER_CROSS
};
Q_ENUM_NS(FretMarker)

enum class Orientation {
    ORIENTATION_VERTICAL,
    ORIENTATION_HORIZONTAL
};
Q_ENUM_NS(Orientation)
}
}
