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
#ifndef MU_NOTATION_INOTATIONCONTEXTMENU_H
#define MU_NOTATION_INOTATIONCONTEXTMENU_H

#include "modularity/imoduleexport.h"
#include "ui/uitypes.h"
#include "notation/notationtypes.h"

namespace mu::notation {
class INotationContextMenu : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(INotationContextMenu)

public:
    virtual ~INotationContextMenu() = default;

    virtual ui::MenuItemList items(const ElementType& elementType) const = 0;
};
}

#endif // MU_NOTATION_INOTATIONCONTEXTMENU_H
