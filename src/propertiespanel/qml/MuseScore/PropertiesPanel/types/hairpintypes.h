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
namespace HairpinTypes {
Q_NAMESPACE;
QML_NAMED_ELEMENT(Hairpin);

enum class VelocityEasing {
    VELOCITY_EASING_LINEAR = int(engraving::ChangeMethod::NORMAL),
    VELOCITY_EASING_EXPONENTIAL = int(engraving::ChangeMethod::EXPONENTIAL),
    VELOCITY_EASING_IN = int(engraving::ChangeMethod::EASE_IN),
    VELOCITY_EASING_OUT = int(engraving::ChangeMethod::EASE_OUT),
    VELOCITY_EASING_IN_OUT = int(engraving::ChangeMethod::EASE_IN_OUT)
};

Q_ENUM_NS(VelocityEasing)
}
}
