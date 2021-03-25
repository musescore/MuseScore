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
#ifndef MU_SHORTCUTS_ISHORTCUTSCONFIGURATION_H
#define MU_SHORTCUTS_ISHORTCUTSCONFIGURATION_H

#include "modularity/imoduleexport.h"
#include "io/path.h"
#include "retval.h"

namespace mu::shortcuts {
class IShortcutsConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IShortcutsConfiguration)

public:
    virtual ~IShortcutsConfiguration() = default;

    virtual ValCh<io::path> shortcutsUserPath() const = 0;
    virtual void setShortcutsUserPath(const io::path& path) = 0;

    virtual io::path shortcutsDefaultPath() const = 0;
};
}

#endif // MU_SHORTCUTS_ISHORTCUTSCONFIGURATION_H
