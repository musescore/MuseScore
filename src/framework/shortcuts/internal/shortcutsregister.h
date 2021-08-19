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
#ifndef MU_SHORTCUTS_SHORTCUTSREGISTER_H
#define MU_SHORTCUTS_SHORTCUTSREGISTER_H

#include "../ishortcutsregister.h"
#include "modularity/ioc.h"
#include "ishortcutsconfiguration.h"
#include "async/asyncable.h"
#include "system/ifilesystem.h"
#include "multiinstances/imultiinstancesprovider.h"

namespace mu::framework {
class XmlReader;
class XmlWriter;
}

namespace mu::shortcuts {
class ShortcutsRegister : public IShortcutsRegister, public async::Asyncable
{
    INJECT(shortcuts, IShortcutsConfiguration, configuration)
    INJECT(shortcuts, system::IFileSystem, fileSystem)
    INJECT(shortcuts, mi::IMultiInstancesProvider, multiInstancesProvider)

public:
    ShortcutsRegister() = default;

    void load();

    const ShortcutList& shortcuts() const override;
    Ret setShortcuts(const ShortcutList& shortcuts) override;
    async::Notification shortcutsChanged() const override;

    const Shortcut& shortcut(const std::string& actionCode) const override;
    const Shortcut& defaultShortcut(const std::string& actionCode) const override;
    ShortcutList shortcutsForSequence(const std::string& sequence) const override;

    Ret importFromFile(const io::path& filePath) override;
    Ret exportToFile(const io::path& filePath) const override;

private:

    bool readFromFile(ShortcutList& shortcuts, const io::path& path) const;
    Shortcut readShortcut(framework::XmlReader& reader) const;

    bool writeToFile(const ShortcutList& shortcuts, const io::path& path) const;
    void writeShortcut(framework::XmlWriter& writer, const Shortcut& shortcut) const;

    void mergeShortcuts(ShortcutList& shortcuts, const ShortcutList& defaultShortcuts) const;
    void expandStandardKeys(ShortcutList& shortcuts) const;

    ShortcutList m_shortcuts;
    ShortcutList m_defaultShortcuts;
    async::Notification m_shortcutsChanged;
};
}

#endif // MU_SHORTCUTS_SHORTCUTSREGISTER_H
