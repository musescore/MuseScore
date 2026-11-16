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
namespace GlissandoTypes {
Q_NAMESPACE;
QML_NAMED_ELEMENT(Glissando);

enum class Style {
    STYLE_CHROMATIC = int(engraving::GlissandoStyle::CHROMATIC),
    STYLE_WHITE_KEYS = int(engraving::GlissandoStyle::WHITE_KEYS),
    STYLE_BLACK_KEYS = int(engraving::GlissandoStyle::BLACK_KEYS),
    STYLE_DIATONIC = int(engraving::GlissandoStyle::DIATONIC),
    STYLE_PORTAMENTO = int(engraving::GlissandoStyle::PORTAMENTO)
};

Q_ENUM_NS(Style)
}
}
