/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

namespace mu::instrumentsscene {
namespace LayoutPanelItemType {
Q_NAMESPACE;
QML_ELEMENT;

enum ItemType {
    UNDEFINED = -1,
    ROOT,
    SHARED_PART,
    PART,
    INSTRUMENT,
    STAFF,
    SYSTEM_OBJECTS_LAYER,
    CONTROL_ADD_STAFF,
    // A shared-staves group added via the panel's Add menu that has no instruments yet; it only exists in the panel
    // (an empty shared part can't exist in the score), and becomes a shared part once an instrument is dropped in it
    EMPTY_SHARED_GROUP,
};
Q_ENUM_NS(ItemType)

// Position of a top-level part row within a shared-staves group
enum SharedGroupRole {
    NOT_IN_GROUP = 0,
    GROUP_HEADER,
    GROUP_MEMBER,
    GROUP_LAST_MEMBER,
};
Q_ENUM_NS(SharedGroupRole)
}
}
