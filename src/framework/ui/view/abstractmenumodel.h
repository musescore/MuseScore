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
#ifndef MU_UI_ABSTRACTMENUMODEL_H
#define MU_UI_ABSTRACTMENUMODEL_H

#include <QObject>

#include <optional>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "../uitypes.h"
#include "../iuiactionsregister.h"
#include "shortcuts/ishortcutsregister.h"

namespace mu::ui {
class AbstractMenuModel : public async::Asyncable
{
    INJECT(ui, IUiActionsRegister, uiactionsRegister)

public:
    QVariantList items() const;

protected:
    void clear();
    void appendItem(const MenuItem& item);
    void listenActionsStateChanges();
    virtual void onActionsStateChanges(const actions::ActionCodeList& codes);

    MenuItem& findItem(const actions::ActionCode& actionCode);
    MenuItem& findItemByIndex(const actions::ActionCode& menuActionCode, int actionIndex);
    MenuItem& findMenu(const actions::ActionCode& subitemsActionCode);

    MenuItem makeMenu(const QString& title, const MenuItemList& items, bool enabled = true,
                      const actions::ActionCode& menuActionCode = "") const;
    MenuItem makeAction(const actions::ActionCode& actionCode) const;
    MenuItem makeSeparator() const;

private:
    MenuItem& item(MenuItemList& items, const actions::ActionCode& actionCode);
    MenuItem& menu(MenuItemList& items, const actions::ActionCode& subitemsActionCode);

    MenuItemList m_items;
};
}

#endif // MU_UI_ABSTRACTMENUMODEL_H
