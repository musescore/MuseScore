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
#ifndef MU_UI_ABSTRACTMENUMODEL_H
#define MU_UI_ABSTRACTMENUMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "ui/uitypes.h"
#include "ui/iuiactionsregister.h"
#include "actions/iactionsdispatcher.h"

namespace mu::ui {
class AbstractMenuModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(ui, IUiActionsRegister, uiactionsRegister)
    INJECT(ui, actions::IActionsDispatcher, dispatcher)

public:
    explicit AbstractMenuModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    virtual void load();

    Q_INVOKABLE void handleAction(const QString& actionCode, int actionIndex = -1);
    Q_INVOKABLE QVariantMap get(int index);

protected:
    virtual void onActionsStateChanges(const actions::ActionCodeList& codes);

    enum Roles {
        CodeRole = Qt::UserRole + 1,
        TitleRole,
        DescriptionRole,
        ShortcutRole,
        IconRole,
        CheckedRole,
        EnabledRole,
        SubitemsRole,
        SectionRole,

        UserRole,
    };

    void clear();

    MenuItem& findItem(const actions::ActionCode& actionCode);
    MenuItem& findItemByIndex(const actions::ActionCode& menuActionCode, int actionIndex);
    MenuItem& findMenu(const actions::ActionCode& subitemsActionCode);

    MenuItem makeMenu(const QString& title, const MenuItemList& items, bool enabled = true,
                      const actions::ActionCode& menuActionCode = "") const;

    MenuItem makeMenuItem(const actions::ActionCode& actionCode) const;
    MenuItem makeSeparator() const;

    bool isIndexValid(int index) const;
    void dispatch(const actions::ActionCode& actionCode, const actions::ActionData& args = actions::ActionData());

    MenuItemList m_items;

private:
    MenuItem& item(MenuItemList& items, const actions::ActionCode& actionCode);
    MenuItem& menu(MenuItemList& items, const actions::ActionCode& subitemsActionCode);
};
}

#endif // MU_UI_ABSTRACTMENUMODEL_H
