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

#ifndef MU_NOTATION_NOTATIONTOOLBARMODEL_H
#define MU_NOTATION_NOTATIONTOOLBARMODEL_H

#include <QAbstractListModel>

#include "context/iglobalcontext.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "ui/iuiactionsregister.h"
#include "actions/iactionsdispatcher.h"

#include "framework/ui/view/iconcodes.h"

namespace mu::notation {
class NotationToolBarModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, context)
    INJECT(notation, ui::IUiActionsRegister, actionsRegister)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)

public:
    explicit NotationToolBarModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void handleAction(const QString& actionCode);

private:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        CodeRole,
        IconRole,
        EnabledRole,
        DescriptionRole,
        ShortcutRole
    };

    ui::MenuItem makeItem(const actions::ActionCode& actionCode) const;

    QList<ui::MenuItem> m_items;
};
}

#endif // MU_NOTATION_NOTATIONTOOLBARMODEL_H
