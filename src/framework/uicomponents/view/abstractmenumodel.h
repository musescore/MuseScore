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

#include <optional>

#include "modularity/ioc.h"
#include "actions/actiontypes.h"
#include "actions/iactionsregister.h"
#include "shortcuts/ishortcutsregister.h"
#include "../uicomponentstypes.h"

namespace mu::uicomponents {
class AbstractMenuModel
{
    INJECT(appshell, actions::IActionsRegister, actionsRegister)
    INJECT(appshell, shortcuts::IShortcutsRegister, shortcutsRegister)

public:
    QVariantList items() const;

    virtual ActionState actionState(const actions::ActionCode& actionCode) const;

protected:
    void clear();
    void appendItem(const MenuItem& item);

    MenuItem& findItem(const actions::ActionCode& actionCode);
    MenuItem& findItemByIndex(const actions::ActionCode& menuActionCode, int actionIndex);
    MenuItem& findMenu(const actions::ActionCode& subitemsActionCode);

    MenuItem makeMenu(const std::string& title, const MenuItemList& actions, bool enabled = true,
                      const actions::ActionCode& menuActionCode = "") const;
    MenuItem makeAction(const actions::ActionCode& actionCode) const;
    MenuItem makeSeparator() const;

    void updateItemsState(const actions::ActionCodeList& actionCodes);

private:
    MenuItem& item(MenuItemList& items, const actions::ActionCode& actionCode);
    MenuItem& menu(MenuItemList& items, const actions::ActionCode& subitemsActionCode);

    MenuItemList m_items;
};
}

#endif // MU_APPSHELL_ABSTRACTMENUMODEL_H
