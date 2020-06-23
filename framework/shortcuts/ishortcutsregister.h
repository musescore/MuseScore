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
#ifndef MU_SHORTCUTS_ISHORTCUTSREGISTER_H
#define MU_SHORTCUTS_ISHORTCUTSREGISTER_H

#include <list>

#include "modularity/imoduleexport.h"
#include "shortcutstypes.h"

namespace mu {
namespace shortcuts {
class IShortcutsRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IShortcutsRegister)
public:
    virtual ~IShortcutsRegister() = default;

    virtual const std::list<Shortcut>& shortcuts() const = 0;
    virtual std::list<Shortcut> shortcutsForSequence(const std::string& sequence) const = 0;
};
}
}

#endif // MU_SHORTCUTS_ISHORTCUTSREGISTER_H
