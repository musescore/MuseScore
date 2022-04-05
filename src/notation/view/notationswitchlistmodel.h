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

#ifndef MU_NOTATION_NOTATIONSWITCHLISTMODEL_H
#define MU_NOTATION_NOTATIONSWITCHLISTMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "actions/iactionsdispatcher.h"
#include "project/inotationproject.h"

namespace mu::notation {
class NotationSwitchListModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, context)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)

public:
    explicit NotationSwitchListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void setCurrentNotation(int index);
    Q_INVOKABLE void closeNotation(int index);

signals:
    void currentNotationIndexChanged(int index);

private:
    void onCurrentProjectChanged();
    void onCurrentNotationChanged();

    IMasterNotationPtr masterNotation() const;

    void loadNotations();
    void listenProjectSavingStatusChanged();
    void listenNotationOpeningStatus(INotationPtr notation);
    void listenNotationTitleChanged(INotationPtr notation);
    bool isIndexValid(int index) const;

    bool isMasterNotation(const INotationPtr notation) const;

    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleNeedSave
    };

    QList<INotationPtr> m_notations;
    std::unique_ptr<async::Asyncable> m_notationChangedReceiver;
};
}

#endif // MU_NOTATION_NOTATIONSWITCHLISTMODEL_H
