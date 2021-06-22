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
#ifndef MU_SHORTCUTS_ISHORTCUTSREGISTER_H
#define MU_SHORTCUTS_ISHORTCUTSREGISTER_H

#include <list>

#include "modularity/imoduleexport.h"
#include "shortcutstypes.h"
#include "async/notification.h"
#include "ret.h"
#include "io/path.h"

namespace mu::shortcuts {
class IShortcutsRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IShortcutsRegister)
public:
    virtual ~IShortcutsRegister() = default;

    virtual const ShortcutList& shortcuts() const = 0;
    virtual Ret setShortcuts(const ShortcutList& shortcuts) = 0;
    virtual async::Notification shortcutsChanged() const = 0;

    virtual const Shortcut& shortcut(const std::string& actionCode) const = 0;
    virtual const Shortcut& defaultShortcut(const std::string& actionCode) const = 0;
    virtual ShortcutList shortcutsForSequence(const std::string& sequence) const = 0;

    virtual Ret importFromFile(const io::path& filePath) = 0;
    virtual Ret exportToFile(const io::path& filePath) const = 0;
};
}

#endif // MU_SHORTCUTS_ISHORTCUTSREGISTER_H
