/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "macosappmenumodelhook.h"

#include <Cocoa/Cocoa.h>

#include "interactive/internal/platform/macos/macosinteractivehelper.h"
#include "notationscene/notationcommands.h"

using namespace mu::appshell;

void MacOSAppMenuModelHook::onAppMenuInited(const muse::uicomponents::MenuItemList& items)
{
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSDisabledDictationMenuItem"];
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSDisabledCharacterPaletteMenuItem"];

    const muse::uicomponents::MenuItem* editMenu = nullptr;
    int editTopLevelIndex = -1;
    for (int i = 0; i < items.size(); ++i) {
        if (items[i] && items[i]->id() == "menu-edit") {
            editMenu = items[i];
            editTopLevelIndex = i;
            break;
        }
    }

    if (!editMenu || editTopLevelIndex < 0) {
        return;
    }

    // AppKit inserts the Application menu at index 0 on macOS, shifting top-level menus by +1
    muse::MacOSInteractiveHelper::setEditMenuIndex(editTopLevelIndex + 1);

    int index = 0;
    std::map<muse::MacOSInteractiveHelper::EditAction, int> structure;

    for (const muse::uicomponents::MenuItem* subitem : editMenu->subitems()) {
        if (!subitem || subitem->id().isEmpty()) {
            index++;
            continue;
        }

        if (subitem->id() == mu::notation::UNDO_COMMAND.toString()) {
            structure[muse::MacOSInteractiveHelper::EditAction::Undo] = index;
        } else if (subitem->id() == mu::notation::REDO_COMMAND.toString()) {
            structure[muse::MacOSInteractiveHelper::EditAction::Redo] = index;
        } else if (subitem->id() == mu::notation::CUT_COMMAND.toString()) {
            structure[muse::MacOSInteractiveHelper::EditAction::Cut] = index;
        } else if (subitem->id() == mu::notation::COPY_COMMAND.toString()) {
            structure[muse::MacOSInteractiveHelper::EditAction::Copy] = index;
        } else if (subitem->id() == mu::notation::PASTE_COMMAND.toString()) {
            structure[muse::MacOSInteractiveHelper::EditAction::Paste] = index;
        } else if (subitem->id() == mu::notation::SELECT_ALL_COMMAND.toString()) {
            structure[muse::MacOSInteractiveHelper::EditAction::SelectAll] = index;
        }

        index++;
    }

    muse::MacOSInteractiveHelper::setEditMenuStructure(structure);
}
