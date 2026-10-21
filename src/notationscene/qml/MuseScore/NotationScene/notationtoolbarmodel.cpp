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

#include "notationtoolbarmodel.h"

#include "uicomponents/qml/Muse/UiComponents/toolbaritem.h"

#include "notationcommands.h"
#include "appshell/appshellcommands.h"

using namespace muse;
using namespace mu::notation;
using namespace muse::uicomponents;

void NotationToolBarModel::load()
{
    if (m_loaded) {
        return;
    }

    AbstractToolBarModel::load();

    updateItems();

    context()->currentMasterNotationChanged().onNotify(this, [this]() {
        load();
    });

    notationConfiguration()->automationUiVariantChanged().onNotify(this, [this]() {
        updateItems();
    });

    m_loaded = true;
}

void NotationToolBarModel::updateItems()
{
    std::vector<rcommand::Command> commands = {
        OPEN_PARTS_COMMAND,
        appshell::DOCK_TOGGLE_MIXER_COMMAND
    };

    //! NOTE: in the Toolbar variant, the automation button lives in the note input toolbar instead
    if (notationConfiguration()->automationUiVariant() == AutomationUiVariant::Widget) {
        commands.push_back(TOGGLE_AUTOMATION_COMMAND);
    }

    ToolBarItemList items;
    for (const rcommand::Command& command : commands) {
        //! NOTE: the command's own title ("&Parts…") is meant for the menu
        const TranslatableString title = command == OPEN_PARTS_COMMAND ? TranslatableString("action", "Parts") : TranslatableString();

        ToolBarItem* item = makeItem(command, title);
        if (!item) {
            continue;
        }

        item->setShowTitle(!isCompactMode());
        item->setIsTitleBold(true);

        items << item;
    }

    setItems(items);
}
