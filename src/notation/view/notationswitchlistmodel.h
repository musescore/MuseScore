/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#pragma once

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "actions/iactionsdispatcher.h"
#include "project/inotationproject.h"

namespace mu::notation {
class NotationSwitchListModel : public QAbstractListModel, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    muse::Inject<context::IGlobalContext> context = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    explicit NotationSwitchListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void setCurrentNotation(int index);
    Q_INVOKABLE void closeNotation(int index);
    Q_INVOKABLE void closeOtherNotations(int index);
    Q_INVOKABLE void closeAllNotations();

    Q_INVOKABLE QVariantList contextMenuItems(int index) const;
    Q_INVOKABLE void handleContextMenuItem(int index, const QString& itemId);

signals:
    void currentNotationIndexChanged(int index);

private:
    void onCurrentProjectChanged();
    void onCurrentNotationChanged();

    INotationPtr currentNotation() const;
    IMasterNotationPtr currentMasterNotation() const;

    void loadNotations();
    void listenProjectSavingStatusChanged();
    void listenNotationOpeningStatus(INotationPtr notation);
    void listenExcerptNotationTitleChanged(IExcerptNotationPtr excerptNotation);

    bool isIndexValid(int index) const;

    bool isMasterNotation(const INotationPtr notation) const;

    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleNeedSave,
        RoleIsCloud
    };

    QList<INotationPtr> m_notations;
    std::unique_ptr<muse::async::Asyncable> m_notationChangedReceiver;
};
}
