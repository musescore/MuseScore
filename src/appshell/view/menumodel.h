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
#ifndef MU_APPSHELL_MENUMODEL_H
#define MU_APPSHELL_MENUMODEL_H

#include <QObject>

#include "modularity/ioc.h"

#include "actions/iactionsregister.h"
#include "actions/iactionsdispatcher.h"
#include "shortcuts/ishortcutsregister.h"
#include "uicomponents/uicomponentstypes.h"

namespace mu::appshell {
class MenuModel : public QObject
{
    Q_OBJECT

    INJECT(appshell, actions::IActionsRegister, actionsRegister)
    INJECT(appshell, actions::IActionsDispatcher, actionsDispatcher)
    INJECT(dock, shortcuts::IShortcutsRegister, shortcutsRegister)

    Q_PROPERTY(QVariantList items READ items NOTIFY itemsChanged)

public:
    explicit MenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void handleAction(const QString& actionCode);

    QVariantList items();

signals:
    void itemsChanged();

private:
    uicomponents::MenuItem fileItem();

    uicomponents::MenuItem makeMenu(const std::string& title, const uicomponents::MenuItemList& actions);
    uicomponents::MenuItem makeAction(const actions::ActionItem& action, const std::string& section = "", bool enabled = true,
                                      bool checked = false) const;
    uicomponents::MenuItem makeSeparator();

    uicomponents::MenuItemList m_items;
};
}

#endif // MU_APPSHELL_MENUMODEL_H
