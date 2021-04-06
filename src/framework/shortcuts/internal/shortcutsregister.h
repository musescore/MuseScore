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
#ifndef MU_SHORTCUTS_SHORTCUTSREGISTER_H
#define MU_SHORTCUTS_SHORTCUTSREGISTER_H

#include "../ishortcutsregister.h"
#include "modularity/ioc.h"
#include "ishortcutsconfiguration.h"
#include "async/asyncable.h"

namespace mu::framework {
class XmlReader;
class XmlWriter;
}

namespace mu::shortcuts {
class ShortcutsRegister : public IShortcutsRegister, public async::Asyncable
{
    INJECT(shortcuts, IShortcutsConfiguration, configuration)

public:
    ShortcutsRegister() = default;

    void load();

    const ShortcutList& shortcuts() const override;
    Ret setShortcuts(const ShortcutList& shortcuts) override;
    async::Notification shortcutsChanged() const override;

    const Shortcut& shortcut(const std::string& actionCode) const override;
    const Shortcut& defaultShortcut(const std::string& actionCode) const override;
    ShortcutList shortcutsForSequence(const std::string& sequence) const override;

    Ret saveToFile(const io::path& filePath) const override;

private:

    bool readFromFile(ShortcutList& shortcuts, const io::path& path) const;
    Shortcut readShortcut(framework::XmlReader& reader) const;

    bool writeToFile(const ShortcutList& shortcuts, const io::path& path) const;
    void writeShortcut(framework::XmlWriter& writer, const Shortcut& shortcut) const;

    void expandStandardKeys(ShortcutList& shortcuts) const;

    ShortcutList m_shortcuts;
    ShortcutList m_defaultShortcuts;
    async::Notification m_shortcutsChanged;
};
}

#endif // MU_SHORTCUTS_SHORTCUTSREGISTER_H
