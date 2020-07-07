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

#include <QString>

#include "../ishortcutsregister.h"
#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

namespace mu {
namespace shortcuts {
class ShortcutsRegister : public IShortcutsRegister
{
    INJECT(shortcut, framework::IGlobalConfiguration, globalConfiguration)
public:

    ShortcutsRegister() = default;

    void load();

    const std::list<Shortcut>& shortcuts() const override;
    std::list<Shortcut> shortcutsForSequence(const std::string& sequence) const override;

private:

    bool loadFromFile(std::list<Shortcut>& shortcuts, const io::path& path) const;
    void expandStandartKeys(std::list<Shortcut>& shortcuts) const;

    std::list<Shortcut> m_shortcuts;
};
}
}

#endif // MU_SHORTCUTS_SHORTCUTSREGISTER_H
