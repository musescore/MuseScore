//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_SHORTCUTS_ISHORTCUTCONTEXTRESOLVER_H
#define MU_SHORTCUTS_ISHORTCUTCONTEXTRESOLVER_H

#include "modularity/imoduleexport.h"
#include "shortcutstypes.h"

//! NOTE This interface should be implemented by someone outside the `shortcut` module,
//! who can determine the current context for shortcuts
//! Most likely it implements `GlobalContext`

namespace mu::shortcuts {
class IShortcutContextResolver : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IShortcutContextResolver)

public:
    virtual ~IShortcutContextResolver() = default;

    virtual ShortcutContext currentShortcutContext() const = 0;
};
}

#endif // MU_SHORTCUTS_ISHORTCUTCONTEXTRESOLVER_H
