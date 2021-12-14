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
#include "appmenuactioncontroller.h"

#include "ui/uitypes.h"

#include "appmenuuiactions.h"

using namespace mu::appshell;
using namespace mu::shortcuts;
using namespace mu::actions;
using namespace mu::ui;

void AppMenuActionController::init()
{
    UiActionList appMenuActions = AppMenuUiActions::allActions();

    QHash<QString, QString> shortcuts;
    for (const UiAction& action: appMenuActions) {
        QString title = action.title;
        QChar activateChar = title[title.indexOf('&') + 1];
        shortcuts.insert(QString::fromStdString(action.code), QString("Alt+") + activateChar);
    }

    //! ============ todo
    ShortcutList allShortcuts = shortcutsRegister()->shortcuts();

    for (const QString& actionCode: shortcuts.keys()) {
        Shortcut sc;
        sc.action = actionCode.toStdString();
        sc.sequences.push_back(shortcuts.value(actionCode).toStdString());

        dispatcher()->reg(this, sc.action, [=](){
            m_openMenuChannel.send(sc.action);
        });

        allShortcuts.push_back(sc);
    }

    shortcutsRegister()->setShortcuts(allShortcuts);
    //! ===========
}

mu::async::Channel<std::string> AppMenuActionController::openMenuChannel() const
{
    return m_openMenuChannel;
}
