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
namespace GlissandoTypes {
Q_NAMESPACE;
QML_NAMED_ELEMENT(Glissando);

//! NOTE: must be in sync with mu::engraving::GlissandoStyle
enum class Style {
    STYLE_CHROMATIC = 0,
    STYLE_WHITE_KEYS,
    STYLE_BLACK_KEYS,
    STYLE_DIATONIC,
    STYLE_PORTAMENTO
};

Q_ENUM_NS(Style)
}
}
