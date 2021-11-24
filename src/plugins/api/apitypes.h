/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_PLUGINS_APITYPES_H
#define MU_PLUGINS_APITYPES_H

#include <QObject>

#include "engraving/types/types.h"

namespace Ms::PluginAPI {
Q_NAMESPACE

enum class Align : char {
    LEFT     = char(mu::engraving::Align::LEFT),
    RIGHT    = char(mu::engraving::Align::RIGHT),
    HCENTER  = char(mu::engraving::Align::HCENTER),
    TOP      = char(mu::engraving::Align::TOP),
    BOTTOM   = char(mu::engraving::Align::BOTTOM),
    VCENTER  = char(mu::engraving::Align::VCENTER),
    BASELINE = char(mu::engraving::Align::BASELINE),
    CENTER = Align::HCENTER | Align::VCENTER,
    HMASK  = Align::LEFT | Align::RIGHT | Align::HCENTER,
    VMASK  = Align::TOP | Align::BOTTOM | Align::VCENTER | Align::BASELINE
};
Q_ENUM_NS(Align);

//! NOTE just Placement for compatibility
enum class Placement {
    ABOVE = int(mu::engraving::PlacementV::ABOVE),
    BELOW = int(mu::engraving::PlacementV::BELOW),
};
Q_ENUM_NS(Placement);

enum class Direction {
    AUTO = int(mu::engraving::DirectionV::AUTO),
    UP   = int(mu::engraving::DirectionV::UP),
    DOWN = int(mu::engraving::DirectionV::DOWN),
};
Q_ENUM_NS(Direction);

enum class DirectionH {
    AUTO  = int(mu::engraving::DirectionH::AUTO),
    LEFT  = int(mu::engraving::DirectionH::LEFT),
    RIGHT = int(mu::engraving::DirectionH::RIGHT),
};
Q_ENUM_NS(DirectionH);

//! HACK to force the build system to run moc on this file
class Mops : public QObject
{
    Q_GADGET
};
}

Q_DECLARE_METATYPE(Ms::PluginAPI::Align);
Q_DECLARE_METATYPE(Ms::PluginAPI::Placement);
Q_DECLARE_METATYPE(Ms::PluginAPI::Direction);
Q_DECLARE_METATYPE(Ms::PluginAPI::DirectionH);

#endif // MU_PLUGINS_APITYPES_H
