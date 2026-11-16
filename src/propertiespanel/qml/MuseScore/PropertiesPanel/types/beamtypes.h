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
namespace BeamTypes {
Q_NAMESPACE;
QML_NAMED_ELEMENT(Beam);

enum class Mode {
    MODE_INVALID = int(engraving::BeamMode::INVALID),
    MODE_AUTO = int(engraving::BeamMode::AUTO),
    MODE_NONE = int(engraving::BeamMode::NONE),
    MODE_BEGIN = int(engraving::BeamMode::BEGIN),
    MODE_BEGIN32 = int(engraving::BeamMode::BEGIN16),
    MODE_BEGIN64 = int(engraving::BeamMode::BEGIN32),
    MODE_MID = int(engraving::BeamMode::MID),
    MODE_END = int(engraving::BeamMode::END)
};
Q_ENUM_NS(Mode)

enum class FeatheringMode {
    FEATHERING_NONE = 0,
    FEATHERED_DECELERATE,
    FEATHERED_ACCELERATE
};

Q_ENUM_NS(FeatheringMode)
}
}
