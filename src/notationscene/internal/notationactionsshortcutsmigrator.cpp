/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "notationactionsshortcutsmigrator.h"

#include "containers.h"
#include "modularity/ioc.h"
#include "shortcuts/ishortcutsregister.h"
#include "shortcuts/shortcutstypes.h"

using namespace muse;
using namespace mu::notation;

// This class is a temporary hack to fix a specific shortcut migration issue:
// https://github.com/musescore/MuseScore/issues/31327#issuecomment-3617247524
// In the future, we should refactor the shortcuts module and the format of
// shortcuts.xml, to make all shortcut migrations automatic. That is currently
// not possible because it is not possible to know if a shortcut was deleted by
// the user or simply did not exist in the version that created the
// shortcuts.xml file.

void NotationActionsShortcutsMigrator::migrate(const muse::modularity::ContextPtr& ctx)
{
    auto shortcutsRegister = modularity::ioc(ctx)->resolve<shortcuts::IShortcutsRegister>("notationscene");
    if (!shortcutsRegister) {
        return;
    }

    int migratableShortcutsSeen = 0;
    bool didMigrate = false;

    shortcuts::ShortcutList shortcuts = shortcutsRegister->shortcuts();

    for (shortcuts::Shortcut& shortcut : shortcuts) {
        if (shortcut.action == "prev-text-element") {
            const std::string backspaceSeq = "Backspace";

            if (!muse::contains(shortcut.sequences, backspaceSeq)) {
                shortcut.sequences.push_back(backspaceSeq);
                didMigrate = true;
            }
            ++migratableShortcutsSeen;
        } else if (shortcut.action == "zoomin") {
            const std::string ctrlPlusSeq = "Ctrl++";

            if (!muse::contains(shortcut.sequences, ctrlPlusSeq)) {
                shortcut.sequences.push_back(ctrlPlusSeq);
                didMigrate = true;
            }
            ++migratableShortcutsSeen;
        } else if (shortcut.action == "zoomout") {
            const std::string ctrlShiftMinusSeq = "Ctrl+Shift+-";

            if (!muse::contains(shortcut.sequences, ctrlShiftMinusSeq)) {
                shortcut.sequences.push_back(ctrlShiftMinusSeq);
                didMigrate = true;
            }
            ++migratableShortcutsSeen;
        }

        if (migratableShortcutsSeen >= 3) {
            break;
        }
    }

    if (didMigrate) {
        //! This way of migrating does not check for conflicts with customised shortcuts,
        //! but the likelihood of such conflicts is too low to add complex logic for that.
        shortcutsRegister->setShortcuts(shortcuts);
    }
}
