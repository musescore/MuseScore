//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_UICOMPONENTS_ABSTRACTMENUMODEL_H
#define MU_UICOMPONENTS_ABSTRACTMENUMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "actions/actiontypes.h"
#include "actions/iactionsregister.h"
#include "shortcuts/ishortcutsregister.h"
#include "../uicomponentstypes.h"

namespace mu::uicomponents {
class AbstractMenuModel : public QObject
{
    Q_OBJECT

    INJECT(appshell, actions::IActionsRegister, actionsRegister)
    INJECT(appshell, shortcuts::IShortcutsRegister, shortcutsRegister)

    Q_PROPERTY(QVariantList items READ items NOTIFY itemsChanged)

public:
    explicit AbstractMenuModel(QObject* parent = nullptr);

    QVariantList items() const;

signals:
    void itemsChanged();

protected:
    void clear();
    void appendItem(const MenuItem& item);

    MenuItem& findItem(const actions::ActionCode& actionCode);
    MenuItem& findItemByIndex(const actions::ActionCode& menuActionCode, int actionIndex);
    MenuItem& findMenu(const actions::ActionCode& subitemsActionCode);

    using EnabledCallBack = std::function<bool ()>;

    MenuItem makeMenu(const std::string& title, const MenuItemList& actions, bool enabled = true,
                      const actions::ActionCode& menuActionCode = "");
    MenuItem makeAction(const actions::ActionCode& actionCode,const std::optional<EnabledCallBack>& enabledCallBack = std::nullopt);
    MenuItem makeAction(const actions::ActionCode& actionCode, bool checked,
                        const std::optional<EnabledCallBack>& enabledCallBack = std::nullopt);
    MenuItem makeSeparator() const;

    void updateItemsEnabled(const std::vector<actions::ActionCode>& actionCodes);

private:
    static bool enabledTrue() { return true; }

    MenuItem& item(MenuItemList& items, const actions::ActionCode& actionCode);
    MenuItem& menu(MenuItemList& items, const actions::ActionCode& subitemsActionCode);

    MenuItemList m_items;

    std::map<actions::ActionCode, EnabledCallBack> m_actionEnabledMap;
};
}

#endif // MU_APPSHELL_ABSTRACTMENUMODEL_H
